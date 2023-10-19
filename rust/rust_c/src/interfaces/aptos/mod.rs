use crate::extract_ptr_with_type;
use crate::interfaces::structs::SimpleResponse;
use crate::interfaces::ur::{UREncodeResult, FRAGMENT_MAX_LENGTH_DEFAULT};
use crate::interfaces::utils::convert_c_char;
use alloc::format;
use alloc::slice;
use alloc::string::ToString;
use alloc::vec::Vec;
use app_aptos;
use app_aptos::errors::AptosError;
use app_aptos::parser::is_tx;
use cty::c_char;
use third_party::hex;
use third_party::hex::FromHex;
use third_party::ur_registry::aptos::aptos_sign_request::{AptosSignRequest, SignType};
use third_party::ur_registry::aptos::aptos_signature::AptosSignature;
use third_party::ur_registry::traits::RegistryItem;

use self::structs::DisplayAptosTx;

use super::errors::RustCError;
use super::structs::{TransactionCheckResult, TransactionParseResult};
use super::types::{PtrBytes, PtrString, PtrT, PtrUR};
use super::utils::recover_c_char;

pub mod structs;

fn build_sign_result(
    ptr: PtrUR,
    seed: &[u8],
    pub_key: PtrString,
) -> app_aptos::errors::Result<AptosSignature> {
    let sign_request = extract_ptr_with_type!(ptr, AptosSignRequest);
    let pub_key = recover_c_char(pub_key);
    let mut path = sign_request.get_authentication_key_derivation_paths()[0]
        .get_path()
        .ok_or(AptosError::InvalidData(
            "invalid derivation path".to_string(),
        ))?;
    if !path.starts_with("m/") {
        path = format!("m/{}", path);
    }
    let signature = app_aptos::sign(sign_request.get_sign_data().to_vec(), &path, seed)?;
    let buf: Vec<u8> = hex::decode(pub_key)?;
    Ok(AptosSignature::new(
        sign_request.get_request_id(),
        signature.to_vec(),
        buf,
    ))
}

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
pub extern "C" fn aptos_check_request(
    ptr: PtrUR,
    master_fingerprint: PtrBytes,
    length: u32,
) -> PtrT<TransactionCheckResult> {
    if length != 4 {
        return TransactionCheckResult::from(RustCError::InvalidMasterFingerprint).c_ptr();
    }
    let mfp = unsafe { slice::from_raw_parts(master_fingerprint, 4) };
    let sign_request = extract_ptr_with_type!(ptr, AptosSignRequest);
    let ur_mfp = sign_request.get_authentication_key_derivation_paths()[0].get_source_fingerprint();

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
pub extern "C" fn aptos_parse(ptr: PtrUR) -> PtrT<TransactionParseResult<DisplayAptosTx>> {
    let sign_request = extract_ptr_with_type!(ptr, AptosSignRequest);
    let sign_data = sign_request.get_sign_data();
    let sign_type = match sign_request.get_sign_type() {
        SignType::Single => {
            if is_tx(&sign_data) {
                SignType::Single
            } else {
                SignType::Message
            }
        }
        SignType::Multi => SignType::Multi,
        SignType::Message => SignType::Message,
    };
    match sign_type {
        SignType::Single => match app_aptos::parse_tx(&sign_data.to_vec()) {
            Ok(v) => TransactionParseResult::success(DisplayAptosTx::from(v).c_ptr()).c_ptr(),
            Err(e) => TransactionParseResult::from(e).c_ptr(),
        },
        SignType::Multi => {
            TransactionParseResult::from(AptosError::ParseTxError("not support".to_string()))
                .c_ptr()
        }
        SignType::Message => match app_aptos::parse_msg(&sign_data.to_vec()) {
            Ok(v) => TransactionParseResult::success(DisplayAptosTx::from(v).c_ptr()).c_ptr(),
            Err(e) => TransactionParseResult::from(e).c_ptr(),
        },
    }
}

#[no_mangle]
pub extern "C" fn aptos_sign_tx(
    ptr: PtrUR,
    seed: PtrBytes,
    seed_len: u32,
    pub_key: PtrString,
) -> PtrT<UREncodeResult> {
    let seed = unsafe { alloc::slice::from_raw_parts(seed, seed_len as usize) };
    build_sign_result(ptr, seed, pub_key)
        .map(|v| v.try_into())
        .map_or_else(
            |e| UREncodeResult::from(e).c_ptr(),
            |v| {
                v.map_or_else(
                    |e| UREncodeResult::from(e).c_ptr(),
                    |data| {
                        UREncodeResult::encode(
                            data,
                            AptosSignature::get_registry_type().get_type(),
                            FRAGMENT_MAX_LENGTH_DEFAULT,
                        )
                        .c_ptr()
                    },
                )
            },
        )
}

#[no_mangle]
pub extern "C" fn aptos_get_path(ptr: PtrUR) -> PtrString {
    let aptos_sign_request = extract_ptr_with_type!(ptr, AptosSignRequest);
    let derivation_path = &aptos_sign_request.get_authentication_key_derivation_paths()[0];
    if let Some(path) = derivation_path.get_path() {
        return convert_c_char(path);
    }
    return convert_c_char("".to_string());
}

#[no_mangle]
pub extern "C" fn test_aptos_parse() -> *mut SimpleResponse<c_char> {
    let data = "8bbbb70ae8b90a8686b2a27f10e21e44f2fb64ffffcaa4bb0242e9f1ea698659010000000000000002000000000000000000000000000000000000000000000000000000000000000104636f696e087472616e73666572010700000000000000000000000000000000000000000000000000000000000000010a6170746f735f636f696e094170746f73436f696e000220834f4b75dcaacbd7c549a993cdd3140676e172d1fee0609bf6876c74aaa7116008400d0300000000009a0e0000000000006400000000000000b6b747630000000021";
    let buf_message = Vec::from_hex(data).unwrap();
    match app_aptos::parse_tx(&buf_message) {
        Ok(result) => {
            SimpleResponse::success(convert_c_char(result.get_result().unwrap()) as *mut c_char)
                .simple_c_ptr()
        }
        Err(e) => SimpleResponse::from(e).simple_c_ptr(),
    }
}
