#![no_std]
#![feature(error_in_core)]
#![allow(unused_unsafe)]
extern crate alloc;
mod errors;
mod signature;

use common_rust_c::utils::recover_c_char;
use core::slice;
use cty::c_char;
use signature::verify_signature;

#[no_mangle]
pub extern "C" fn verify_frimware_signature(
    signature_ptr: *mut c_char,
    message_hash_ptr: *mut u8,
    pubkey_ptr: *mut u8,
) -> bool {
    let signature = recover_c_char(signature_ptr);
    let message_hash = unsafe { slice::from_raw_parts(message_hash_ptr, 32) };
    let publick_key = unsafe { slice::from_raw_parts(pubkey_ptr, 65) };
    match hex::decode(signature) {
        Ok(data) => verify_signature(&data, message_hash, publick_key).unwrap_or(false),
        Err(_) => false,
    }
}
