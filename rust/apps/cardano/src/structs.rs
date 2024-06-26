use crate::address::{derive_address, derive_pubkey_hash, AddressType};
use crate::errors::{CardanoError, R};
use alloc::collections::BTreeMap;
use alloc::string::{String, ToString};
use alloc::vec;
use alloc::vec::Vec;
use app_utils::{impl_internal_struct, impl_public_struct};
use cardano_serialization_lib::address::{
    self, Address, BaseAddress, EnterpriseAddress, RewardAddress,
};

use cardano_serialization_lib::protocol_types::governance::DRepKind;
use cardano_serialization_lib::utils::{from_bignum, BigNum};
use cardano_serialization_lib::{
    protocol_types::fixed_tx::FixedTransaction as Transaction, Certificate, CertificateKind,
    NetworkId, NetworkIdKind,
};

use alloc::format;
use core::ops::Div;
use third_party::bitcoin::bip32::ChildNumber::{Hardened, Normal};
use third_party::bitcoin::bip32::DerivationPath;

use third_party::hex;

impl_public_struct!(ParseContext {
    utxos: Vec<CardanoUtxo>,
    cert_keys: Vec<CardanoCertKey>,
    cardano_xpub: String,
    master_fingerprint: Vec<u8>
});

impl_public_struct!(CardanoUtxo {
    master_fingerprint: Vec<u8>,
    address: String,
    path: DerivationPath,
    value: u64,
    transaction_hash: Vec<u8>,
    index: u32
});

impl_public_struct!(CardanoCertKey {
    master_fingerprint: Vec<u8>,
    key_hash: Vec<u8>,
    path: DerivationPath
});

impl_public_struct!(ParsedCardanoSignData {
    sign_data: String
});


impl_public_struct!(ParsedCardanoTx {
    fee: String,
    total_input: String,
    total_output: String,
    from: Vec<CardanoFrom>,
    to: Vec<CardanoTo>,
    network: String,
    certificates: Vec<CardanoCertificate>,
    withdrawals: Vec<CardanoWithdrawal>,
    auxiliary_data: Option<String>
});

impl_public_struct!(SignDataResult {
    pub_key: String,
    signature: String
});

impl_public_struct!(CardanoCertificate {
    cert_type: String,
    address: String,
    pool: Option<String>
});

impl_public_struct!(CardanoWithdrawal {
    address: String,
    amount: String
});

impl_internal_struct!(ParsedCardanoInput {
    transaction_hash: Vec<u8>,
    index: u32,
    value: Option<u64>,
    address: Option<String>,
    path: Option<String>,
    is_mine: bool
});

impl_public_struct!(CardanoFrom {
    address: String,
    amount: String,
    path: Option<String>,
    value: u64
});

impl_public_struct!(CardanoTo {
    address: String,
    amount: String,
    assets: BTreeMap<String, ParsedCardanoMultiAsset>,
    assets_text: Option<String>,
    value: u64
});

impl_internal_struct!(ParsedCardanoOutput {
    address: String,
    amount: String,
    value: u64,
    assets: Option<Vec<ParsedCardanoMultiAsset>>
});

impl_public_struct!(ParsedCardanoMultiAsset {
    id: String,
    policy_id: Vec<u8>,
    name: Vec<u8>,
    amount: String,
    value: u64
});

#[derive(Clone)]
enum CardanoCertAction {
    Registration(Registration),
    Deregistration(Deregistration),
    Delegation(Delegation),
}

#[derive(Clone)]
struct Registration {
    stake_key: RewardAddress,
}

#[derive(Clone)]
struct Deregistration {
    stake_key: RewardAddress,
}

#[derive(Clone)]
struct Delegation {
    pool: String,
    stake_key: RewardAddress,
}

impl ParsedCardanoSignData {
    pub fn from_sign_data(sign_data: Vec<u8>) -> R<Self> {
        Ok(Self {
            sign_data: hex::encode(sign_data),
        })
    }
}

impl ParsedCardanoTx {
    pub fn from_cardano_tx(tx: Transaction, context: ParseContext) -> R<Self> {
        let network_id = Self::judge_network_id(&tx);
        let network = match network_id {
            1 => "Cardano Mainnet".to_string(),
            _ => "Cardano Testnet".to_string(),
        };
        let parsed_inputs = Self::parse_inputs(&tx, &context, network_id)?;
        let parsed_outputs = Self::parse_outputs(&tx)?;

        let fee = from_bignum(&tx.body().fee());

        let total_output_amount = {
            let _v = parsed_outputs.iter().fold(0u64, |acc, cur| acc + cur.value);
            normalize_coin(_v)
        };

        let _from_list = {
            let mut _list = vec![];
            for (key, _) in Self::get_from_list(parsed_inputs.clone()) {
                _list.push(key)
            }
            _list
        };

        let from_list_detail = {
            let mut _list = vec![];
            for (_, value) in Self::get_from_list(parsed_inputs.clone()) {
                _list.push(value)
            }
            _list
        };

        let _to_list = {
            let mut _list = vec![];
            for (key, _) in Self::get_to_list(parsed_outputs.clone()) {
                _list.push(key)
            }
            _list
        };

        let to_list_detail = {
            let mut _list = vec![];
            for (_, value) in Self::get_to_list(parsed_outputs) {
                _list.push(value)
            }
            _list
        };

        let fee = normalize_coin(fee);

        let total_input_amount = {
            let _v = parsed_inputs.iter().fold(0u64, |acc, cur| match cur.value {
                Some(v) => acc + v,
                None => acc,
            });
            normalize_coin(_v)
        };

        Ok(Self {
            total_input: total_input_amount,
            total_output: total_output_amount,
            from: from_list_detail,
            to: to_list_detail,
            fee: fee.clone(),
            network: network.clone(),
            certificates: Self::parse_certificates(&tx, network_id)?,
            withdrawals: Self::parse_withdrawals(&tx)?,
            auxiliary_data: Self::parse_auxiliary_data(&tx)?,
        })
    }

    fn judge_network_id(tx: &Transaction) -> u8 {
        match tx.body().network_id() {
            None => match tx.body().outputs().get(0).address().network_id() {
                Ok(id) => id,
                Err(_) => 1,
            },
            Some(id) => match id.kind() {
                NetworkIdKind::Mainnet => 1,
                NetworkIdKind::Testnet => 0,
            },
        }
    }

    fn parse_auxiliary_data(tx: &Transaction) -> R<Option<String>> {
        tx.auxiliary_data()
            .map(|v| {
                v.to_json()
                    .map_err(|e| CardanoError::InvalidTransaction(e.to_string()))
            })
            .transpose()
    }

    fn parse_certificates(tx: &Transaction, network_id: u8) -> R<Vec<CardanoCertificate>> {
        let mut certs = vec![];
        if let Some(_certs) = tx.body().certs() {
            let len = _certs.len();
            for i in 0..len {
                let cert = _certs.get(i);
                if let Some(_cert) = cert.as_stake_delegation() {
                    certs.push(CardanoCertificate::new(
                        "Stake Pool Delegation".to_string(),
                        RewardAddress::new(network_id, &_cert.stake_credential())
                            .to_address()
                            .to_bech32(None)
                            .map_err(|e| CardanoError::InvalidTransaction(e.to_string()))?,
                        Some(
                            _cert
                                .pool_keyhash()
                                .to_bech32("pool")
                                .map_err(|e| CardanoError::InvalidTransaction(e.to_string()))?,
                        ),
                    ));
                }
                if let Some(_cert) = cert.as_stake_deregistration() {
                    certs.push(CardanoCertificate::new(
                        "Stake Deregistration".to_string(),
                        RewardAddress::new(network_id, &_cert.stake_credential())
                            .to_address()
                            .to_bech32(None)
                            .map_err(|e| CardanoError::InvalidTransaction(e.to_string()))?,
                        None,
                    ));
                }
                if let Some(_cert) = cert.as_stake_registration() {
                    let deposit = normalize_coin(from_bignum(&_cert.coin().unwrap_or(BigNum::zero())));
                    certs.push(CardanoCertificate::new(
                        "Stake Registration".to_string(),
                        RewardAddress::new(network_id, &_cert.stake_credential())
                            .to_address()
                            .to_bech32(None)
                            .map_err(|e| CardanoError::InvalidTransaction(e.to_string()))?,
                        Some(deposit),
                    ));
                }
                if let Some(_cert) = cert.as_vote_delegation() {
                    let drep = match _cert.drep().kind() {
                        DRepKind::AlwaysAbstain => "Abstain".to_string(),
                        DRepKind::AlwaysNoConfidence => "No Confidence".to_string(),
                        DRepKind::KeyHash => _cert
                            .drep()
                            .to_key_hash()
                            .unwrap()
                            .to_bech32("drep")
                            .map_err(|e| CardanoError::InvalidTransaction(e.to_string()))?,
                        DRepKind::ScriptHash => _cert
                            .drep()
                            .to_script_hash()
                            .unwrap()
                            .to_bech32("")
                            .map_err(|e| CardanoError::InvalidTransaction(e.to_string()))?,
                    };
                    certs.push(CardanoCertificate::new(
                        "DRep Delegation".to_string(),
                        RewardAddress::new(network_id, &_cert.stake_credential())
                            .to_address()
                            .to_bech32(None)
                            .map_err(|e| CardanoError::InvalidTransaction(e.to_string()))?,
                        Some(drep),
                    ));
                }
                if let Some(_cert) = cert.as_pool_registration() {
                    certs.push(CardanoCertificate::new(
                        "Pool Registration".to_string(),
                        _cert
                            .pool_params()
                            .reward_account()
                            .to_address()
                            .to_bech32(None)
                            .map_err(|e| CardanoError::InvalidTransaction(e.to_string()))?,
                        None,
                    ));
                }
                if let Some(_cert) = cert.as_pool_retirement() {
                    certs.push(CardanoCertificate::new(
                        "Pool Retirement".to_string(),
                        _cert
                            .pool_keyhash()
                            .to_bech32("pool")
                            .map_err(|e| CardanoError::InvalidTransaction(e.to_string()))?,
                        None,
                    ));
                }
                if let Some(_cert) = cert.as_genesis_key_delegation() {
                    certs.push(CardanoCertificate::new(
                        "Genesis Key Delegation".to_string(),
                        "None".to_string(),
                        None,
                    ));
                }
                if let Some(_cert) = cert.as_move_instantaneous_rewards_cert() {
                    certs.push(CardanoCertificate::new(
                        "MoveInstantaneousRewardsCert".to_string(),
                        "None".to_string(),
                        None,
                    ));
                }
                if let Some(_cert) = cert.as_committee_hot_auth() {
                    certs.push(CardanoCertificate::new(
                        "CommitteeHotAuth".to_string(),
                        "None".to_string(),
                        None,
                    ));
                }
                if let Some(_cert) = cert.as_committee_cold_resign() {
                    certs.push(CardanoCertificate::new(
                        "CommitteeColdResign".to_string(),
                        "None".to_string(),
                        None,
                    ));
                }
                if let Some(_cert) = cert.as_drep_deregistration() {
                    let deposit = normalize_coin(from_bignum(&_cert.coin()));
                    certs.push(CardanoCertificate::new(
                        "DrepDeregistration".to_string(),
                        RewardAddress::new(network_id, &_cert.voting_credential())
                            .to_address()
                            .to_bech32(None)
                            .map_err(|e| CardanoError::InvalidTransaction(e.to_string()))?,
                        Some(deposit),
                    ));
                }
                if let Some(_cert) = cert.as_drep_registration() {
                    let deposit = normalize_coin(from_bignum(&_cert.coin()));
                    certs.push(CardanoCertificate::new(
                        "DrepRegistration".to_string(),
                        RewardAddress::new(network_id, &_cert.voting_credential())
                            .to_address()
                            .to_bech32(None)
                            .map_err(|e| CardanoError::InvalidTransaction(e.to_string()))?,
                        Some(deposit),
                    ));
                }
                if let Some(_cert) = cert.as_drep_update() {
                    certs.push(CardanoCertificate::new(
                        "DrepUpdate".to_string(),
                        RewardAddress::new(network_id, &_cert.voting_credential())
                            .to_address()
                            .to_bech32(None)
                            .map_err(|e| CardanoError::InvalidTransaction(e.to_string()))?,
                        None,
                    ));
                }
                if let Some(_cert) = cert.as_stake_and_vote_delegation() {
                    let drep = match _cert.drep().kind() {
                        DRepKind::AlwaysAbstain => "Abstain".to_string(),
                        DRepKind::AlwaysNoConfidence => "No Confidence".to_string(),
                        DRepKind::KeyHash => _cert
                            .drep()
                            .to_key_hash()
                            .unwrap()
                            .to_bech32("drep")
                            .map_err(|e| CardanoError::InvalidTransaction(e.to_string()))?,
                        DRepKind::ScriptHash => _cert
                            .drep()
                            .to_script_hash()
                            .unwrap()
                            .to_bech32("")
                            .map_err(|e| CardanoError::InvalidTransaction(e.to_string()))?,
                    };
                    certs.push(CardanoCertificate::new(
                        "StakeAndVoteDelegation".to_string(),
                        RewardAddress::new(network_id, &_cert.stake_credential())
                            .to_address()
                            .to_bech32(None)
                            .map_err(|e| CardanoError::InvalidTransaction(e.to_string()))?,
                        Some(drep),
                    ));
                }
                if let Some(_cert) = cert.as_stake_registration_and_delegation() {
                    certs.push(CardanoCertificate::new(
                        "StakeRegistrationAndDelegation".to_string(),
                        RewardAddress::new(network_id, &_cert.stake_credential())
                            .to_address()
                            .to_bech32(None)
                            .map_err(|e| CardanoError::InvalidTransaction(e.to_string()))?,
                        Some(
                            _cert
                                .pool_keyhash()
                                .to_bech32("pool")
                                .map_err(|e| CardanoError::InvalidTransaction(e.to_string()))?,
                        ),
                    ));
                }
                if let Some(_cert) = cert.as_stake_vote_registration_and_delegation() {
                    certs.push(CardanoCertificate::new(
                        "StakeVoteRegistrationAndDelegation".to_string(),
                        RewardAddress::new(network_id, &_cert.stake_credential())
                            .to_address()
                            .to_bech32(None)
                            .map_err(|e| CardanoError::InvalidTransaction(e.to_string()))?,
                        Some(
                            _cert
                                .pool_keyhash()
                                .to_bech32("pool")
                                .map_err(|e| CardanoError::InvalidTransaction(e.to_string()))?,
                        ),
                    ));
                }
                if let Some(_cert) = cert.as_vote_registration_and_delegation() {
                    let drep = match _cert.drep().kind() {
                        DRepKind::AlwaysAbstain => "Abstain".to_string(),
                        DRepKind::AlwaysNoConfidence => "No Confidence".to_string(),
                        DRepKind::KeyHash => _cert
                            .drep()
                            .to_key_hash()
                            .unwrap()
                            .to_bech32("drep")
                            .map_err(|e| CardanoError::InvalidTransaction(e.to_string()))?,
                        DRepKind::ScriptHash => _cert
                            .drep()
                            .to_script_hash()
                            .unwrap()
                            .to_bech32("")
                            .map_err(|e| CardanoError::InvalidTransaction(e.to_string()))?,
                    };
                    certs.push(CardanoCertificate::new(
                        "VoteRegistrationAndDelegation".to_string(),
                        RewardAddress::new(network_id, &_cert.stake_credential())
                            .to_address()
                            .to_bech32(None)
                            .map_err(|e| CardanoError::InvalidTransaction(e.to_string()))?,
                        Some(drep),
                    ));
                }
            }
        }
        Ok(certs)
    }

    fn parse_withdrawals(tx: &Transaction) -> R<Vec<CardanoWithdrawal>> {
        let mut withdrawals = vec![];
        if let Some(_withdrawals) = tx.body().withdrawals() {
            let keys = _withdrawals.keys();
            let len = keys.len();
            for i in 0..len {
                let address = keys.get(i);
                let value = _withdrawals.get(&address);
                if let Some(_v) = value {
                    withdrawals.push(CardanoWithdrawal::new(
                        address
                            .to_address()
                            .to_bech32(None)
                            .map_err(|e| CardanoError::InvalidTransaction(e.to_string()))?,
                        normalize_coin(from_bignum(&_v)),
                    ))
                }
            }
        }
        Ok(withdrawals)
    }

    fn get_from_list(inputs: Vec<ParsedCardanoInput>) -> BTreeMap<String, CardanoFrom> {
        let mut map = BTreeMap::<String, CardanoFrom>::new();
        for input in inputs {
            let address = match input.address {
                Some(v) => v,
                None => "Unknown address".to_string(),
            };
            match map.get(&address) {
                Some(existing) => {
                    let mut new_from = existing.clone();
                    if let Some(_v) = input.value {
                        new_from.value += _v;
                        new_from.amount = normalize_coin(new_from.value);
                    }
                    map.insert(address, new_from);
                }
                None => {
                    let cardano_from = CardanoFrom {
                        address: address.clone(),
                        value: match input.value {
                            Some(v) => v,
                            None => 0,
                        },
                        amount: match input.value {
                            Some(v) => normalize_coin(v),
                            None => "Unknown amount".to_string(),
                        },
                        path: input.path,
                    };
                    map.insert(address, cardano_from);
                }
            }
        }
        map
    }

    fn get_to_list(outputs: Vec<ParsedCardanoOutput>) -> BTreeMap<String, CardanoTo> {
        let mut map = BTreeMap::<String, CardanoTo>::new();
        for output in outputs {
            let address = output.address;
            match map.get(&address) {
                Some(existing) => {
                    let mut to = existing.clone();
                    to.value = to.value + output.value;
                    to.amount = normalize_coin(to.value);
                    if let Some(assets) = output.assets {
                        for x in assets {
                            match to.assets.get(&x.id) {
                                Some(asset) => {
                                    let mut new_asset = asset.clone();
                                    new_asset.value = new_asset.value + x.value;
                                    new_asset.amount = normalize_value(new_asset.value);
                                    to.assets.insert(new_asset.id.clone(), new_asset);
                                }
                                None => {
                                    to.assets.insert(x.id.clone(), x);
                                }
                            }
                        }
                    }
                    to.assets_text = match to.assets.len() {
                        0 => None,
                        x => Some(format!("{} more assets", x)),
                    };
                    map.insert(address, to);
                }
                None => {
                    let mut assets_map = BTreeMap::<String, ParsedCardanoMultiAsset>::new();

                    if let Some(assets) = output.assets {
                        for x in assets {
                            assets_map.insert(x.id.clone(), x);
                        }
                    }

                    let to = CardanoTo {
                        address: address.clone(),
                        amount: normalize_coin(output.value),
                        value: output.value,
                        assets: assets_map.clone(),
                        assets_text: match assets_map.len() {
                            0 => None,
                            x => Some(format!("{} more assets", x)),
                        },
                    };
                    map.insert(address, to);
                }
            }
        }
        map
    }

    pub fn verify(tx: Transaction, context: ParseContext) -> R<()> {
        let network_id = Self::judge_network_id(&tx);
        let parsed_inputs = Self::parse_inputs(&tx, &context, network_id)?;

        let mfp = hex::encode(context.get_master_fingerprint());
        let has_my_signer = context
            .get_cert_keys()
            .iter()
            .filter(|v| hex::encode(v.get_master_fingerprint()).eq(&mfp))
            .fold(false, |acc, cur| {
                acc || hex::encode(cur.get_master_fingerprint()).eq(&mfp)
            });

        if has_my_signer {
            return Ok(());
        }

        if parsed_inputs
            .iter()
            .filter(|v| v.address.is_some())
            .collect::<Vec<&ParsedCardanoInput>>()
            .is_empty()
        {
            return Err(CardanoError::InvalidTransaction(
                "no input related to this account".to_string(),
            ));
        }
        Self::parse_certificates(&tx, network_id)?;
        Ok(())
    }

    fn parse_inputs(
        tx: &Transaction,
        context: &ParseContext,
        network_id: u8,
    ) -> R<Vec<ParsedCardanoInput>> {
        let inputs_len = tx.body().inputs().len();
        let mut parsed_inputs = vec![];
        for i in 0..inputs_len {
            let input = tx.body().inputs().get(i);
            let hash = input.transaction_id().to_hex();
            let index = input.index();
            let m = context.utxos.iter().find(|v| {
                hash.eq_ignore_ascii_case(&hex::encode(&v.transaction_hash)) && index.eq(&v.index)
            });
            match m {
                //known utxo
                Some(utxo) => {
                    let mut iter = utxo.path.into_iter();
                    let _root = match iter.next() {
                        Some(Hardened { index: 1852 }) => Ok(1852u32),
                        _ => Err(CardanoError::DerivationError(
                            "invalid derivation path".to_string(),
                        )),
                    }?;
                    let _coin_type = match iter.next() {
                        Some(Hardened { index: 1815 }) => Ok(1815u32),
                        _ => Err(CardanoError::DerivationError(
                            "invalid derivation path".to_string(),
                        )),
                    }?;
                    let _account = match iter.next() {
                        Some(Hardened { index: _i }) => Ok(_i),
                        _ => Err(CardanoError::DerivationError(
                            "invalid derivation path".to_string(),
                        )),
                    }?;
                    let change = match iter.next() {
                        Some(Normal { index: _i }) => Ok(_i),
                        _ => Err(CardanoError::DerivationError(
                            "invalid derivation path".to_string(),
                        )),
                    }?;
                    let index = match iter.next() {
                        Some(Normal { index: _i }) => Ok(_i),
                        _ => Err(CardanoError::DerivationError(
                            "invalid derivation path".to_string(),
                        )),
                    }?;
                    //check utxo address with payment keyhash;
                    let my_pubkey_hash = hex::encode(derive_pubkey_hash(
                        context.get_cardano_xpub(),
                        change.clone(),
                        index.clone(),
                    )?);

                    let mut address = utxo.address.clone();

                    let addr_in_utxo = Address::from_bech32(&utxo.address)
                        .map_err(|e| CardanoError::InvalidTransaction(e.to_string()))?;

                    let mut pubkey_hash_paired = false;

                    if let Some(addr) = BaseAddress::from_address(&addr_in_utxo) {
                        match addr.payment_cred().to_keyhash() {
                            Some(keyhash) => {
                                if my_pubkey_hash.eq(&keyhash.to_hex()) {
                                    pubkey_hash_paired = true;
                                }
                            }
                            None => {}
                        }
                    }

                    if let Some(addr) = EnterpriseAddress::from_address(&addr_in_utxo) {
                        match addr.payment_cred().to_keyhash() {
                            Some(keyhash) => {
                                if my_pubkey_hash.eq(&keyhash.to_hex()) {
                                    pubkey_hash_paired = true;
                                }
                            }
                            None => {}
                        }
                    }

                    if !pubkey_hash_paired {
                        return Err(CardanoError::InvalidTransaction(
                            "invalid address".to_string(),
                        ));
                    }
                    parsed_inputs.push(ParsedCardanoInput {
                        transaction_hash: utxo.transaction_hash.clone(),
                        index: utxo.index,
                        value: Some(utxo.value),
                        address: Some(address),
                        path: Some(utxo.path.to_string()),
                        is_mine: utxo.master_fingerprint.eq(&context.master_fingerprint),
                    })
                }
                None => parsed_inputs.push(ParsedCardanoInput {
                    transaction_hash: input.transaction_id().to_bytes(),
                    index: input.index(),
                    value: None,
                    address: None,
                    path: None,
                    is_mine: false,
                }),
            }
        }
        Ok(parsed_inputs)
    }

    fn parse_outputs(tx: &Transaction) -> R<Vec<ParsedCardanoOutput>> {
        let outputs_len = tx.body().outputs().len();
        let mut parsed_outputs = vec![];
        for i in 0..outputs_len {
            let output = tx.body().outputs().get(i);
            let parsed_output = ParsedCardanoOutput {
                address: output
                    .address()
                    .to_bech32(None)
                    .map_err(|e| CardanoError::AddressEncodingError(e.to_string()))?,
                amount: normalize_coin(from_bignum(&output.amount().coin())),
                value: from_bignum(&output.amount().coin()),
                assets: output.amount().multiasset().map(|v| {
                    let mut parsed_multi_assets = vec![];
                    // temporary comment multi assets parse logic because it consumes a lot of memory but we don't display it on UI

                    // let len = v.keys().len();
                    // for _j in 0..len {
                    //     let policy_id = v.keys().get(_j);
                    //     let multi_assets = v.get(&policy_id);
                    //     if let Some(assets) = multi_assets {
                    //         let names = assets.keys();
                    //         let names_len = names.len();
                    //         for k in 0..names_len {
                    //             let name = names.get(k);
                    //             let value = assets.get(&name);
                    //             if let Some(asset_value) = value {
                    //                 let multi_asset = ParsedCardanoMultiAsset {
                    //                     policy_id: policy_id.clone().to_bytes(),
                    //                     name: name.to_bytes(),
                    //                     amount: normalize_value(from_bignum(&asset_value)),
                    //                     value: from_bignum(&asset_value),
                    //                     id: format!(
                    //                         "{}#{}",
                    //                         hex::encode(policy_id.to_bytes()),
                    //                         hex::encode(name.to_bytes())
                    //                     ),
                    //                 };
                    //                 parsed_multi_assets.push(multi_asset)
                    //             }
                    //         }
                    //     }
                    // }
                    parsed_multi_assets
                }),
            };
            parsed_outputs.push(parsed_output);
        }
        Ok(parsed_outputs)
    }
}

static DIVIDER: f64 = 1_000_000f64;

fn normalize_coin(value: u64) -> String {
    format!("{} ADA", (value as f64).div(DIVIDER))
}

fn normalize_value(value: u64) -> String {
    format!("{}", (value as f64).div(DIVIDER))
}
