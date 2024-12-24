#![cfg_attr(feature = "use-allocator", no_std)]
#![cfg_attr(feature = "use-allocator", feature(alloc_error_handler))]
#[cfg(feature = "use-allocator")]
extern crate alloc;

mod bindings;
mod trng;

#[cfg(feature = "use-allocator")]
mod allocator;
#[cfg(feature = "use-allocator")]
mod my_alloc;

#[allow(unused)]
use common_rust_c;
#[allow(unused)]
use wallet_rust_c;

#[cfg(feature = "test_cmd")]
#[allow(unused)]
use test_cmd;

//chains
#[cfg(feature = "aptos")]
#[allow(unused)]
use aptos_rust_c;
#[cfg(feature = "arweave")]
#[allow(unused)]
use arweave_rust_c;
#[cfg(feature = "bitcoin")]
#[allow(unused)]
use bitcoin_rust_c;
#[cfg(feature = "cardano")]
#[allow(unused)]
use cardano_rust_c;
#[cfg(feature = "cosmos")]
#[allow(unused)]
use cosmos_rust_c;
#[cfg(feature = "ethereum")]
#[allow(unused)]
use ethereum_rust_c;
#[cfg(feature = "near")]
#[allow(unused)]
use near_rust_c;
#[cfg(feature = "solana")]
#[allow(unused)]
use solana_rust_c;
#[cfg(feature = "stellar")]
#[allow(unused)]
use stellar_rust_c;
#[cfg(feature = "sui")]
#[allow(unused)]
use sui_rust_c;
#[cfg(feature = "ton")]
#[allow(unused)]
use ton_rust_c;
#[cfg(feature = "tron")]
#[allow(unused)]
use tron_rust_c;
#[cfg(feature = "xrp")]
#[allow(unused)]
use xrp_rust_c;
#[cfg(feature = "zcash")]
#[allow(unused)]
use zcash_rust_c;

#[cfg(feature = "simulator")]
#[allow(unused)]
use simulator_rust_c;
