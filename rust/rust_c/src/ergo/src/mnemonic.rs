use alloc::string::ToString;
use common_rust_c::structs::SimpleResponse;
use common_rust_c::types::{PtrBytes, PtrString};
use common_rust_c::utils::{convert_c_char, recover_c_char};

use core::slice;
use cty::c_char;

#[no_mangle]
pub extern "C" fn ergo_extended_pubkey_by_seed(
    seed: PtrBytes,
    seed_len: u32,
    path: PtrString,
) -> *mut SimpleResponse<c_char> {
    let path = recover_c_char(path);
    let seed = unsafe { slice::from_raw_parts(seed, seed_len as usize) };

    let extended_key = app_ergo::mnemonic::ergo_master_seed_to_extended_pubkey(seed, path);
    match extended_key {
        Ok(result) => SimpleResponse::success(convert_c_char(result.to_string())).simple_c_ptr(),
        Err(e) => SimpleResponse::from(e).simple_c_ptr(),
    }
}
