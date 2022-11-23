use crate::address::{derive_address, AddressType};
use crate::errors::{CardanoError, R};
use alloc::collections::BTreeMap;
use alloc::string::{String, ToString};
use alloc::vec;
use alloc::vec::Vec;
use app_utils::{impl_internal_struct, impl_public_struct};
use cardano_serialization_lib::address::RewardAddress;

use cardano_serialization_lib::utils::from_bignum;
use cardano_serialization_lib::{CertificateKind, NetworkId, NetworkIdKind, Transaction};

use crate::detail::{
    CardanoDetail, CardanoDetailStakeAction, CardanoRegistration, CardanoStake, CardanoWithdrawal,
};
use crate::overview::{
    CardanoHeaderCard, CardanoOverview, CardanoOverviewStakeCard, CardanoOverviewTransferCard,
    CardanoOverviewWithdrawalCard,
};
use alloc::format;
use core::ops::Div;
use third_party::bitcoin::bip32::ChildNumber::{Hardened, Normal};
use third_party::bitcoin::bip32::DerivationPath;

use third_party::hex;
use third_party::itertools::Itertools;

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

impl_public_struct!(ParsedCardanoTx {
    overview: CardanoOverview,
    detail: CardanoDetail,
    fee: String,
    from: Vec<CardanoFrom>,
    to: Vec<CardanoTo>,
    network: String,
    method: String
});

// method label on ui
#[derive(Clone, Debug, Default)]
pub enum CardanoMethod {
    #[default]
    Transfer,
    Stake,
    Withdrawal,
}

impl ToString for CardanoMethod {
    fn to_string(&self) -> String {
        match &self {
            CardanoMethod::Transfer => "Transfer".to_string(),
            CardanoMethod::Stake => "Stake".to_string(),
            CardanoMethod::Withdrawal => "Withdrawals".to_string(),
        }
    }
}

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

impl ParsedCardanoTx {
    pub fn from_cardano_tx(tx: Transaction, context: ParseContext) -> R<Self> {
        let parsed_inputs = Self::parse_inputs(&tx, &context)?;
        let parsed_outputs = Self::parse_outputs(&tx)?;
        let cert_actions = Self::parse_certs(&tx)?;
        let stake_actions = Self::parse_stake_actions(&tx, &cert_actions)?;

        let fee = from_bignum(&tx.body().fee());
        let method = Self::detect_method(&tx);

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

        let network = match tx.body().network_id() {
            None => "Cardano Mainnet".to_string(),
            Some(id) => match id.kind() {
                NetworkIdKind::Mainnet => "Cardano Mainnet".to_string(),
                NetworkIdKind::Testnet => "Cardano Testnet".to_string(),
            },
        };

        let fee = normalize_coin(fee);

        let deposit = match cert_actions
            .iter()
            .filter(|v| match v {
                CardanoCertAction::Registration(_) => true,
                _ => false,
            })
            .collect::<Vec<&CardanoCertAction>>()
            .len()
        {
            0 => None,
            x => Some(format!("{} ADA", x * 2)),
        };

        let deposit_reclaim = {
            let actions = cert_actions
                .iter()
                .filter(|v| match v {
                    CardanoCertAction::Deregistration(_) => true,
                    _ => false,
                })
                .collect::<Vec<&CardanoCertAction>>();
            match actions.len() {
                0 => None,
                x => Some(format!("{} ADA", x * 2)),
            }
        };

        let reward_amount = {
            let value = stake_actions.iter().fold(0, |acc, cur| match cur {
                CardanoDetailStakeAction::Withdrawal(withdraw) => acc + withdraw.get_value(),
                _ => acc,
            });
            normalize_coin(value)
        };

        let reward_account = {
            let addresses = stake_actions
                .iter()
                .filter(|v| match v {
                    CardanoDetailStakeAction::Withdrawal(_) => true,
                    _ => false,
                })
                .map(|v| match v {
                    CardanoDetailStakeAction::Withdrawal(x) => x.get_reward_address(),
                    _ => None,
                })
                .collect::<Vec<Option<String>>>();
            let mut addrs = vec![];
            for x in addresses {
                if let Some(value) = x {
                    addrs.push(value);
                }
            }
            match addrs.len() {
                0 => None,
                _ => Some(addrs.join(",")),
            }
        };

        let mut transaction_overview = CardanoOverview::default();

        transaction_overview.set_header_card(match method {
            CardanoMethod::Transfer => CardanoHeaderCard::Transfer(
                CardanoOverviewTransferCard::new(total_output_amount.clone()),
            ),
            CardanoMethod::Stake => CardanoHeaderCard::Stake(CardanoOverviewStakeCard::new(
                total_output_amount.clone(),
                deposit.clone(),
            )),
            CardanoMethod::Withdrawal => {
                CardanoHeaderCard::Withdrawal(CardanoOverviewWithdrawalCard::new(
                    reward_amount,
                    deposit_reclaim.clone(),
                    reward_account,
                ))
            }
        });

        let total_input_amount = {
            let _v = parsed_inputs.iter().fold(0u64, |acc, cur| match cur.value {
                Some(v) => acc + v,
                None => acc,
            });
            normalize_coin(_v)
        };

        let transaction_detail = CardanoDetail::new(
            total_input_amount,
            total_output_amount,
            deposit_reclaim.clone(),
            deposit.clone(),
            None,
        );

        Ok(Self {
            overview: transaction_overview,
            detail: transaction_detail,
            from: from_list_detail,
            to: to_list_detail,
            fee: fee.clone(),
            network: network.clone(),
            method: method.to_string(),
        })
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
                        amount: normalize_value(output.value),
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

    fn detect_method(tx: &Transaction) -> CardanoMethod {
        if let Some(withdrawals) = tx.body().withdrawals() {
            if withdrawals.len() > 0 {
                return CardanoMethod::Withdrawal;
            }
        }
        if let Some(certs) = tx.body().certs() {
            if certs.len() > 0 {
                let len = certs.len();
                for i in 0..len {
                    if let CertificateKind::StakeDeregistration = certs.get(i).kind() {
                        return CardanoMethod::Withdrawal;
                    }
                }
                return CardanoMethod::Stake;
            }
        }
        return CardanoMethod::Transfer;
    }

    pub fn verify(tx: Transaction, context: ParseContext) -> R<()> {
        let parsed_inputs = Self::parse_inputs(&tx, &context)?;
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
        Self::parse_certs(&tx)?;
        Ok(())
    }

    fn parse_inputs(tx: &Transaction, context: &ParseContext) -> R<Vec<ParsedCardanoInput>> {
        let inputs_len = tx.body().inputs().len();
        let mut parsed_inputs = vec![];
        for i in 0..inputs_len {
            let input = tx.body().inputs().get(i);
            let hash = input.transaction_id().to_hex();
            let index = input.index();
            let m = context.utxos.iter().find(|v| {
                hash.eq_ignore_ascii_case(&hex::encode(&v.transaction_hash)) & index.eq(&v.index)
            });
            match m {
                //known utxo
                Some(utxo) => {
                    let index =
                        match utxo
                            .path
                            .into_iter()
                            .last()
                            .ok_or(CardanoError::DerivationError(
                                "invalid derivation path".to_string(),
                            ))? {
                            Normal { index: i } => i,
                            Hardened { index: i } => i,
                        };

                    let address = derive_address(
                        context.get_cardano_xpub(),
                        index.clone(),
                        AddressType::Base,
                        1,
                    )?;

                    parsed_inputs.push(ParsedCardanoInput {
                        transaction_hash: utxo.transaction_hash.clone(),
                        index: utxo.index,
                        value: Some(utxo.value),
                        address: Some(address),
                        path: None,
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
                    let len = v.keys().len();
                    for _j in 0..len {
                        let policy_id = v.keys().get(i);
                        let multi_assets = v.get(&policy_id);
                        if let Some(assets) = multi_assets {
                            let names = assets.keys();
                            let names_len = names.len();
                            for k in 0..names_len {
                                let name = names.get(k);
                                let value = assets.get(&name);
                                if let Some(asset_value) = value {
                                    let multi_asset = ParsedCardanoMultiAsset {
                                        policy_id: policy_id.clone().to_bytes(),
                                        name: name.to_bytes(),
                                        amount: normalize_value(from_bignum(&asset_value)),
                                        value: from_bignum(&asset_value),
                                        id: format!(
                                            "{}#{}",
                                            hex::encode(policy_id.to_bytes()),
                                            hex::encode(name.to_bytes())
                                        ),
                                    };
                                    parsed_multi_assets.push(multi_asset)
                                }
                            }
                        }
                    }
                    parsed_multi_assets
                }),
            };
            parsed_outputs.push(parsed_output);
        }
        Ok(parsed_outputs)
    }

    fn parse_certs(tx: &Transaction) -> R<Vec<CardanoCertAction>> {
        let mut parsed_certs = vec![];
        let network_id = match tx
            .body()
            .network_id()
            .unwrap_or(NetworkId::mainnet())
            .kind()
        {
            NetworkIdKind::Mainnet => 0,
            NetworkIdKind::Testnet => 1,
        };
        if let Some(certs) = tx.body().certs() {
            let len = certs.len();
            for i in 0..len {
                let cert = certs.get(i);
                let result = match cert.kind() {
                    CertificateKind::StakeRegistration => cert.as_stake_registration().map(|v| {
                        Ok(CardanoCertAction::Registration(Registration {
                            stake_key: RewardAddress::new(network_id, &v.stake_credential()),
                        }))
                    }),
                    CertificateKind::StakeDeregistration => {
                        cert.as_stake_deregistration().map(|v| {
                            Ok(CardanoCertAction::Deregistration(Deregistration {
                                stake_key: RewardAddress::new(network_id, &v.stake_credential()),
                            }))
                        })
                    }
                    CertificateKind::StakeDelegation => cert.as_stake_delegation().map(|v| {
                        Ok(CardanoCertAction::Delegation(Delegation {
                            pool: v.pool_keyhash().to_hex(),
                            stake_key: RewardAddress::new(network_id, &v.stake_credential()),
                        }))
                    }),
                    CertificateKind::PoolRegistration => {
                        return Err(CardanoError::UnsupportedTransaction(
                            "PoolRegistration".to_string(),
                        ));
                    }
                    CertificateKind::PoolRetirement => {
                        return Err(CardanoError::UnsupportedTransaction(
                            "PoolRetirement".to_string(),
                        ));
                    }
                    CertificateKind::GenesisKeyDelegation => {
                        return Err(CardanoError::UnsupportedTransaction(
                            "GenesisKeyDelegation".to_string(),
                        ));
                    }
                    CertificateKind::MoveInstantaneousRewardsCert => {
                        return Err(CardanoError::UnsupportedTransaction(
                            "MoveInstantaneousRewardsCert".to_string(),
                        ));
                    }
                };
                if let Some(parsed_cert) = result {
                    parsed_certs.push(parsed_cert);
                }
            }
        }
        parsed_certs.into_iter().collect()
    }

    fn parse_stake_actions(
        tx: &Transaction,
        certs: &Vec<CardanoCertAction>,
    ) -> R<Vec<CardanoDetailStakeAction>> {
        let mut parsed_actions = vec![];

        let mut registrations: Vec<Registration> = vec![];
        certs.iter().for_each(|v| match v {
            CardanoCertAction::Registration(x) => registrations.push(x.clone()),
            _ => {}
        });

        let mut deregistrations = vec![];
        certs.iter().for_each(|v| {
            if let CardanoCertAction::Deregistration(x) = v {
                deregistrations.push(x.clone());
            }
        });

        let mut delegations = vec![];
        certs.iter().for_each(|v| match v {
            CardanoCertAction::Delegation(x) => delegations.push(x.clone()),
            _ => {}
        });

        if let Some(_withdrawals) = tx.body().withdrawals() {
            let len = _withdrawals.keys().len();
            for i in 0..len {
                let key = _withdrawals.keys().get(i);
                let value = match _withdrawals.get(&key) {
                    None => 0,
                    Some(_v) => from_bignum(&_v),
                };

                let mut cardano_withdrawal = CardanoWithdrawal::new(
                    Some(
                        key.to_address()
                            .to_bech32(None)
                            .map_err(|e| CardanoError::InvalidTransaction(e.to_string()))?,
                    ),
                    Some(normalize_coin(value)),
                    value,
                    None,
                );

                if let Some((index, _)) = deregistrations
                    .iter()
                    .find_position(|v| v.stake_key.eq(&key))
                {
                    deregistrations.remove(index);
                    //they are the same actually.
                    cardano_withdrawal
                        .set_deregistration_stake_key(cardano_withdrawal.get_reward_address());
                }
                parsed_actions.push(CardanoDetailStakeAction::Withdrawal(cardano_withdrawal))
            }
        }

        for v in deregistrations {
            let withdraw = CardanoWithdrawal::new(
                None,
                None,
                0,
                Some(
                    v.stake_key
                        .to_address()
                        .to_bech32(None)
                        .map_err(|e| CardanoError::InvalidTransaction(e.to_string()))?,
                ),
            );
            parsed_actions.push(CardanoDetailStakeAction::Withdrawal(withdraw))
        }

        for v in delegations {
            let key = v.stake_key;
            let delegation = CardanoStake::new(
                key.to_address()
                    .to_bech32(None)
                    .map_err(|e| CardanoError::InvalidTransaction(e.to_string()))?,
                v.pool,
            );
            if let Some((index, _)) = registrations.iter().find_position(|v| v.stake_key.eq(&key)) {
                registrations.remove(index);
            }
            parsed_actions.push(CardanoDetailStakeAction::Stake(delegation))
        }

        for v in registrations {
            let registration = CardanoRegistration::new(
                v.stake_key
                    .to_address()
                    .to_bech32(None)
                    .map_err(|e| CardanoError::InvalidTransaction(e.to_string()))?,
            );
            parsed_actions.push(CardanoDetailStakeAction::Registration(registration))
        }

        Ok(parsed_actions)
    }
}

static DIVIDER: f64 = 1_000_000_00f64;

fn normalize_coin(value: u64) -> String {
    format!("{} ADA", (value as f64).div(DIVIDER))
}

fn normalize_value(value: u64) -> String {
    format!("{}", (value as f64).div(DIVIDER))
}
