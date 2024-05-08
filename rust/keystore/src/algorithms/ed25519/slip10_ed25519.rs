use alloc::string::String;
use alloc::vec::Vec;
use core::str::FromStr;

use third_party::bitcoin::bip32::{ChildNumber, DerivationPath};

use crate::algorithms::crypto::hmac_sha512;
use crate::algorithms::utils::normalize_path;
use crate::errors::{KeystoreError, Result};

pub fn get_private_key_by_seed(seed: &[u8], path: &String) -> Result<[u8; 32]> {
    let i = get_master_key_by_seed(seed);
    let path = normalize_path(path);
    let derivation_path = DerivationPath::from_str(path.as_str())
        .map_err(|e| KeystoreError::InvalidDerivationPath(format!("{}", e)))?;
    let children: Vec<ChildNumber> = derivation_path.into();
    let indexes: Vec<u32> = children.iter().fold(Ok(vec![]), |acc, cur| match acc {
        Ok(vec) => {
            if let ChildNumber::Hardened { index } = cur {
                let mut new_vec = vec;
                new_vec.push(index + 0x80000000);
                return Ok(new_vec);
            }
            Err(KeystoreError::InvalidDerivationPath(format!(
                "non hardened derivation is not supported for slip10-ed25519"
            )))
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
    let (_, public_key) = third_party::cryptoxide::ed25519::keypair(&secret_key);
    Ok(public_key)
}

pub fn sign_message_by_seed(seed: &[u8], path: &String, message: &[u8]) -> Result<[u8; 64]> {
    let secret_key = get_private_key_by_seed(seed, path)?;
    let (keypair, _) = third_party::cryptoxide::ed25519::keypair(&secret_key);
    Ok(third_party::cryptoxide::ed25519::signature(
        message, &keypair,
    ))
}

fn get_master_key_by_seed(seed: &[u8]) -> [u8; 64] {
    hmac_sha512(b"ed25519 seed", seed)
}

#[cfg(test)]
mod tests {
    use alloc::string::ToString;
    use third_party::hex;

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
}
