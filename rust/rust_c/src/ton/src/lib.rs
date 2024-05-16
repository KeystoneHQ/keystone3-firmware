#![no_std]
extern crate alloc;

pub mod structs;
use core::ptr::slice_from_raw_parts;

use alloc::{
    boxed::Box,
    slice,
    string::{String, ToString},
    vec::Vec,
};
use app_ton::mnemonic::ton_mnemonic_validate;
use common_rust_c::{
    ffi::VecFFI,
    structs::SimpleResponse,
    types::{Ptr, PtrBytes, PtrString},
    utils::recover_c_char,
};

#[no_mangle]
pub extern "C" fn ton_parse_transaction() {
    unimplemented!();
}

#[no_mangle]
pub extern "C" fn ton_check_transaction() {
    unimplemented!();
}

#[no_mangle]
pub extern "C" fn ton_sign_transaction() {
    unimplemented!();
}

#[no_mangle]
pub extern "C" fn ton_verify_mnemonic(mnemonic: PtrString) -> bool {
    let mnemonic = recover_c_char(mnemonic);
    let words: Vec<String> = mnemonic.split(" ").map(|v| v.to_lowercase()).collect();
    match ton_mnemonic_validate(&words, &None) {
        Ok(_) => true,
        Err(_) => false,
    }
}

#[no_mangle]
pub extern "C" fn ton_mnemonic_to_entropy(mnemonic: PtrString) -> Ptr<VecFFI<u8>> {
    let mnemonic = recover_c_char(mnemonic);
    let words: Vec<String> = mnemonic.split(" ").map(|v| v.to_lowercase()).collect();
    let entropy = app_ton::mnemonic::ton_mnemonic_to_entropy(&words, &None);
    VecFFI::from(entropy).c_ptr()
}

#[no_mangle]
pub extern "C" fn ton_entropy_to_seed(
    entropy: PtrBytes,
    entropy_len: u32,
) -> *mut SimpleResponse<u8> {
    let entropy = unsafe { slice::from_raw_parts(entropy, entropy_len as usize) }.to_vec();
    let seed = app_ton::mnemonic::ton_entropy_to_seed(entropy);
    SimpleResponse::success(Box::into_raw(Box::new(seed)) as *mut u8).simple_c_ptr()
}
