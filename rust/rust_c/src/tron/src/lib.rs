#![no_std]

extern crate alloc;

pub mod structs;

use crate::structs::DisplayTron;
use alloc::boxed::Box;
use alloc::slice;
use common_rust_c::errors::RustCError;
use common_rust_c::keystone;
use common_rust_c::structs::{SimpleResponse, TransactionCheckResult, TransactionParseResult};
use common_rust_c::types::{PtrBytes, PtrString, PtrT, PtrUR};
use common_rust_c::ur::{QRCodeType, UREncodeResult};
use common_rust_c::utils::{convert_c_char, recover_c_char};
use cty::c_char;

#[no_mangle]
pub extern "C" fn tron_check_keystone(
    ptr: PtrUR,
    ur_type: QRCodeType,
    master_fingerprint: PtrBytes,
    length: u32,
    x_pub: PtrString,
) -> PtrT<TransactionCheckResult> {
    keystone::check(ptr, ur_type, master_fingerprint, length, x_pub)
}

#[no_mangle]
pub extern "C" fn tron_parse_keystone(
    ptr: PtrUR,
    ur_type: QRCodeType,
    master_fingerprint: PtrBytes,
    length: u32,
    x_pub: PtrString,
) -> *mut TransactionParseResult<DisplayTron> {
    if length != 4 {
        return TransactionParseResult::from(RustCError::InvalidMasterFingerprint).c_ptr();
    }
    keystone::build_payload(ptr, ur_type).map_or_else(
        |e| TransactionParseResult::from(e).c_ptr(),
        |payload| {
            keystone::build_parse_context(master_fingerprint, x_pub).map_or_else(
                |e| TransactionParseResult::from(e).c_ptr(),
                |context| {
                    app_tron::parse_raw_tx(payload, context).map_or_else(
                        |e| TransactionParseResult::from(e).c_ptr(),
                        |res| {
                            TransactionParseResult::success(Box::into_raw(Box::new(
                                DisplayTron::from(res),
                            )))
                            .c_ptr()
                        },
                    )
                },
            )
        },
    )
}

#[no_mangle]
pub extern "C" fn tron_sign_keystone(
    ptr: PtrUR,
    ur_type: QRCodeType,
    master_fingerprint: PtrBytes,
    length: u32,
    x_pub: PtrString,
    cold_version: i32,
    seed: PtrBytes,
    seed_len: u32,
) -> *mut UREncodeResult {
    let seed = unsafe { slice::from_raw_parts(seed, seed_len as usize) };
    keystone::sign(
        ptr,
        ur_type,
        master_fingerprint,
        length,
        x_pub,
        cold_version,
        seed,
    )
}

#[no_mangle]
pub extern "C" fn tron_get_address(
    hd_path: PtrString,
    x_pub: PtrString,
) -> *mut SimpleResponse<c_char> {
    let x_pub = recover_c_char(x_pub);
    let hd_path = recover_c_char(hd_path);
    let address = app_tron::get_address(hd_path, &x_pub);
    match address {
        Ok(result) => SimpleResponse::success(convert_c_char(result) as *mut c_char).simple_c_ptr(),
        Err(e) => SimpleResponse::from(e).simple_c_ptr(),
    }
}
