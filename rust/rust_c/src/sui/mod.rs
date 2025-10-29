use alloc::string::{String, ToString};
use alloc::vec::Vec;
use alloc::{format, vec};

use cty::c_char;
use ur_registry::sui::sui_sign_hash_request::SuiSignHashRequest;
use ur_registry::sui::sui_sign_request::SuiSignRequest;
use ur_registry::sui::sui_signature::SuiSignature;
use ur_registry::traits::RegistryItem;

use crate::common::errors::RustCError;
use crate::common::structs::{SimpleResponse, TransactionCheckResult, TransactionParseResult};
use crate::common::types::{PtrBytes, PtrString, PtrT, PtrUR};
use crate::common::ur::{UREncodeResult, FRAGMENT_MAX_LENGTH_DEFAULT};
use crate::common::utils::{convert_c_char, recover_c_char};
use crate::{extract_array, extract_array_mut, extract_ptr_with_type};
use app_sui::errors::SuiError;
use app_utils::normalize_path;
use structs::DisplaySuiIntentMessage;
use structs::DisplaySuiSignMessageHash;
use zeroize::Zeroize;

pub mod structs;

pub fn get_public_key(seed: &[u8], path: &String) -> Result<Vec<u8>, SuiError> {
    let path = normalize_path(path);
    let public_key =
        match keystore::algorithms::ed25519::slip10_ed25519::get_public_key_by_seed(seed, &path) {
            Ok(pub_key) => pub_key,
            Err(e) => {
                return Err(SuiError::SignFailure(format!(
                    "derive public key failed {e:?}"
                )))
            }
        };
    Ok(public_key.to_vec())
}

#[no_mangle]
pub unsafe extern "C" fn sui_check_request(
    ptr: PtrUR,
    master_fingerprint: PtrBytes,
    length: u32,
) -> PtrT<TransactionCheckResult> {
    if length != 4 {
        return TransactionCheckResult::from(RustCError::InvalidMasterFingerprint).c_ptr();
    }
    let mfp = extract_array!(master_fingerprint, u8, 4);
    let sign_request = extract_ptr_with_type!(ptr, SuiSignRequest);
    
    let paths = sign_request.get_derivation_paths();
    if paths.is_empty() {
        return TransactionCheckResult::from(RustCError::InvalidHDPath).c_ptr();
    }
    
    // According to SDK convention, index 0 is the signing path
    let ur_mfp = paths[0].get_source_fingerprint();

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
pub unsafe extern "C" fn sui_check_sign_hash_request(
    ptr: PtrUR,
    master_fingerprint: PtrBytes,
    length: u32,
) -> PtrT<TransactionCheckResult> {
    if length != 4 {
        return TransactionCheckResult::from(RustCError::InvalidMasterFingerprint).c_ptr();
    }
    let mfp = extract_array!(master_fingerprint, u8, 4);
    let sign_hash_request = extract_ptr_with_type!(ptr, SuiSignHashRequest);
    
    let paths = sign_hash_request.get_derivation_paths();
    if paths.is_empty() {
        return TransactionCheckResult::from(RustCError::InvalidHDPath).c_ptr();
    }
    
    // According to SDK convention, index 0 is the signing path
    let ur_mfp = paths[0].get_source_fingerprint();

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
pub unsafe extern "C" fn sui_generate_address(pub_key: PtrString) -> *mut SimpleResponse<c_char> {
    let pub_key = recover_c_char(pub_key);
    let address = app_sui::generate_address(&pub_key);
    match address {
        Ok(result) => SimpleResponse::success(convert_c_char(result) as *mut c_char).simple_c_ptr(),
        Err(e) => SimpleResponse::from(e).simple_c_ptr(),
    }
}

#[no_mangle]
pub unsafe extern "C" fn sui_parse_intent(
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
pub unsafe extern "C" fn sui_parse_sign_message_hash(
    ptr: PtrUR,
) -> PtrT<TransactionParseResult<DisplaySuiSignMessageHash>> {
    let sign_hash_request = extract_ptr_with_type!(ptr, SuiSignHashRequest);
    let message = sign_hash_request.get_message_hash();
    
    let paths = sign_hash_request.get_derivation_paths();
    let path = if paths.is_empty() {
        "No Path".to_string()
    } else {
        paths[0].get_path().unwrap_or("No Path".to_string())
    };
    
    let network = "Sui".to_string();
    let address = sign_hash_request.get_addresses().unwrap_or(vec![]);
    let address_hex = if address.is_empty() {
        "".to_string()
    } else {
        hex::encode(&address[0])
    };
    
    TransactionParseResult::success(
        DisplaySuiSignMessageHash::new(
            network,
            path,
            message,
            address_hex,
        )
        .c_ptr(),
    )
    .c_ptr()
}

unsafe fn sui_sign_internal<F>(
    seed: &mut [u8],
    path: &str,
    sign_fn: F,
) -> Result<([u8; 64], Vec<u8>), UREncodeResult>
where
    F: FnOnce(&[u8], &str) -> Result<[u8; 64], SuiError>,
{
    let signature = sign_fn(seed, path).map_err(|e| {
        seed.zeroize();
        UREncodeResult::from(e)
    })?;

    let pub_key = get_public_key(seed, &path.to_string()).map_err(|e| {
        seed.zeroize();
        UREncodeResult::from(e)
    })?;

    Ok((signature, pub_key))
}

unsafe fn build_sui_signature_result(
    seed: &mut [u8],
    request_id: Option<Vec<u8>>,
    signature: [u8; 64],
    pub_key: Vec<u8>,
) -> PtrT<UREncodeResult> {
    let sig = SuiSignature::new(request_id, signature.to_vec(), Some(pub_key));
    
    let sig_data: Vec<u8> = match sig.try_into() {
        Ok(v) => v,
        Err(e) => {
            seed.zeroize();
            return UREncodeResult::from(e).c_ptr();
        }
    };

    seed.zeroize();

    UREncodeResult::encode(
        sig_data,
        SuiSignature::get_registry_type().get_type(),
        FRAGMENT_MAX_LENGTH_DEFAULT,
    )
    .c_ptr()
}

#[no_mangle]
pub unsafe extern "C" fn sui_sign_hash(
    ptr: PtrUR,
    seed: PtrBytes,
    seed_len: u32,
) -> PtrT<UREncodeResult> {
    let mut seed = extract_array_mut!(seed, u8, seed_len as usize);
    let sign_request = extract_ptr_with_type!(ptr, SuiSignHashRequest);
    
    let paths = sign_request.get_derivation_paths();
    if paths.is_empty() {
        seed.zeroize();
        return UREncodeResult::from(RustCError::InvalidHDPath).c_ptr();
    }
    let path = match paths[0].get_path() {
        Some(p) => p,
        None => {
            seed.zeroize();
            return UREncodeResult::from(SuiError::SignFailure(
                "invalid derivation path".to_string(),
            ))
            .c_ptr()
        }
    };

    let hash = sign_request.get_message_hash();
    let hash_bytes = match hex::decode(hash) {
        Ok(bytes) => bytes,
        Err(e) => {
            seed.zeroize();
            return UREncodeResult::from(RustCError::InvalidHex(e.to_string())).c_ptr()
        }
    };

    let (signature, pub_key) = match sui_sign_internal(seed, &path, |s, p| {
        app_sui::sign_hash(s, &p.to_string(), &hash_bytes)
    }) {
        Ok(result) => result,
        Err(err) => return err.c_ptr(),
    };

    build_sui_signature_result(
        seed,
        sign_request.get_request_id(),
        signature,
        pub_key,
    )
}

#[no_mangle]
pub unsafe extern "C" fn sui_sign_intent(
    ptr: PtrUR,
    seed: PtrBytes,
    seed_len: u32,
) -> PtrT<UREncodeResult> {
    let mut seed = extract_array_mut!(seed, u8, seed_len as usize);
    let sign_request = extract_ptr_with_type!(ptr, SuiSignRequest);
    
    let paths = sign_request.get_derivation_paths();
    if paths.is_empty() {
        seed.zeroize();
        return UREncodeResult::from(RustCError::InvalidHDPath).c_ptr();
    }
    let path = match paths[0].get_path() {
        Some(p) => p,
        None => {
            seed.zeroize();
            return UREncodeResult::from(SuiError::SignFailure(
                "invalid derivation path".to_string(),
            ))
            .c_ptr()
        }
    };

    let sign_data = sign_request.get_intent_message();

    let (signature, pub_key) = match sui_sign_internal(seed, &path, |s, p| {
        app_sui::sign_intent(s, &p.to_string(), &sign_data.to_vec())
    }) {
        Ok(result) => result,
        Err(err) => return err.c_ptr(),
    };

    build_sui_signature_result(
        seed,
        sign_request.get_request_id(),
        signature,
        pub_key,
    )
}
