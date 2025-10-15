use alloc::borrow::ToOwned;
use alloc::boxed::Box;
use alloc::slice;
use alloc::string::ToString;
use cty::c_char;

use crate::common::errors::RustCError;
use crate::common::structs::{SimpleResponse, TransactionCheckResult, TransactionParseResult};
use crate::common::types::{PtrBytes, PtrString, PtrT, PtrUR};
use crate::common::ur::{UREncodeResult, FRAGMENT_MAX_LENGTH_DEFAULT};
use crate::common::utils::{convert_c_char, recover_c_char};
use crate::extract_array;
use crate::extract_ptr_with_type;
use app_stellar::strkeys::{sign_hash, sign_signature_base};
use app_stellar::{address::get_address, base_to_xdr};

use structs::DisplayStellarTx;
use ur_registry::stellar::stellar_sign_request::{SignType, StellarSignRequest};
use ur_registry::stellar::stellar_signature::StellarSignature;
use ur_registry::traits::{RegistryItem, To};

pub mod structs;

#[no_mangle]
pub unsafe extern "C" fn stellar_get_address(pubkey: PtrString) -> *mut SimpleResponse<c_char> {
    let x_pub = recover_c_char(pubkey);
    let address = get_address(&x_pub);
    match address {
        Ok(result) => SimpleResponse::success(convert_c_char(result) as *mut c_char).simple_c_ptr(),
        Err(e) => SimpleResponse::from(e).simple_c_ptr(),
    }
}

#[no_mangle]
pub unsafe extern "C" fn stellar_parse(
    ptr: PtrUR,
) -> PtrT<TransactionParseResult<DisplayStellarTx>> {
    let sign_request = extract_ptr_with_type!(ptr, StellarSignRequest);
    let raw_message = match sign_request.get_sign_type() {
        SignType::Transaction => base_to_xdr(&sign_request.get_sign_data()),
        SignType::TransactionHash => hex::encode(sign_request.get_sign_data()),
        _ => {
            return TransactionParseResult::from(RustCError::UnsupportedTransaction(
                "Transaction".to_string(),
            ))
            .c_ptr();
        }
    };
    let display_data = DisplayStellarTx {
        raw_message: convert_c_char(raw_message),
    };
    TransactionParseResult::success(Box::into_raw(Box::new(display_data)) as *mut DisplayStellarTx)
        .c_ptr()
}

#[no_mangle]
pub unsafe extern "C" fn stellar_check_tx(
    ptr: PtrUR,
    master_fingerprint: PtrBytes,
    length: u32,
) -> PtrT<TransactionCheckResult> {
    if length != 4 {
        return TransactionCheckResult::from(RustCError::InvalidMasterFingerprint).c_ptr();
    }
    let mfp = extract_array!(master_fingerprint, u8, 4);
    let sign_request = extract_ptr_with_type!(ptr, StellarSignRequest);
    if let Ok(mfp) = (mfp.try_into() as Result<[u8; 4], _>) {
        let derivation_path: ur_registry::crypto_key_path::CryptoKeyPath =
            sign_request.get_derivation_path();
        if let Some(ur_mfp) = derivation_path.get_source_fingerprint() {
            return if mfp == ur_mfp {
                TransactionCheckResult::new().c_ptr()
            } else {
                TransactionCheckResult::from(RustCError::MasterFingerprintMismatch).c_ptr()
            };
        }
        return TransactionCheckResult::from(RustCError::MasterFingerprintMismatch).c_ptr();
    };
    TransactionCheckResult::from(RustCError::InvalidMasterFingerprint).c_ptr()
}

fn build_signature_data(
    signature: &[u8],
    sign_request: StellarSignRequest,
) -> PtrT<UREncodeResult> {
    let data = StellarSignature::new(sign_request.get_request_id(), signature.to_vec())
        .try_into()
        .unwrap();
    UREncodeResult::encode(
        data,
        StellarSignature::get_registry_type().get_type(),
        FRAGMENT_MAX_LENGTH_DEFAULT,
    )
    .c_ptr()
}

#[no_mangle]
pub unsafe extern "C" fn stellar_sign(
    ptr: PtrUR,
    seed: PtrBytes,
    seed_len: u32,
) -> PtrT<UREncodeResult> {
    let seed = extract_array!(seed, u8, seed_len as usize);
    let sign_request = extract_ptr_with_type!(ptr, StellarSignRequest);
    let sign_data = sign_request.get_sign_data();
    let path = sign_request.get_derivation_path().get_path().unwrap();
    match sign_request.get_sign_type() {
        SignType::Transaction => match sign_signature_base(&sign_data, seed, &path) {
            Ok(signature) => build_signature_data(&signature, sign_request.to_owned()),
            Err(e) => UREncodeResult::from(e).c_ptr(),
        },
        SignType::TransactionHash => match sign_hash(&sign_data, seed, &path) {
            Ok(signature) => build_signature_data(&signature, sign_request.to_owned()),
            Err(e) => UREncodeResult::from(e).c_ptr(),
        },
        _ => UREncodeResult::from(RustCError::UnsupportedTransaction(
            "Transaction".to_string(),
        ))
        .c_ptr(),
    }
}
