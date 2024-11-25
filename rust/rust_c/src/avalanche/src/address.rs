use alloc::format;
use app_avalanche;
use cty::c_char;

use app_avalanche::{errors::AvaxError, network::Network};
use common_rust_c::structs::SimpleResponse;
use common_rust_c::types::PtrString;
use common_rust_c::utils::{convert_c_char, recover_c_char};

#[no_mangle]
pub extern "C" fn avalanche_get_x_p_address(x_pub: PtrString) -> *mut SimpleResponse<c_char> {
    let x_pub = recover_c_char(x_pub);
    let address = app_avalanche::get_address(Network::AvaxMainNet, &x_pub);
    match address {
        Ok(result) => SimpleResponse::success(convert_c_char(result) as *mut c_char).simple_c_ptr(),
        Err(e) => SimpleResponse::from(e).simple_c_ptr(),
    }
}
