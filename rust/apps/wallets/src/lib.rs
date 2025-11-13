#![no_std]

#[allow(unused_imports)] // stupid compiler
#[macro_use]
extern crate alloc;
extern crate core;
#[cfg(test)]
#[macro_use]
extern crate std;
use alloc::vec::Vec;

use app_utils::impl_public_struct;
use bitcoin::bip32::DerivationPath;

pub mod backpack;
pub mod bitget;
pub mod blue_wallet;
mod common;
pub mod core_wallet;
pub mod keplr;
pub mod keystone_connect;
pub mod metamask;
pub mod okx;
pub mod thor_wallet;
pub mod tonkeeper;
mod utils;
pub mod xrp_toolkit;
pub mod zcash;
//TODO: get these value from device
pub const DEVICE_TYPE: &str = "Keystone 3 Pro";
pub const DESCRIPTION: &str = "keystone qrcode";

pub use utils::generate_crypto_multi_accounts_sync_ur;

//key maybe 78 bytes xpub (k1) or just 32 bytes public key(ed25519) or 64 bytes bip32-ed25519 xpub
//they are differed in path
//polkadot is not included in this scope
impl_public_struct!(ExtendedPublicKey {
        path: DerivationPath,
        key: Vec<u8>
    }
);
