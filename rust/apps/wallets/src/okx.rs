use core::str::FromStr;

use alloc::{
    string::{String, ToString},
    vec::Vec,
};
use {
    bitcoin::bip32::{ChildNumber, DerivationPath},
    bitcoin::secp256k1::Secp256k1,
    ur_registry::{
        crypto_hd_key::CryptoHDKey,
        crypto_key_path::{CryptoKeyPath, PathComponent},
        error::{URError, URResult},
        extend::crypto_multi_accounts::CryptoMultiAccounts,
    },
};

use crate::{common::get_path_component, ExtendedPublicKey};

fn get_device_id(serial_number: &str) -> String {
    use cryptoxide::hashing::sha256;
    hex::encode(&sha256(&sha256(serial_number.as_bytes()))[0..20])
}

const BTC_LEGACY_PREFIX: &str = "44'/0'/0'";
const BTC_SEGWIT_PREFIX: &str = "49'/0'/0'";
const BTC_NATIVE_SEGWIT_PREFIX: &str = "84'/0'/0'";
const BTC_TAPROOT_PREFIX: &str = "86'/0'/0'";
const ETH_STANDARD_PREFIX: &str = "44'/60'/0'";
const ETH_LEDGER_LIVE_PREFIX: &str = "44'/60'"; //overlap with ETH_STANDARD at 0
const TRX_PREFIX: &str = "44'/195'/0'";
const LTC_PREFIX: &str = "49'/2'/0'";
const BCH_PREFIX: &str = "44'/145'/0'";
const DASH_PREFIX: &str = "44'/5'/0'";

pub fn generate_crypto_multi_accounts(
    master_fingerprint: [u8; 4],
    serial_number: &str,
    extended_public_keys: Vec<ExtendedPublicKey>,
    device_type: &str,
    device_version: &str,
) -> URResult<CryptoMultiAccounts> {
    let device_id = get_device_id(serial_number);
    let mut keys = vec![];
    let k1_keys = [
        BTC_LEGACY_PREFIX.to_string(),
        BTC_SEGWIT_PREFIX.to_string(),
        BTC_NATIVE_SEGWIT_PREFIX.to_string(),
        BTC_TAPROOT_PREFIX.to_string(),
        TRX_PREFIX.to_string(),
        LTC_PREFIX.to_string(),
        BCH_PREFIX.to_string(),
        DASH_PREFIX.to_string(),
    ];
    for ele in extended_public_keys {
        match ele.get_path() {
            _path if k1_keys.contains(&_path.to_string().to_lowercase()) => {
                keys.push(generate_k1_normal_key(
                    master_fingerprint,
                    ele.clone(),
                    None,
                )?);
            }
            _path if _path.to_string().to_lowercase().eq(ETH_STANDARD_PREFIX) => {
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
            _path
                if _path
                    .to_string()
                    .to_lowercase()
                    .starts_with(ETH_LEDGER_LIVE_PREFIX) =>
            {
                keys.push(generate_eth_ledger_live_key(
                    master_fingerprint,
                    ele,
                    Some("account.ledger_live".to_string()),
                )?);
            }
            _ => {
                return Err(URError::UrEncodeError(format!(
                    "Unknown key path: {}",
                    ele.path
                )))
            }
        }
    }

    Ok(CryptoMultiAccounts::new(
        master_fingerprint,
        keys,
        Some(device_type.to_string()),
        Some(device_id),
        Some(device_version.to_string()),
    ))
}

fn generate_k1_normal_key(
    mfp: [u8; 4],
    key: ExtendedPublicKey,
    note: Option<String>,
) -> URResult<CryptoHDKey> {
    let xpub = bitcoin::bip32::Xpub::decode(&key.get_key())
        .map_err(|_e| URError::UrEncodeError(_e.to_string()))?;
    let path = key.get_path();
    let key_path = CryptoKeyPath::new(
        path.into_iter()
            .map(|v| match v {
                ChildNumber::Normal { index } => get_path_component(Some(*index), false),
                ChildNumber::Hardened { index } => get_path_component(Some(*index), true),
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
    let xpub = bitcoin::bip32::Xpub::decode(&key.get_key())
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
                ChildNumber::Normal { index } => get_path_component(Some(*index), false),
                ChildNumber::Hardened { index } => get_path_component(Some(*index), true),
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

#[cfg(test)]
mod tests {
    use super::*;
    use alloc::vec;
    use bitcoin::bip32::{DerivationPath, Xpub};
    use core::str::FromStr;

    fn serialize_xpub(xpub: &Xpub) -> Vec<u8> {
        let mut bytes = Vec::new();
        // Version: xpub (Mainnet) = 0x0488B21E
        bytes.extend_from_slice(&[0x04, 0x88, 0xB2, 0x1E]);
        bytes.push(xpub.depth);
        bytes.extend_from_slice(xpub.parent_fingerprint.as_bytes());
        // ChildNumber to u32
        let child_num: u32 = xpub.child_number.into();
        bytes.extend_from_slice(&child_num.to_be_bytes());
        bytes.extend_from_slice(xpub.chain_code.as_bytes());
        bytes.extend_from_slice(&xpub.public_key.serialize());
        bytes
    }

    #[test]
    fn test_generate_crypto_multi_accounts() {
        let mfp = [0x75, 0x7e, 0x6f, 0xc9];
        let serial = "123456";
        let device_type = "Keystone 3 Pro";
        let device_version = "1.0.0";

        // ETH Standard xpub
        let eth_xpub_str = "xpub6C8zKiZZ8V75XynjThhvdjy7hbnJHAFkhW7jL9EvBCsRFSRov4sXUJATU6CqUF9BxAbryiU3eghdHDLbwgF8ASE4AwHTzkLHaHsbwiCnkHc";
        let eth_xpub = Xpub::from_str(eth_xpub_str).unwrap();
        let eth_xpub_bytes = serialize_xpub(&eth_xpub);
        
        let eth_path = DerivationPath::from_str("m/44'/60'/0'").unwrap();
        let eth_key = ExtendedPublicKey {
            path: eth_path,
            key: eth_xpub_bytes,
        };

        // BTC Native Segwit xpub
        let btc_xpub_str = "xpub6C8zKiZZ8V75aXTgYAswqCgYUHeBYg1663a4Ri6zdJ4GW58r67Kmj4Fr8pbBK9usq45o8iQ8dBM75o67ct1M38yeb6RhFTWBxiYeq8Kg84Z";
        let btc_xpub = Xpub::from_str(btc_xpub_str).unwrap();
        let btc_xpub_bytes = serialize_xpub(&btc_xpub);

        let btc_path = DerivationPath::from_str("m/84'/0'/0'").unwrap();
        let btc_key = ExtendedPublicKey {
            path: btc_path,
            key: btc_xpub_bytes,
        };

        let keys = vec![eth_key, btc_key];

        let result = generate_crypto_multi_accounts(mfp, serial, keys, device_type, device_version);
        assert!(result.is_ok());
        
        let multi_accounts = result.unwrap();
        let cbor: Vec<u8> = multi_accounts.clone().try_into().unwrap();
        assert!(!cbor.is_empty());
        
        // Verify device info
        assert_eq!(multi_accounts.get_device(), Some(device_type.to_string()));
        assert_eq!(multi_accounts.get_device_version(), Some(device_version.to_string()));
        
        // Verify keys count
        // ETH generates 2 keys (standard + ledger live), BTC generates 1 key. Total 3.
        assert_eq!(multi_accounts.get_keys().len(), 3);
    }
}
