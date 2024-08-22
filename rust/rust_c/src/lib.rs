#![cfg_attr(feature = "use-allocator", no_std)]
#![cfg_attr(feature = "use-allocator", feature(alloc_error_handler))]
#[cfg(feature = "use-allocator")]
extern crate alloc;

#[cfg(feature = "use-allocator")]
mod allocator;
#[cfg(feature = "use-allocator")]
mod bindings;
#[cfg(feature = "use-allocator")]
mod my_alloc;

#[allow(unused)]
use bitcoin_rust_c;
#[allow(unused)]
use common_rust_c;
#[allow(unused)]
use test_cmd;
#[allow(unused)]
use wallet_rust_c;

#[cfg(feature = "multi-coins")]
#[allow(unused)]
use aptos_rust_c;
#[cfg(feature = "multi-coins")]
#[allow(unused)]
use arweave_rust_c;
#[cfg(feature = "multi-coins")]
#[allow(unused)]
use cardano_rust_c;
#[cfg(feature = "multi-coins")]
#[allow(unused)]
use cosmos_rust_c;
#[cfg(feature = "multi-coins")]
#[allow(unused)]
use ethereum_rust_c;
#[cfg(feature = "multi-coins")]
#[allow(unused)]
use near_rust_c;
#[cfg(feature = "multi-coins")]
#[allow(unused)]
use solana_rust_c;
#[cfg(feature = "multi-coins")]
#[allow(unused)]
use stellar_rust_c;
#[cfg(feature = "multi-coins")]
#[allow(unused)]
use sui_rust_c;
#[cfg(feature = "multi-coins")]
#[allow(unused)]
use ton_rust_c;
#[cfg(feature = "multi-coins")]
#[allow(unused)]
use tron_rust_c;
#[cfg(feature = "multi-coins")]
#[allow(unused)]
use xrp_rust_c;
#[cfg(feature = "multi-coins")]
#[allow(unused)]
use zcash_rust_c;


#[cfg(any(feature = "simulator", feature = "simulator_btc_only"))]
#[allow(unused)]
use simulator_rust_c;
