#![no_std]
#![feature(error_in_core)]
extern crate alloc;

use crate::utils::{calc_subaddress_m, hash_to_scalar, keccak256};
use alloc::format;
use alloc::string::ToString;
use alloc::vec::Vec;
use base58_monero::{decode, encode, Error};
use curve25519_dalek::edwards::{CompressedEdwardsY, EdwardsPoint};
use curve25519_dalek::scalar::Scalar;
use keystore::algorithms::ed25519::bip32_ed25519;
use third_party::cryptoxide::digest::Digest;
use third_party::cryptoxide::sha3::Keccak256;
use third_party::hex;

pub mod address;
pub mod key;
pub mod structs;
pub mod utils;
