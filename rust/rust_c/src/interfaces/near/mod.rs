pub mod structs;

use crate::extract_ptr_with_type;
use crate::interfaces::errors::RustCError;
use crate::interfaces::near::structs::DisplayNearTx;
use crate::interfaces::structs::{TransactionCheckResult, TransactionParseResult};
use crate::interfaces::types::{PtrBytes, PtrT, PtrUR};
use crate::interfaces::ur::{UREncodeResult, FRAGMENT_MAX_LENGTH_DEFAULT};

use alloc::string::ToString;
use alloc::{format, vec};
use app_near::errors::NearError;
use third_party::ur_registry::near::near_sign_request::NearSignRequest;
use third_party::ur_registry::near::near_signature::NearSignature;
use third_party::ur_registry::traits::RegistryItem;

fn build_sign_result(ptr: PtrUR, seed: &[u8]) -> Result<NearSignature, NearError> {
    let sign_request = extract_ptr_with_type!(ptr, NearSignRequest);
    let mut path =
        sign_request
            .get_derivation_path()
            .get_path()
            .ok_or(NearError::InvalidHDPath(
                "invalid derivation path".to_string(),
            ))?;
    if !path.starts_with("m/") {
        path = format!("m/{}", path);
    }
    let sign_data = sign_request.get_sign_data();
    if sign_data.len() != 1 {
        return Err(NearError::SignFailure(
            "multiple sign does not supported".to_string(),
        ));
    }
    let signature = app_near::sign(&sign_data[0], &path, seed)?;
    Ok(NearSignature::new(
        sign_request.get_request_id(),
        vec![signature.to_vec()],
    ))
}

#[no_mangle]
pub extern "C" fn near_check(
    ptr: PtrUR,
    master_fingerprint: PtrBytes,
    length: u32,
) -> PtrT<TransactionCheckResult> {
    if length != 4 {
        return TransactionCheckResult::from(RustCError::InvalidMasterFingerprint).c_ptr();
    }
    let near_sign_request = extract_ptr_with_type!(ptr, NearSignRequest);
    let mfp = unsafe { core::slice::from_raw_parts(master_fingerprint, 4) };
    if let Ok(mfp) = mfp.try_into() as Result<[u8; 4], _> {
        let derivation_path: third_party::ur_registry::crypto_key_path::CryptoKeyPath =
            near_sign_request.get_derivation_path();
        if let Some(ur_mfp) = derivation_path.get_source_fingerprint() {
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
pub extern "C" fn near_parse_tx(ptr: PtrUR) -> PtrT<TransactionParseResult<DisplayNearTx>> {
    let near_sign_reqeust = extract_ptr_with_type!(ptr, NearSignRequest);
    let sign_data = near_sign_reqeust.get_sign_data();
    if sign_data.len() != 1 {
        return TransactionParseResult::from(NearError::SignFailure(
            "multiple tx parse does not supported".to_string(),
        ))
        .c_ptr();
    }
    match app_near::parse(&sign_data[0]) {
        Ok(v) => TransactionParseResult::success(DisplayNearTx::from(v).c_ptr()).c_ptr(),
        Err(e) => TransactionParseResult::from(e).c_ptr(),
    }
}

#[no_mangle]
pub extern "C" fn near_sign_tx(ptr: PtrUR, seed: PtrBytes, seed_len: u32) -> PtrT<UREncodeResult> {
    let seed = unsafe { alloc::slice::from_raw_parts(seed, seed_len as usize) };
    build_sign_result(ptr, seed)
        .map(|v| v.try_into())
        .map_or_else(
            |e| UREncodeResult::from(e).c_ptr(),
            |v| {
                v.map_or_else(
                    |e| UREncodeResult::from(e).c_ptr(),
                    |data| {
                        UREncodeResult::encode(
                            data,
                            NearSignature::get_registry_type().get_type(),
                            FRAGMENT_MAX_LENGTH_DEFAULT,
                        )
                        .c_ptr()
                    },
                )
            },
        )
}
