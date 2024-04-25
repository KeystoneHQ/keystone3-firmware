use crate::structs::DisplayBtcMsg;
use alloc::{
    slice,
    string::{String, ToString},
    vec::Vec,
};
use common_rust_c::{
    errors::RustCError,
    extract_ptr_with_type,
    structs::{TransactionCheckResult, TransactionParseResult},
    types::{PtrBytes, PtrT, PtrUR},
    ur::{UREncodeResult, FRAGMENT_MAX_LENGTH_DEFAULT},
    utils::convert_c_char,
};
use keystore::algorithms::secp256k1;
use third_party::ur_registry::bitcoin::btc_sign_request::{BtcSignRequest, DataType};
use third_party::ur_registry::bitcoin::btc_signature::BtcSignature;
use third_party::ur_registry::traits::RegistryItem;

#[no_mangle]
pub extern "C" fn btc_check_msg(
    ptr: PtrUR,
    master_fingerprint: PtrBytes,
    length: u32,
) -> PtrT<TransactionCheckResult> {
    if length != 4 {
        return TransactionCheckResult::from(RustCError::InvalidMasterFingerprint).c_ptr();
    }
    let mfp = unsafe { slice::from_raw_parts(master_fingerprint, 4) };
    let sign_request = extract_ptr_with_type!(ptr, BtcSignRequest);
    let ur_mfp = sign_request.get_derivation_paths()[0].get_source_fingerprint();

    if let Ok(mfp) = mfp.try_into() as Result<[u8; 4], _> {
        if let Some(ur_mfp) = ur_mfp {
            return if mfp == ur_mfp {
                TransactionCheckResult::new().c_ptr()
            } else {
                TransactionCheckResult::from(RustCError::MasterFingerprintMismatch).c_ptr()
            };
        }
        TransactionCheckResult::from(RustCError::MasterFingerprintMismatch).c_ptr()
    } else {
        TransactionCheckResult::from(RustCError::InvalidMasterFingerprint).c_ptr()
    }
}

#[no_mangle]
pub extern "C" fn btc_parse_msg(
    ptr: PtrUR,
    master_fingerprint: PtrBytes,
    length: u32,
) -> *mut TransactionParseResult<DisplayBtcMsg> {
    if length != 4 {
        return TransactionParseResult::from(RustCError::InvalidMasterFingerprint).c_ptr();
    }
    let req = extract_ptr_with_type!(ptr, BtcSignRequest);
    unsafe {
        let mfp = alloc::slice::from_raw_parts(master_fingerprint, 4);
        if let Some(q_mfp) = req.get_derivation_paths()[0].get_source_fingerprint() {
            if q_mfp.eq(mfp) {
                match req.get_data_type() {
                    DataType::Message => {
                        let msg = req.get_sign_data();
                        if let Ok(msg_uft8) = String::from_utf8(msg.to_vec()) {
                            let display_msg = DisplayBtcMsg {
                                detail: convert_c_char(msg_uft8),
                            };
                            return TransactionParseResult::success(display_msg.c_ptr()).c_ptr();
                        }
                    }
                }
            }
        }
        TransactionParseResult::from(RustCError::MasterFingerprintMismatch).c_ptr()
    }
}

#[no_mangle]
pub extern "C" fn btc_sign_msg(
    ptr: PtrUR,
    seed: PtrBytes,
    seed_len: u32,
    master_fingerprint: PtrBytes,
    master_fingerprint_len: u32,
) -> *mut UREncodeResult {
    if master_fingerprint_len != 4 {
        return UREncodeResult::from(RustCError::InvalidMasterFingerprint).c_ptr();
    }
    let req = extract_ptr_with_type!(ptr, BtcSignRequest);
    unsafe {
        let mfp = alloc::slice::from_raw_parts(master_fingerprint, 4);
        let seed = alloc::slice::from_raw_parts(seed, seed_len as usize);
        if let Some(q_mfp) = req.get_derivation_paths()[0].get_source_fingerprint() {
            if q_mfp.eq(mfp) {
                match req.get_data_type() {
                    DataType::Message => {
                        let msg_utf8 = String::from_utf8_unchecked(req.get_sign_data().to_vec());
                        if let Some(path) = req.get_derivation_paths()[0].get_path() {
                            if let Ok(sig) = app_bitcoin::sign_msg(msg_utf8.as_str(), seed, &path) {
                                if let Ok(extended_key) =
                                    secp256k1::get_extended_public_key_by_seed(seed, &path)
                                {
                                    let btc_signature = BtcSignature::new(
                                        req.get_request_id(),
                                        sig,
                                        extended_key.to_pub().to_bytes(),
                                    );
                                    let data: Vec<u8> = match btc_signature.try_into() {
                                        Ok(v) => v,
                                        Err(e) => return UREncodeResult::from(e).c_ptr(),
                                    };
                                    return UREncodeResult::encode(
                                        data,
                                        BtcSignature::get_registry_type().get_type(),
                                        FRAGMENT_MAX_LENGTH_DEFAULT.clone(),
                                    )
                                    .c_ptr();
                                }
                            }
                            return UREncodeResult::from(RustCError::UnexpectedError(
                                "failed to sign".to_string(),
                            ))
                            .c_ptr();
                        }
                        return UREncodeResult::from(RustCError::InvalidHDPath).c_ptr();
                    }
                }
            }
        }
    }
    UREncodeResult::from(RustCError::MasterFingerprintMismatch).c_ptr()
}
