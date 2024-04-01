use alloc::boxed::Box;
use alloc::collections::BTreeMap;
use alloc::slice;
use alloc::string::ToString;
use alloc::vec::Vec;
use core::str::FromStr;

use app_bitcoin;
use app_bitcoin::parsed_tx::ParseContext;
use common_rust_c::errors::RustCError;
use common_rust_c::extract_ptr_with_type;
use common_rust_c::ffi::CSliceFFI;
use common_rust_c::structs::{
    ExtendedPublicKey, Response, TransactionCheckResult, TransactionParseResult,
};
use common_rust_c::types::{Ptr, PtrBytes, PtrString, PtrT, PtrUR};
use common_rust_c::ur::{UREncodeResult, FRAGMENT_MAX_LENGTH_DEFAULT};
use common_rust_c::utils::{recover_c_array, recover_c_char};
use third_party::bitcoin::bip32::{DerivationPath, Xpub};
use third_party::hex;
use third_party::ur_registry::crypto_psbt::CryptoPSBT;
use third_party::ur_registry::traits::RegistryItem;

use crate::structs::{DisplayTx, PsbtSignResult};

#[no_mangle]
pub extern "C" fn btc_parse_psbt(
    ptr: PtrUR,
    master_fingerprint: PtrBytes,
    length: u32,
    public_keys: PtrT<CSliceFFI<ExtendedPublicKey>>,
) -> *mut TransactionParseResult<DisplayTx> {
    if length != 4 {
        return TransactionParseResult::from(RustCError::InvalidMasterFingerprint).c_ptr();
    }
    let crypto_psbt = extract_ptr_with_type!(ptr, CryptoPSBT);
    let psbt = crypto_psbt.get_psbt();

    unsafe {
        let mfp = core::slice::from_raw_parts(master_fingerprint, 4);
        let public_keys = recover_c_array(public_keys);
        parse_psbt(mfp, public_keys, psbt)
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
                FRAGMENT_MAX_LENGTH_DEFAULT.clone(),
            )
            .c_ptr(),
            Err(e) => UREncodeResult::from(e).c_ptr(),
        },
        Err(e) => UREncodeResult::from(e).c_ptr(),
    }
}

#[no_mangle]
pub extern "C" fn btc_check_psbt(
    ptr: PtrUR,
    master_fingerprint: PtrBytes,
    length: u32,
    public_keys: PtrT<CSliceFFI<ExtendedPublicKey>>,
) -> PtrT<TransactionCheckResult> {
    if length != 4 {
        return TransactionCheckResult::from(RustCError::InvalidMasterFingerprint).c_ptr();
    }
    let crypto_psbt = extract_ptr_with_type!(ptr, CryptoPSBT);
    let psbt = crypto_psbt.get_psbt();

    unsafe {
        let mfp = core::slice::from_raw_parts(master_fingerprint, 4);
        let public_keys = recover_c_array(public_keys);
        check_psbt(mfp, public_keys, psbt)
    }
}

#[no_mangle]
pub extern "C" fn btc_check_psbt_str(
    psbt_str: PtrString,
    master_fingerprint: PtrBytes,
    length: u32,
    public_keys: PtrT<CSliceFFI<ExtendedPublicKey>>,
) -> PtrT<TransactionCheckResult> {
    if length != 4 {
        return TransactionCheckResult::from(RustCError::InvalidMasterFingerprint).c_ptr();
    }

    let psbt_str = recover_c_char(psbt_str);
    let psbt = match get_psbt_bytes(&psbt_str) {
        Ok(psbt) => psbt,
        Err(e) => return TransactionCheckResult::from(e).c_ptr(),
    };

    unsafe {
        let mfp = core::slice::from_raw_parts(master_fingerprint, 4);
        let public_keys = recover_c_array(public_keys);
        check_psbt(mfp, public_keys, psbt)
    }
}

#[no_mangle]
pub extern "C" fn btc_parse_psbt_str(
    psbt_str: PtrString,
    master_fingerprint: PtrBytes,
    length: u32,
    public_keys: PtrT<CSliceFFI<ExtendedPublicKey>>,
) -> *mut TransactionParseResult<DisplayTx> {
    if length != 4 {
        return TransactionParseResult::from(RustCError::InvalidMasterFingerprint).c_ptr();
    }
    let psbt_str = recover_c_char(psbt_str);
    let psbt = match get_psbt_bytes(&psbt_str) {
        Ok(psbt) => psbt,
        Err(e) => return TransactionParseResult::from(e).c_ptr(),
    };

    unsafe {
        let mfp = core::slice::from_raw_parts(master_fingerprint, 4);
        let public_keys = recover_c_array(public_keys);
        parse_psbt(mfp, public_keys, psbt)
    }
}

#[no_mangle]
pub extern "C" fn btc_sign_psbt_str(
    psbt_str: PtrString,
    seed: PtrBytes,
    seed_len: u32,
    master_fingerprint: PtrBytes,
    master_fingerprint_len: u32,
) -> Ptr<Response<PsbtSignResult>> {
    if master_fingerprint_len != 4 {
        return Response::from(RustCError::InvalidMasterFingerprint).c_ptr();
    }
    let master_fingerprint = unsafe { core::slice::from_raw_parts(master_fingerprint, 4) };
    let master_fingerprint = match third_party::bitcoin::bip32::Fingerprint::from_str(
        hex::encode(master_fingerprint.to_vec()).as_str(),
    )
    .map_err(|_e| RustCError::InvalidMasterFingerprint)
    {
        Ok(mfp) => mfp,
        Err(e) => {
            return Response::from(e).c_ptr();
        }
    };

    let seed = unsafe { slice::from_raw_parts(seed, seed_len as usize) };

    let psbt_str = recover_c_char(psbt_str);
    let psbt = match get_psbt_bytes(&psbt_str) {
        Ok(psbt) => psbt,
        Err(e) => return Response::from(e).c_ptr(),
    };

    let psbt_bytes = match app_bitcoin::sign_psbt(psbt, seed, master_fingerprint) {
        Ok(psbt_bytes) => psbt_bytes,
        Err(e) => return Response::from(e).c_ptr(),
    };

    let ur_result = match CryptoPSBT::new(psbt_bytes.clone()).try_into() {
        Ok(data) => UREncodeResult::encode(
            data,
            CryptoPSBT::get_registry_type().get_type(),
            FRAGMENT_MAX_LENGTH_DEFAULT.clone(),
        )
        .c_ptr(),
        Err(e) => return Response::from(e).c_ptr(),
    };

    Response::success(PsbtSignResult::new(&psbt_bytes, ur_result)).c_ptr()
}

fn parse_psbt(
    mfp: &[u8],
    public_keys: &[ExtendedPublicKey],
    psbt: Vec<u8>,
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
            let context = ParseContext::new(fp, keys);
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
            let context = ParseContext::new(fp, keys);
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
fn is_base64(input: &str) -> bool {
    third_party::base64::decode(input).is_ok()
}

fn get_psbt_bytes(psbt_str: &str) -> Result<Vec<u8>, RustCError> {
    if is_base64(psbt_str) {
        third_party::base64::decode(psbt_str).map_err(|e| RustCError::InvalidData(e.to_string()))
    } else {
        hex::decode(psbt_str).map_err(|e| RustCError::InvalidData(e.to_string()))
    }
}
