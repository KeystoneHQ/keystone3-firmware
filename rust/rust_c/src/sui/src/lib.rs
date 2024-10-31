#![no_std]

extern crate alloc;

pub mod structs;

use crate::structs::DisplaySuiIntentMessage;
use alloc::format;
use alloc::string::{String, ToString};
use alloc::vec::Vec;
use app_sui::errors::SuiError;
use app_utils::normalize_path;
use common_rust_c::errors::RustCError;
use common_rust_c::extract_ptr_with_type;
use common_rust_c::structs::{SimpleResponse, TransactionCheckResult, TransactionParseResult};
use common_rust_c::types::{PtrBytes, PtrString, PtrT, PtrUR};
use common_rust_c::ur::{UREncodeResult, FRAGMENT_MAX_LENGTH_DEFAULT};
use common_rust_c::utils::{convert_c_char, recover_c_char};
use core::slice;
use cty::c_char;
use ur_registry::sui::sui_sign_request::SuiSignRequest;
use ur_registry::sui::sui_signature::SuiSignature;
use ur_registry::traits::RegistryItem;

fn get_public_key(seed: &[u8], path: &String) -> Result<Vec<u8>, SuiError> {
    let path = normalize_path(path);
    let public_key =
        match keystore::algorithms::ed25519::slip10_ed25519::get_public_key_by_seed(seed, &path) {
            Ok(pub_key) => pub_key,
            Err(e) => {
                return Err(SuiError::SignFailure(format!(
                    "derive public key failed {:?}",
                    e
                )))
            }
        };
    Ok(public_key.to_vec())
}

#[no_mangle]
pub extern "C" fn sui_check_request(
    ptr: PtrUR,
    master_fingerprint: PtrBytes,
    length: u32,
) -> PtrT<TransactionCheckResult> {
    if length != 4 {
        return TransactionCheckResult::from(RustCError::InvalidMasterFingerprint).c_ptr();
    }
    let mfp = unsafe { slice::from_raw_parts(master_fingerprint, 4) };
    let sign_request = extract_ptr_with_type!(ptr, SuiSignRequest);
    let ur_mfp = sign_request.get_derivation_paths()[0].get_source_fingerprint();

    if let Ok(mfp) = mfp.try_into() as Result<[u8; 4], _> {
        if let Some(ur_mfp) = ur_mfp {
            return if mfp == ur_mfp {
                TransactionCheckResult::new().c_ptr()
            } else {
                TransactionCheckResult::from(RustCError::MasterFingerprintMismatch).c_ptr()
            };
        }
        TransactionCheckResult::from(RustCError::MasterFingerprintMismatch).c_ptr()
    } else {
        TransactionCheckResult::from(RustCError::InvalidMasterFingerprint).c_ptr()
    }
}

#[no_mangle]
pub extern "C" fn sui_generate_address(pub_key: PtrString) -> *mut SimpleResponse<c_char> {
    let pub_key = recover_c_char(pub_key);
    let address = app_sui::generate_address(&pub_key);
    match address {
        Ok(result) => SimpleResponse::success(convert_c_char(result) as *mut c_char).simple_c_ptr(),
        Err(e) => SimpleResponse::from(e).simple_c_ptr(),
    }
}

#[no_mangle]
pub extern "C" fn sui_parse_intent(
    ptr: PtrUR,
) -> PtrT<TransactionParseResult<DisplaySuiIntentMessage>> {
    let sign_request = extract_ptr_with_type!(ptr, SuiSignRequest);
    let sign_data = sign_request.get_intent_message();
    match app_sui::parse_intent(&sign_data.to_vec()) {
        Ok(v) => TransactionParseResult::success(DisplaySuiIntentMessage::from(v).c_ptr()).c_ptr(),
        Err(e) => TransactionParseResult::from(e).c_ptr(),
    }
}

#[no_mangle]
pub extern "C" fn sui_sign_intent(
    ptr: PtrUR,
    seed: PtrBytes,
    seed_len: u32,
) -> PtrT<UREncodeResult> {
    let seed = unsafe { slice::from_raw_parts(seed, seed_len as usize) };
    let sign_request = extract_ptr_with_type!(ptr, SuiSignRequest);
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
    let sig = SuiSignature::new(
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
        SuiSignature::get_registry_type().get_type(),
        FRAGMENT_MAX_LENGTH_DEFAULT.clone(),
    )
    .c_ptr()
}
