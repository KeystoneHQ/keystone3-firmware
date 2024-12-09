#![no_std]
extern crate alloc;

pub mod structs;

use alloc::boxed::Box;
use app_zcash::get_address;
use common_rust_c::{
    check_and_free_ptr, extract_ptr_with_type,
    free::Free,
    make_free_method,
    structs::{SimpleResponse, TransactionCheckResult, TransactionParseResult},
    types::{Ptr, PtrBytes, PtrString, PtrT, PtrUR},
    ur::{UREncodeResult, FRAGMENT_MAX_LENGTH_DEFAULT},
    utils::{convert_c_char, recover_c_char},
};
use core::slice;
use cty::c_char;
use keystore::algorithms::zcash::{calculate_seed_fingerprint, derive_ufvk};
use structs::DisplayPczt;
use ur_registry::{traits::RegistryItem, zcash::zcash_pczt::ZcashPczt};

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
pub extern "C" fn calculate_zcash_seed_fingerprint(
    seed: PtrBytes,
    seed_len: u32,
) -> *mut SimpleResponse<u8> {
    let seed = unsafe { slice::from_raw_parts(seed, seed_len as usize) };
    let sfp = calculate_seed_fingerprint(seed);
    match sfp {
        Ok(bytes) => {
            SimpleResponse::success(Box::into_raw(Box::new(bytes)) as *mut u8).simple_c_ptr()
        }
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

#[no_mangle]
pub extern "C" fn check_zcash_tx(
    tx: PtrUR,
    ufvk: PtrString,
    seed_fingerprint: PtrBytes,
) -> *mut TransactionCheckResult {
    let pczt = extract_ptr_with_type!(tx, ZcashPczt);
    let ufvk_text = recover_c_char(ufvk);
    let seed_fingerprint = unsafe { slice::from_raw_parts(seed_fingerprint, 32) };
    let seed_fingerprint = seed_fingerprint.try_into().unwrap();
    match app_zcash::check_pczt(&pczt.get_data(), &ufvk_text, seed_fingerprint) {
        Ok(pczt) => TransactionCheckResult::new().c_ptr(),
        Err(e) => TransactionCheckResult::from(e).c_ptr(),
    }
}

#[no_mangle]
pub extern "C" fn parse_zcash_tx(
    tx: PtrUR,
    ufvk: PtrString,
    seed_fingerprint: PtrBytes,
) -> Ptr<TransactionParseResult<DisplayPczt>> {
    let pczt = extract_ptr_with_type!(tx, ZcashPczt);
    let ufvk_text = recover_c_char(ufvk);
    let seed_fingerprint = unsafe { slice::from_raw_parts(seed_fingerprint, 32) };
    let seed_fingerprint = seed_fingerprint.try_into().unwrap();
    match app_zcash::parse_pczt(&pczt.get_data(), &ufvk_text, seed_fingerprint) {
        Ok(pczt) => TransactionParseResult::success(DisplayPczt::from(&pczt).c_ptr()).c_ptr(),
        Err(e) => TransactionParseResult::from(e).c_ptr(),
    }
}

#[no_mangle]
pub extern "C" fn sign_zcash_tx(tx: PtrUR, seed: PtrBytes, seed_len: u32) -> *mut UREncodeResult {
    let pczt = extract_ptr_with_type!(tx, ZcashPczt);
    let seed = unsafe { slice::from_raw_parts(seed, seed_len as usize) };
    match app_zcash::sign_pczt(&pczt.get_data(), seed) {
        Ok(pczt) => match ZcashPczt::new(pczt).try_into() {
            Err(e) => UREncodeResult::from(e).c_ptr(),
            Ok(v) => UREncodeResult::encode(
                v,
                ZcashPczt::get_registry_type().get_type(),
                FRAGMENT_MAX_LENGTH_DEFAULT.clone(),
            )
            .c_ptr(),
        },
        Err(e) => UREncodeResult::from(e).c_ptr(),
    }
}

make_free_method!(TransactionParseResult<DisplayPczt>);
