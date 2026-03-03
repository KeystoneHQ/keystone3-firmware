use crate::errors::Result;
use alloc::string::String;
use alloc::vec::Vec;
use bitcoin::base58;
use cryptoxide::digest::Digest;
use cryptoxide::sha3::Keccak256;

pub fn base58check_to_u8_slice(input: String) -> Result<Vec<u8>> {
    let result = base58::decode_check(input.as_str())?;
    Ok(result)
}

pub fn keccak256(input: &[u8]) -> [u8; 32] {
    let mut hasher = Keccak256::new();
    hasher.input(input);
    let mut output = [0u8; 32];
    hasher.result(&mut output);
    output
}

