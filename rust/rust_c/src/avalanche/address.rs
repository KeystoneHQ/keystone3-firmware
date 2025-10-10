use alloc::format;
use app_avalanche;
use cty::c_char;

use crate::common::structs::{SimpleResponse, TransactionCheckResult, TransactionParseResult};
use crate::common::types::{PtrString, PtrT};
use crate::common::utils::{convert_c_char, recover_c_char};
use app_avalanche::{errors::AvaxError, network::Network};

#[no_mangle]
pub unsafe extern "C" fn avalanche_get_x_p_address(
    hd_path: PtrString,
    root_x_pub: PtrString,
    root_path: PtrString,
) -> *mut SimpleResponse<c_char> {
    let hd_path = recover_c_char(hd_path);
    let root_x_pub = recover_c_char(root_x_pub);
    let root_path = recover_c_char(root_path);
    let address =
        app_avalanche::get_address(Network::AvaxMainNet, &hd_path, &root_x_pub, &root_path);
    match address {
        Ok(result) => SimpleResponse::success(convert_c_char(result) as *mut c_char).simple_c_ptr(),
        Err(e) => SimpleResponse::from(e).simple_c_ptr(),
    }
}
