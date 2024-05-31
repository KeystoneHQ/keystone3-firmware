#![no_std]

extern crate alloc;
use alloc::boxed::Box;
use alloc::{format, slice};
use cty::c_char;

use crate::structs::DisplayStellarTx;
use app_stellar::{address::get_address, base_to_xdr, strkeys::sign_signature_base};
use common_rust_c::errors::{ErrorCodes, RustCError};
use common_rust_c::extract_ptr_with_type;
use common_rust_c::structs::{SimpleResponse, TransactionCheckResult, TransactionParseResult};
use common_rust_c::types::{PtrBytes, PtrString, PtrT, PtrUR};
use common_rust_c::ur::{UREncodeResult, FRAGMENT_MAX_LENGTH_DEFAULT};
use common_rust_c::utils::{convert_c_char, recover_c_char};
use third_party::hex;
use third_party::ur_registry::stellar::stellar_sign_request::StellarSignRequest;
use third_party::ur_registry::stellar::stellar_signature::StellarSignature;
use third_party::ur_registry::traits::{RegistryItem, To};

pub mod structs;

#[no_mangle]
pub extern "C" fn stellar_get_address(pubkey: PtrString) -> *mut SimpleResponse<c_char> {
    let x_pub = recover_c_char(pubkey);
    let address = get_address(&x_pub);
    match address {
        Ok(result) => SimpleResponse::success(convert_c_char(result) as *mut c_char).simple_c_ptr(),
        Err(e) => SimpleResponse::from(e).simple_c_ptr(),
    }
}

#[no_mangle]
pub extern "C" fn stellar_parse(ptr: PtrUR) -> PtrT<TransactionParseResult<DisplayStellarTx>> {
    let sign_request = extract_ptr_with_type!(ptr, StellarSignRequest);
    let raw_message = base_to_xdr(&sign_request.get_sign_data());
    let display_data = DisplayStellarTx {
        raw_message: convert_c_char(raw_message),
    };
    TransactionParseResult::success(Box::into_raw(Box::new(display_data)) as *mut DisplayStellarTx)
        .c_ptr()
}

#[no_mangle]
pub extern "C" fn stellar_check_tx(
    ptr: PtrUR,
    master_fingerprint: PtrBytes,
    length: u32,
) -> PtrT<TransactionCheckResult> {
    if length != 4 {
        return TransactionCheckResult::from(RustCError::InvalidMasterFingerprint).c_ptr();
    }
    let mfp = unsafe { slice::from_raw_parts(master_fingerprint, 4) };
    let sign_request = extract_ptr_with_type!(ptr, StellarSignRequest);
    if let Some(mfp) = (mfp.try_into() as Result<[u8; 4], _>).ok() {
        let derivation_path: third_party::ur_registry::crypto_key_path::CryptoKeyPath =
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

#[no_mangle]
pub extern "C" fn stellar_sign(ptr: PtrUR, seed: PtrBytes, seed_len: u32) -> PtrT<UREncodeResult> {
    let seed = unsafe { alloc::slice::from_raw_parts(seed, seed_len as usize) };
    let sign_request = extract_ptr_with_type!(ptr, StellarSignRequest);
    let sign_data = sign_request.get_sign_data();
    let path = sign_request.get_derivation_path().get_path().unwrap();
    match sign_signature_base(&sign_data, &seed, &path) {
        Ok(signature) => UREncodeResult::encode(
            signature.to_vec(),
            StellarSignature::get_registry_type().get_type(),
            FRAGMENT_MAX_LENGTH_DEFAULT.clone(),
        )
        .c_ptr(),
        Err(e) => UREncodeResult::from(e).c_ptr(),
    }
}
