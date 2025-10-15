use crate::common::structs::SimpleResponse;
use crate::common::structs::{TransactionCheckResult, TransactionParseResult};
use crate::common::types::{PtrBytes, PtrString, PtrT, PtrUR};
use crate::common::ur::{UREncodeResult, FRAGMENT_MAX_LENGTH_DEFAULT};
use crate::common::utils::{convert_c_char, recover_c_char};
use crate::extract_array;
use crate::extract_ptr_with_type;
use crate::sui::get_public_key;
use alloc::format;
use alloc::vec::Vec;
use alloc::{
    string::{String, ToString},
    vec,
};
use app_sui::errors::SuiError;
use app_sui::Intent;
use bitcoin::bip32::DerivationPath;
use core::str::FromStr;
use cty::c_char;
use structs::DisplayIotaIntentData;
use structs::DisplayIotaSignMessageHash;
use ur_registry::iota::iota_signature::IotaSignature;
use ur_registry::iota::{
    iota_sign_hash_request::IotaSignHashRequest, iota_sign_request::IotaSignRequest,
};
use ur_registry::traits::RegistryItem;

pub mod structs;

#[no_mangle]
pub unsafe extern "C" fn iota_get_address_from_pubkey(
    xpub: PtrString,
) -> *mut SimpleResponse<c_char> {
    let xpub = recover_c_char(xpub);
    match app_iota::address::get_address_from_pubkey(xpub) {
        Ok(result) => SimpleResponse::success(convert_c_char(result)).simple_c_ptr(),
        Err(e) => SimpleResponse::from(e).simple_c_ptr(),
    }
}

#[no_mangle]
pub unsafe extern "C" fn iota_parse_intent(
    ptr: PtrUR,
) -> PtrT<TransactionParseResult<DisplayIotaIntentData>> {
    let sign_request = extract_ptr_with_type!(ptr, IotaSignRequest);
    let sign_data = sign_request.get_intent_message();
    let address = format!(
        "0x{}",
        hex::encode(sign_request.get_addresses().unwrap_or(vec![])[0].clone())
    );
    let data = app_sui::parse_intent(&sign_data);
    match data {
        Ok(data) => match data {
            Intent::TransactionData(ref transaction_data) => {
                TransactionParseResult::success(DisplayIotaIntentData::from(data).c_ptr()).c_ptr()
            }
            Intent::PersonalMessage(ref personal_message) => TransactionParseResult::success(
                DisplayIotaIntentData::from(data)
                    .with_address(address)
                    .c_ptr(),
            )
            .c_ptr(),
            _ => TransactionParseResult::from(SuiError::InvalidData("Invalid intent".to_string()))
                .c_ptr(),
        },
        Err(e) => TransactionParseResult::from(e).c_ptr(),
    }
}

#[no_mangle]
pub unsafe extern "C" fn iota_parse_sign_message_hash(
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

#[no_mangle]
pub unsafe extern "C" fn iota_sign_hash(
    ptr: PtrUR,
    seed: PtrBytes,
    seed_len: u32,
) -> PtrT<UREncodeResult> {
    let seed = extract_array!(seed, u8, seed_len as usize);
    let sign_request = extract_ptr_with_type!(ptr, IotaSignHashRequest);
    let hash = sign_request.get_message_hash();
    let path = match sign_request.get_derivation_paths()[0].get_path() {
        Some(p) => p,
        None => {
            return UREncodeResult::from(SuiError::SignFailure(
                "invalid derivation path".to_string(),
            ))
            .c_ptr()
        }
    };
    let signature = match app_sui::sign_hash(seed, &path, &hex::decode(hash).unwrap()) {
        Ok(v) => v,
        Err(e) => return UREncodeResult::from(e).c_ptr(),
    };
    let pub_key = match get_public_key(seed, &path) {
        Ok(v) => v,
        Err(e) => return UREncodeResult::from(e).c_ptr(),
    };
    let sig = IotaSignature::new(
        sign_request.get_request_id(),
        signature.to_vec(),
        Some(pub_key),
    );
    let sig_data: Vec<u8> = match sig.try_into() {
        Ok(v) => v,
        Err(e) => return UREncodeResult::from(e).c_ptr(),
    };
    UREncodeResult::encode(
        sig_data,
        IotaSignature::get_registry_type().get_type(),
        FRAGMENT_MAX_LENGTH_DEFAULT,
    )
    .c_ptr()
}

#[no_mangle]
pub unsafe extern "C" fn iota_sign_intent(
    ptr: PtrUR,
    seed: PtrBytes,
    seed_len: u32,
) -> PtrT<UREncodeResult> {
    let seed = extract_array!(seed, u8, seed_len as usize);
    let sign_request = extract_ptr_with_type!(ptr, IotaSignRequest);
    let sign_data = sign_request.get_intent_message();
    let path = match sign_request.get_derivation_paths()[0].get_path() {
        Some(p) => p,
        None => {
            return UREncodeResult::from(SuiError::SignFailure(
                "invalid derivation path".to_string(),
            ))
            .c_ptr()
        }
    };
    let signature = match app_sui::sign_intent(seed, &path, &sign_data.to_vec()) {
        Ok(v) => v,
        Err(e) => return UREncodeResult::from(e).c_ptr(),
    };
    let pub_key = match get_public_key(seed, &path) {
        Ok(v) => v,
        Err(e) => return UREncodeResult::from(e).c_ptr(),
    };
    let sig = IotaSignature::new(
        sign_request.get_request_id(),
        signature.to_vec(),
        Some(pub_key),
    );
    let sig_data: Vec<u8> = match sig.try_into() {
        Ok(v) => v,
        Err(e) => return UREncodeResult::from(e).c_ptr(),
    };
    UREncodeResult::encode(
        sig_data,
        IotaSignature::get_registry_type().get_type(),
        FRAGMENT_MAX_LENGTH_DEFAULT,
    )
    .c_ptr()
}
