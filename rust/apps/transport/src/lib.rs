#![no_std]
// #![feature(error_in_core)]

extern crate alloc;
extern crate core;

#[cfg(test)]
#[macro_use]
extern crate std;

mod protocol;
pub use protocol::external_parser::frame_parser;