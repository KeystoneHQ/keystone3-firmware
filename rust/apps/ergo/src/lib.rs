#![no_std]
#![feature(error_in_core)]

#[allow(unused_imports)] // stupid compiler
#[macro_use]
extern crate alloc;
extern crate core;


use alloc::vec::Vec;

pub mod address;
pub mod errors;
mod macros;
pub mod mnemonic;
