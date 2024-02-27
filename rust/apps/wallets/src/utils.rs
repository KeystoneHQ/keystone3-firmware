use alloc::collections::BTreeMap;
use alloc::string::{String, ToString};
use alloc::vec::Vec;
use third_party::bitcoin::bip32::{ChildNumber, DerivationPath};
use third_party::hex;
use third_party::ur_registry::crypto_hd_key::CryptoHDKey;
use third_party::ur_registry::crypto_key_path::{CryptoKeyPath, PathComponent};
use third_party::ur_registry::error::{URError, URResult};
use third_party::ur_registry::extend::crypto_multi_accounts::CryptoMultiAccounts;

pub const DEVICE_TYPE: &str = "keystone 3";

pub fn get_origin(
    master_fingerprint: &[u8; 4],
    depth: usize,
    path: DerivationPath,
) -> URResult<CryptoKeyPath> {
    let components = path
        .into_iter()
        .map(|child_number| {
            let index = match child_number {
                ChildNumber::Hardened { index } => index,
                ChildNumber::Normal { index } => index,
            };
            PathComponent::new(Some(index.clone()), child_number.is_hardened()).unwrap()
        })
        .collect::<Vec<PathComponent>>();
    Ok(CryptoKeyPath::new(
        components,
        Some(master_fingerprint.clone()),
        Some(depth as u32),
    ))
}

pub fn generate_crypto_multi_accounts_sync_ur(
    master_fingerprint: &[u8; 4],
    public_keys: BTreeMap<DerivationPath, String>,
    account_prefix: &str,
) -> URResult<CryptoMultiAccounts> {
    let mut keys: Vec<CryptoHDKey> = Vec::new();
    for (index, (path, pubkey)) in public_keys.into_iter().enumerate() {
        let depth = path.len();
        let pubkey_slice = hex::decode(pubkey)
            .map_err(|_e| URError::UrEncodeError("invalid public key".to_string()))?;

        if let Ok(origin) = get_origin(master_fingerprint, depth, path) {
            let hd_key: CryptoHDKey = CryptoHDKey::new_extended_key(
                Some(false),
                pubkey_slice,
                None,
                None,
                Some(origin),
                None,
                None,
                Some(format!("{}-{}", account_prefix, index)),
                None,
            );
            keys.push(hd_key);
        }
    }
    Ok(CryptoMultiAccounts::new(
        master_fingerprint.clone(),
        keys,
        Some(DEVICE_TYPE.to_string()),
        None,
        None,
    ))
}
