use crate::interfaces::errors::RustCError;
use crate::interfaces::structs::SimpleResponse;
use alloc::boxed::Box;
use alloc::string::ToString;
use core::slice;
use cty::c_char;
use third_party::hex;
use third_party::hex::ToHex;

use crate::interfaces::types::{PtrBytes, PtrString};
use crate::interfaces::utils::{convert_c_char, recover_c_char};

mod aptos;
mod bitcoin;
mod cardano;
mod companion_app;
mod connect_wallets;
mod cosmos;
mod errors;
mod ethereum;
mod ffi;
mod free;
mod macros;
mod near;
mod solana;
mod structs;
mod sui;
mod test_cmds;
mod tron;
mod types;
mod ur;
mod ur_ext;
mod utils;
mod web_auth;
mod xrp;
mod transport;

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
pub extern "C" fn get_extended_pubkey_by_seed(
    seed: PtrBytes,
    seed_len: u32,
    path: PtrString,
) -> *mut SimpleResponse<c_char> {
    let path = recover_c_char(path);
    let seed = unsafe { slice::from_raw_parts(seed, seed_len as usize) };
    let extended_key =
        keystore::algorithms::secp256k1::get_extended_public_key_by_seed(seed, &path);
    match extended_key {
        Ok(result) => SimpleResponse::success(convert_c_char(result.to_string())).simple_c_ptr(),
        Err(e) => SimpleResponse::from(e).simple_c_ptr(),
    }
}

#[no_mangle]
pub extern "C" fn get_ed25519_pubkey_by_seed(
    seed: PtrBytes,
    seed_len: u32,
    path: PtrString,
) -> *mut SimpleResponse<c_char> {
    let seed = unsafe { slice::from_raw_parts(seed, seed_len as usize) };
    let path = recover_c_char(path);
    let extended_key =
        keystore::algorithms::ed25519::slip10_ed25519::get_public_key_by_seed(&seed, &path);
    match extended_key {
        Ok(result) => SimpleResponse::success(convert_c_char(hex::encode(result))).simple_c_ptr(),
        Err(e) => SimpleResponse::from(e).simple_c_ptr(),
    }
}

#[no_mangle]
pub extern "C" fn get_bip32_ed25519_extended_pubkey(
    entropy: PtrBytes,
    entropy_len: u32,
    passphrase: PtrString,
    path: PtrString,
) -> *mut SimpleResponse<c_char> {
    let entropy = unsafe { slice::from_raw_parts(entropy, entropy_len as usize) };
    let path = recover_c_char(path);
    let passphrase = recover_c_char(passphrase);
    let extended_key =
        keystore::algorithms::ed25519::bip32_ed25519::get_extended_public_key_by_entropy(
            &entropy,
            passphrase.as_bytes(),
            &path,
        );
    match extended_key {
        Ok(result) => SimpleResponse::success(convert_c_char(result.encode_hex())).simple_c_ptr(),
        Err(e) => SimpleResponse::from(e).simple_c_ptr(),
    }
}

#[no_mangle]
pub extern "C" fn get_icarus_master_key(
    entropy: PtrBytes,
    entropy_len: u32,
    passphrase: PtrString,
) -> *mut SimpleResponse<c_char> {
    let entropy = unsafe { slice::from_raw_parts(entropy, entropy_len as usize) };
    let passphrase = recover_c_char(passphrase);
    let master_key = keystore::algorithms::ed25519::bip32_ed25519::get_icarus_master_key_by_entropy(
        &entropy,
        passphrase.as_bytes(),
    );
    match master_key {
        Ok(result) => SimpleResponse::success(convert_c_char(result.encode_hex())).simple_c_ptr(),
        Err(e) => SimpleResponse::from(e).simple_c_ptr(),
    }
}

#[no_mangle]
pub extern "C" fn derive_bip32_ed25519_extended_pubkey(
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
pub extern "C" fn k1_sign_message_hash_by_private_key(
    private_key: PtrBytes,
    message_hash: PtrBytes,
) -> *mut SimpleResponse<c_char> {
    let private_key_bytes = unsafe { slice::from_raw_parts(private_key, 32) };
    let message_hash_bytes = unsafe { slice::from_raw_parts(message_hash, 32) };
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
pub extern "C" fn k1_verify_signature(
    signature: PtrBytes,
    message_hash: PtrBytes,
    public_key: PtrBytes,
) -> bool {
    let signature_bytes = unsafe { slice::from_raw_parts(signature, 64) };
    let message_hash_bytes = unsafe { slice::from_raw_parts(message_hash, 32) };
    let public_key_bytes = unsafe { slice::from_raw_parts(public_key, 65) };
    let result = keystore::algorithms::secp256k1::verify_signature(
        signature_bytes,
        message_hash_bytes,
        public_key_bytes,
    );
    match result {
        Ok(data) => data,
        Err(_e) => false,
    }
}
