#![cfg_attr(not(feature = "no-allocator"), no_std)]

#[cfg(not(feature = "no-allocator"))]
use kt_allocator::my_alloc::KTAllocator;

#[cfg(not(feature = "no-allocator"))]
#[cfg(not(test))]
#[global_allocator]
static KT_ALLOCATOR: KTAllocator = KTAllocator;

use bitcoin_rust_c;
use common_rust_c;
use test_cmd;
use wallet_rust_c;

#[cfg(feature = "multi-coins")]
use aptos_rust_c;
#[cfg(feature = "multi-coins")]
use arweave_rust_c;
#[cfg(feature = "multi-coins")]
use cardano_rust_c;
#[cfg(feature = "multi-coins")]
use cosmos_rust_c;
#[cfg(feature = "multi-coins")]
use ethereum_rust_c;
#[cfg(feature = "multi-coins")]
use near_rust_c;
#[cfg(feature = "multi-coins")]
use solana_rust_c;
#[cfg(feature = "multi-coins")]
use sui_rust_c;
#[cfg(feature = "multi-coins")]
use ton_rust_c;
#[cfg(feature = "multi-coins")]
use tron_rust_c;
#[cfg(feature = "multi-coins")]
use xrp_rust_c;
