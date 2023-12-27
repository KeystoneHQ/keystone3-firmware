#![no_std]
#![feature(error_in_core)]

use alloc::vec::Vec;
use app_utils::impl_public_struct;
use third_party::bitcoin::bip32::DerivationPath;

#[allow(unused_imports)] // stupid compiler
#[macro_use]
extern crate alloc;
extern crate core;

#[cfg(test)]
#[macro_use]
extern crate std;

pub mod aptos;
pub mod blue_wallet;
mod common;
pub mod companion_app;
pub mod keplr;
pub mod metamask;
pub mod okx;
pub mod solana;
pub mod sui;
mod utils;
pub mod xrp_toolkit;
pub mod near;

//TODO: get these value from device
pub const DEVICE_TYPE: &str = "Keystone 3 Pro";
pub const DEVICE_VERSION: &str = "1.1.0";

//key maybe 78 bytes xpub (k1) or just 32 bytes public key(ed25519) or 64 bytes bip32-ed25519 xpub
//they are differed in path
//polkadot is not included in this scope
impl_public_struct!(ExtendedPublicKey {
        path: DerivationPath,
        key: Vec<u8>
    }
);
