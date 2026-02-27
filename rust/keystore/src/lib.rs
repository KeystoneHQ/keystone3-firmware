#![no_std]
#![feature(error_in_core)]

pub mod algorithms;
mod bindings;
pub mod errors;
pub mod structs;

#[macro_use]
extern crate alloc;
