use crate::common::errors::RustCError;
use crate::common::structs::TransactionParseResult;
use crate::common::structs::{ExtendedPublicKey, SimpleResponse};
use crate::common::types::{PtrBytes, PtrString, PtrT, PtrUR};
use crate::common::ur::{UREncodeResult, FRAGMENT_MAX_LENGTH_DEFAULT};
use crate::common::utils::{convert_c_char, recover_c_char};
use crate::extract_ptr_with_type;
use crate::sui::get_public_key;
use crate::{extract_array, extract_array_mut};
use alloc::vec::Vec;
use alloc::{
    string::{String, ToString},
    vec,
};
use app_sui::errors::SuiError;
use app_sui::types::intent::{IntentMessage, PersonalMessage};
use app_sui::Intent;
use app_utils::normalize_path;
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

fn extract_personal_message_bytes(intent: &[u8]) -> Result<Vec<u8>, SuiError> {
    if intent.get(..3) != Some(&[3, 0, 0]) {
        return Err(SuiError::InvalidData(
            "Invalid personal message intent".to_string(),
        ));
    }
    bcs::from_bytes::<IntentMessage<PersonalMessage>>(intent)
        .map(|message| message.value.message)
        .map_err(SuiError::from)
}

fn signing_path(request: &IotaSignRequest) -> Result<String, SuiError> {
    request
        .get_derivation_paths()
        .first()
        .and_then(|path| path.get_path())
        .map(|path| normalize_path(&path))
        .ok_or_else(|| SuiError::InvalidData("Invalid signing path".to_string()))
}

fn verified_message_address(request: &IotaSignRequest, pubkey: &str) -> Result<String, SuiError> {
    let address = app_iota::address::get_address_from_pubkey(pubkey.to_string())
        .map_err(|e| SuiError::InvalidData(e.to_string()))?;
    if let Some(addresses) = request.get_addresses() {
        let supplied = addresses
            .first()
            .filter(|address| address.len() == 32)
            .ok_or_else(|| SuiError::InvalidData("Invalid signing address".to_string()))?;
        if hex::encode(supplied) != address[2..] {
            return Err(SuiError::InvalidData(
                "Signing address does not match device key".to_string(),
            ));
        }
    }
    Ok(address)
}

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
    keys: PtrT<ExtendedPublicKey>,
    keys_len: u32,
) -> PtrT<TransactionParseResult<DisplayIotaIntentData>> {
    let sign_request = extract_ptr_with_type!(ptr, IotaSignRequest);
    let keys = if keys.is_null() || keys_len == 0 {
        &[]
    } else {
        extract_array!(keys, ExtendedPublicKey, keys_len)
    };
    match parse_iota_request(sign_request, keys) {
        Ok(data) => TransactionParseResult::success(data.c_ptr()).c_ptr(),
        Err(e) => TransactionParseResult::from(e).c_ptr(),
    }
}

unsafe fn parse_iota_request(
    sign_request: &IotaSignRequest,
    keys: &[ExtendedPublicKey],
) -> Result<DisplayIotaIntentData, SuiError> {
    let sign_data = sign_request.get_intent_message();

    if sign_data.first() == Some(&3) {
        let message = extract_personal_message_bytes(&sign_data)?;
        let path = signing_path(sign_request)?;
        let key = keys
            .iter()
            .find(|key| {
                !key.path.is_null()
                    && !key.xpub.is_null()
                    && normalize_path(&recover_c_char(key.path)) == path
            })
            .ok_or_else(|| {
                SuiError::InvalidData(
                    "Unsupported IOTA signing path or missing device key".to_string(),
                )
            })?;
        let address = verified_message_address(sign_request, &recover_c_char(key.xpub))?;
        return Ok(DisplayIotaIntentData::personal_message(&message, address));
    }
    match app_sui::parse_intent(&sign_data)? {
        data @ Intent::TransactionData(_) => Ok(DisplayIotaIntentData::from(data)),
        _ => Err(SuiError::InvalidData("Invalid intent".to_string())),
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
        if sign_data.first() == Some(&3) {
            extract_personal_message_bytes(&sign_data)?;
            let pubkey = get_public_key(s, &p.to_string())?;
            verified_message_address(sign_request, &hex::encode(pubkey))?;
        }
        app_sui::sign_intent(s, &p.to_string(), &sign_data.to_vec())
    }) {
        Ok(result) => result,
        Err(err) => return err.c_ptr(),
    };

    build_iota_signature_result(seed, sign_request.get_request_id(), signature, pub_key)
}
