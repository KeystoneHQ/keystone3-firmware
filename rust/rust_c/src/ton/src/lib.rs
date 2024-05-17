#![no_std]
extern crate alloc;

pub mod structs;
use core::{ffi::c_char, ptr::slice_from_raw_parts};

use alloc::{
    boxed::Box,
    format, slice,
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
use rust_tools::convert_c_char;
use third_party::hex;

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

#[no_mangle]
pub extern "C" fn ton_mnemonic_to_seed(mnemonic: PtrString) -> *mut SimpleResponse<u8> {
    let mnemonic = recover_c_char(mnemonic);
    let words: Vec<String> = mnemonic.split(" ").map(|v| v.to_lowercase()).collect();
    let seed = app_ton::mnemonic::ton_mnemonic_to_master_seed(words, None);
    match seed {
        Ok(seed) => {
            SimpleResponse::success(Box::into_raw(Box::new(seed)) as *mut u8).simple_c_ptr()
        }
        Err(e) => SimpleResponse::from(e).simple_c_ptr(),
    }
}

#[no_mangle]
pub extern "C" fn ton_seed_to_publickey(
    seed: PtrBytes,
    seed_len: u32,
) -> *mut SimpleResponse<c_char> {
    let seed = unsafe { slice::from_raw_parts(seed, seed_len as usize) }.to_vec();
    match seed.try_into() {
        Ok(_seed) => {
            let public_key = app_ton::mnemonic::ton_master_seed_to_public_key(_seed);
            SimpleResponse::success(convert_c_char(hex::encode(public_key))).simple_c_ptr()
        }
        Err(_e) => SimpleResponse::from(common_rust_c::errors::RustCError::InvalidData(format!(
            "seed length should be 64"
        )))
        .simple_c_ptr(),
    }
}
