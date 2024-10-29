use alloc::collections::BTreeMap;
use alloc::string::{String, ToString};
use alloc::vec::Vec;
use bitcoin::bip32::{ChildNumber, DerivationPath};
use hex;
use ur_registry::crypto_hd_key::CryptoHDKey;
use ur_registry::crypto_key_path::{CryptoKeyPath, PathComponent};
use ur_registry::error::{URError, URResult};
use ur_registry::extend::crypto_multi_accounts::CryptoMultiAccounts;

pub const DEVICE_TYPE: &str = "keystone 3";

fn get_origin(
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

#[cfg(test)]
mod tests {
    use super::*;
    use core::str::FromStr;

    #[test]
    fn test_generate_crypto_multi_accounts_sync_ur() {
        let master_fingerprint = [0u8; 4];
        let public_keys = BTreeMap::new();
        let result =
            generate_crypto_multi_accounts_sync_ur(&master_fingerprint, public_keys, "SUI");
        assert!(result.is_ok());
        assert_eq!(result.unwrap().get_master_fingerprint(), master_fingerprint);
    }

    #[test]
    fn test_get_origin() {
        let master_fingerprint = [0u8; 4];
        let path = DerivationPath::from_str("m/48'/0'/0'/2'").unwrap();
        let result = get_origin(&master_fingerprint, 2, path);
        assert!(result.is_ok());
        let origin = result.unwrap();
        assert_eq!(origin.get_depth().unwrap(), 2 as u32);
        assert_eq!(origin.get_components().len(), 4);
        assert_eq!(origin.get_source_fingerprint().unwrap(), master_fingerprint);
    }
}
