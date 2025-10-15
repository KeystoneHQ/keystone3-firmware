#![no_std]
#![feature(error_in_core)]
extern crate alloc;

#[cfg(test)]
#[macro_use]
extern crate std;

pub mod address;
pub mod errors;
pub mod governance;
pub mod slip23;
pub mod structs;
pub mod transaction;
