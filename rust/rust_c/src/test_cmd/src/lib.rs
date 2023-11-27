#![no_std]
#![feature(vec_into_raw_parts)]
#![feature(error_in_core)]
#![allow(unused_unsafe)]
extern crate alloc;

use btc_test_cmd;

#[cfg(feature = "multi-coins")]
use general_test_cmd;
