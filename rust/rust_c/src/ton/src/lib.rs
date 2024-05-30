#![no_std]
extern crate alloc;

pub mod structs;
use alloc::{
    boxed::Box,
    format, slice,
    string::{String, ToString},
    vec::Vec,
};
use app_ton::mnemonic::ton_mnemonic_validate;
use common_rust_c::{
    extract_ptr_with_type,
    ffi::VecFFI,
    impl_c_ptr,
    structs::{SimpleResponse, TransactionCheckResult, TransactionParseResult},
    types::{Ptr, PtrBytes, PtrString, PtrT, PtrUR},
    ur::{UREncodeResult, FRAGMENT_MAX_LENGTH_DEFAULT},
    utils::recover_c_char,
};
use cty::c_char;
use rust_tools::convert_c_char;
use structs::{DisplayTonProof, DisplayTonTransaction};
use third_party::{
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
pub extern "C" fn ton_parse_transaction(
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
pub extern "C" fn ton_parse_proof(ptr: PtrUR) -> PtrT<TransactionParseResult<DisplayTonProof>> {
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
pub extern "C" fn ton_check_transaction(
    ptr: PtrUR,
    master_fingerprint: PtrBytes,
    length: u32,
) -> PtrT<TransactionCheckResult> {
    return TransactionCheckResult::new().c_ptr();
}

#[no_mangle]
pub extern "C" fn ton_sign_transaction(
    ptr: PtrUR,
    seed: PtrBytes,
    seed_len: u32,
) -> PtrT<UREncodeResult> {
    let ton_tx = extract_ptr_with_type!(ptr, TonSignRequest);
    let seed = unsafe { slice::from_raw_parts(seed, seed_len as usize) };
    let mut sk: [u8; 32] = [0; 32];
    for i in 0..32 {
        sk[i] = seed[i]
    }
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
                    FRAGMENT_MAX_LENGTH_DEFAULT.clone(),
                )
                .c_ptr(),
            }
        }
        Err(e) => UREncodeResult::from(e).c_ptr(),
    }
}

#[no_mangle]
pub extern "C" fn ton_sign_proof(
    ptr: PtrUR,
    seed: PtrBytes,
    seed_len: u32,
) -> PtrT<UREncodeResult> {
    let ton_tx = extract_ptr_with_type!(ptr, TonSignRequest);
    let seed = unsafe { slice::from_raw_parts(seed, seed_len as usize) };
    rust_tools::debug!(format!("seed: {}", hex::encode(seed)));
    let mut sk: [u8; 32] = [0; 32];
    for i in 0..32 {
        sk[i] = seed[i]
    }
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
                    FRAGMENT_MAX_LENGTH_DEFAULT.clone(),
                )
                .c_ptr(),
            }
        }
        Err(e) => UREncodeResult::from(e).c_ptr(),
    }
}

#[no_mangle]
pub extern "C" fn ton_verify_mnemonic(mnemonic: PtrString) -> bool {
    let mnemonic = recover_c_char(mnemonic);
    let words: Vec<String> = mnemonic.split(" ").map(|v| v.to_lowercase()).collect();
    match ton_mnemonic_validate(&words, &None) {
        Ok(_) => true,
        Err(_) => false,
    }
}

#[no_mangle]
pub extern "C" fn ton_mnemonic_to_entropy(mnemonic: PtrString) -> Ptr<VecFFI<u8>> {
    let mnemonic = recover_c_char(mnemonic);
    let words: Vec<String> = mnemonic.split(" ").map(|v| v.to_lowercase()).collect();
    let entropy = app_ton::mnemonic::ton_mnemonic_to_entropy(&words, &None);
    VecFFI::from(entropy).c_ptr()
}

#[no_mangle]
pub extern "C" fn ton_entropy_to_seed(
    entropy: PtrBytes,
    entropy_len: u32,
) -> *mut SimpleResponse<u8> {
    let entropy = unsafe { slice::from_raw_parts(entropy, entropy_len as usize) }.to_vec();
    let seed = app_ton::mnemonic::ton_entropy_to_seed(entropy);
    SimpleResponse::success(Box::into_raw(Box::new(seed)) as *mut u8).simple_c_ptr()
}

#[no_mangle]
pub extern "C" fn ton_mnemonic_to_seed(mnemonic: PtrString) -> *mut SimpleResponse<u8> {
    let mnemonic = recover_c_char(mnemonic);
    let words: Vec<String> = mnemonic.split(" ").map(|v| v.to_lowercase()).collect();
    let seed = app_ton::mnemonic::ton_mnemonic_to_master_seed(words, None);
    match seed {
        Ok(seed) => {
            SimpleResponse::success(Box::into_raw(Box::new(seed)) as *mut u8).simple_c_ptr()
        }
        Err(e) => SimpleResponse::from(e).simple_c_ptr(),
    }
}

#[no_mangle]
pub extern "C" fn ton_seed_to_publickey(
    seed: PtrBytes,
    seed_len: u32,
) -> *mut SimpleResponse<c_char> {
    let seed = unsafe { slice::from_raw_parts(seed, seed_len as usize) }.to_vec();
    match seed.try_into() {
        Ok(_seed) => {
            let public_key = app_ton::mnemonic::ton_master_seed_to_public_key(_seed);
            SimpleResponse::success(convert_c_char(hex::encode(public_key))).simple_c_ptr()
        }
        Err(_e) => SimpleResponse::from(common_rust_c::errors::RustCError::InvalidData(format!(
            "seed length should be 64"
        )))
        .simple_c_ptr(),
    }
}

#[no_mangle]
pub extern "C" fn ton_get_address(public_key: PtrString) -> *mut SimpleResponse<c_char> {
    let pk = recover_c_char(public_key);
    match hex::decode(pk) {
        Ok(pk) => match app_ton::ton_public_key_to_address(pk) {
            Ok(address) => SimpleResponse::success(convert_c_char(address)).simple_c_ptr(),
            Err(e) => SimpleResponse::from(e).simple_c_ptr(),
        },
        Err(e) => {
            SimpleResponse::from(common_rust_c::errors::RustCError::InvalidHex(e.to_string()))
                .simple_c_ptr()
        }
    }
}
