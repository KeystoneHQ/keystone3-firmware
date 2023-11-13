use crate::interfaces::bitcoin::structs::DisplayTx;
use crate::interfaces::companion_app;
use crate::interfaces::companion_app::{build_parse_context, build_payload};
use crate::interfaces::errors::RustCError;
use crate::interfaces::structs::{TransactionCheckResult, TransactionParseResult};
use crate::interfaces::types::{PtrBytes, PtrString, PtrT, PtrUR};
use crate::interfaces::ur::{UREncodeResult, URType};
use alloc::boxed::Box;
use alloc::slice;

#[no_mangle]
pub extern "C" fn utxo_parse_companion_app(
    ptr: PtrUR,
    ur_type: URType,
    master_fingerprint: PtrBytes,
    length: u32,
    x_pub: PtrString,
) -> *mut TransactionParseResult<DisplayTx> {
    if length != 4 {
        return TransactionParseResult::from(RustCError::InvalidMasterFingerprint).c_ptr();
    }
    build_payload(ptr, ur_type).map_or_else(
        |e| TransactionParseResult::from(e).c_ptr(),
        |payload| {
            build_parse_context(master_fingerprint, x_pub).map_or_else(
                |e| TransactionParseResult::from(e).c_ptr(),
                |context| {
                    app_bitcoin::parse_raw_tx(payload, context).map_or_else(
                        |e| TransactionParseResult::from(e).c_ptr(),
                        |res| {
                            TransactionParseResult::success(Box::into_raw(Box::new(
                                DisplayTx::from(res),
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
pub extern "C" fn utxo_sign_companion_app(
    ptr: PtrUR,
    ur_type: URType,
    master_fingerprint: PtrBytes,
    length: u32,
    x_pub: PtrString,
    cold_version: i32,
    seed: PtrBytes,
    seed_len: u32,
) -> *mut UREncodeResult {
    let seed = unsafe { slice::from_raw_parts(seed, seed_len as usize) };
    companion_app::sign(
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
pub extern "C" fn utxo_check_companion_app(
    ptr: PtrUR,
    ur_type: URType,
    master_fingerprint: PtrBytes,
    length: u32,
    x_pub: PtrString,
) -> PtrT<TransactionCheckResult> {
    companion_app::check(ptr, ur_type, master_fingerprint, length, x_pub)
}
