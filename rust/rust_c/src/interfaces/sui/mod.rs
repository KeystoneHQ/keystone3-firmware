use self::structs::DisplaySuiIntentMessage;
use crate::extract_ptr_with_type;
use crate::interfaces::errors::RustCError;
use crate::interfaces::structs::{SimpleResponse, TransactionCheckResult, TransactionParseResult};
use crate::interfaces::types::{PtrBytes, PtrString, PtrT, PtrUR};
use crate::interfaces::ur::{UREncodeResult, FRAGMENT_MAX_LENGTH_DEFAULT};
use crate::interfaces::utils::{convert_c_char, recover_c_char};
use alloc::string::ToString;
use app_sui::errors::SuiError;
use core::slice;
use cty::c_char;
use third_party::ur_registry::sui::sui_sign_request::SuiSignRequest;
use third_party::ur_registry::sui::sui_signature::SuiSignature;
use third_party::ur_registry::traits::RegistryItem;

pub mod structs;

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
pub extern "C" fn sui_generate_address(x_pub: PtrString) -> *mut SimpleResponse<c_char> {
    let x_pub = recover_c_char(x_pub);
    let address = app_sui::generate_address(&x_pub);
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
    UREncodeResult::encode(
        signature.to_vec(),
        SuiSignature::get_registry_type().get_type(),
        FRAGMENT_MAX_LENGTH_DEFAULT,
    )
    .c_ptr()
}
