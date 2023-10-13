use crate::interfaces::structs::SimpleResponse;
use crate::interfaces::utils::convert_c_char;
use alloc::vec::Vec;
use app_aptos;
use cty::c_char;
use third_party::hex::FromHex;

use super::types::PtrString;
use super::utils::recover_c_char;

#[no_mangle]
pub extern "C" fn aptos_generate_address(pub_key: PtrString) -> *mut SimpleResponse<c_char> {
    let pub_key = recover_c_char(pub_key);
    let address = app_aptos::generate_address(&pub_key);
    match address {
        Ok(result) => SimpleResponse::success(convert_c_char(result) as *mut c_char).simple_c_ptr(),
        Err(e) => SimpleResponse::from(e).simple_c_ptr(),
    }
}

#[no_mangle]
pub extern "C" fn test_aptos_parse() -> *mut SimpleResponse<c_char> {
    let data = "8bbbb70ae8b90a8686b2a27f10e21e44f2fb64ffffcaa4bb0242e9f1ea698659010000000000000002000000000000000000000000000000000000000000000000000000000000000104636f696e087472616e73666572010700000000000000000000000000000000000000000000000000000000000000010a6170746f735f636f696e094170746f73436f696e000220834f4b75dcaacbd7c549a993cdd3140676e172d1fee0609bf6876c74aaa7116008400d0300000000009a0e0000000000006400000000000000b6b747630000000021";
    let buf_message = Vec::from_hex(data).unwrap();
    match app_aptos::parse(&buf_message) {
        Ok(result) => SimpleResponse::success(convert_c_char(result) as *mut c_char).simple_c_ptr(),
        Err(e) => SimpleResponse::from(e).simple_c_ptr(),
    }
}
