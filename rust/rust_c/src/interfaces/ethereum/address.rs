use alloc::format;
use app_ethereum;
use cty::c_char;

use crate::interfaces::structs::SimpleResponse;
use crate::interfaces::types::PtrString;
use crate::interfaces::utils::{convert_c_char, recover_c_char};

use app_ethereum::errors::EthereumError;

#[no_mangle]
pub extern "C" fn eth_get_address(
    hd_path: PtrString,
    root_x_pub: PtrString,
    root_path: PtrString,
) -> *mut SimpleResponse<c_char> {
    let hd_path = recover_c_char(hd_path);
    let root_x_pub = recover_c_char(root_x_pub);
    let root_path = recover_c_char(root_path);
    if !hd_path.starts_with(root_path.as_str()) {
        return SimpleResponse::from(EthereumError::InvalidHDPath(format!(
            "{} does not match {}",
            hd_path, root_path
        )))
        .simple_c_ptr();
    }
    let address = app_ethereum::address::derive_address(&hd_path, &root_x_pub, &root_path);
    match address {
        Ok(result) => SimpleResponse::success(convert_c_char(result) as *mut c_char).simple_c_ptr(),
        Err(e) => SimpleResponse::from(e).simple_c_ptr(),
    }
}
