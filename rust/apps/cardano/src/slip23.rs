use crate::errors::{CardanoError, Result};
use alloc::{format, string::ToString, vec::Vec};
use cryptoxide::hashing::sha512;
use ed25519_bip32_core::{DerivationScheme, XPrv};
use keystore::algorithms::crypto::hmac_sha512;

// https://github.com/satoshilabs/slips/blob/master/slip-0023.md
pub fn from_seed_slip23(seed: &[u8]) -> Result<XPrv> {
    if seed.is_empty() || seed.iter().all(|b| *b == 0x00) || seed.iter().all(|b| *b == 0xFF) {
        return Err(CardanoError::InvalidSeed("seed is invalid".to_string()));
    }

    // Step 2: Calculate I := HMAC-SHA512(Key = "ed25519 cardano seed", Data = S)
    let i = hmac_sha512(b"ed25519 cardano seed", seed);

    // Step 3: Split I into two 32-byte sequences: IL := I[0:32] and IR := I[32:64]
    let il = &i[0..32];
    let ir = &i[32..64];

    // Step 4: Let k := SHA-512(IL)
    let mut k = [0u8; 64];
    k.copy_from_slice(&sha512(il));

    // Step 5: Modify k by specific bit operations for EdDSA compatibility
    k[0] &= 0xf8; // Clear the 3 least significant bits
    k[31] = (k[31] & 0x1f) | 0x40; // Set the 6th bit and clear the 3 most significant bits

    // Step 6: Construct the 96-byte extended private key
    let mut extended_key = [0u8; 96];

    // kL := k[0:32] (interpreted as 256-bit integer in little-endian)
    extended_key[0..32].copy_from_slice(&k[0..32]);

    // kR := k[32:64]
    extended_key[32..64].copy_from_slice(&k[32..64]);

    // c := IR (root chain code)
    extended_key[64..96].copy_from_slice(ir);

    Ok(XPrv::normalize_bytes_force3rd(extended_key))
}

pub fn from_seed_slip23_path(seed: &[u8], path: &str) -> Result<XPrv> {
    let mut current_xprv = from_seed_slip23(seed)?;
    let components = parse_derivation_path(path)?;

    for component in components {
        current_xprv = current_xprv.derive(DerivationScheme::V2, component);
    }

    Ok(current_xprv)
}

fn parse_derivation_path(path: &str) -> Result<Vec<u32>> {
    let mut components = Vec::new();

    let path = path.strip_prefix("m/").unwrap_or(path);
    for part in path.split('/') {
        if part.is_empty() {
            continue;
        }

        let hardened = part.ends_with('\'');
        let index_str = if hardened {
            &part[..part.len() - 1]
        } else {
            part
        };

        let index: u32 = index_str.parse().map_err(|_| {
            CardanoError::DerivationError(format!("Invalid path component: {part}"))
        })?;

        if hardened {
            components.push(index + 0x80000000);
        } else {
            components.push(index);
        }
    }

    Ok(components)
}

#[cfg(test)]
mod tests {
    use super::*;
    use hex;

    #[test]
    fn test_from_seed_slip23() {
        let seed = hex::decode("578d685d20b602683dc5171df411d3e2").unwrap();
        let result = from_seed_slip23(&seed);
        assert!(result.is_ok());

        let pubkey = result.unwrap().public().public_key();
        assert_eq!(pubkey.len(), 32);
        assert_eq!(
            "83e3ecaf57f90f022c45e10d1b8cb78499c30819515ad9a81ad82139fdb12a90",
            hex::encode(pubkey)
        );
    }

    #[test]
    fn test_parse_derivation_path() {
        let path = "m/1852'/1815'/0'/0/0";
        let result = parse_derivation_path(path);
        assert!(result.is_ok());
        assert_eq!(
            result.unwrap(),
            vec![2147485500, 2147485463, 2147483648, 0, 0]
        );
    }

    #[test]
    fn test_from_seed_slip23_path() {
        let seed = hex::decode("578d685d20b602683dc5171df411d3e2").unwrap();
        let path = "m/1852'/1815'/0'/0/0";
        let result = from_seed_slip23_path(&seed, path);
        assert!(result.is_ok());
        let pubkey = result.unwrap().public().public_key();
        assert_eq!(pubkey.len(), 32);
        assert_eq!(
            "4510fd55f00653b0dec9153bdc65feba664ccd543a66f5a1438c759a0bc41e1c",
            hex::encode(pubkey)
        );
    }

    #[test]
    fn test_from_seed_slip23_empty_seed() {
        let seed = vec![];
        let result = from_seed_slip23(&seed);
        assert!(result.is_err());
        assert!(matches!(result.unwrap_err(), CardanoError::InvalidSeed(_)));
    }

    #[test]
    fn test_from_seed_slip23_different_seeds() {
        let seed1 = hex::decode("578d685d20b602683dc5171df411d3e2").unwrap();
        let seed2 = hex::decode("00000000000000000000000000000000").unwrap();

        let result1 = from_seed_slip23(&seed1).unwrap();
        let result2 = from_seed_slip23(&seed2).unwrap();

        assert_ne!(result1.public().public_key(), result2.public().public_key());
    }

    #[test]
    fn test_parse_derivation_path_without_prefix() {
        let path = "1852'/1815'/0'/0/0";
        let result = parse_derivation_path(path);
        assert!(result.is_ok());
        assert_eq!(
            result.unwrap(),
            vec![2147485500, 2147485463, 2147483648, 0, 0]
        );
    }

    #[test]
    fn test_parse_derivation_path_mixed_hardened() {
        let path = "m/1852'/1815/0'/0/0";
        let result = parse_derivation_path(path);
        assert!(result.is_ok());
        let components = result.unwrap();
        assert_eq!(components[0], 2147485500); // hardened
        assert_eq!(components[1], 1815); // not hardened
        assert_eq!(components[2], 2147483648); // hardened
    }

    #[test]
    fn test_parse_derivation_path_invalid_component() {
        let path = "m/1852'/invalid/0'";
        let result = parse_derivation_path(path);
        assert!(result.is_err());
    }

    #[test]
    fn test_parse_derivation_path_empty() {
        let path = "";
        let result = parse_derivation_path(path);
        assert!(result.is_ok());
        assert_eq!(result.unwrap(), Vec::<u32>::new());
    }

    #[test]
    fn test_from_seed_slip23_path_different_paths() {
        let seed = hex::decode("578d685d20b602683dc5171df411d3e2").unwrap();
        let path1 = "m/1852'/1815'/0'/0/0";
        let path2 = "m/1852'/1815'/0'/0/1";

        let result1 = from_seed_slip23_path(&seed, path1).unwrap();
        let result2 = from_seed_slip23_path(&seed, path2).unwrap();

        assert_ne!(result1.public().public_key(), result2.public().public_key());
    }
}
