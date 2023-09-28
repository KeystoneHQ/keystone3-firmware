use core::str::FromStr;

use alloc::{
    string::{String, ToString},
    vec::Vec,
};
use third_party::{
    bitcoin::bip32::{ChildNumber, DerivationPath},
    secp256k1::Secp256k1,
    ur_registry::{
        crypto_hd_key::CryptoHDKey,
        crypto_key_path::{CryptoKeyPath, PathComponent},
        error::{URError, URResult},
        extend::crypto_multi_accounts::CryptoMultiAccounts,
    },
};

use crate::{common::get_path_component, ExtendedPublicKey, DEVICE_TYPE, DEVICE_VERSION};

fn get_device_id(serial_number: &str) -> String {
    use third_party::cryptoxide::hashing::sha256;
    third_party::hex::encode(&sha256(&sha256(serial_number.as_bytes()))[0..20])
}

const BTC_LEGACY_PREFIX: &str = "m/44'/0'/0'";
const BTC_SEGWIT_PREFIX: &str = "m/49'/0'/0'";
const BTC_NATIVE_SEGWIT_PREFIX: &str = "m/84'/0'/0'";
const ETH_STANDARD_PREFIX: &str = "m/44'/60'/0'";
const ETH_LEDGER_LIVE_PREFIX: &str = "m/44'/60'"; //overlap with ETH_STANDARD at 0

pub fn generate_crypto_multi_accounts(
    master_fingerprint: [u8; 4],
    serial_number: &str,
    extended_public_keys: Vec<ExtendedPublicKey>,
) -> URResult<CryptoMultiAccounts> {
    let device_id = get_device_id(serial_number);
    let mut keys = vec![];
    let btc_keys = vec![
        BTC_LEGACY_PREFIX.to_string(),
        BTC_SEGWIT_PREFIX.to_string(),
        BTC_NATIVE_SEGWIT_PREFIX.to_string(),
    ];
    for ele in extended_public_keys {
        match ele.get_path() {
            _path if btc_keys.contains(&_path.to_string()) => {
                keys.push(generate_k1_normal_key(
                    master_fingerprint,
                    ele.clone(),
                    None,
                )?);
            }
            _path if _path.to_string().eq(ETH_STANDARD_PREFIX) => {
                keys.push(generate_k1_normal_key(
                    master_fingerprint,
                    ele.clone(),
                    Some("account.standard".to_string()),
                )?);
                keys.push(generate_eth_ledger_live_key(
                    master_fingerprint,
                    ele,
                    Some("account.ledger_live".to_string()),
                )?);
            }
            _path if _path.to_string().starts_with(ETH_LEDGER_LIVE_PREFIX) => {
                keys.push(generate_eth_ledger_live_key(
                    master_fingerprint,
                    ele,
                    Some("account.ledger_live".to_string()),
                )?);
            }
            _ => {
                return Err(URError::UrEncodeError(format!(
                    "Unknown key path: {}",
                    ele.path.to_string()
                )))
            }
        }
    }

    Ok(CryptoMultiAccounts::new(
        master_fingerprint,
        keys,
        Some(DEVICE_TYPE.to_string()),
        Some(device_id),
        Some(DEVICE_VERSION.to_string()),
    ))
}

fn generate_k1_normal_key(
    mfp: [u8; 4],
    key: ExtendedPublicKey,
    note: Option<String>,
) -> URResult<CryptoHDKey> {
    let xpub = third_party::bitcoin::bip32::ExtendedPubKey::decode(&key.get_key())
        .map_err(|_e| URError::UrEncodeError(_e.to_string()))?;
    let path = key.get_path();
    let key_path = CryptoKeyPath::new(
        path.into_iter()
            .map(|v| match v {
                ChildNumber::Normal { index } => get_path_component(Some(index.clone()), false),
                ChildNumber::Hardened { index } => get_path_component(Some(index.clone()), true),
            })
            .collect::<URResult<Vec<PathComponent>>>()?,
        Some(mfp),
        Some(xpub.depth as u32),
    );
    Ok(CryptoHDKey::new_extended_key(
        Some(false),
        xpub.public_key.serialize().to_vec(),
        Some(xpub.chain_code.to_bytes().to_vec()),
        None,
        Some(key_path),
        None,
        Some(xpub.parent_fingerprint.to_bytes()),
        Some("Keystone".to_string()),
        note,
    ))
}

fn generate_eth_ledger_live_key(
    mfp: [u8; 4],
    key: ExtendedPublicKey,
    note: Option<String>,
) -> URResult<CryptoHDKey> {
    let xpub = third_party::bitcoin::bip32::ExtendedPubKey::decode(&key.get_key())
        .map_err(|_e| URError::UrEncodeError(_e.to_string()))?;
    let path = key.get_path();
    let sub_path =
        DerivationPath::from_str("m/0/0").map_err(|_e| URError::UrEncodeError(_e.to_string()))?;
    let _target_key = xpub
        .derive_pub(&Secp256k1::new(), &sub_path)
        .map_err(|_e| URError::UrEncodeError(_e.to_string()))?;
    let target_path = path
        .child(ChildNumber::Normal { index: 0 })
        .child(ChildNumber::Normal { index: 0 });
    let key_path = CryptoKeyPath::new(
        target_path
            .into_iter()
            .map(|v| match v {
                ChildNumber::Normal { index } => get_path_component(Some(index.clone()), false),
                ChildNumber::Hardened { index } => get_path_component(Some(index.clone()), true),
            })
            .collect::<URResult<Vec<PathComponent>>>()?,
        Some(mfp),
        Some(xpub.depth as u32),
    );
    Ok(CryptoHDKey::new_extended_key(
        Some(false),
        _target_key.public_key.serialize().to_vec(),
        None,
        None,
        Some(key_path),
        None,
        Some(_target_key.parent_fingerprint.to_bytes()),
        Some("Keystone".to_string()),
        note,
    ))
}
