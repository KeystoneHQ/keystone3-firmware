#![no_std]

extern crate alloc;

pub use btc_only_wallet_rust_c;
pub use btc_only_wallet_rust_c::*;
#[cfg(feature = "multi-coins")]
pub use multi_coins_wallet_rust_c;
