use alloc::string::{String, ToString};
use alloc::vec::Vec;
use core::str::FromStr;

use bitcoin::bip32::{ChildNumber, DerivationPath};

use crate::algorithms::crypto::hmac_sha512;
use crate::algorithms::utils::normalize_path;
use crate::errors::{KeystoreError, Result};

pub fn get_private_key_by_seed(seed: &[u8], path: &String) -> Result<[u8; 32]> {
    let i = get_master_key_by_seed(seed);
    let path = normalize_path(path);
    let derivation_path = DerivationPath::from_str(path.as_str())
        .map_err(|e| KeystoreError::InvalidDerivationPath(format!("{e}")))?;
    let children: Vec<ChildNumber> = derivation_path.into();
    let indexes: Vec<u32> = children.iter().fold(Ok(vec![]), |acc, cur| match acc {
        Ok(vec) => {
            if let ChildNumber::Hardened { index } = cur {
                let mut new_vec = vec;
                new_vec.push(index + 0x80000000);
                return Ok(new_vec);
            }
            Err(KeystoreError::InvalidDerivationPath(
                "non hardened derivation is not supported for slip10-ed25519".to_string(),
            ))
        }
        e => e,
    })?;

    let final_i = indexes.iter().fold(i, |acc, cur| {
        let il = &acc[0..32];
        let ir = &acc[32..];

        let mut data = vec![00u8];
        data.extend_from_slice(il);
        data.extend_from_slice(&cur.to_be_bytes());

        hmac_sha512(ir, &data)
    });

    let mut result = [0u8; 32];
    result.clone_from_slice(&final_i[..32]);
    Ok(result)
}

pub fn get_public_key_by_seed(seed: &[u8], path: &String) -> Result<[u8; 32]> {
    let secret_key = get_private_key_by_seed(seed, path)?;
    let (_, public_key) = cryptoxide::ed25519::keypair(&secret_key);
    Ok(public_key)
}

pub fn sign_message_by_seed(seed: &[u8], path: &String, message: &[u8]) -> Result<[u8; 64]> {
    let secret_key = get_private_key_by_seed(seed, path)?;
    let (keypair, _) = cryptoxide::ed25519::keypair(&secret_key);
    Ok(cryptoxide::ed25519::signature(message, &keypair))
}

pub fn get_master_key_by_seed(seed: &[u8]) -> [u8; 64] {
    hmac_sha512(b"ed25519 seed", seed)
}

#[cfg(test)]
mod tests {
    use alloc::string::ToString;
    use hex;

    use super::*;

    extern crate std;

    #[test]
    fn test() {
        {
            let path = "m".to_string();
            let seed = hex::decode("5eb00bbddcf069084889a8ab9155568165f5c453ccb85e70811aaed6f6da5fc19a5ac40b389cd370d086206dec8aa6c43daea6690f20ad3d8d48b2d2ce9e38e4").unwrap();
            let key = get_private_key_by_seed(&seed, &path).unwrap();
            assert_eq!(
                "560f9f3c94558b6551928bb781cf6092c6b8800b4fc544af2c9444ed126d51aa",
                hex::encode(key)
            );
        }
        {
            let path = "m/0'".to_string();
            let seed = hex::decode("5eb00bbddcf069084889a8ab9155568165f5c453ccb85e70811aaed6f6da5fc19a5ac40b389cd370d086206dec8aa6c43daea6690f20ad3d8d48b2d2ce9e38e4").unwrap();
            let key = get_private_key_by_seed(&seed, &path).unwrap();
            assert_eq!(
                "56d8f4c43bce7186c171808633ba5ce4712b51cedffaa426611c8d7362a82a0c",
                hex::encode(key)
            );
        }
        {
            let path = "m/0'/1'".to_string();
            let seed = hex::decode("5eb00bbddcf069084889a8ab9155568165f5c453ccb85e70811aaed6f6da5fc19a5ac40b389cd370d086206dec8aa6c43daea6690f20ad3d8d48b2d2ce9e38e4").unwrap();
            let key = get_private_key_by_seed(&seed, &path).unwrap();
            assert_eq!(
                "e8ad866785e4152c7e7533454cf69a5c30761002f18ac268d20860e5ecdba44a",
                hex::encode(key)
            );
            let path_missing_m = "0'/1'".to_string();
            let key_missing_m = get_private_key_by_seed(&seed, &path_missing_m).unwrap();
            assert_eq!(
                "e8ad866785e4152c7e7533454cf69a5c30761002f18ac268d20860e5ecdba44a",
                hex::encode(key_missing_m)
            );
        }
        {
            let path = "m/0'/1'/2'".to_string();
            let seed = hex::decode("5eb00bbddcf069084889a8ab9155568165f5c453ccb85e70811aaed6f6da5fc19a5ac40b389cd370d086206dec8aa6c43daea6690f20ad3d8d48b2d2ce9e38e4").unwrap();
            let key = get_private_key_by_seed(&seed, &path).unwrap();
            assert_eq!(
                "ff8028185a58098fb6b4fd86b31a48fb8db7b015e86d7c5a2434996220ac7505",
                hex::encode(key)
            );
        }
        {
            let wrong_path = "x/y'/z'/1'".to_string();
            let seed = hex::decode("5eb00bbddcf069084889a8ab9155568165f5c453ccb85e70811aaed6f6da5fc19a5ac40b389cd370d086206dec8aa6c43daea6690f20ad3d8d48b2d2ce9e38e4").unwrap();
            let result = get_private_key_by_seed(&seed, &wrong_path);
            assert!(result.is_err());
            assert!(matches!(result, Err(KeystoreError::InvalidDerivationPath(_))));

            let none_harden_path = "m/0'/1'/2".to_string();
            let seed = hex::decode("5eb00bbddcf069084889a8ab9155568165f5c453ccb85e70811aaed6f6da5fc19a5ac40b389cd370d086206dec8aa6c43daea6690f20ad3d8d48b2d2ce9e38e4").unwrap();
            let result = get_private_key_by_seed(&seed, &none_harden_path);
            assert!(result.is_err());
            assert!(matches!(result, Err(KeystoreError::InvalidDerivationPath(e)) if e == "non hardened derivation is not supported for slip10-ed25519"));
        };
    }

    #[test]
    fn test_keystone_public_key() {
        let path = "m/44'/1557192335'/0'/0'/0'".to_string();
        let seed = hex::decode("5eb00bbddcf069084889a8ab9155568165f5c453ccb85e70811aaed6f6da5fc19a5ac40b389cd370d086206dec8aa6c43daea6690f20ad3d8d48b2d2ce9e38e4").unwrap();
        let key = get_private_key_by_seed(&seed, &path).unwrap();
        assert_eq!(
            "a9986894ceaf91d40256157d153c43c64d541287aca102b918a04bea9e388e46",
            hex::encode(key)
        );
    }

    #[test]
    fn test_slip10_ed25519_public_key() {
        {
            let path = "m".to_string();
            let seed = hex::decode("5eb00bbddcf069084889a8ab9155568165f5c453ccb85e70811aaed6f6da5fc19a5ac40b389cd370d086206dec8aa6c43daea6690f20ad3d8d48b2d2ce9e38e4").unwrap();
            let key = get_public_key_by_seed(&seed, &path).unwrap();
            assert_eq!(
                "e96b1c6b8769fdb0b34fbecfdf85c33b053cecad9517e1ab88cba614335775c1",
                hex::encode(key)
            );
        }
    }

    #[test]
    fn test_get_master_key_by_seed() {
        // Test with standard seed
        {
            let seed = hex::decode("5eb00bbddcf069084889a8ab9155568165f5c453ccb85e70811aaed6f6da5fc19a5ac40b389cd370d086206dec8aa6c43daea6690f20ad3d8d48b2d2ce9e38e4").unwrap();
            let master_key = get_master_key_by_seed(&seed);
            assert_eq!(64, master_key.len());
            // First 32 bytes should match the master private key at path "m"
            assert_eq!(
                "560f9f3c94558b6551928bb781cf6092c6b8800b4fc544af2c9444ed126d51aa",
                hex::encode(&master_key[..32])
            );
        }

        // Test with different seed
        {
            let seed = hex::decode("000102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f").unwrap();
            let master_key = get_master_key_by_seed(&seed);
            assert_eq!(64, master_key.len());
            // Verify it produces different result
            assert_ne!(
                "560f9f3c94558b6551928bb781cf6092c6b8800b4fc544af2c9444ed126d51aa",
                hex::encode(&master_key[..32])
            );
        }

        // Test with minimal seed
        {
            let seed = vec![0u8; 16];
            let master_key = get_master_key_by_seed(&seed);
            assert_eq!(64, master_key.len());
        }

        // Test with empty seed (edge case)
        {
            let seed = vec![];
            let master_key = get_master_key_by_seed(&seed);
            assert_eq!(64, master_key.len());
        }
    }

    #[test]
    fn test_get_public_key_by_seed_multiple_paths() {
        let seed = hex::decode("5eb00bbddcf069084889a8ab9155568165f5c453ccb85e70811aaed6f6da5fc19a5ac40b389cd370d086206dec8aa6c43daea6690f20ad3d8d48b2d2ce9e38e4").unwrap();

        // Test m/0'
        {
            let path = "m/0'".to_string();
            let pubkey = get_public_key_by_seed(&seed, &path).unwrap();
            assert_eq!(32, pubkey.len());
            assert_eq!(
                "b26871edccf7db469c5812977df531ad2f6174dd435f381e6ed2a0556f896fa7",
                hex::encode(pubkey)
            );
        }

        // Test m/0'/1'
        {
            let path = "m/0'/1'".to_string();
            let pubkey = get_public_key_by_seed(&seed, &path).unwrap();
            assert_eq!(32, pubkey.len());
            assert_eq!(
                "73f99d07ffd7ddbbd61f3b12b8391aa441a8e26b79d8dba7ee3a7c7f9608415b",
                hex::encode(pubkey)
            );
        }

        // Test m/0'/1'/2'
        {
            let path = "m/0'/1'/2'".to_string();
            let pubkey = get_public_key_by_seed(&seed, &path).unwrap();
            assert_eq!(32, pubkey.len());
        }

        // Test deep derivation path
        {
            let path = "m/44'/501'/0'/0'/0'".to_string();
            let pubkey = get_public_key_by_seed(&seed, &path).unwrap();
            assert_eq!(32, pubkey.len());
        }

        // Test without 'm' prefix
        {
            let path = "0'/1'".to_string();
            let pubkey = get_public_key_by_seed(&seed, &path).unwrap();
            assert_eq!(32, pubkey.len());
        }
    }

    #[test]
    fn test_get_public_key_invalid_paths() {
        let seed = hex::decode("5eb00bbddcf069084889a8ab9155568165f5c453ccb85e70811aaed6f6da5fc19a5ac40b389cd370d086206dec8aa6c43daea6690f20ad3d8d48b2d2ce9e38e4").unwrap();

        // Non-hardened path should fail
        {
            let path = "m/0'/1".to_string();
            let result = get_public_key_by_seed(&seed, &path);
            assert!(result.is_err());
            assert!(matches!(result, Err(KeystoreError::InvalidDerivationPath(_))));
        }

        // Invalid path format
        {
            let path = "invalid/path".to_string();
            let result = get_public_key_by_seed(&seed, &path);
            assert!(result.is_err());
        }
    }

    #[test]
    fn test_sign_message_by_seed() {
        let seed = hex::decode("5eb00bbddcf069084889a8ab9155568165f5c453ccb85e70811aaed6f6da5fc19a5ac40b389cd370d086206dec8aa6c43daea6690f20ad3d8d48b2d2ce9e38e4").unwrap();
        
        // Test signing with master key
        {
            let path = "m".to_string();
            let message = b"Hello, SLIP-10!";
            let signature = sign_message_by_seed(&seed, &path, message).unwrap();
            
            assert_eq!(64, signature.len());
            
            // Verify signature is valid
            let pubkey = get_public_key_by_seed(&seed, &path).unwrap();
            let is_valid = cryptoxide::ed25519::verify(message, &pubkey, &signature);
            assert!(is_valid);
        }

        // Test signing with derived key
        {
            let path = "m/44'/501'/0'".to_string();
            let message = b"Test message for Solana";
            let signature = sign_message_by_seed(&seed, &path, message).unwrap();
            
            assert_eq!(64, signature.len());
            
            // Verify signature
            let pubkey = get_public_key_by_seed(&seed, &path).unwrap();
            let is_valid = cryptoxide::ed25519::verify(message, &pubkey, &signature);
            assert!(is_valid);
        }

        // Test with empty message
        {
            let path = "m/0'".to_string();
            let message = b"";
            let signature = sign_message_by_seed(&seed, &path, message).unwrap();
            
            assert_eq!(64, signature.len());
            
            let pubkey = get_public_key_by_seed(&seed, &path).unwrap();
            let is_valid = cryptoxide::ed25519::verify(message, &pubkey, &signature);
            assert!(is_valid);
        }

        // Test with long message
        {
            let path = "m/0'/1'".to_string();
            let message = vec![0x42u8; 1000];
            let signature = sign_message_by_seed(&seed, &path, &message).unwrap();
            
            assert_eq!(64, signature.len());
            
            let pubkey = get_public_key_by_seed(&seed, &path).unwrap();
            let is_valid = cryptoxide::ed25519::verify(&message, &pubkey, &signature);
            assert!(is_valid);
        }

        // Test that different messages produce different signatures
        {
            let path = "m/0'".to_string();
            let message1 = b"message1";
            let message2 = b"message2";
            
            let sig1 = sign_message_by_seed(&seed, &path, message1).unwrap();
            let sig2 = sign_message_by_seed(&seed, &path, message2).unwrap();
            
            assert_ne!(sig1, sig2);
        }

        // Test that different paths produce different signatures for same message
        {
            let message = b"same message";
            let sig1 = sign_message_by_seed(&seed, &"m/0'".to_string(), message).unwrap();
            let sig2 = sign_message_by_seed(&seed, &"m/1'".to_string(), message).unwrap();
            
            assert_ne!(sig1, sig2);
        }
    }

    #[test]
    fn test_sign_message_invalid_signature() {
        let seed = hex::decode("5eb00bbddcf069084889a8ab9155568165f5c453ccb85e70811aaed6f6da5fc19a5ac40b389cd370d086206dec8aa6c43daea6690f20ad3d8d48b2d2ce9e38e4").unwrap();
        
        // Sign a message
        let path = "m/0'".to_string();
        let message = b"Original message";
        let signature = sign_message_by_seed(&seed, &path, message).unwrap();
        let pubkey = get_public_key_by_seed(&seed, &path).unwrap();
        
        // Verify with tampered message should fail
        let tampered_message = b"Tampered message";
        let is_valid = cryptoxide::ed25519::verify(tampered_message, &pubkey, &signature);
        assert!(!is_valid);
        
        // Verify with wrong public key should fail
        let wrong_pubkey = get_public_key_by_seed(&seed, &"m/1'".to_string()).unwrap();
        let is_valid = cryptoxide::ed25519::verify(message, &wrong_pubkey, &signature);
        assert!(!is_valid);
    }

    #[test]
    fn test_sign_with_invalid_path() {
        let seed = hex::decode("5eb00bbddcf069084889a8ab9155568165f5c453ccb85e70811aaed6f6da5fc19a5ac40b389cd370d086206dec8aa6c43daea6690f20ad3d8d48b2d2ce9e38e4").unwrap();
        
        // Non-hardened path should fail
        {
            let path = "m/0".to_string();
            let message = b"test";
            let result = sign_message_by_seed(&seed, &path, message);
            assert!(result.is_err());
            assert!(matches!(result, Err(KeystoreError::InvalidDerivationPath(_))));
        }

        // Invalid path format
        {
            let path = "not/a/valid/path".to_string();
            let message = b"test";
            let result = sign_message_by_seed(&seed, &path, message);
            assert!(result.is_err());
        }
    }

    #[test]
    fn test_private_key_edge_cases() {
        let seed = hex::decode("5eb00bbddcf069084889a8ab9155568165f5c453ccb85e70811aaed6f6da5fc19a5ac40b389cd370d086206dec8aa6c43daea6690f20ad3d8d48b2d2ce9e38e4").unwrap();

        // Test maximum hardened index (2^31 - 1)
        {
            let path = "m/2147483647'".to_string();
            let key = get_private_key_by_seed(&seed, &path);
            assert!(key.is_ok());
            assert_eq!(32, key.unwrap().len());
        }

        // Test very deep derivation path
        {
            let path = "m/0'/1'/2'/3'/4'/5'/6'/7'/8'/9'".to_string();
            let key = get_private_key_by_seed(&seed, &path);
            assert!(key.is_ok());
            assert_eq!(32, key.unwrap().len());
        }

        // Test with different seed
        {
            let seed2 = hex::decode("ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff").unwrap();
            let path = "m/0'".to_string();
            let key1 = get_private_key_by_seed(&seed, &path).unwrap();
            let key2 = get_private_key_by_seed(&seed2, &path).unwrap();
            assert_ne!(key1, key2);
        }
    }

    #[test]
    fn test_deterministic_derivation() {
        let seed = hex::decode("5eb00bbddcf069084889a8ab9155568165f5c453ccb85e70811aaed6f6da5fc19a5ac40b389cd370d086206dec8aa6c43daea6690f20ad3d8d48b2d2ce9e38e4").unwrap();

        // Same path should always produce same key
        {
            let path = "m/44'/501'/0'".to_string();
            let key1 = get_private_key_by_seed(&seed, &path).unwrap();
            let key2 = get_private_key_by_seed(&seed, &path).unwrap();
            assert_eq!(key1, key2);
        }

        // Same path should always produce same public key
        {
            let path = "m/44'/501'/0'".to_string();
            let pubkey1 = get_public_key_by_seed(&seed, &path).unwrap();
            let pubkey2 = get_public_key_by_seed(&seed, &path).unwrap();
            assert_eq!(pubkey1, pubkey2);
        }

        // Same message should always produce same signature (Ed25519 is deterministic)
        {
            let path = "m/0'".to_string();
            let message = b"deterministic test";
            let sig1 = sign_message_by_seed(&seed, &path, message).unwrap();
            let sig2 = sign_message_by_seed(&seed, &path, message).unwrap();
            assert_eq!(sig1, sig2);
        }
    }
}
