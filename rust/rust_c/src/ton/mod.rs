pub mod structs;
use crate::{
    common::{
        errors::RustCError,
        ffi::VecFFI,
        structs::{SimpleResponse, TransactionCheckResult, TransactionParseResult},
        types::{Ptr, PtrBytes, PtrString, PtrT, PtrUR},
        ur::{UREncodeResult, FRAGMENT_MAX_LENGTH_DEFAULT},
        utils::recover_c_char,
    },
    extract_array,
};
use alloc::{
    boxed::Box,
    format, slice,
    string::{String, ToString},
    vec::Vec,
};
use app_ton::{mnemonic::ton_mnemonic_validate, ton_compare_address_and_public_key};

use crate::{extract_ptr_with_type, impl_c_ptr};
use cty::c_char;
use keystore::algorithms::ed25519;
use rust_tools::convert_c_char;
use structs::{DisplayTonProof, DisplayTonTransaction};
use {
    hex,
    ur_registry::{
        ton::{ton_sign_request::TonSignRequest, ton_signature::TonSignature},
        traits::RegistryItem,
    },
};

#[repr(C)]
pub struct DisplayTon {
    text: PtrString,
}

impl_c_ptr!(DisplayTon);

#[no_mangle]
pub unsafe extern "C" fn ton_parse_transaction(
    ptr: PtrUR,
) -> PtrT<TransactionParseResult<DisplayTonTransaction>> {
    let ton_tx = extract_ptr_with_type!(ptr, TonSignRequest);

    let serial = ton_tx.get_sign_data();
    let tx = app_ton::transaction::parse_transaction(&serial);
    match tx {
        Ok(tx) => {
            let display_tx = DisplayTonTransaction::from(&tx);
            TransactionParseResult::success(display_tx.c_ptr()).c_ptr()
        }
        Err(e) => TransactionParseResult::from(e).c_ptr(),
    }
}

#[no_mangle]
pub unsafe extern "C" fn ton_parse_proof(
    ptr: PtrUR,
) -> PtrT<TransactionParseResult<DisplayTonProof>> {
    let ton_tx = extract_ptr_with_type!(ptr, TonSignRequest);

    let serial = ton_tx.get_sign_data();
    let tx = app_ton::transaction::parse_proof(&serial);
    match tx {
        Ok(tx) => {
            let display_tx = DisplayTonProof::from(&tx);
            TransactionParseResult::success(display_tx.c_ptr()).c_ptr()
        }
        Err(e) => TransactionParseResult::from(e).c_ptr(),
    }
}

#[no_mangle]
pub unsafe extern "C" fn ton_check_transaction(
    ptr: PtrUR,
    public_key: PtrString,
) -> PtrT<TransactionCheckResult> {
    let ton_tx = extract_ptr_with_type!(ptr, TonSignRequest);
    let pk = recover_c_char(public_key);
    match hex::decode(pk) {
        Ok(pk) => {
            if ton_compare_address_and_public_key(pk, ton_tx.get_address()) {
                TransactionCheckResult::new().c_ptr()
            } else {
                TransactionCheckResult::from(RustCError::MasterFingerprintMismatch).c_ptr()
            }
        }
        Err(e) => TransactionCheckResult::from(RustCError::InvalidHex(e.to_string())).c_ptr(),
    }
}

#[no_mangle]
pub unsafe extern "C" fn ton_not_supported_error() -> PtrT<TransactionCheckResult> {
    TransactionCheckResult::from(RustCError::UnsupportedTransaction(
        "ton transaction".to_string(),
    ))
    .c_ptr()
}

fn get_secret_key(tx: &TonSignRequest, seed: &[u8]) -> Result<[u8; 32], RustCError> {
    let mut sk: [u8; 32] = [0; 32];
    match tx.get_derivation_path() {
        Some(derivation_path) => {
            let path = derivation_path
                .get_path()
                .ok_or(RustCError::InvalidHDPath)?;
            match ed25519::slip10_ed25519::get_private_key_by_seed(seed, &path) {
                Ok(_sk) => {
                    for i in 0..32 {
                        sk[i] = _sk[i]
                    }
                }
                Err(e) => return Err(RustCError::UnexpectedError(e.to_string())),
            }
        }
        None => {
            for i in 0..32 {
                sk[i] = seed[i]
            }
        }
    };
    Ok(sk)
}

#[no_mangle]
pub unsafe extern "C" fn ton_sign_transaction(
    ptr: PtrUR,
    seed: PtrBytes,
    seed_len: u32,
) -> PtrT<UREncodeResult> {
    let ton_tx = extract_ptr_with_type!(ptr, TonSignRequest);
    let seed = extract_array!(seed, u8, seed_len as usize);
    let sk = match get_secret_key(ton_tx, seed) {
        Ok(_sk) => _sk,
        Err(e) => return UREncodeResult::from(e).c_ptr(),
    };
    let result = app_ton::transaction::sign_transaction(&ton_tx.get_sign_data(), sk);
    match result {
        Ok(sig) => {
            let ton_signature = TonSignature::new(
                ton_tx.get_request_id(),
                sig.to_vec(),
                Some("Keystone".to_string()),
            );
            match ton_signature.try_into() {
                Err(e) => UREncodeResult::from(e).c_ptr(),
                Ok(v) => UREncodeResult::encode(
                    v,
                    TonSignature::get_registry_type().get_type(),
                    FRAGMENT_MAX_LENGTH_DEFAULT,
                )
                .c_ptr(),
            }
        }
        Err(e) => UREncodeResult::from(e).c_ptr(),
    }
}

#[no_mangle]
pub unsafe extern "C" fn ton_sign_proof(
    ptr: PtrUR,
    seed: PtrBytes,
    seed_len: u32,
) -> PtrT<UREncodeResult> {
    let ton_tx = extract_ptr_with_type!(ptr, TonSignRequest);
    let seed = extract_array!(seed, u8, seed_len as usize);
    let sk = match get_secret_key(ton_tx, seed) {
        Ok(_sk) => _sk,
        Err(e) => return UREncodeResult::from(e).c_ptr(),
    };
    let result = app_ton::transaction::sign_proof(&ton_tx.get_sign_data(), sk);
    match result {
        Ok(sig) => {
            let ton_signature = TonSignature::new(
                ton_tx.get_request_id(),
                sig.to_vec(),
                Some("Keystone".to_string()),
            );
            match ton_signature.try_into() {
                Err(e) => UREncodeResult::from(e).c_ptr(),
                Ok(v) => UREncodeResult::encode(
                    v,
                    TonSignature::get_registry_type().get_type(),
                    FRAGMENT_MAX_LENGTH_DEFAULT,
                )
                .c_ptr(),
            }
        }
        Err(e) => UREncodeResult::from(e).c_ptr(),
    }
}

#[no_mangle]
pub unsafe extern "C" fn ton_verify_mnemonic(mnemonic: PtrString) -> bool {
    let mnemonic = recover_c_char(mnemonic);
    let words: Vec<String> = mnemonic.split(' ').map(|v| v.to_lowercase()).collect();
    ton_mnemonic_validate(&words, &None).is_ok()
}

#[no_mangle]
pub unsafe extern "C" fn ton_mnemonic_to_entropy(mnemonic: PtrString) -> Ptr<VecFFI<u8>> {
    let mnemonic = recover_c_char(mnemonic);
    let words: Vec<String> = mnemonic.split(' ').map(|v| v.to_lowercase()).collect();
    let entropy = app_ton::mnemonic::ton_mnemonic_to_entropy(&words, &None);
    VecFFI::from(entropy).c_ptr()
}

#[no_mangle]
pub unsafe extern "C" fn ton_entropy_to_seed(
    entropy: PtrBytes,
    entropy_len: u32,
) -> *mut SimpleResponse<u8> {
    let entropy = extract_array!(entropy, u8, entropy_len as usize);
    let seed = app_ton::mnemonic::ton_entropy_to_seed(entropy);
    SimpleResponse::success(Box::into_raw(Box::new(seed)) as *mut u8).simple_c_ptr()
}

#[no_mangle]
pub unsafe extern "C" fn ton_mnemonic_to_seed(mnemonic: PtrString) -> *mut SimpleResponse<u8> {
    let mnemonic = recover_c_char(mnemonic);
    let words: Vec<String> = mnemonic.split(' ').map(|v| v.to_lowercase()).collect();
    let seed = app_ton::mnemonic::ton_mnemonic_to_master_seed(words, None);
    match seed {
        Ok(seed) => {
            SimpleResponse::success(Box::into_raw(Box::new(seed)) as *mut u8).simple_c_ptr()
        }
        Err(e) => SimpleResponse::from(e).simple_c_ptr(),
    }
}

#[no_mangle]
pub unsafe extern "C" fn ton_seed_to_publickey(
    seed: PtrBytes,
    seed_len: u32,
) -> *mut SimpleResponse<c_char> {
    let seed = extract_array!(seed, u8, seed_len as usize);
    match seed.try_into() {
        Ok(_seed) => {
            let public_key = app_ton::mnemonic::ton_master_seed_to_public_key(_seed);
            SimpleResponse::success(convert_c_char(hex::encode(public_key))).simple_c_ptr()
        }
        Err(_e) => SimpleResponse::from(crate::common::errors::RustCError::InvalidData(
            "seed length should be 64".to_string(),
        ))
        .simple_c_ptr(),
    }
}

#[no_mangle]
pub unsafe extern "C" fn ton_get_address(public_key: PtrString) -> *mut SimpleResponse<c_char> {
    let pk = recover_c_char(public_key);
    match hex::decode(pk) {
        Ok(pk) => match app_ton::ton_public_key_to_address(pk) {
            Ok(address) => SimpleResponse::success(convert_c_char(address)).simple_c_ptr(),
            Err(e) => SimpleResponse::from(e).simple_c_ptr(),
        },
        Err(e) => {
            SimpleResponse::from(crate::common::errors::RustCError::InvalidHex(e.to_string()))
                .simple_c_ptr()
        }
    }
}
