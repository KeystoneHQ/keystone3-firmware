use crate::errors::{CardanoError, R};
use alloc::{format, string::ToString, vec::Vec};
use cryptoxide::hashing::sha512;
use ed25519_bip32_core::{DerivationScheme, XPrv};
use keystore::algorithms::crypto::hmac_sha512;

#[derive(Debug, Clone)]
pub struct CardanoHDNode {
    pub xprv: XPrv,
    pub fingerprint: [u8; 4],
}

impl CardanoHDNode {
    pub fn new(xprv: XPrv) -> Self {
        let fingerprint = Self::calculate_fingerprint(&xprv);
        Self { xprv, fingerprint }
    }

    fn calculate_fingerprint(xprv: &XPrv) -> [u8; 4] {
        let pubkey = xprv.public().public_key();
        let mut fingerprint = [0u8; 4];
        fingerprint.copy_from_slice(&pubkey[..4]);
        fingerprint
    }
}

// https://github.com/satoshilabs/slips/blob/master/slip-0023.md
pub fn from_seed_slip23(seed: &[u8]) -> R<CardanoHDNode> {
    if seed.is_empty() {
        return Err(CardanoError::InvalidSeed("seed is empty".to_string()));
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
    k[0] = k[0] & 0xf8; // Clear the 3 least significant bits
    k[31] = (k[31] & 0x1f) | 0x40; // Set the 6th bit and clear the 3 most significant bits

    // Step 6: Construct the 96-byte extended private key
    let mut extended_key = [0u8; 96];

    // kL := k[0:32] (interpreted as 256-bit integer in little-endian)
    extended_key[0..32].copy_from_slice(&k[0..32]);

    // kR := k[32:64]
    extended_key[32..64].copy_from_slice(&k[32..64]);

    // c := IR (root chain code)
    extended_key[64..96].copy_from_slice(ir);

    // Create XPrv using normalize_bytes_force3rd
    let xprv = XPrv::normalize_bytes_force3rd(extended_key);

    let hd_node = CardanoHDNode::new(xprv);

    Ok(hd_node)
}

pub fn from_seed_slip23_path(seed: &[u8], path: &str) -> R<CardanoHDNode> {
    let root_node = from_seed_slip23(seed)?;

    let components = parse_derivation_path(path)?;
    let mut current_xprv = root_node.xprv;

    for component in components {
        current_xprv = current_xprv.derive(DerivationScheme::V2, component);
    }

    Ok(CardanoHDNode::new(current_xprv))
}

fn parse_derivation_path(path: &str) -> R<Vec<u32>> {
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
            CardanoError::DerivationError(format!("Invalid path component: {}", part))
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

        let pubkey = result.unwrap().xprv.public().public_key();
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
        assert_eq!(result.unwrap(), vec![2147485500, 2147485463, 2147483648, 0, 0]);
    }

    #[test]
    fn test_from_seed_slip23_path() {
        let seed = hex::decode("578d685d20b602683dc5171df411d3e2").unwrap();
        let path = "m/1852'/1815'/0'/0/0";
        let result = from_seed_slip23_path(&seed, path);
        assert!(result.is_ok());
        let pubkey = result.unwrap().xprv.public().public_key();
        assert_eq!(pubkey.len(), 32);
        assert_eq!(
            "4510fd55f00653b0dec9153bdc65feba664ccd543a66f5a1438c759a0bc41e1c",
            hex::encode(pubkey)
        );
    }
}
