use crate::common::structs::SimpleResponse;
use crate::common::structs::{TransactionCheckResult, TransactionParseResult};
use crate::common::types::{PtrString, PtrT, PtrUR};
use crate::common::utils::{convert_c_char, recover_c_char};
use crate::extract_ptr_with_type;
use alloc::vec::Vec;
use alloc::{format, slice};
use alloc::{
    string::{String, ToString},
    vec,
};
use app_sui::Intent;
use bitcoin::bip32::DerivationPath;
use core::str::FromStr;
use cty::c_char;
use structs::DisplayIotaIntentData;
use structs::DisplayIotaSignMessageHash;
use ur_registry::iota::{iota_sign_request::IotaSignRequest, iota_sign_hash_request::IotaSignHashRequest};

pub mod structs;

#[no_mangle]
pub extern "C" fn iota_get_address_from_pubkey(xpub: PtrString) -> *mut SimpleResponse<c_char> {
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
    let sign_data = sign_request.get_intent_message();
    let data = app_sui::parse_intent(&sign_data);
    println!("...data..: {:?}", data);
    match data {
        Ok(data) => {
            TransactionParseResult::success(DisplayIotaIntentData::from(data).c_ptr()).c_ptr()
        }
        Err(e) => TransactionParseResult::from(e).c_ptr(),
    }
}

#[no_mangle]
pub extern "C" fn iota_parse_sign_message_hash(
    ptr: PtrUR,
) -> PtrT<TransactionParseResult<DisplayIotaSignMessageHash>> {
    let sign_hash_request = extract_ptr_with_type!(ptr, IotaSignHashRequest);
    let message = sign_hash_request.get_message_hash();
    let path = sign_hash_request.get_derivation_paths()[0].get_path();
    let network = "IOTA".to_string();
    let address = sign_hash_request.get_addresses().unwrap_or(vec![]);
    TransactionParseResult::success(
        DisplayIotaSignMessageHash::new(
            network,
            path.unwrap_or("No Path".to_string()),
            message,
            hex::encode(address[0].clone()),
        )
        .c_ptr(),
    )
    .c_ptr()
}

#[cfg(test)]
mod tests {
    use super::*;
}
