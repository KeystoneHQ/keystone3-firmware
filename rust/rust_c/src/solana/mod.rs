use crate::common::errors::RustCError;
use crate::common::structs::{SimpleResponse, TransactionCheckResult, TransactionParseResult};
use crate::common::types::{PtrBytes, PtrString, PtrT, PtrUR};
use crate::common::ur::{UREncodeResult, FRAGMENT_MAX_LENGTH_DEFAULT};
use crate::common::utils::{convert_c_char, recover_c_char};
use crate::{extract_array, extract_ptr_with_type};
use alloc::format;
use alloc::string::ToString;
use app_solana::errors::SolanaError;
use app_solana::parse_message;
use cty::c_char;
use structs::{DisplaySolanaMessage, DisplaySolanaTx};
use ur_registry::solana::sol_sign_request::SolSignRequest;
use ur_registry::solana::sol_signature::SolSignature;
use ur_registry::traits::RegistryItem;

pub mod structs;

unsafe fn build_sign_result(ptr: PtrUR, seed: &[u8]) -> Result<SolSignature, SolanaError> {
    let sign_request = extract_ptr_with_type!(ptr, SolSignRequest);
    let sign_data = sign_request.get_sign_data();
    let is_complete_transaction = match app_solana::classify_payload(&sign_data) {
        app_solana::SolanaPayloadType::Transaction => true,
        app_solana::SolanaPayloadType::Message => false,
        app_solana::SolanaPayloadType::MalformedTransaction => {
            return Err(SolanaError::InvalidData(
                "invalid or unsupported transaction data".to_string(),
            ));
        }
    };
    let mut path =
        sign_request
            .get_derivation_path()
            .get_path()
            .ok_or(SolanaError::InvalidData(
                "invalid derivation path".to_string(),
            ))?;
    if !path.starts_with("m/") {
        path = format!("m/{path}");
    }
    if is_complete_transaction {
        let signer = app_solana::get_public_key(seed, &path)?;
        app_solana::validate_tx_signer(&mut sign_data.clone(), &signer)?;
    }
    let signature = app_solana::sign(sign_data, &path, seed)?;
    Ok(SolSignature::new(
        sign_request.get_request_id(),
        signature.to_vec(),
    ))
}

#[no_mangle]
pub unsafe extern "C" fn solana_get_address(pubkey: PtrString) -> *mut SimpleResponse<c_char> {
    let x_pub = recover_c_char(pubkey);
    let address = app_solana::get_address(&x_pub);
    match address {
        Ok(result) => SimpleResponse::success(convert_c_char(result) as *mut c_char).simple_c_ptr(),
        Err(e) => SimpleResponse::from(e).simple_c_ptr(),
    }
}

#[no_mangle]
pub unsafe extern "C" fn solana_check(
    ptr: PtrUR,
    master_fingerprint: PtrBytes,
    length: u32,
) -> PtrT<TransactionCheckResult> {
    if length != 4 {
        return TransactionCheckResult::from(RustCError::InvalidMasterFingerprint).c_ptr();
    }
    let sol_sign_request = extract_ptr_with_type!(ptr, SolSignRequest);
    // A transaction with bytes appended after the serialized message must be
    // rejected during the scan/check phase.  Otherwise it passes the generic
    // fingerprint check and the UI opens the transaction detail page before
    // discovering the malformed payload during parsing/signing.
    if matches!(
        app_solana::classify_payload(&sol_sign_request.get_sign_data()),
        app_solana::SolanaPayloadType::MalformedTransaction
    ) {
        return TransactionCheckResult::from(SolanaError::InvalidData(
            "invalid or unsupported transaction data".to_string(),
        ))
        .c_ptr();
    }
    let mfp = extract_array!(master_fingerprint, u8, 4);
    if let Ok(mfp) = (mfp.try_into() as Result<[u8; 4], _>) {
        let derivation_path: ur_registry::crypto_key_path::CryptoKeyPath =
            sol_sign_request.get_derivation_path();
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
pub unsafe extern "C" fn solana_parse_tx(
    ptr: PtrUR,
) -> PtrT<TransactionParseResult<DisplaySolanaTx>> {
    let solan_sign_reqeust = extract_ptr_with_type!(ptr, SolSignRequest);
    let tx_hex = solan_sign_reqeust.get_sign_data();
    if matches!(
        app_solana::classify_payload(&tx_hex),
        app_solana::SolanaPayloadType::MalformedTransaction
    ) {
        return TransactionParseResult::from(SolanaError::InvalidData(
            "invalid or unsupported transaction data".to_string(),
        ))
        .c_ptr();
    }
    match app_solana::parse(&tx_hex.to_vec()) {
        Ok(v) => TransactionParseResult::success(DisplaySolanaTx::from(v).c_ptr()).c_ptr(),
        Err(e) => TransactionParseResult::from(e).c_ptr(),
    }
}

#[no_mangle]
pub unsafe extern "C" fn solana_parse_tx_with_pubkey(
    ptr: PtrUR,
    pubkey: PtrString,
) -> PtrT<TransactionParseResult<DisplaySolanaTx>> {
    let solan_sign_reqeust = extract_ptr_with_type!(ptr, SolSignRequest);
    let tx_hex = solan_sign_reqeust.get_sign_data();
    if matches!(
        app_solana::classify_payload(&tx_hex),
        app_solana::SolanaPayloadType::MalformedTransaction
    ) {
        return TransactionParseResult::from(SolanaError::InvalidData(
            "invalid or unsupported transaction data".to_string(),
        ))
        .c_ptr();
    }
    let pubkey = recover_c_char(pubkey);
    let signer: [u8; 32] = match hex::decode(pubkey)
        .ok()
        .and_then(|value| value.try_into().ok())
    {
        Some(value) => value,
        None => {
            return TransactionParseResult::from(SolanaError::InvalidData(
                "invalid Solana signer public key".to_string(),
            ))
            .c_ptr()
        }
    };
    match app_solana::parse_for_signer(&tx_hex.to_vec(), &signer) {
        Ok(v) => TransactionParseResult::success(DisplaySolanaTx::from(v).c_ptr()).c_ptr(),
        Err(e) => TransactionParseResult::from(e).c_ptr(),
    }
}

#[no_mangle]
// this function is used to sign the tx and message
pub unsafe extern "C" fn solana_sign_tx(
    ptr: PtrUR,
    seed: PtrBytes,
    seed_len: u32,
) -> PtrT<UREncodeResult> {
    let seed = extract_array!(seed, u8, seed_len as usize);
    build_sign_result(ptr, seed)
        .map(|v| v.try_into())
        .map_or_else(
            |e| UREncodeResult::from(e).c_ptr(),
            |v| {
                v.map_or_else(
                    |e| UREncodeResult::from(e).c_ptr(),
                    |data| {
                        UREncodeResult::encode(
                            data,
                            SolSignature::get_registry_type().get_type(),
                            FRAGMENT_MAX_LENGTH_DEFAULT,
                        )
                        .c_ptr()
                    },
                )
            },
        )
}

#[no_mangle]
pub unsafe extern "C" fn solana_parse_message(
    ptr: PtrUR,
    pubkey: PtrString,
) -> PtrT<TransactionParseResult<DisplaySolanaMessage>> {
    let sol_sign_request = extract_ptr_with_type!(ptr, SolSignRequest);
    let pubkey = recover_c_char(pubkey);
    match app_solana::classify_payload(&sol_sign_request.get_sign_data()) {
        app_solana::SolanaPayloadType::Message => {}
        app_solana::SolanaPayloadType::Transaction => {
            return TransactionParseResult::from(RustCError::UnsupportedTransaction(
                "Transaction".to_string(),
            ))
            .c_ptr();
        }
        app_solana::SolanaPayloadType::MalformedTransaction => {
            return TransactionParseResult::from(SolanaError::InvalidData(
                "invalid or unsupported transaction data".to_string(),
            ))
            .c_ptr();
        }
    }
    match parse_message(sol_sign_request.get_sign_data(), &pubkey.to_string()) {
        Ok(t) => TransactionParseResult::success(DisplaySolanaMessage::from(t).c_ptr()).c_ptr(),
        Err(e) => TransactionParseResult::from(e).c_ptr(),
    }
}

#[no_mangle]
pub unsafe extern "C" fn sol_get_path(ptr: PtrUR) -> PtrString {
    let sol_sign_request = extract_ptr_with_type!(ptr, SolSignRequest);
    let derivation_path = sol_sign_request.get_derivation_path();
    if let Some(path) = derivation_path.get_path() {
        return convert_c_char(path);
    }
    convert_c_char("".to_string())
}
