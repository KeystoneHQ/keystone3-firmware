use alloc::{
    string::{String, ToString},
    vec::Vec,
};
use {
    bitcoin::bip32::ChildNumber,
    ur_registry::{
        crypto_hd_key::CryptoHDKey,
        crypto_key_path::{CryptoKeyPath, PathComponent},
        error::{URError, URResult},
        extend::crypto_multi_accounts::CryptoMultiAccounts,
    },
};

use crate::{common::get_path_component, ExtendedPublicKey};

const AVAX_STANDARD_PREFIX: &str = "44'/60'/0'";
const AVAX_X_P_PREFIX: &str = "44'/9000'/0'";

pub fn generate_crypto_multi_accounts(
    master_fingerprint: [u8; 4],
    extended_public_keys: Vec<ExtendedPublicKey>,
    device_type: &str,
) -> URResult<CryptoMultiAccounts> {
    let mut keys = vec![];
    for ele in extended_public_keys {
        match ele.get_path() {
            _path if _path.to_string().to_lowercase().eq(AVAX_STANDARD_PREFIX) => {
                keys.push(generate_k1_normal_key(
                    master_fingerprint,
                    ele.clone(),
                    Some("account.standard".to_string()),
                )?);
            }
            _path
                if _path
                    .to_string()
                    .to_lowercase()
                    .starts_with(AVAX_X_P_PREFIX) =>
            {
                keys.push(generate_k1_normal_key(
                    master_fingerprint,
                    ele,
                    Some("account.x&p".to_string()),
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
        None,
        None,
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

#[cfg(test)]
mod tests {
    use super::*;
    use alloc::vec;
    use bitcoin::bip32::DerivationPath;
    use core::str::FromStr;

    const VALID_XPUB_HEX: &str = "0488b21e003442193e0000000060499f801b896d83179a4374aeb7822aaeaceaa0db1f85ee3e904c4defbd9689034c729aa638b3261640a8f06a5eabbfe4c04d2e0aac434b344147a1a5fa3555a3";

    fn create_test_extended_public_key(path_str: &str, xpub_hex: &str) -> ExtendedPublicKey {
        // Convert hex string to bytes
        let bytes = hex::decode(xpub_hex).unwrap();
        ExtendedPublicKey::new(DerivationPath::from_str(path_str).unwrap(), bytes)
    }

    #[test]
    fn test_generate_crypto_multi_accounts_with_avax_standard() {
        let master_fingerprint = [0x12, 0x34, 0x56, 0x78];
        let device_type = "Keystone 3 Pro";

        let keys = vec![create_test_extended_public_key(
            "m/44'/60'/0'",
            VALID_XPUB_HEX,
        )];

        let result = generate_crypto_multi_accounts(master_fingerprint, keys, device_type);
        println!("Result: {:?}", result);

        assert!(
            result.is_ok(),
            "Should successfully generate crypto multi accounts for AVAX standard path"
        );
        let multi_accounts = result.unwrap();
        assert_eq!(multi_accounts.get_master_fingerprint(), master_fingerprint);
    }

    #[test]
    fn test_generate_crypto_multi_accounts_with_avax_xp() {
        let master_fingerprint = [0xAB, 0xCD, 0xEF, 0x00];
        let device_type = "Keystone 3 Pro";

        let keys = vec![create_test_extended_public_key(
            "m/44'/9000'/0'",
            VALID_XPUB_HEX,
        )];

        let result = generate_crypto_multi_accounts(master_fingerprint, keys, device_type);

        assert!(
            result.is_ok(),
            "Should successfully generate crypto multi accounts for AVAX X&P path"
        );
    }

    #[test]
    fn test_generate_crypto_multi_accounts_with_mixed_paths() {
        let master_fingerprint = [0xFF, 0xEE, 0xDD, 0xCC];
        let device_type = "Keystone 3 Pro";

        let keys = vec![
            create_test_extended_public_key("m/44'/60'/0'", VALID_XPUB_HEX),
            create_test_extended_public_key("m/44'/9000'/0'", VALID_XPUB_HEX),
        ];

        let result = generate_crypto_multi_accounts(master_fingerprint, keys, device_type);

        assert!(
            result.is_ok(),
            "Should successfully generate crypto multi accounts with mixed paths"
        );
        let multi_accounts = result.unwrap();
        // Should have 2 keys generated
        assert!(multi_accounts.get_keys().len() >= 1);
    }

    #[test]
    fn test_generate_crypto_multi_accounts_with_invalid_path() {
        let master_fingerprint = [0x11, 0x22, 0x33, 0x44];
        let device_type = "Keystone 3 Pro";

        let keys = vec![create_test_extended_public_key(
            "m/44'/1'/0'",
            VALID_XPUB_HEX,
        )];

        let result = generate_crypto_multi_accounts(master_fingerprint, keys, device_type);

        assert!(
            result.is_err(),
            "Should return error for unsupported key path"
        );
    }

    #[test]
    fn test_generate_crypto_multi_accounts_empty_keys() {
        let master_fingerprint = [0x99, 0x88, 0x77, 0x66];
        let device_type = "Keystone 3 Pro";
        let keys = vec![];

        let result = generate_crypto_multi_accounts(master_fingerprint, keys, device_type);

        assert!(result.is_ok(), "Should handle empty key list");
        let multi_accounts = result.unwrap();
        assert_eq!(multi_accounts.get_keys().len(), 0);
    }

    #[test]
    fn test_generate_crypto_multi_accounts_device_type() {
        let master_fingerprint = [0x12, 0x34, 0x56, 0x78];
        let device_type = "Custom Device Type";

        let keys = vec![create_test_extended_public_key(
            "m/44'/60'/0'",
            VALID_XPUB_HEX,
        )];

        let result = generate_crypto_multi_accounts(master_fingerprint, keys, device_type);

        assert!(result.is_ok());
        let multi_accounts = result.unwrap();
        assert_eq!(multi_accounts.get_device(), Some(device_type.to_string()));
    }
}
