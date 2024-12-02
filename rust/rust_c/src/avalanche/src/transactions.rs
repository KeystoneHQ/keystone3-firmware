use alloc::boxed::Box;
use alloc::slice;
use common_rust_c::errors::RustCError;
use common_rust_c::keystone;
use common_rust_c::keystone::{build_parse_context, build_payload};
use common_rust_c::structs::{TransactionCheckResult, TransactionParseResult};
use common_rust_c::types::{PtrBytes, PtrString, PtrT, PtrUR};
use common_rust_c::ur::{QRCodeType, UREncodeResult};

#[no_mangle]
pub extern "C" fn avax_parse(
    ptr: PtrUR,
    ur_type: QRCodeType,
    master_fingerprint: PtrBytes,
    length: u32,
    x_pub: PtrString,
// ) -> *mut TransactionParseResult<DisplayTx> {
) {
    if length != 4 {
        // return TransactionParseResult::from(RustCError::InvalidMasterFingerprint).c_ptr();
    }
    // build_payload(ptr, ur_type).map_or_else(
    //     |e| TransactionParseResult::from(e).c_ptr(),
    //     |payload| {
    //         build_parse_context(master_fingerprint, x_pub).map_or_else(
    //             |e| TransactionParseResult::from(e).c_ptr(),
    //             |context| {
    //                 app_bitcoin::parse_raw_tx(payload, context).map_or_else(
    //                     |e| TransactionParseResult::from(e).c_ptr(),
    //                     |res| {
    //                         TransactionParseResult::success(Box::into_raw(Box::new(
    //                             DisplayTx::from(res),
    //                         )))
    //                         .c_ptr()
    //                     },
    //                 )
    //             },
    //         )
    //     },
    // )
}

#[no_mangle]
pub extern "C" fn avax_sign(
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

// #[no_mangle]
// pub extern "C" fn avax_check(
//     ptr: PtrUR,
//     ur_type: QRCodeType,
//     master_fingerprint: PtrBytes,
//     length: u32,
//     x_pub: PtrString,
// ) -> PtrT<TransactionCheckResult> {
//     keystone::check(ptr, ur_type, master_fingerprint, length, x_pub)
// }
