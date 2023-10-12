use app_cardano;
use app_cardano::address::AddressType;
use cty::c_char;
use crate::interfaces::structs::SimpleResponse;
use crate::interfaces::types::PtrString;
use crate::interfaces::utils::{convert_c_char, recover_c_char};

#[no_mangle]
pub extern "C" fn ada_get_base_address(xpub: PtrString, index: u32, network_id: u8) -> *mut SimpleResponse<c_char> {
    let xpub = recover_c_char(xpub);
    match app_cardano::address::derive_address(xpub, 0, index, 0, AddressType::Base, network_id)
    {
        Ok(result) => SimpleResponse::success(convert_c_char(result)).simple_c_ptr(),
        Err(e) => SimpleResponse::from(e).simple_c_ptr()
    }
}

#[no_mangle]
pub extern "C" fn ada_get_enterprise_address(xpub: PtrString, index: u32, network_id: u8) -> *mut SimpleResponse<c_char> {
    let xpub = recover_c_char(xpub);
    match app_cardano::address::derive_address(xpub, 0, index, 0, AddressType::Enterprise, network_id)
    {
        Ok(result) => SimpleResponse::success(convert_c_char(result)).simple_c_ptr(),
        Err(e) => SimpleResponse::from(e).simple_c_ptr()
    }
}

#[no_mangle]
pub extern "C" fn ada_get_stake_address(xpub: PtrString, index: u32, network_id: u8) -> *mut SimpleResponse<c_char> {
    let xpub = recover_c_char(xpub);
    match app_cardano::address::derive_address(xpub, 0, 0, index, AddressType::Stake, network_id)
    {
        Ok(result) => SimpleResponse::success(convert_c_char(result)).simple_c_ptr(),
        Err(e) => SimpleResponse::from(e).simple_c_ptr()
    }
}