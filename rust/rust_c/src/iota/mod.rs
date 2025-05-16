use alloc::{format, slice};
use alloc::{
    string::{String, ToString},
    vec,
};
use alloc::vec::Vec;
use bitcoin::bip32::DerivationPath;
use core::str::FromStr;
use cty::c_char;
use crate::common::structs::SimpleResponse;
use crate::common::types::{PtrString, PtrUR, PtrT};
use crate::common::utils::{convert_c_char, recover_c_char};
use crate::extract_ptr_with_type;
use crate::common::structs::{TransactionParseResult, TransactionCheckResult};
use app_sui::Intent;
use ur_registry::iota::iota_sign_request::IotaSignRequest;
use structs::DisplayIotaIntentData;

pub mod structs;

#[no_mangle]
pub extern "C" fn iota_get_address_from_pubkey(
    xpub: PtrString,
) -> *mut SimpleResponse<c_char> {
    let xpub = recover_c_char(xpub);
    match app_iota::address::get_address_from_pubkey(xpub) {
        Ok(result) => SimpleResponse::success(convert_c_char(result)).simple_c_ptr(),
        Err(e) => SimpleResponse::from(e).simple_c_ptr(),
    }
}

#[no_mangle]
pub extern "C" fn iota_parse_intent(
    ptr: PtrUR,
) -> PtrT<TransactionParseResult<DisplayIotaIntentData>> {
    let sign_request = extract_ptr_with_type!(ptr, IotaSignRequest);
    println!("sign_request: {:?}", sign_request);
    let sign_data = sign_request.get_intent_message();
    let data = app_sui::parse_intent(&sign_data);
    println!("data: {:?}", data);
    TransactionParseResult::success(DisplayIotaIntentData::from(data.unwrap()).c_ptr()).c_ptr()
}

#[cfg(test)]
mod tests {
    use super::*;
}
