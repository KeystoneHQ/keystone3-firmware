#![feature(vec_into_raw_parts)]
#![cfg_attr(feature = "use-allocator", no_std)]
#![cfg_attr(feature = "use-allocator", feature(alloc_error_handler))]

extern crate alloc;

mod bindings;
mod trng;

#[cfg(feature = "use-allocator")]
mod allocator;
#[cfg(feature = "use-allocator")]
mod my_alloc;

#[allow(unused)]
mod common;
#[allow(unused)]
mod wallet;

#[cfg(feature = "test_cmd")]
#[allow(unused)]
mod test_cmd;

//chains
#[cfg(feature = "aptos")]
#[allow(unused)]
mod aptos;
#[cfg(feature = "arweave")]
#[allow(unused)]
mod arweave;
#[cfg(feature = "bitcoin")]
#[allow(unused)]
mod bitcoin;
#[cfg(feature = "cardano")]
#[allow(unused)]
mod cardano;
#[cfg(feature = "cosmos")]
#[allow(unused)]
mod cosmos;
#[cfg(feature = "ethereum")]
#[allow(unused)]
mod ethereum;
#[cfg(feature = "monero")]
#[allow(unused)]
mod monero;
#[cfg(feature = "near")]
#[allow(unused)]
mod near;
#[cfg(feature = "solana")]
#[allow(unused)]
mod solana;
#[cfg(feature = "stellar")]
#[allow(unused)]
mod stellar;
#[cfg(feature = "sui")]
#[allow(unused)]
mod sui;
#[cfg(feature = "ton")]
#[allow(unused)]
mod ton;
#[cfg(feature = "tron")]
#[allow(unused)]
mod tron;
#[cfg(feature = "xrp")]
#[allow(unused)]
mod xrp;
#[cfg(feature = "zcash")]
#[allow(unused)]
mod zcash;

#[cfg(feature = "simulator")]
#[allow(unused)]
mod simulator;
