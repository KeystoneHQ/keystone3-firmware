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
