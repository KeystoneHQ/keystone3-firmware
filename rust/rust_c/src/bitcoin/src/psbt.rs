use alloc::boxed::Box;
use alloc::collections::BTreeMap;
use alloc::slice;
use alloc::string::{String, ToString};
use alloc::vec::Vec;
use app_bitcoin::multi_sig::wallet::parse_wallet_config;
use core::ptr::null_mut;
use core::str::FromStr;

use app_bitcoin::parsed_tx::ParseContext;
use app_bitcoin::{self, parse_psbt_hex_sign_status, parse_psbt_sign_status};
use common_rust_c::errors::RustCError;
use common_rust_c::extract_ptr_with_type;
use common_rust_c::ffi::{CSliceFFI, VecFFI};
use common_rust_c::structs::{
    ExtendedPublicKey, Response, TransactionCheckResult, TransactionParseResult,
};
use common_rust_c::types::{Ptr, PtrBytes, PtrString, PtrT, PtrUR};
use common_rust_c::ur::{UREncodeResult, FRAGMENT_MAX_LENGTH_DEFAULT, FRAGMENT_UNLIMITED_LENGTH};
use common_rust_c::utils::{convert_c_char, recover_c_array, recover_c_char};
use third_party::bitcoin::bip32::{DerivationPath, Xpub};
use third_party::hex;
use third_party::ur_registry::crypto_psbt::CryptoPSBT;
use third_party::ur_registry::traits::RegistryItem;

use crate::multi_sig::structs::MultisigSignResult;
use crate::structs::DisplayTx;

#[no_mangle]
pub extern "C" fn btc_parse_psbt(
    ptr: PtrUR,
    master_fingerprint: PtrBytes,
    length: u32,
    public_keys: PtrT<CSliceFFI<ExtendedPublicKey>>,
    multisig_wallet_config: PtrString,
) -> *mut TransactionParseResult<DisplayTx> {
    if length != 4 {
        return TransactionParseResult::from(RustCError::InvalidMasterFingerprint).c_ptr();
    }
    let crypto_psbt = extract_ptr_with_type!(ptr, CryptoPSBT);
    let psbt = crypto_psbt.get_psbt();

    unsafe {
        let multisig_wallet_config = if multisig_wallet_config.is_null() {
            None
        } else {
            Some(recover_c_char(multisig_wallet_config))
        };
        let mfp = core::slice::from_raw_parts(master_fingerprint, 4);
        let public_keys = recover_c_array(public_keys);
        parse_psbt(mfp, public_keys, psbt, multisig_wallet_config)
    }
}

#[no_mangle]
fn btc_sign_psbt_dynamic(
    ptr: PtrUR,
    seed: PtrBytes,
    seed_len: u32,
    master_fingerprint: PtrBytes,
    master_fingerprint_len: u32,
    fragment_length: usize,
) -> *mut UREncodeResult {
    if master_fingerprint_len != 4 {
        return UREncodeResult::from(RustCError::InvalidMasterFingerprint).c_ptr();
    }
    let master_fingerprint = unsafe { core::slice::from_raw_parts(master_fingerprint, 4) };
    let master_fingerprint = match third_party::bitcoin::bip32::Fingerprint::from_str(
        hex::encode(master_fingerprint.to_vec()).as_str(),
    )
    .map_err(|_e| RustCError::InvalidMasterFingerprint)
    {
        Ok(mfp) => mfp,
        Err(e) => {
            return UREncodeResult::from(e).c_ptr();
        }
    };

    let crypto_psbt = extract_ptr_with_type!(ptr, CryptoPSBT);
    let psbt = crypto_psbt.get_psbt();

    let seed = unsafe { slice::from_raw_parts(seed, seed_len as usize) };

    let result = app_bitcoin::sign_psbt(psbt, seed, master_fingerprint);
    match result.map(|v| CryptoPSBT::new(v).try_into()) {
        Ok(v) => match v {
            Ok(data) => UREncodeResult::encode(
                data,
                CryptoPSBT::get_registry_type().get_type(),
                fragment_length.clone(),
            )
            .c_ptr(),
            Err(e) => UREncodeResult::from(e).c_ptr(),
        },
        Err(e) => UREncodeResult::from(e).c_ptr(),
    }
}

#[no_mangle]
pub extern "C" fn btc_sign_psbt(
    ptr: PtrUR,
    seed: PtrBytes,
    seed_len: u32,
    master_fingerprint: PtrBytes,
    master_fingerprint_len: u32,
) -> *mut UREncodeResult {
    btc_sign_psbt_dynamic(
        ptr,
        seed,
        seed_len,
        master_fingerprint,
        master_fingerprint_len,
        FRAGMENT_MAX_LENGTH_DEFAULT.clone(),
    )
}

#[no_mangle]
pub extern "C" fn btc_sign_psbt_unlimited(
    ptr: PtrUR,
    seed: PtrBytes,
    seed_len: u32,
    master_fingerprint: PtrBytes,
    master_fingerprint_len: u32,
) -> *mut UREncodeResult {
    btc_sign_psbt_dynamic(
        ptr,
        seed,
        seed_len,
        master_fingerprint,
        master_fingerprint_len,
        FRAGMENT_UNLIMITED_LENGTH.clone(),
    )
}

#[no_mangle]
pub extern "C" fn btc_sign_multisig_psbt(
    ptr: PtrUR,
    seed: PtrBytes,
    seed_len: u32,
    master_fingerprint: PtrBytes,
    master_fingerprint_len: u32,
) -> *mut MultisigSignResult {
    if master_fingerprint_len != 4 {
        return MultisigSignResult {
            ur_result: UREncodeResult::from(RustCError::InvalidMasterFingerprint).c_ptr(),
            sign_status: null_mut(),
            is_completed: false,
            psbt_hex: null_mut(),
            psbt_len: 0,
        }
        .c_ptr();
    }
    let master_fingerprint = unsafe { core::slice::from_raw_parts(master_fingerprint, 4) };
    let master_fingerprint = match third_party::bitcoin::bip32::Fingerprint::from_str(
        hex::encode(master_fingerprint.to_vec()).as_str(),
    )
    .map_err(|_e| RustCError::InvalidMasterFingerprint)
    {
        Ok(mfp) => mfp,
        Err(e) => {
            return MultisigSignResult {
                ur_result: UREncodeResult::from(e).c_ptr(),
                sign_status: null_mut(),
                is_completed: false,
                psbt_hex: null_mut(),
                psbt_len: 0,
            }
            .c_ptr();
        }
    };

    let crypto_psbt = extract_ptr_with_type!(ptr, CryptoPSBT);
    let psbt = crypto_psbt.get_psbt();

    let seed = unsafe { slice::from_raw_parts(seed, seed_len as usize) };

    let result = app_bitcoin::sign_psbt_no_serialize(psbt, seed, master_fingerprint);
    match result.map(|v| {
        let buf = v.serialize();
        let sign_state = parse_psbt_sign_status(v);
        CryptoPSBT::new(buf.clone())
            .try_into()
            .map(|v| (sign_state, v, buf))
    }) {
        Ok(v) => match v {
            Ok((sign_state, data, psbt_hex)) => {
                let (ptr, size, _cap) = psbt_hex.into_raw_parts();
                MultisigSignResult {
                    ur_result: UREncodeResult::encode(
                        data,
                        CryptoPSBT::get_registry_type().get_type(),
                        FRAGMENT_MAX_LENGTH_DEFAULT.clone(),
                    )
                    .c_ptr(),
                    sign_status: convert_c_char(sign_state.sign_status.unwrap_or("".to_string())),
                    is_completed: sign_state.is_completed,
                    psbt_hex: ptr,
                    psbt_len: size as u32,
                }
                .c_ptr()
            }
            Err(e) => MultisigSignResult {
                ur_result: UREncodeResult::from(e).c_ptr(),
                sign_status: null_mut(),
                is_completed: false,
                psbt_hex: null_mut(),
                psbt_len: 0,
            }
            .c_ptr(),
        },
        Err(e) => MultisigSignResult {
            ur_result: UREncodeResult::from(e).c_ptr(),
            sign_status: null_mut(),
            is_completed: false,
            psbt_hex: null_mut(),
            psbt_len: 0,
        }
        .c_ptr(),
    }
}

#[no_mangle]
pub extern "C" fn btc_export_multisig_psbt(ptr: PtrUR) -> *mut MultisigSignResult {
    let crypto_psbt = extract_ptr_with_type!(ptr, CryptoPSBT);
    let psbt = crypto_psbt.get_psbt();
    let sign_state = parse_psbt_hex_sign_status(&psbt);
    match sign_state {
        Ok(state) => {
            let (ptr, size, _cap) = psbt.clone().into_raw_parts();
            MultisigSignResult {
                ur_result: UREncodeResult::encode(
                    psbt,
                    CryptoPSBT::get_registry_type().get_type(),
                    FRAGMENT_MAX_LENGTH_DEFAULT.clone(),
                )
                .c_ptr(),
                sign_status: convert_c_char(state.sign_status.unwrap_or("".to_string())),
                is_completed: state.is_completed,
                psbt_hex: ptr,
                psbt_len: size as u32,
            }
            .c_ptr()
        }
        Err(e) => MultisigSignResult {
            ur_result: UREncodeResult::from(e).c_ptr(),
            sign_status: null_mut(),
            is_completed: false,
            psbt_hex: null_mut(),
            psbt_len: 0,
        }
        .c_ptr(),
    }
}

#[no_mangle]
pub extern "C" fn btc_export_multisig_psbt_bytes(
    psbt_bytes: PtrBytes,
    psbt_bytes_length: u32,
) -> *mut MultisigSignResult {
    unsafe {
        let psbt = core::slice::from_raw_parts(psbt_bytes, psbt_bytes_length as usize);
        let psbt = psbt.to_vec();
        let sign_state = parse_psbt_hex_sign_status(&psbt);
        match sign_state {
            Ok(state) => {
                let (ptr, size, _cap) = psbt.clone().into_raw_parts();
                MultisigSignResult {
                    ur_result: UREncodeResult::encode(
                        psbt,
                        CryptoPSBT::get_registry_type().get_type(),
                        FRAGMENT_MAX_LENGTH_DEFAULT.clone(),
                    )
                    .c_ptr(),
                    sign_status: convert_c_char(state.sign_status.unwrap_or("".to_string())),
                    is_completed: state.is_completed,
                    psbt_hex: ptr,
                    psbt_len: size as u32,
                }
                .c_ptr()
            }
            Err(e) => MultisigSignResult {
                ur_result: UREncodeResult::from(e).c_ptr(),
                sign_status: null_mut(),
                is_completed: false,
                psbt_hex: null_mut(),
                psbt_len: 0,
            }
            .c_ptr(),
        }
    }
}

#[no_mangle]
pub extern "C" fn btc_check_psbt(
    ptr: PtrUR,
    master_fingerprint: PtrBytes,
    length: u32,
    public_keys: PtrT<CSliceFFI<ExtendedPublicKey>>,
    verify_code: PtrString,
    multisig_wallet_config: PtrString,
) -> PtrT<TransactionCheckResult> {
    if length != 4 {
        return TransactionCheckResult::from(RustCError::InvalidMasterFingerprint).c_ptr();
    }
    let crypto_psbt = extract_ptr_with_type!(ptr, CryptoPSBT);
    let psbt = crypto_psbt.get_psbt();

    unsafe {
        let verify_code = if verify_code.is_null() {
            None
        } else {
            Some(recover_c_char(verify_code))
        };
        let multisig_wallet_config = if multisig_wallet_config.is_null() {
            None
        } else {
            Some(recover_c_char(multisig_wallet_config))
        };
        let mfp = core::slice::from_raw_parts(master_fingerprint, 4);
        let public_keys = recover_c_array(public_keys);
        check_psbt(mfp, public_keys, psbt, verify_code, multisig_wallet_config)
    }
}

#[no_mangle]
pub extern "C" fn btc_check_psbt_bytes(
    psbt_bytes: PtrBytes,
    psbt_bytes_length: u32,
    master_fingerprint: PtrBytes,
    length: u32,
    public_keys: PtrT<CSliceFFI<ExtendedPublicKey>>,
    verify_code: PtrString,
    multisig_wallet_config: PtrString,
) -> PtrT<TransactionCheckResult> {
    if length != 4 {
        return TransactionCheckResult::from(RustCError::InvalidMasterFingerprint).c_ptr();
    }
    unsafe {
        let psbt = core::slice::from_raw_parts(psbt_bytes, psbt_bytes_length as usize);
        let psbt = match get_psbt_bytes(psbt) {
            Ok(psbt) => psbt,
            Err(e) => return TransactionCheckResult::from(e).c_ptr(),
        };

        let verify_code = if verify_code.is_null() {
            None
        } else {
            Some(recover_c_char(verify_code))
        };

        let multisig_wallet_config = if multisig_wallet_config.is_null() {
            None
        } else {
            Some(recover_c_char(multisig_wallet_config))
        };
        let mfp = core::slice::from_raw_parts(master_fingerprint, 4);
        let public_keys = recover_c_array(public_keys);
        check_psbt(mfp, public_keys, psbt, verify_code, multisig_wallet_config)
    }
}

#[no_mangle]
pub extern "C" fn btc_parse_psbt_bytes(
    psbt_bytes: PtrBytes,
    psbt_bytes_length: u32,
    master_fingerprint: PtrBytes,
    length: u32,
    public_keys: PtrT<CSliceFFI<ExtendedPublicKey>>,
    multisig_wallet_config: PtrString,
) -> *mut TransactionParseResult<DisplayTx> {
    if length != 4 {
        return TransactionParseResult::from(RustCError::InvalidMasterFingerprint).c_ptr();
    }
    unsafe {
        let psbt = core::slice::from_raw_parts(psbt_bytes, psbt_bytes_length as usize);
        let psbt = match get_psbt_bytes(psbt) {
            Ok(psbt) => psbt,
            Err(e) => return TransactionParseResult::from(e).c_ptr(),
        };
        let multisig_wallet_config = if multisig_wallet_config.is_null() {
            None
        } else {
            Some(recover_c_char(multisig_wallet_config))
        };
        let mfp = core::slice::from_raw_parts(master_fingerprint, 4);
        let public_keys = recover_c_array(public_keys);
        parse_psbt(mfp, public_keys, psbt, multisig_wallet_config)
    }
}

#[no_mangle]
pub extern "C" fn btc_sign_multisig_psbt_bytes(
    psbt_bytes: PtrBytes,
    psbt_bytes_length: u32,
    seed: PtrBytes,
    seed_len: u32,
    master_fingerprint: PtrBytes,
    master_fingerprint_len: u32,
) -> *mut MultisigSignResult {
    if master_fingerprint_len != 4 {
        return MultisigSignResult {
            ur_result: UREncodeResult::from(RustCError::InvalidMasterFingerprint).c_ptr(),
            sign_status: null_mut(),
            is_completed: false,
            psbt_hex: null_mut(),
            psbt_len: 0,
        }
        .c_ptr();
    }
    let master_fingerprint = unsafe { core::slice::from_raw_parts(master_fingerprint, 4) };
    let master_fingerprint = match third_party::bitcoin::bip32::Fingerprint::from_str(
        hex::encode(master_fingerprint.to_vec()).as_str(),
    )
    .map_err(|_e| RustCError::InvalidMasterFingerprint)
    {
        Ok(mfp) => mfp,
        Err(e) => {
            return MultisigSignResult {
                ur_result: UREncodeResult::from(e).c_ptr(),
                sign_status: null_mut(),
                is_completed: false,
                psbt_hex: null_mut(),
                psbt_len: 0,
            }
            .c_ptr();
        }
    };

    let psbt = unsafe {
        let psbt = core::slice::from_raw_parts(psbt_bytes, psbt_bytes_length as usize);
        let psbt = match get_psbt_bytes(psbt) {
            Ok(psbt) => psbt,
            Err(e) => {
                return MultisigSignResult {
                    ur_result: UREncodeResult::from(e).c_ptr(),
                    sign_status: null_mut(),
                    is_completed: false,
                    psbt_hex: null_mut(),
                    psbt_len: 0,
                }
                .c_ptr()
            }
        };
        psbt
    };

    let seed = unsafe { slice::from_raw_parts(seed, seed_len as usize) };

    let result = app_bitcoin::sign_psbt_no_serialize(psbt, seed, master_fingerprint);
    match result.map(|v| {
        let buf = v.serialize();
        let sign_state = parse_psbt_sign_status(v);
        CryptoPSBT::new(buf.clone())
            .try_into()
            .map(|v| (sign_state, v, buf))
    }) {
        Ok(v) => match v {
            Ok((sign_state, data, psbt_hex)) => {
                let (ptr, size, _cap) = psbt_hex.into_raw_parts();
                MultisigSignResult {
                    ur_result: UREncodeResult::encode(
                        data,
                        CryptoPSBT::get_registry_type().get_type(),
                        FRAGMENT_MAX_LENGTH_DEFAULT.clone(),
                    )
                    .c_ptr(),
                    sign_status: convert_c_char(sign_state.sign_status.unwrap_or("".to_string())),
                    is_completed: sign_state.is_completed,
                    psbt_hex: ptr,
                    psbt_len: size as u32,
                }
                .c_ptr()
            }
            Err(e) => MultisigSignResult {
                ur_result: UREncodeResult::from(e).c_ptr(),
                sign_status: null_mut(),
                is_completed: false,
                psbt_hex: null_mut(),
                psbt_len: 0,
            }
            .c_ptr(),
        },
        Err(e) => MultisigSignResult {
            ur_result: UREncodeResult::from(e).c_ptr(),
            sign_status: null_mut(),
            is_completed: false,
            psbt_hex: null_mut(),
            psbt_len: 0,
        }
        .c_ptr(),
    }
}

fn parse_psbt(
    mfp: &[u8],
    public_keys: &[ExtendedPublicKey],
    psbt: Vec<u8>,
    multisig_wallet_config: Option<String>,
) -> *mut TransactionParseResult<DisplayTx> {
    let master_fingerprint =
        third_party::bitcoin::bip32::Fingerprint::from_str(hex::encode(mfp.to_vec()).as_str())
            .map_err(|_e| RustCError::InvalidMasterFingerprint);
    match master_fingerprint {
        Ok(fp) => {
            let mut keys = BTreeMap::new();
            for x in public_keys {
                let xpub = recover_c_char(x.xpub);
                let path = recover_c_char(x.path);
                let extended_public_key =
                    Xpub::from_str(xpub.as_str()).map_err(|_e| RustCError::InvalidXPub);
                let derivation_path =
                    DerivationPath::from_str(path.as_str()).map_err(|_e| RustCError::InvalidHDPath);
                match extended_public_key.and_then(|k| derivation_path.and_then(|p| Ok((k, p)))) {
                    Ok((k, p)) => {
                        keys.insert(p, k);
                    }
                    Err(e) => {
                        return TransactionParseResult::from(e).c_ptr();
                    }
                }
            }
            let wallet_config = match multisig_wallet_config
                .map(|v| parse_wallet_config(&v, &fp.to_string()))
                .transpose()
            {
                Ok(t) => t,
                Err(e) => return TransactionParseResult::from(e).c_ptr(),
            };
            let context = ParseContext::new(fp, keys, None, wallet_config);
            let parsed_psbt = app_bitcoin::parse_psbt(psbt, context);
            match parsed_psbt {
                Ok(res) => {
                    TransactionParseResult::success(Box::into_raw(Box::new(DisplayTx::from(res))))
                        .c_ptr()
                }
                Err(e) => TransactionParseResult::from(e).c_ptr(),
            }
        }
        Err(e) => TransactionParseResult::from(e).c_ptr(),
    }
}

fn check_psbt(
    mfp: &[u8],
    public_keys: &[ExtendedPublicKey],
    psbt: Vec<u8>,
    verify_code: Option<String>,
    multisig_wallet_config: Option<String>,
) -> PtrT<TransactionCheckResult> {
    let master_fingerprint =
        third_party::bitcoin::bip32::Fingerprint::from_str(hex::encode(mfp.to_vec()).as_str())
            .map_err(|_e| RustCError::InvalidMasterFingerprint);
    match master_fingerprint {
        Ok(fp) => {
            let mut keys = BTreeMap::new();
            for x in public_keys {
                let xpub = recover_c_char(x.xpub);
                let path = recover_c_char(x.path);
                let extended_public_key =
                    Xpub::from_str(xpub.as_str()).map_err(|_e| RustCError::InvalidXPub);
                let derivation_path =
                    DerivationPath::from_str(path.as_str()).map_err(|_e| RustCError::InvalidHDPath);
                match extended_public_key.and_then(|k| derivation_path.and_then(|p| Ok((k, p)))) {
                    Ok((k, p)) => {
                        keys.insert(p, k);
                    }
                    Err(e) => {
                        return TransactionCheckResult::from(e).c_ptr();
                    }
                }
            }
            let wallet_config = match multisig_wallet_config
                .map(|v| parse_wallet_config(&v, &fp.to_string()))
                .transpose()
            {
                Ok(t) => t,
                Err(e) => return TransactionCheckResult::from(e).c_ptr(),
            };
            let context = ParseContext::new(fp, keys, verify_code, wallet_config);
            let check_result = app_bitcoin::check_psbt(psbt, context);
            match check_result {
                Ok(_) => TransactionCheckResult::new().c_ptr(),
                Err(e) => TransactionCheckResult::from(e).c_ptr(),
            }
        }
        Err(e) => TransactionCheckResult::from(e).c_ptr(),
    }
}

//To be optimized, use a more efficient way to determine whether it is base64 encoding.
fn is_base64_hex(input: &[u8]) -> bool {
    third_party::base64::decode(input).is_ok()
}

fn get_psbt_bytes(psbt_bytes: &[u8]) -> Result<Vec<u8>, RustCError> {
    if is_base64_hex(psbt_bytes) {
        third_party::base64::decode(psbt_bytes).map_err(|e| RustCError::InvalidData(e.to_string()))
    } else {
        Ok(psbt_bytes.to_vec())
    }
}
