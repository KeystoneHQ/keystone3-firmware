use alloc::vec::Vec;
use crate::utils::constants::*;
use curve25519_dalek::scalar::Scalar;
use cryptoxide::digest::Digest;
use cryptoxide::hashing;
use cryptoxide::ripemd160::Ripemd160;
use cryptoxide::sha3::Keccak256;


pub(crate) fn sha256_digest(data: &[u8]) -> Vec<u8> {
  hashing::sha256(&data).to_vec()
}

fn ripemd160_digest(data: &[u8]) -> [u8; 20] {
  let mut hasher = Ripemd160::new();
  hasher.input(data);
  let mut output = [0u8; 20];
  hasher.result(&mut output);
  output
}

pub fn hash_to_scalar(data: &[u8]) -> Scalar {
  Scalar::from_bytes_mod_order(keccak256(data))
}

pub fn hash160(data: &[u8]) -> [u8; 20] {
  ripemd160_digest(&sha256_digest(data))
}

pub fn keccak256(data: &[u8]) -> [u8; PUBKEY_LEH] {
  let mut hasher = Keccak256::new();
  hasher.input(data);
  let mut result = [0u8; PUBKEY_LEH];
  hasher.result(&mut result);
  result
}