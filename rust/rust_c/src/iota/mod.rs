use crate::common::errors::RustCError;
use crate::common::structs::SimpleResponse;
use crate::common::structs::{TransactionCheckResult, TransactionParseResult};
use crate::common::types::{PtrBytes, PtrString, PtrT, PtrUR};
use crate::common::ur::{UREncodeResult, FRAGMENT_MAX_LENGTH_DEFAULT};
use crate::common::utils::{convert_c_char, recover_c_char};
use crate::extract_ptr_with_type;
use crate::sui::get_public_key;
use crate::{extract_array, extract_array_mut};
use alloc::format;
use alloc::vec::Vec;
use alloc::{
    string::{String, ToString},
    vec,
};
use app_sui::errors::SuiError;
use app_sui::Intent;
use cty::c_char;
use structs::DisplayIotaIntentData;
use structs::DisplayIotaSignMessageHash;
use ur_registry::iota::iota_signature::IotaSignature;
use ur_registry::iota::{
    iota_sign_hash_request::IotaSignHashRequest, iota_sign_request::IotaSignRequest,
};
use ur_registry::traits::RegistryItem;
use zeroize::Zeroize;

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

    let address = sign_request
        .get_addresses()
        .and_then(|addrs| addrs.first().cloned())
        .map(|addr| format!("0x{}", hex::encode(addr)))
        .unwrap_or_else(|| "0x".to_string());

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

    // Check derivation paths is not empty
    let paths = sign_hash_request.get_derivation_paths();
    let path = if paths.is_empty() {
        "No Path".to_string()
    } else {
        paths[0].get_path().unwrap_or("No Path".to_string())
    };

    let network = "IOTA".to_string();
    let address = sign_hash_request.get_addresses().unwrap_or(vec![]);
    let address_hex = if address.is_empty() {
        "".to_string()
    } else {
        hex::encode(&address[0])
    };

    TransactionParseResult::success(
        DisplayIotaSignMessageHash::new(network, path, message, address_hex).c_ptr(),
    )
    .c_ptr()
}

unsafe fn iota_sign_internal<F>(
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

unsafe fn build_iota_signature_result(
    seed: &mut [u8],
    request_id: Option<Vec<u8>>,
    signature: [u8; 64],
    pub_key: Vec<u8>,
) -> PtrT<UREncodeResult> {
    let sig = IotaSignature::new(request_id, signature.to_vec(), Some(pub_key));

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
        IotaSignature::get_registry_type().get_type(),
        FRAGMENT_MAX_LENGTH_DEFAULT,
    )
    .c_ptr()
}

#[no_mangle]
pub unsafe extern "C" fn iota_sign_hash(
    ptr: PtrUR,
    seed: PtrBytes,
    seed_len: u32,
) -> PtrT<UREncodeResult> {
    let mut seed = extract_array_mut!(seed, u8, seed_len as usize);
    let sign_request = extract_ptr_with_type!(ptr, IotaSignHashRequest);

    // Extract and validate path
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
            .c_ptr();
        }
    };

    let hash = sign_request.get_message_hash();
    let hash_bytes = match hex::decode(hash) {
        Ok(bytes) => bytes,
        Err(e) => {
            seed.zeroize();
            return UREncodeResult::from(RustCError::InvalidHex(e.to_string())).c_ptr();
        }
    };

    let (signature, pub_key) = match iota_sign_internal(seed, &path, |s, p| {
        app_sui::sign_hash(s, &p.to_string(), &hash_bytes)
    }) {
        Ok(result) => result,
        Err(err) => return err.c_ptr(),
    };

    build_iota_signature_result(seed, sign_request.get_request_id(), signature, pub_key)
}

#[no_mangle]
pub unsafe extern "C" fn iota_sign_intent(
    ptr: PtrUR,
    seed: PtrBytes,
    seed_len: u32,
) -> PtrT<UREncodeResult> {
    let mut seed = extract_array_mut!(seed, u8, seed_len as usize);
    let sign_request = extract_ptr_with_type!(ptr, IotaSignRequest);

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
            .c_ptr();
        }
    };

    let sign_data = sign_request.get_intent_message();

    let (signature, pub_key) = match iota_sign_internal(seed, &path, |s, p| {
        app_sui::sign_intent(s, &p.to_string(), &sign_data.to_vec())
    }) {
        Ok(result) => result,
        Err(err) => return err.c_ptr(),
    };

    build_iota_signature_result(seed, sign_request.get_request_id(), signature, pub_key)
}
