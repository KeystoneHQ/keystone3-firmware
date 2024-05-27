#![no_std]

extern crate alloc;
use cty::c_char;

use app_stellar::address::get_address;
use common_rust_c::structs::SimpleResponse;
use common_rust_c::types::PtrString;
use common_rust_c::utils::{convert_c_char, recover_c_char};

#[no_mangle]
pub extern "C" fn stellar_get_address(pubkey: PtrString) -> *mut SimpleResponse<c_char> {
    let x_pub = recover_c_char(pubkey);
    let address = get_address(&x_pub);
    match address {
        Ok(result) => SimpleResponse::success(convert_c_char(result) as *mut c_char).simple_c_ptr(),
        Err(e) => SimpleResponse::from(e).simple_c_ptr(),
    }
}
