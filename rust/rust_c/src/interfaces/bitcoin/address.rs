use app_bitcoin;
use third_party::secp256k1::ffi::types::c_char;

use crate::interfaces::structs::SimpleResponse;
use crate::interfaces::types::PtrString;
use crate::interfaces::utils::{convert_c_char, recover_c_char};

#[no_mangle]
pub extern "C" fn utxo_get_address(
    hd_path: PtrString,
    x_pub: PtrString,
) -> *mut SimpleResponse<c_char> {
    let x_pub = recover_c_char(x_pub);
    let hd_path = recover_c_char(hd_path);
    let address = app_bitcoin::get_address(hd_path, &x_pub);
    match address {
        Ok(result) => SimpleResponse::success(convert_c_char(result) as *mut c_char).simple_c_ptr(),
        Err(e) => SimpleResponse::from(e).simple_c_ptr(),
    }
}
