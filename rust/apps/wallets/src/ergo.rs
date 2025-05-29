use alloc::string::{String, ToString};
use alloc::vec::Vec;
use bitcoin::bip32::ChildNumber;

use crate::common::get_path_component;
use crate::ExtendedPublicKey;
use ur_registry::crypto_hd_key::CryptoHDKey;
use ur_registry::crypto_key_path::{CryptoKeyPath, PathComponent};
use ur_registry::error::{URError, URResult};
use ur_registry::extend::crypto_multi_accounts::CryptoMultiAccounts;

fn get_device_id(serial_number: &str) -> String {
    use cryptoxide::hashing::sha256;
    hex::encode(&sha256(&sha256(serial_number.as_bytes()))[0..20])
}

pub fn generate_general_multi_accounts_ur(
    master_fingerprint: [u8; 4],
    serial_number: &str,
    extended_public_keys: Vec<ExtendedPublicKey>,
    device_type: &str,
    device_version: &str,
) -> URResult<CryptoMultiAccounts> {
    let device_id = get_device_id(serial_number);
    let mut keys = vec![];
    for (index, ele) in extended_public_keys.iter().enumerate() {
        keys.push(generate_k1_normal_key(
            master_fingerprint,
            ele.clone(),
            None,
            &format!("ERG-{}", index),
        )?);
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
    key_name: &str,
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

    let children = CryptoKeyPath::new(
        vec![
            get_path_component(Some(0), false)?,
            get_path_component(None, false)?,
        ],
        None,
        Some(0),
    );
    Ok(CryptoHDKey::new_extended_key(
        Some(false),
        xpub.public_key.serialize().to_vec(),
        Some(xpub.chain_code.to_bytes().to_vec()),
        None,
        Some(key_path),
        Some(children),
        Some(xpub.parent_fingerprint.to_bytes()),
        Some(format!("Keystone-{}", key_name)),
        note,
    ))
}
