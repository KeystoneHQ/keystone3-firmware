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

use cardano_serialization_lib::crypto::{Ed25519KeyHash, ScriptHash};
use cardano_serialization_lib::protocol_types::governance::{Anchor, DRepKind};
use cardano_serialization_lib::utils::{from_bignum, BigNum};
use cardano_serialization_lib::{
    protocol_types::fixed_tx::FixedTransaction as Transaction,
    protocol_types::governance::VoteKind, Certificate, CertificateKind, NetworkId, NetworkIdKind,
};

use alloc::format;
use core::ops::Div;
use third_party::bitcoin::bip32::ChildNumber::{Hardened, Normal};
use third_party::bitcoin::bip32::DerivationPath;
use third_party::ur_registry::cardano::cardano_sign_structure::CardanoSignStructure;
use third_party::ur_registry::traits::From;

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
    payload: String,
    derivation_path: String,
    message_hash: String,
    xpub: String
});

impl_public_struct!(VotingProcedure {
    voter: String,
    transaction_id: String,
    index: String,
    vote: String
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
    auxiliary_data: Option<String>,
    voting_procedures: Vec<VotingProcedure>
});

impl_public_struct!(SignDataResult {
    pub_key: Vec<u8>,
    signature: Vec<u8>
});

impl_public_struct!(SignVotingRegistrationResult {
    signature: Vec<u8>
});

impl_public_struct!(CertField {
    label: String,
    value: String
});

impl_public_struct!(CardanoCertificate {
    cert_type: String,
    fields: Vec<CertField>
});

const LABEL_ADDRESS: &str = "Address";
const LABEL_POOL: &str = "Pool";
const LABEL_DEPOSIT: &str = "Deposit";
const LABEL_DREP: &str = "DRep";
const LABEL_VOTE: &str = "Vote";
const LABEL_ABCHOR: &str = "Anchor";
const LABEL_ANCHOR_URL: &str = "Anchor URL";
const LABEL_ANCHOR_DATA_HASH: &str = "Anchor Data Hash";
const LABEL_COLD_KEY: &str = "Cold Key";
const LABEL_HOT_KEY: &str = "Hot Key";

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
    pub fn build(sign_data: Vec<u8>, derivation_path: String, xpub: String) -> R<Self> {
        let sign_structure = CardanoSignStructure::from_cbor(sign_data.clone());
        match sign_structure {
            Ok(sign_structure) => {
                let raw_payload = sign_structure.get_payload();
                let payload = String::from_utf8(hex::decode(raw_payload.clone()).unwrap())
                    .unwrap_or_else(|_| raw_payload.clone());
                Ok(Self {
                    payload,
                    derivation_path,
                    message_hash: hex::encode(raw_payload),
                    xpub,
                })
            }
            Err(e) => Ok(Self {
                payload: hex::encode(sign_data.clone()),
                derivation_path,
                message_hash: hex::encode(sign_data),
                xpub,
            }),
        }
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

        let voting_procedures = match tx.body().voting_procedures() {
            Some(v) => {
                let voters = v.get_voters();
                let mut voting_procedures = vec![];
                for voter_index in 0..voters.len() {
                    let voter = voters.get(voter_index).unwrap();
                    let actions = v.get_governance_action_ids_by_voter(&voter);
                    for i in 0..actions.len() {
                        let action = actions.get(i).unwrap();
                        let procedure = v.get(&voter, &action);
                        let vote = match procedure.unwrap().vote_kind() {
                            VoteKind::No => "No".to_string(),
                            VoteKind::Yes => "Yes".to_string(),
                            VoteKind::Abstain => "Abstain".to_string(),
                        };
                        voting_procedures.push(VotingProcedure {
                            voter: voter.to_key_hash().unwrap().to_string(),
                            transaction_id: action.transaction_id().to_string(),
                            index: action.index().to_string(),
                            vote,
                        })
                    }
                }
                voting_procedures
            }
            None => vec![],
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
            voting_procedures,
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
                    let fields = vec![
                        CertField {
                            label: LABEL_ADDRESS.to_string(),
                            value: RewardAddress::new(network_id, &_cert.stake_credential())
                                .to_address()
                                .to_bech32(None)
                                .map_err(|e| CardanoError::InvalidTransaction(e.to_string()))?,
                        },
                        CertField {
                            label: LABEL_POOL.to_string(),
                            value: _cert
                                .pool_keyhash()
                                .to_bech32("pool")
                                .map_err(|e| CardanoError::InvalidTransaction(e.to_string()))?,
                        },
                    ];
                    certs.push(CardanoCertificate::new(
                        "Stake Pool Delegation".to_string(),
                        fields,
                    ));
                }
                if let Some(_cert) = cert.as_stake_deregistration() {
                    let mut fields = vec![CertField {
                        label: LABEL_ADDRESS.to_string(),
                        value: RewardAddress::new(network_id, &_cert.stake_credential())
                            .to_address()
                            .to_bech32(None)
                            .map_err(|e| CardanoError::InvalidTransaction(e.to_string()))?,
                    }];
                    match _cert.coin() {
                        Some(v) => {
                            fields.push(CertField {
                                label: LABEL_DEPOSIT.to_string(),
                                value: normalize_coin(from_bignum(&v)),
                            });
                        }
                        None => {}
                    }
                    certs.push(CardanoCertificate::new(
                        "Stake Deregistration".to_string(),
                        fields,
                    ));
                }
                if let Some(_cert) = cert.as_stake_registration() {
                    let mut fields = vec![CertField {
                        label: LABEL_ADDRESS.to_string(),
                        value: RewardAddress::new(network_id, &_cert.stake_credential())
                            .to_address()
                            .to_bech32(None)
                            .map_err(|e| CardanoError::InvalidTransaction(e.to_string()))?,
                    }];
                    match _cert.coin() {
                        Some(v) => {
                            fields.push(CertField {
                                label: LABEL_DEPOSIT.to_string(),
                                value: normalize_coin(from_bignum(&v)),
                            });
                        }
                        None => {}
                    }
                    certs.push(CardanoCertificate::new(
                        "Account Registration".to_string(),
                        fields,
                    ));
                }
                if let Some(_cert) = cert.as_vote_delegation() {
                    let (variant2, variant2_label) = match _cert.drep().kind() {
                        DRepKind::AlwaysAbstain => ("Abstain".to_string(), LABEL_VOTE.to_string()),
                        DRepKind::AlwaysNoConfidence => {
                            ("No Confidence".to_string(), LABEL_VOTE.to_string())
                        }
                        DRepKind::KeyHash => (
                            _cert
                                .drep()
                                .to_key_hash()
                                .unwrap()
                                .to_bech32("drep")
                                .map_err(|e| CardanoError::InvalidTransaction(e.to_string()))?,
                            LABEL_DREP.to_string(),
                        ),
                        DRepKind::ScriptHash => (
                            _cert
                                .drep()
                                .to_script_hash()
                                .unwrap()
                                .to_bech32("")
                                .map_err(|e| CardanoError::InvalidTransaction(e.to_string()))?,
                            LABEL_DREP.to_string(),
                        ),
                    };
                    let fields = vec![
                        CertField {
                            label: LABEL_ADDRESS.to_string(),
                            value: RewardAddress::new(network_id, &_cert.stake_credential())
                                .to_address()
                                .to_bech32(None)
                                .map_err(|e| CardanoError::InvalidTransaction(e.to_string()))?,
                        },
                        CertField {
                            label: variant2_label,
                            value: variant2,
                        },
                    ];
                    certs.push(CardanoCertificate::new(
                        "DRep Delegation".to_string(),
                        fields,
                    ));
                }
                if let Some(_cert) = cert.as_pool_registration() {
                    let fields = vec![CertField {
                        label: LABEL_ADDRESS.to_string(),
                        value: _cert
                            .pool_params()
                            .reward_account()
                            .to_address()
                            .to_bech32(None)
                            .map_err(|e| CardanoError::InvalidTransaction(e.to_string()))?,
                    }];
                    certs.push(CardanoCertificate::new(
                        "Pool Registration".to_string(),
                        fields,
                    ));
                }
                if let Some(_cert) = cert.as_pool_retirement() {
                    let fields = vec![CertField {
                        label: LABEL_POOL.to_string(),
                        value: _cert
                            .pool_keyhash()
                            .to_bech32("pool")
                            .map_err(|e| CardanoError::InvalidTransaction(e.to_string()))?,
                    }];
                    certs.push(CardanoCertificate::new(
                        "Pool Retirement".to_string(),
                        fields,
                    ));
                }
                if let Some(_cert) = cert.as_genesis_key_delegation() {
                    let fields = vec![CertField {
                        label: LABEL_ADDRESS.to_string(),
                        value: "None".to_string(),
                    }];
                    certs.push(CardanoCertificate::new(
                        "Genesis Key Delegation".to_string(),
                        fields,
                    ));
                }
                if let Some(_cert) = cert.as_move_instantaneous_rewards_cert() {
                    let fields = vec![CertField {
                        label: LABEL_ADDRESS.to_string(),
                        value: "None".to_string(),
                    }];
                    certs.push(CardanoCertificate::new(
                        "Move Instantaneous Rewards Cert".to_string(),
                        fields,
                    ));
                }
                if let Some(_cert) = cert.as_committee_hot_auth() {
                    let fields = vec![
                        CertField {
                            label: LABEL_HOT_KEY.to_string(),
                            value: match _cert.committee_hot_key().kind() {
                                Ed25519KeyHash => {
                                    _cert.committee_hot_key().to_keyhash().unwrap().to_string()
                                }
                                ScriptHash => _cert
                                    .committee_hot_key()
                                    .to_scripthash()
                                    .unwrap()
                                    .to_string(),
                            },
                        },
                        CertField {
                            label: LABEL_COLD_KEY.to_string(),
                            value: match _cert.committee_cold_key().kind() {
                                Ed25519KeyHash => {
                                    _cert.committee_cold_key().to_keyhash().unwrap().to_string()
                                }
                                ScriptHash => _cert
                                    .committee_cold_key()
                                    .to_scripthash()
                                    .unwrap()
                                    .to_string(),
                            },
                        },
                    ];
                    certs.push(CardanoCertificate::new(
                        "Committee Hot Auth".to_string(),
                        fields,
                    ));
                }
                if let Some(_cert) = cert.as_committee_cold_resign() {
                    let mut fields = vec![CertField {
                        label: LABEL_COLD_KEY.to_string(),
                        value: match _cert.committee_cold_key().kind() {
                            Ed25519KeyHash => {
                                _cert.committee_cold_key().to_keyhash().unwrap().to_string()
                            }
                            ScriptHash => _cert
                                .committee_cold_key()
                                .to_scripthash()
                                .unwrap()
                                .to_string(),
                        },
                    }];
                    if let Some(anchor) = _cert.anchor() {
                        let fields = vec![
                            CertField {
                                label: LABEL_ANCHOR_URL.to_string(),
                                value: anchor.url().url(),
                            },
                            CertField {
                                label: LABEL_ANCHOR_DATA_HASH.to_string(),
                                value: anchor.anchor_data_hash().to_string(),
                            },
                        ];
                    }
                    certs.push(CardanoCertificate::new(
                        "Committee Cold Resign".to_string(),
                        fields,
                    ));
                }
                if let Some(_cert) = cert.as_drep_deregistration() {
                    let deposit = normalize_coin(from_bignum(&_cert.coin()));
                    let (variant1, variant1_label) = match _cert.voting_credential().kind() {
                        Ed25519KeyHash => (
                            _cert
                                .voting_credential()
                                .to_keyhash()
                                .unwrap()
                                .to_bech32("drep")
                                .map_err(|e| CardanoError::InvalidTransaction(e.to_string()))?,
                            LABEL_DREP.to_string(),
                        ),
                        ScriptHash => (
                            _cert
                                .voting_credential()
                                .to_scripthash()
                                .unwrap()
                                .to_bech32("")
                                .map_err(|e| CardanoError::InvalidTransaction(e.to_string()))?,
                            LABEL_DREP.to_string(),
                        ),
                    };
                    let fields = vec![
                        CertField {
                            label: variant1_label,
                            value: variant1,
                        },
                        CertField {
                            label: LABEL_DEPOSIT.to_string(),
                            value: deposit,
                        },
                    ];
                    certs.push(CardanoCertificate::new(
                        "Drep Deregistration".to_string(),
                        fields,
                    ));
                }
                if let Some(_cert) = cert.as_drep_registration() {
                    let deposit = normalize_coin(from_bignum(&_cert.coin()));
                    let (variant1, variant1_label) = match _cert.voting_credential().kind() {
                        Ed25519KeyHash => (
                            _cert
                                .voting_credential()
                                .to_keyhash()
                                .unwrap()
                                .to_bech32("drep")
                                .map_err(|e| CardanoError::InvalidTransaction(e.to_string()))?,
                            LABEL_DREP.to_string(),
                        ),
                        ScriptHash => (
                            _cert
                                .voting_credential()
                                .to_scripthash()
                                .unwrap()
                                .to_bech32("")
                                .map_err(|e| CardanoError::InvalidTransaction(e.to_string()))?,
                            LABEL_DREP.to_string(),
                        ),
                    };
                    let fields = vec![
                        CertField {
                            label: variant1_label,
                            value: variant1,
                        },
                        CertField {
                            label: LABEL_DEPOSIT.to_string(),
                            value: deposit,
                        },
                        CertField {
                            label: LABEL_ANCHOR_URL.to_string(),
                            value: _cert
                                .anchor()
                                .map(|v| v.url().url())
                                .unwrap_or("None".to_string()),
                        },
                        CertField {
                            label: LABEL_ANCHOR_DATA_HASH.to_string(),
                            value: _cert
                                .anchor()
                                .map(|v| v.anchor_data_hash().to_string())
                                .unwrap_or("None".to_string()),
                        },
                    ];
                    certs.push(CardanoCertificate::new(
                        "Drep Registration".to_string(),
                        fields,
                    ));
                }
                if let Some(_cert) = cert.as_drep_update() {
                    let anchor_data_hash = match _cert.anchor() {
                        Some(anchor) => Some(anchor.anchor_data_hash().to_string()),
                        None => None,
                    };
                    let (variant1, variant1_label) = match _cert.voting_credential().kind() {
                        Ed25519KeyHash => (
                            _cert
                                .voting_credential()
                                .to_keyhash()
                                .unwrap()
                                .to_bech32("drep")
                                .map_err(|e| CardanoError::InvalidTransaction(e.to_string()))?,
                            LABEL_DREP.to_string(),
                        ),
                        ScriptHash => (
                            _cert
                                .voting_credential()
                                .to_scripthash()
                                .unwrap()
                                .to_bech32("")
                                .map_err(|e| CardanoError::InvalidTransaction(e.to_string()))?,
                            LABEL_DREP.to_string(),
                        ),
                    };
                    let fields = vec![
                        CertField {
                            label: variant1_label,
                            value: variant1,
                        },
                        CertField {
                            label: LABEL_ANCHOR_URL.to_string(),
                            value: _cert
                                .anchor()
                                .map(|v| v.url().url())
                                .unwrap_or("None".to_string()),
                        },
                        CertField {
                            label: LABEL_ANCHOR_DATA_HASH.to_string(),
                            value: anchor_data_hash.unwrap_or("None".to_string()),
                        },
                    ];
                    certs.push(CardanoCertificate::new("Drep Update".to_string(), fields));
                }
                if let Some(_cert) = cert.as_stake_and_vote_delegation() {
                    let (variant3, variant3_label) = match _cert.drep().kind() {
                        DRepKind::AlwaysAbstain => ("Abstain".to_string(), LABEL_VOTE.to_string()),
                        DRepKind::AlwaysNoConfidence => {
                            ("No Confidence".to_string(), LABEL_VOTE.to_string())
                        }
                        DRepKind::KeyHash => (
                            _cert
                                .drep()
                                .to_key_hash()
                                .unwrap()
                                .to_bech32("drep")
                                .map_err(|e| CardanoError::InvalidTransaction(e.to_string()))?,
                            LABEL_DREP.to_string(),
                        ),
                        DRepKind::ScriptHash => (
                            _cert
                                .drep()
                                .to_script_hash()
                                .unwrap()
                                .to_bech32("")
                                .map_err(|e| CardanoError::InvalidTransaction(e.to_string()))?,
                            LABEL_DREP.to_string(),
                        ),
                    };
                    let fields = vec![
                        CertField {
                            label: LABEL_ADDRESS.to_string(),
                            value: RewardAddress::new(network_id, &_cert.stake_credential())
                                .to_address()
                                .to_bech32(None)
                                .map_err(|e| CardanoError::InvalidTransaction(e.to_string()))?,
                        },
                        CertField {
                            label: LABEL_POOL.to_string(),
                            value: _cert
                                .pool_keyhash()
                                .to_bech32("pool")
                                .map_err(|e| CardanoError::InvalidTransaction(e.to_string()))?,
                        },
                        CertField {
                            label: variant3_label,
                            value: variant3,
                        },
                    ];
                    certs.push(CardanoCertificate::new(
                        "Stake And Vote Delegation".to_string(),
                        fields,
                    ));
                }
                if let Some(_cert) = cert.as_stake_registration_and_delegation() {
                    let deposit = normalize_coin(from_bignum(&_cert.coin()));
                    let fields = vec![
                        CertField {
                            label: LABEL_ADDRESS.to_string(),
                            value: RewardAddress::new(network_id, &_cert.stake_credential())
                                .to_address()
                                .to_bech32(None)
                                .map_err(|e| CardanoError::InvalidTransaction(e.to_string()))?,
                        },
                        CertField {
                            label: LABEL_POOL.to_string(),
                            value: _cert
                                .pool_keyhash()
                                .to_bech32("pool")
                                .map_err(|e| CardanoError::InvalidTransaction(e.to_string()))?,
                        },
                        CertField {
                            label: LABEL_DEPOSIT.to_string(),
                            value: deposit,
                        },
                    ];
                    certs.push(CardanoCertificate::new(
                        "Stake Registration & Delegation".to_string(),
                        fields,
                    ));
                }
                if let Some(_cert) = cert.as_stake_vote_registration_and_delegation() {
                    let (variant3, variant3_label) = match _cert.drep().kind() {
                        DRepKind::AlwaysAbstain => ("Abstain".to_string(), LABEL_VOTE.to_string()),
                        DRepKind::AlwaysNoConfidence => {
                            ("No Confidence".to_string(), LABEL_VOTE.to_string())
                        }
                        DRepKind::KeyHash => (
                            _cert
                                .drep()
                                .to_key_hash()
                                .unwrap()
                                .to_bech32("drep")
                                .map_err(|e| CardanoError::InvalidTransaction(e.to_string()))?,
                            LABEL_DREP.to_string(),
                        ),
                        DRepKind::ScriptHash => (
                            _cert
                                .drep()
                                .to_script_hash()
                                .unwrap()
                                .to_bech32("")
                                .map_err(|e| CardanoError::InvalidTransaction(e.to_string()))?,
                            LABEL_DREP.to_string(),
                        ),
                    };
                    let deposit = normalize_coin(from_bignum(&_cert.coin()));
                    let fields = vec![
                        CertField {
                            label: LABEL_ADDRESS.to_string(),
                            value: RewardAddress::new(network_id, &_cert.stake_credential())
                                .to_address()
                                .to_bech32(None)
                                .map_err(|e| CardanoError::InvalidTransaction(e.to_string()))?,
                        },
                        CertField {
                            label: LABEL_POOL.to_string(),
                            value: _cert
                                .pool_keyhash()
                                .to_bech32("pool")
                                .map_err(|e| CardanoError::InvalidTransaction(e.to_string()))?,
                        },
                        CertField {
                            label: variant3_label,
                            value: variant3,
                        },
                        CertField {
                            label: LABEL_DEPOSIT.to_string(),
                            value: deposit,
                        },
                    ];
                    certs.push(CardanoCertificate::new(
                        "Stake Vote Registration & Delegation".to_string(),
                        fields,
                    ));
                }
                if let Some(_cert) = cert.as_vote_registration_and_delegation() {
                    let (variant2, variant2_label) = match _cert.drep().kind() {
                        DRepKind::AlwaysAbstain => ("Abstain".to_string(), LABEL_VOTE.to_string()),
                        DRepKind::AlwaysNoConfidence => {
                            ("No Confidence".to_string(), LABEL_VOTE.to_string())
                        }
                        DRepKind::KeyHash => (
                            _cert
                                .drep()
                                .to_key_hash()
                                .unwrap()
                                .to_bech32("drep")
                                .map_err(|e| CardanoError::InvalidTransaction(e.to_string()))?,
                            LABEL_DREP.to_string(),
                        ),
                        DRepKind::ScriptHash => (
                            _cert
                                .drep()
                                .to_script_hash()
                                .unwrap()
                                .to_bech32("")
                                .map_err(|e| CardanoError::InvalidTransaction(e.to_string()))?,
                            LABEL_DREP.to_string(),
                        ),
                    };
                    let deposit = normalize_coin(from_bignum(&_cert.coin()));
                    let fields = vec![
                        CertField {
                            label: LABEL_ADDRESS.to_string(),
                            value: RewardAddress::new(network_id, &_cert.stake_credential())
                                .to_address()
                                .to_bech32(None)
                                .map_err(|e| CardanoError::InvalidTransaction(e.to_string()))?,
                        },
                        CertField {
                            label: variant2_label,
                            value: variant2,
                        },
                        CertField {
                            label: LABEL_DEPOSIT.to_string(),
                            value: deposit,
                        },
                    ];
                    certs.push(CardanoCertificate::new(
                        "Vote Registration And Delegation".to_string(),
                        fields,
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

#[cfg(test)]
mod tests {
    use super::*;
    use third_party::ur_registry::cardano::cardano_sign_request::CardanoSignRequest;

    #[test]
    fn test_normalize_coin() {
        let value = 1_000_000u64;
        let result = normalize_coin(value);
        assert_eq!(result, "1 ADA");
    }

    #[test]
    fn test_normalize_value() {
        let value = 1_000_000u64;
        let result = normalize_value(value);
        assert_eq!(result, "1");
    }

    #[test]
    fn test_parse_sign_data() {
        let payload = "846a5369676e6174757265315882a301270458390069fa1bd9338574702283d8fb71f8cce1831c3ea4854563f5e4043aea33a4f1f468454744b2ff3644b2ab79d48e76a3187f902fe8a1bcfaad676164647265737358390069fa1bd9338574702283d8fb71f8cce1831c3ea4854563f5e4043aea33a4f1f468454744b2ff3644b2ab79d48e76a3187f902fe8a1bcfaad4043abc123";
        let xpub = "ca0e65d9bb8d0dca5e88adc5e1c644cc7d62e5a139350330281ed7e3a6938d2c";
        let data = ParsedCardanoSignData::build(
            hex::decode(payload).unwrap(),
            "m/1852'/1815'/0'/0/0".to_string(),
            xpub.to_string(),
        )
        .unwrap();
        assert_eq!(data.get_derivation_path(), "m/1852'/1815'/0'/0/0");
        assert_eq!(hex::encode(data.get_payload()), "616263313233");
    }

    #[test]
    fn test_parse_sign() {
        let sign_data = hex::decode("84a400828258204e3a6e7fdcb0d0efa17bf79c13aed2b4cb9baf37fb1aa2e39553d5bd720c5c99038258204e3a6e7fdcb0d0efa17bf79c13aed2b4cb9baf37fb1aa2e39553d5bd720c5c99040182a200581d6179df4c75f7616d7d1fd39cbc1a6ea6b40a0d7b89fea62fc0909b6c370119c350a200581d61c9b0c9761fd1dc0404abd55efc895026628b5035ac623c614fbad0310119c35002198ecb0300a0f5f6").unwrap();
        let request = CardanoSignRequest::new(
            Some(
                hex::decode("9b1deb4d3b7d4bad9bdd2b0d7b3dcb6d")
                    .unwrap()
                    .try_into()
                    .unwrap(),
            ),
            sign_data.clone().try_into().unwrap(),
            vec![],
            vec![],
            Some("".to_string()),
        );
        let xpub = hex::encode("ca0e65d9bb8d0dca5e88adc5e1c644cc7d62e5a139350330281ed7e3a6938d2c");
        let master_fingerprint = hex::decode("52744703").unwrap();
        let context = ParseContext::new(vec![], vec![], xpub, master_fingerprint);
        let tx = Transaction::from_hex(&hex::encode(sign_data)).unwrap();

        let network_id = ParsedCardanoTx::judge_network_id(&tx);
        assert_eq!(network_id, 1);

        let auxiliary_data = ParsedCardanoTx::parse_auxiliary_data(&tx);
        assert_eq!(auxiliary_data.unwrap(), None);

        let certificates = ParsedCardanoTx::parse_certificates(&tx, network_id);
        assert_eq!(certificates.unwrap().len(), 0);

        let withdrawals = ParsedCardanoTx::parse_withdrawals(&tx);
        assert_eq!(withdrawals.unwrap().len(), 0);

        let cardano_tx = ParsedCardanoTx::from_cardano_tx(tx, context);
        assert_eq!(cardano_tx.is_ok(), true);
    }
}
