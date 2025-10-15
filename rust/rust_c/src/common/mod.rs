use alloc::boxed::Box;
use alloc::format;
use alloc::string::{String, ToString};
use alloc::vec::Vec;
use core::ptr::null_mut;
use core::slice;
use cryptoxide::hashing::sha256;
use ffi::VecFFI;
use keystore::algorithms::ed25519::slip10_ed25519::get_private_key_by_seed;

use bitcoin::hex::Case;
use bitcoin_hashes::hex::DisplayHex;
use cty::c_char;

use hex::ToHex;

use errors::ErrorCodes;
use structs::{Response, TransactionCheckResult};
use types::Ptr;

use errors::RustCError;
use structs::SimpleResponse;
use types::{PtrBytes, PtrString};
use utils::{convert_c_char, recover_c_char};

use crate::extract_array;

pub mod errors;
pub mod ffi;
pub mod free;
pub mod keystone;
pub mod macros;
pub mod qrcode;
pub mod structs;
pub mod types;
pub mod ur;
mod ur_ext;
pub mod utils;
pub mod web_auth;

pub static KEYSTONE: &str = "keystone";

#[no_mangle]
pub extern "C" fn get_master_fingerprint(seed: PtrBytes, seed_len: u32) -> *mut SimpleResponse<u8> {
    let s = unsafe { slice::from_raw_parts(seed, seed_len as usize).to_vec() };
    let master_fingerprint = keystore::algorithms::secp256k1::get_master_fingerprint_by_seed(&s);
    match master_fingerprint {
        Ok(result) => {
            SimpleResponse::success(Box::into_raw(Box::new(result)) as *mut u8).simple_c_ptr()
        }
        Err(e) => SimpleResponse::from(e).simple_c_ptr(),
    }
}

#[no_mangle]
pub extern "C" fn dummy_function_to_export_error_codes() -> ErrorCodes {
    ErrorCodes::Success
}

#[no_mangle]
pub unsafe extern "C" fn format_value_with_decimals(
    value: PtrString,
    decimals: u32,
) -> *mut SimpleResponse<c_char> {
    let value = recover_c_char(value);
    let decimals = decimals as usize;
    let mut chars: Vec<char> = value.chars().collect();

    while chars.len() < decimals {
        chars.insert(0, '0');
    }

    chars.insert(chars.len() - decimals, '.');

    if chars[0] == '.' {
        chars.insert(0, '0');
    }

    let formatted = chars.into_iter().collect::<String>();

    let trimmed = formatted.trim_end_matches('0');

    if trimmed.ends_with('.') {
        SimpleResponse::success(convert_c_char(trimmed[0..trimmed.len() - 1].to_string()))
            .simple_c_ptr()
    } else {
        SimpleResponse::success(convert_c_char(trimmed.to_string())).simple_c_ptr()
    }
}

#[no_mangle]
pub unsafe extern "C" fn get_extended_pubkey_by_seed(
    seed: PtrBytes,
    seed_len: u32,
    path: PtrString,
) -> *mut SimpleResponse<c_char> {
    let path = recover_c_char(path);
    let seed = extract_array!(seed, u8, seed_len as usize);
    let extended_key =
        keystore::algorithms::secp256k1::get_extended_public_key_by_seed(seed, &path);
    match extended_key {
        Ok(result) => SimpleResponse::success(convert_c_char(result.to_string())).simple_c_ptr(),
        Err(e) => SimpleResponse::from(e).simple_c_ptr(),
    }
}

#[no_mangle]
pub unsafe extern "C" fn get_extended_pubkey_bytes_by_seed(
    seed: PtrBytes,
    seed_len: u32,
    path: PtrString,
) -> *mut SimpleResponse<c_char> {
    let path = recover_c_char(path);
    let seed = extract_array!(seed, u8, seed_len as usize);
    let extended_key =
        keystore::algorithms::secp256k1::get_extended_public_key_by_seed(seed, &path);
    match extended_key {
        Ok(result) => {
            SimpleResponse::success(convert_c_char(result.encode().to_hex_string(Case::Lower)))
                .simple_c_ptr()
        }
        Err(e) => SimpleResponse::from(e).simple_c_ptr(),
    }
}

#[no_mangle]
pub unsafe extern "C" fn get_ed25519_pubkey_by_seed(
    seed: PtrBytes,
    seed_len: u32,
    path: PtrString,
) -> *mut SimpleResponse<c_char> {
    let seed = extract_array!(seed, u8, seed_len as usize);
    let path = recover_c_char(path);
    let extended_key =
        keystore::algorithms::ed25519::slip10_ed25519::get_public_key_by_seed(seed, &path);
    match extended_key {
        Ok(result) => SimpleResponse::success(convert_c_char(hex::encode(result))).simple_c_ptr(),
        Err(e) => SimpleResponse::from(e).simple_c_ptr(),
    }
}

#[no_mangle]
pub unsafe extern "C" fn get_rsa_pubkey_by_seed(
    seed: PtrBytes,
    seed_len: u32,
) -> *mut SimpleResponse<u8> {
    let seed = extract_array!(seed, u8, seed_len as usize);
    let public_key = keystore::algorithms::rsa::get_rsa_pubkey_by_seed(seed);
    match public_key {
        Ok(result) => {
            SimpleResponse::success(Box::into_raw(Box::new(result)) as *mut u8).simple_c_ptr()
        }
        Err(e) => SimpleResponse::from(e).simple_c_ptr(),
    }
}

#[no_mangle]
pub unsafe extern "C" fn get_bip32_ed25519_extended_pubkey(
    entropy: PtrBytes,
    entropy_len: u32,
    passphrase: PtrString,
    path: PtrString,
) -> *mut SimpleResponse<c_char> {
    let entropy = extract_array!(entropy, u8, entropy_len as usize);
    let path = recover_c_char(path);
    let passphrase = recover_c_char(passphrase);
    let extended_key =
        keystore::algorithms::ed25519::bip32_ed25519::get_extended_public_key_by_entropy(
            entropy,
            passphrase.as_bytes(),
            &path,
        );
    match extended_key {
        Ok(result) => SimpleResponse::success(convert_c_char(result.encode_hex())).simple_c_ptr(),
        Err(e) => SimpleResponse::from(e).simple_c_ptr(),
    }
}

#[no_mangle]
pub unsafe extern "C" fn get_ledger_bitbox02_master_key(
    mnemonic: PtrString,
    passphrase: PtrString,
) -> *mut SimpleResponse<c_char> {
    let mnemonic = recover_c_char(mnemonic);
    let passphrase = recover_c_char(passphrase);
    let master_key =
        keystore::algorithms::ed25519::bip32_ed25519::get_ledger_bitbox02_master_key_by_mnemonic(
            passphrase.as_bytes(),
            mnemonic,
        );
    match master_key {
        Ok(result) => SimpleResponse::success(convert_c_char(result.encode_hex())).simple_c_ptr(),
        Err(e) => SimpleResponse::from(e).simple_c_ptr(),
    }
}

#[no_mangle]
pub unsafe extern "C" fn get_icarus_master_key(
    entropy: PtrBytes,
    entropy_len: u32,
    passphrase: PtrString,
) -> *mut SimpleResponse<c_char> {
    let entropy = extract_array!(entropy, u8, entropy_len as usize);
    let passphrase = recover_c_char(passphrase);
    let master_key = keystore::algorithms::ed25519::bip32_ed25519::get_icarus_master_key_by_entropy(
        entropy,
        passphrase.as_bytes(),
    );
    match master_key {
        Ok(result) => SimpleResponse::success(convert_c_char(result.encode_hex())).simple_c_ptr(),
        Err(e) => SimpleResponse::from(e).simple_c_ptr(),
    }
}

#[no_mangle]
pub unsafe extern "C" fn derive_bip32_ed25519_extended_pubkey(
    master_key: PtrString,
    path: PtrString,
) -> *mut SimpleResponse<c_char> {
    let master_key = recover_c_char(master_key);
    match hex::decode(master_key).map_err(|e| RustCError::InvalidHex(e.to_string())) {
        Ok(root) => {
            let path = recover_c_char(path);
            let master_key =
                keystore::algorithms::ed25519::bip32_ed25519::derive_extended_pubkey_by_icarus_master_key(&root, &path);
            match master_key {
                Ok(result) => {
                    SimpleResponse::success(convert_c_char(result.encode_hex())).simple_c_ptr()
                }
                Err(e) => SimpleResponse::from(e).simple_c_ptr(),
            }
        }
        Err(e) => SimpleResponse::from(e).simple_c_ptr(),
    }
}

#[no_mangle]
pub unsafe extern "C" fn k1_sign_message_hash_by_private_key(
    private_key: PtrBytes,
    message_hash: PtrBytes,
) -> *mut SimpleResponse<c_char> {
    let private_key_bytes = extract_array!(private_key, u8, 32);
    let message_hash_bytes = extract_array!(message_hash, u8, 32);
    let signature = keystore::algorithms::secp256k1::sign_message_hash_by_private_key(
        message_hash_bytes,
        private_key_bytes,
    );
    match signature {
        Ok(result) => SimpleResponse::success(convert_c_char(hex::encode(result))).simple_c_ptr(),
        Err(e) => SimpleResponse::from(e).simple_c_ptr(),
    }
}

#[no_mangle]
pub unsafe extern "C" fn k1_verify_signature(
    signature: PtrBytes,
    message_hash: PtrBytes,
    public_key: PtrBytes,
) -> bool {
    let signature_bytes = extract_array!(signature, u8, 64);
    let message_hash_bytes = extract_array!(message_hash, u8, 32);
    let public_key_bytes = extract_array!(public_key, u8, 65);
    let result = keystore::algorithms::secp256k1::verify_signature(
        signature_bytes,
        message_hash_bytes,
        public_key_bytes,
    );
    result.unwrap_or_default()
}

#[no_mangle]
pub unsafe extern "C" fn k1_generate_ecdh_sharekey(
    privkey: PtrBytes,
    privkey_len: u32,
    pubkey: PtrBytes,
    pubkey_len: u32,
) -> *mut SimpleResponse<u8> {
    let private_key = extract_array!(privkey, u8, privkey_len as usize);
    let public_key = extract_array!(pubkey, u8, pubkey_len as usize);
    let result = keystore::algorithms::secp256k1::get_share_key(private_key, public_key);
    match result {
        Ok(share_key) => {
            SimpleResponse::success(Box::into_raw(Box::new(share_key)) as *mut u8).simple_c_ptr()
        }
        Err(e) => SimpleResponse::from(e).simple_c_ptr(),
    }
}

#[no_mangle]
pub unsafe extern "C" fn k1_generate_pubkey_by_privkey(
    privkey: PtrBytes,
    privkey_len: u32,
) -> *mut SimpleResponse<u8> {
    let private_key = extract_array!(privkey, u8, privkey_len as usize);
    let result = keystore::algorithms::secp256k1::get_public_key(private_key);
    match result {
        Ok(pubkey) => {
            SimpleResponse::success(Box::into_raw(Box::new(pubkey)) as *mut u8).simple_c_ptr()
        }
        Err(e) => SimpleResponse::from(e).simple_c_ptr(),
    }
}

#[no_mangle]
pub unsafe extern "C" fn pbkdf2_rust(
    password: PtrBytes,
    salt: PtrBytes,
    iterations: u32,
) -> *mut SimpleResponse<u8> {
    let password_bytes = extract_array!(password, u8, 32);
    let salt_bytes = extract_array!(salt, u8, 32);
    let output = keystore::algorithms::crypto::hkdf(password_bytes, salt_bytes, iterations);
    SimpleResponse::success(Box::into_raw(Box::new(output)) as *mut u8).simple_c_ptr()
}

#[no_mangle]
pub unsafe extern "C" fn pbkdf2_rust_64(
    password: PtrBytes,
    salt: PtrBytes,
    iterations: u32,
) -> *mut SimpleResponse<u8> {
    let password_bytes = extract_array!(password, u8, 64);
    let salt_bytes = extract_array!(salt, u8, 64);
    let output = keystore::algorithms::crypto::hkdf64(password_bytes, salt_bytes, iterations);
    SimpleResponse::success(Box::into_raw(Box::new(output)) as *mut u8).simple_c_ptr()
}

#[no_mangle]
pub extern "C" fn tx_check_pass() -> Ptr<TransactionCheckResult> {
    TransactionCheckResult::new().c_ptr()
}
