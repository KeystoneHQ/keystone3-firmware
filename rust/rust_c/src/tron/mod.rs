pub mod structs;

use crate::common::errors::{KeystoneError, RustCError};
use crate::common::keystone;
use crate::common::structs::{SimpleResponse, TransactionCheckResult, TransactionParseResult};
use crate::common::types::{PtrBytes, PtrString, PtrT, PtrUR};
use crate::common::ur::{QRCodeType, UREncodeResult};
use crate::common::utils::{convert_c_char, recover_c_char};
use crate::extract_array;
use alloc::boxed::Box;
use alloc::slice;
use alloc::string::{String, ToString};
use alloc::vec::Vec;
use cty::c_char;
use structs::{DisplayTron, TransactionType};

use crate::extract_ptr_with_type;
use alloc::format;
use keystore::algorithms::secp256k1::derive_public_key;
use structs::DisplayTRONPersonalMessage;
use ur_registry::traits::{RegistryItem, To};
use ur_registry::tron::tron_sign_request::TronSignRequest;
use ur_registry::tron::tron_signature::TronSignature;

const TRON_DEFAULT_PATH: &str = "m/44'/195'/0'/0/0";

#[no_mangle]
pub unsafe extern "C" fn tron_check_sign_request(
    ptr: PtrUR,
    x_pub: PtrString,
    master_fingerprint: PtrBytes,
    length: u32,
) -> PtrT<TransactionCheckResult> {
    if length != 4 {
        return TransactionCheckResult::from(RustCError::InvalidMasterFingerprint).c_ptr();
    }
    let req = extract_ptr_with_type!(ptr, TronSignRequest);
    let mfp = extract_array!(master_fingerprint, u8, 4);
    if let Ok(mfp) = (mfp.try_into() as Result<[u8; 4], _>) {
        let derivation_path = req.get_derivation_path();
        if let Some(ur_mfp) = derivation_path.get_source_fingerprint() {
            if mfp != ur_mfp {
                return TransactionCheckResult::from(RustCError::MasterFingerprintMismatch).c_ptr();
            }
        } else {
            return TransactionCheckResult::from(RustCError::MasterFingerprintMismatch).c_ptr();
        }
    } else {
        return TransactionCheckResult::from(RustCError::InvalidMasterFingerprint).c_ptr();
    }

    let x_pub_recovered = recover_c_char(x_pub);
    let xpub_str = x_pub_recovered.as_str();
    let sign_data = req.get_sign_data();
    let path = match req.get_derivation_path().get_path() {
        Some(p) => p,
        None => return TransactionCheckResult::from(RustCError::InvalidHDPath).c_ptr(),
    };

    let transaction_type = TransactionType::from(req.get_data_type());
    match transaction_type {
        TransactionType::Transaction => {
            match app_tron::check_tx_request(&sign_data, &path, xpub_str) {
                Ok(_) => TransactionCheckResult::new().c_ptr(),
                Err(e) => TransactionCheckResult::from(e).c_ptr(),
            }
        }
        TransactionType::PersonalMessage => TransactionCheckResult::new().c_ptr(),
    }
}

#[no_mangle]
pub unsafe extern "C" fn tron_parse_sign_request(
    ptr: PtrUR,
) -> *mut TransactionParseResult<DisplayTron> {
    let req = extract_ptr_with_type!(ptr, TronSignRequest);
    let json_bytes = req.get_sign_data();
    let path = req
        .get_derivation_path()
        .get_path()
        .unwrap_or_else(|| String::from(TRON_DEFAULT_PATH));

    app_tron::parse_tx_request(&json_bytes, &path).map_or_else(
        |e| TransactionParseResult::from(e).c_ptr(),
        |parsed_tx| {
            let display_tx = DisplayTron::from(parsed_tx);

            TransactionParseResult::success(Box::into_raw(Box::new(display_tx))).c_ptr()
        },
    )
}

#[no_mangle]
pub unsafe extern "C" fn tron_sign_request(
    ptr: PtrUR,
    seed: PtrBytes,
    seed_len: u32,
    fragment_len: usize,
) -> *mut UREncodeResult {
    let req = extract_ptr_with_type!(ptr, TronSignRequest);
    let seed_slice = extract_array!(seed, u8, seed_len as usize);

    let sign_res = (|| -> Result<Vec<u8>, KeystoneError> {
        let sign_data = req.get_sign_data();
        let request_id = req.get_request_id();
        let path = req
            .get_derivation_path()
            .get_path()
            .unwrap_or_else(|| String::from(TRON_DEFAULT_PATH));

        let signed_tx_hex = match TransactionType::from(req.get_data_type()) {
            TransactionType::Transaction => {
                app_tron::sign_tx_request(&sign_data, &path, seed_slice)
                    .map_err(|e| KeystoneError::SignTxFailed(e.to_string()))?
            }
            TransactionType::PersonalMessage => {
                app_tron::sign_personal_message(&sign_data, &path, seed_slice)
                    .map_err(|e| KeystoneError::SignTxFailed(e.to_string()))?
            }
        };

        let signed_tx_bytes = hex::decode(signed_tx_hex)
            .map_err(|_| KeystoneError::SignTxFailed("Invalid Hex output".to_string()))?;
        let sig_obj = TronSignature::new(request_id, signed_tx_bytes);
        sig_obj
            .to_bytes()
            .map_err(|e| KeystoneError::SignTxFailed(e.to_string()))
    })();

    match sign_res {
        Ok(data) => UREncodeResult::encode(
            data,
            TronSignature::get_registry_type().get_type(),
            fragment_len,
        )
        .c_ptr(),
        Err(e) => UREncodeResult::from(e).c_ptr(),
    }
}

#[no_mangle]
pub unsafe extern "C" fn tron_check_keystone(
    ptr: PtrUR,
    ur_type: QRCodeType,
    master_fingerprint: PtrBytes,
    length: u32,
    x_pub: PtrString,
) -> PtrT<TransactionCheckResult> {
    keystone::check(ptr, ur_type, master_fingerprint, length, x_pub)
}

#[no_mangle]
pub unsafe extern "C" fn tron_parse_keystone(
    ptr: PtrUR,
    ur_type: QRCodeType,
    master_fingerprint: PtrBytes,
    length: u32,
    x_pub: PtrString,
) -> *mut TransactionParseResult<DisplayTron> {
    if length != 4 {
        return TransactionParseResult::from(RustCError::InvalidMasterFingerprint).c_ptr();
    }
    keystone::build_payload(ptr, ur_type).map_or_else(
        |e| TransactionParseResult::from(e).c_ptr(),
        |payload| {
            keystone::build_parse_context(master_fingerprint, x_pub).map_or_else(
                |e| TransactionParseResult::from(e).c_ptr(),
                |context| {
                    app_tron::parse_raw_tx(payload, context).map_or_else(
                        |e| TransactionParseResult::from(e).c_ptr(),
                        |res| {
                            TransactionParseResult::success(Box::into_raw(Box::new(
                                DisplayTron::from(res),
                            )))
                            .c_ptr()
                        },
                    )
                },
            )
        },
    )
}

#[no_mangle]
pub unsafe extern "C" fn tron_sign_keystone(
    ptr: PtrUR,
    ur_type: QRCodeType,
    master_fingerprint: PtrBytes,
    length: u32,
    x_pub: PtrString,
    cold_version: i32,
    seed: PtrBytes,
    seed_len: u32,
) -> *mut UREncodeResult {
    let seed = extract_array!(seed, u8, seed_len as usize);
    keystone::sign(
        ptr,
        ur_type,
        master_fingerprint,
        length,
        x_pub,
        cold_version,
        seed,
    )
}

#[no_mangle]
pub unsafe extern "C" fn tron_get_address(
    hd_path: PtrString,
    x_pub: PtrString,
) -> *mut SimpleResponse<c_char> {
    let x_pub = recover_c_char(x_pub);
    let hd_path = recover_c_char(hd_path);
    let address = app_tron::get_address(hd_path, &x_pub);
    match address {
        Ok(result) => SimpleResponse::success(convert_c_char(result) as *mut c_char).simple_c_ptr(),
        Err(e) => SimpleResponse::from(e).simple_c_ptr(),
    }
}

fn parse_trx_sub_path(path: String) -> Option<String> {
    let root_paths = ["m/44'/195'/", "44'/195'/", "m/44'/194'/", "44'/194'/"];

    root_paths.iter().find_map(|root| {
        path.strip_prefix(root).and_then(|remaining| {
            remaining
                .find('/')
                .map(|index| remaining[index + 1..].to_string())
        })
    })
}

fn try_get_trx_public_key(
    xpub: String,
    trx_sign_request: &TronSignRequest,
) -> Result<bitcoin::secp256k1::PublicKey, RustCError> {
    match trx_sign_request.get_derivation_path().get_path() {
        None => Err(RustCError::InvalidHDPath),
        Some(path) => {
            if let Some(sub_path) = parse_trx_sub_path(path.clone()) {
                derive_public_key(&xpub, &format!("m/{sub_path}")).map_err(|_e| {
                    RustCError::UnexpectedError("unable to derive TRX pubkey".to_string())
                })
            } else {
                Err(RustCError::InvalidHDPath)
            }
        }
    }
}

#[no_mangle]
pub unsafe extern "C" fn tron_parse_personal_message(
    ptr: PtrUR,
    xpub: PtrString,
) -> PtrT<TransactionParseResult<DisplayTRONPersonalMessage>> {
    let crypto_trx = extract_ptr_with_type!(ptr, TronSignRequest);
    let xpub = recover_c_char(xpub);

    let pubkey = try_get_trx_public_key(xpub, crypto_trx).ok();
    let transaction_type = TransactionType::from(crypto_trx.get_data_type());

    match transaction_type {
        TransactionType::PersonalMessage => {
            match app_tron::parse_personal_message(&crypto_trx.get_sign_data(), pubkey) {
                Ok(tx) => {
                    TransactionParseResult::success(DisplayTRONPersonalMessage::from(tx).c_ptr())
                        .c_ptr()
                }
                Err(e) => TransactionParseResult::from(e).c_ptr(),
            }
        }
        _ => TransactionParseResult::from(RustCError::UnsupportedTransaction(
            "TypedTransaction or TypedData".to_string(),
        ))
        .c_ptr(),
    }
}
