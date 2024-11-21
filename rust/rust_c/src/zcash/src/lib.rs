#![no_std]
extern crate alloc;

pub mod structs;

use core::slice;

use alloc::boxed::Box;
use app_zcash::get_address;
use common_rust_c::{
    structs::{Response, SimpleResponse},
    types::{PtrBytes, PtrString},
    utils::{convert_c_char, recover_c_char},
};
use keystore::algorithms::zcash::{self, calculate_seed_fingerprint, derive_ufvk};
use cty::c_char;

#[no_mangle]
pub extern "C" fn derive_zcash_ufvk(seed: PtrBytes, seed_len: u32) -> *mut SimpleResponse<c_char> {
    let seed = unsafe { slice::from_raw_parts(seed, seed_len as usize) };
    let ufvk_text = derive_ufvk(seed);
    match ufvk_text {
        Ok(text) => SimpleResponse::success(convert_c_char(text)).simple_c_ptr(),
        Err(e) => SimpleResponse::from(e).simple_c_ptr(),
    }
}

#[no_mangle]
pub extern "C" fn calculate_zcash_seed_fingerprint(seed: PtrBytes, seed_len: u32) -> *mut SimpleResponse<u8> {
    let seed = unsafe { slice::from_raw_parts(seed, seed_len as usize) };
    let sfp = calculate_seed_fingerprint(seed);
    match sfp {
        Ok(bytes) => SimpleResponse::success(Box::into_raw(Box::new(bytes)) as *mut u8).simple_c_ptr(),
        Err(e) => SimpleResponse::from(e).simple_c_ptr(),
    }
}

#[no_mangle]
pub extern "C" fn generate_zcash_default_address(
    ufvk_text: PtrString,
) -> *mut SimpleResponse<c_char> {
    let ufvk_text = recover_c_char(ufvk_text);
    let address = get_address(&ufvk_text);
    match address {
        Ok(text) => SimpleResponse::success(convert_c_char(text)).simple_c_ptr(),
        Err(e) => SimpleResponse::from(e).simple_c_ptr(),
    }
}
