use alloc::boxed::Box;
use alloc::collections::BTreeMap;
use alloc::slice;
use core::str::FromStr;

use app_bitcoin;
use app_bitcoin::parsed_tx::ParseContext;
use common_rust_c::errors::RustCError;
use common_rust_c::extract_ptr_with_type;
use common_rust_c::ffi::CSliceFFI;
use common_rust_c::structs::{ExtendedPublicKey, TransactionCheckResult, TransactionParseResult};
use common_rust_c::types::{PtrBytes, PtrT, PtrUR};
use common_rust_c::ur::{UREncodeResult, FRAGMENT_MAX_LENGTH_DEFAULT};
use common_rust_c::utils::{recover_c_array, recover_c_char};
use third_party::bitcoin::bip32::{DerivationPath, ExtendedPubKey};
use third_party::hex;
use third_party::ur_registry::crypto_psbt::CryptoPSBT;
use third_party::ur_registry::traits::RegistryItem;

use crate::structs::DisplayTx;

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
        let master_fingerprint =
            third_party::bitcoin::bip32::Fingerprint::from_str(hex::encode(mfp.to_vec()).as_str())
                .map_err(|_e| RustCError::InvalidMasterFingerprint);
        match master_fingerprint {
            Ok(fp) => {
                let mut keys = BTreeMap::new();
                for x in public_keys {
                    let xpub = recover_c_char(x.xpub);
                    let path = recover_c_char(x.path);
                    let extended_public_key = ExtendedPubKey::from_str(xpub.as_str())
                        .map_err(|_e| RustCError::InvalidXPub);
                    let derivation_path = DerivationPath::from_str(path.as_str())
                        .map_err(|_e| RustCError::InvalidHDPath);
                    match extended_public_key.and_then(|k| derivation_path.and_then(|p| Ok((k, p))))
                    {
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
                    Ok(res) => TransactionParseResult::success(Box::into_raw(Box::new(
                        DisplayTx::from(res),
                    )))
                    .c_ptr(),
                    Err(e) => TransactionParseResult::from(e).c_ptr(),
                }
            }
            Err(e) => TransactionParseResult::from(e).c_ptr(),
        }
    }
}

#[no_mangle]
pub extern "C" fn btc_sign_psbt(ptr: PtrUR, seed: PtrBytes, seed_len: u32) -> *mut UREncodeResult {
    let crypto_psbt = extract_ptr_with_type!(ptr, CryptoPSBT);
    let seed = unsafe { slice::from_raw_parts(seed, seed_len as usize) };
    let psbt = crypto_psbt.get_psbt();
    let result = app_bitcoin::sign_psbt(psbt, seed);
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
        let master_fingerprint =
            third_party::bitcoin::bip32::Fingerprint::from_str(hex::encode(mfp.to_vec()).as_str())
                .map_err(|_e| RustCError::InvalidMasterFingerprint);
        match master_fingerprint {
            Ok(fp) => {
                let mut keys = BTreeMap::new();
                for x in public_keys {
                    let xpub = recover_c_char(x.xpub);
                    let path = recover_c_char(x.path);
                    let extended_public_key = ExtendedPubKey::from_str(xpub.as_str())
                        .map_err(|_e| RustCError::InvalidXPub);
                    let derivation_path = DerivationPath::from_str(path.as_str())
                        .map_err(|_e| RustCError::InvalidHDPath);
                    match extended_public_key.and_then(|k| derivation_path.and_then(|p| Ok((k, p))))
                    {
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
}
