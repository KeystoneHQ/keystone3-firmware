#![no_std]
#![feature(error_in_core)]

#[macro_use]
extern crate alloc;
extern crate core;

#[cfg(test)]
#[macro_use]
extern crate std;

use alloc::{string::String, vec::Vec};
use third_party::{hex, base58};
use crate::errors::Result;
use blake2::{Blake2bVar, digest::{VariableOutput, Update}};

pub mod errors;

pub fn generate_address(xpub: &str) -> Result<String> {
  let mut hasher = Blake2bVar::new(32).unwrap();
  let mut xpub_buf = base58::decode(xpub)?;
  let len = xpub_buf.len();
  let mut buf: Vec<u8> = xpub_buf.drain(len - 4 - 32..len - 4).collect();
  // insert flag, ed25519 is 0, secp256k1 is 1, secp256r1 is 2, multi sign is 3.
  buf.insert(0, 0);
  hasher.update(&buf);
  let mut addr = [0u8; 32];
  hasher.finalize_variable(&mut addr).unwrap();
  Ok(format!("0x{}", hex::encode(addr)))
}

#[cfg(test)]
mod tests {
  use super::*;
  
  #[test]
  fn test_generate_address() {
    let xpub = "xpub6FpeLDgZhZfkYXMwMZtxLqNDzWfNyPQKoLAQE9re4Qcv3zZmKWiwfkg8HEGstz1uNoKtYqCXJzWMuQhYw7EYKLzqty13z1SE4yrjYSuTcPd";
    let address = generate_address(xpub).unwrap();
    assert_eq!("0xf195b51c63745071891b1f53170cac2cab2a49da6ee1fe8eabe50989234c8119", address);
  }
}
