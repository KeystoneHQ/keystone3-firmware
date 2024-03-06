use alloc::{slice, string::{String, ToString}};
use common_rust_c::{errors::RustCError, extract_ptr_with_type, structs::{TransactionCheckResult, TransactionParseResult}, types::{PtrBytes, PtrT, PtrUR}, ur::{UREncodeResult, URType, FRAGMENT_MAX_LENGTH_DEFAULT}, utils::convert_c_char};
use third_party::ur_registry::bitcoin::btc_sign_request::{BtcSignRequest, DataType};
use third_party::ur_registry::bitcoin::btc_signature::BtcSignature;
use third_party::ur_registry::traits::RegistryItem;
use crate::structs::DisplayBtcMsg;

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
        let q_mfp = req.get_derivation_paths()[0].get_source_fingerprint();
        if let Some(q_mfp) = q_mfp {
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
        let q_mfp = req.get_derivation_paths()[0].get_source_fingerprint();
        let seed = alloc::slice::from_raw_parts(seed, seed_len as usize);
        if let Some(q_mfp) = q_mfp {
            if q_mfp.eq(mfp) {
                match req.get_data_type() {
                    DataType::Message => {
                        let msg = req.get_sign_data();
                        if let Some(path) = req.get_derivation_paths()[0].get_path() {
                            if let Ok(sig) = app_bitcoin::sign_msg(msg, seed, &path) {
                                return UREncodeResult::encode(sig, BtcSignature::get_registry_type().get_type(), FRAGMENT_MAX_LENGTH_DEFAULT.clone()).c_ptr();
                            } else {
                                return UREncodeResult::from(RustCError::UnexpectedError("failed to sign".to_string())).c_ptr();
                            }
                        } else {
                            return UREncodeResult::from(RustCError::InvalidHDPath).c_ptr();
                        }
                    }
                }
            }
        }
    }

    UREncodeResult::from(RustCError::MasterFingerprintMismatch).c_ptr()
}
