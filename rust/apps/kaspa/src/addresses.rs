use crate::errors::{KaspaError, Result};
use alloc::string::{String, ToString};
use alloc::vec::Vec;

pub const KASPA_PREFIX: &str = "kaspa";

pub const KASPA_COIN_TYPE: u32 = 111111;

/// Get Kaspa address for a given derivation path
pub fn get_address(xpub: &str, path: &str) -> Result<String> {
    use bitcoin::bip32::{DerivationPath, Xpub};
    use core::str::FromStr;
    use bitcoin::secp256k1::Secp256k1;

    let xpub = Xpub::from_str(xpub)
        .map_err(|e| KaspaError::DerivePublicKeyError(e.to_string()))?;

    let path = DerivationPath::from_str(path)
        .map_err(|e| KaspaError::DerivePublicKeyError(e.to_string()))?;

    let secp = Secp256k1::verification_only();
    let derived_xpub = xpub.derive_pub(&secp, &path)
        .map_err(|e| KaspaError::DerivePublicKeyError(e.to_string()))?;

    pubkey_to_address(&derived_xpub.public_key)
    
}

/// Convert secp256k1 public key to Kaspa address
/// Kaspa uses Blake2b hashing and CashAddr encoding (similar to Bitcoin Cash)
fn pubkey_to_address(public_key: &bitcoin::secp256k1::PublicKey) -> Result<String> {
    let (x_only_pubkey, _parity) = public_key.x_only_public_key();
    let pubkey_bytes = x_only_pubkey.serialize();
    encode_kaspa_address(&pubkey_bytes)
}

pub fn encode_kaspa_address(hash: &[u8]) -> Result<String> {
    let version_byte: u8 = 0u8; 
    
    let mut payload = Vec::with_capacity(1 + hash.len());
    payload.push(version_byte);
    payload.extend_from_slice(hash);
    
    let base32_data = convert_bits(&payload, 8, 5, true)?;
    
    let checksum = polymod_cashaddr(KASPA_PREFIX, &base32_data);
    
    let mut full_data = base32_data;
    for i in 0..8 {
        full_data.push(((checksum >> (5 * (7 - i))) & 0x1f) as u8);
    }
    
    let charset = b"qpzry9x8gf2tvdw0s3jn54khce6mua7l";
    let encoded: String = full_data
        .iter()
        .map(|&b| charset[b as usize] as char)
        .collect();
    
    Ok(alloc::format!("{}:{}", KASPA_PREFIX, encoded))
}

fn polymod_cashaddr(prefix: &str, data: &[u8]) -> u64 {
    let mut c: u64 = 1;
    
    // 步骤 A: 前缀处理
    for ch in prefix.bytes() {
        // 每个字符映射为低 5 位
        c = polymod_step(c) ^ ((ch as u64) & 0x1f);
    }
    
    // 步骤 B: 关键分隔符位
    c = polymod_step(c);
    
    // 步骤 C: 数据处理
    for &value in data {
        c = polymod_step(c) ^ (value as u64);
    }
    
    // 步骤 D: 校验和位预留
    for _ in 0..8 {
        c = polymod_step(c);
    }
    
    c ^ 1
}

/// Convert between bit groups (used for CashAddr encoding)
fn convert_bits(data: &[u8], from_bits: u32, to_bits: u32, pad: bool) -> Result<Vec<u8>> {
    let mut acc: u32 = 0;
    let mut bits: u32 = 0;
    let mut result = Vec::new();
    let maxv: u32 = (1 << to_bits) - 1;
    
    for &value in data {
        let v = value as u32;
        if v >> from_bits != 0 {
            return Err(KaspaError::InvalidAddress("Invalid data for convert_bits".to_string()));
        }
        acc = (acc << from_bits) | v;
        bits += from_bits;
        while bits >= to_bits {
            bits -= to_bits;
            result.push(((acc >> bits) & maxv) as u8);
        }
    }
    
    if pad {
        if bits > 0 {
            result.push(((acc << (to_bits - bits)) & maxv) as u8);
        }
    } else if bits >= from_bits || ((acc << (to_bits - bits)) & maxv) != 0 {
        return Err(KaspaError::InvalidAddress("Invalid padding in convert_bits".to_string()));
    }
    
    Ok(result)
}

/// Single step of BCH polymod calculation
fn polymod_step(pre: u64) -> u64 {
    let b = pre >> 35;
    ((pre & 0x07ffffffff) << 5)
        ^ (if b & 0x01 != 0 { 0x98f2bc8e61 } else { 0 })
        ^ (if b & 0x02 != 0 { 0x79b76d99e2 } else { 0 })
        ^ (if b & 0x04 != 0 { 0xf33e5fb3c4 } else { 0 })
        ^ (if b & 0x08 != 0 { 0xae2eabe2a8 } else { 0 })
        ^ (if b & 0x10 != 0 { 0x1e4f43e470 } else { 0 })
}

pub fn get_kaspa_derivation_path(account: u32, change: u32, index: u32) -> String {
    alloc::format!("m/44'/{}'/{}'/{}/{}", KASPA_COIN_TYPE, account, change, index)
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_kaspa_derivation_path() {
        let path = get_kaspa_derivation_path(0, 0, 0);
        assert_eq!(path, "m/44'/111111'/0'/0/0");

        let path = get_kaspa_derivation_path(0, 0, 5);
        assert_eq!(path, "m/44'/111111'/0'/0/5");
    }

    #[test]
    fn test_kaspa_address_encoding_logic() {
        let zero_pubkey_hex = "0000000000000000000000000000000000000000000000000000000000000000";
        let pubkey_bytes = hex::decode(zero_pubkey_hex).unwrap();
        
        use blake2b_simd::Params;
        let hash = Params::new()
            .hash_length(32)
            .to_state()
            .update(&pubkey_bytes)
            .finalize();
            
        let address = encode_kaspa_address(hash.as_bytes()).unwrap();
        
        assert!(address.starts_with("kaspa:q"));
        assert_eq!(address.len(), 69);
        println!("Generated address for zeros: {}", address);
    }
}
