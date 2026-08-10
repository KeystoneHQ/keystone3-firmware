pub mod btc_only_wallet;
pub use btc_only_wallet::*;
#[cfg(feature = "multi-coins")]
pub mod babylon;
#[cfg(feature = "cypherpunk")]
pub mod cypherpunk_wallet;
#[cfg(feature = "multi-coins")]
pub mod multi_coins_wallet;
mod structs;

use alloc::collections::BTreeMap;
use alloc::format;
use alloc::string::String;
use alloc::string::ToString;
use alloc::vec::Vec;
use bitcoin::bip32::DerivationPath;
use core::slice;
use core::str::FromStr;

use app_wallets::{generate_crypto_multi_accounts_sync_ur, DEVICE_TYPE};
use cty::uint32_t;
use keystore::algorithms::secp256k1::derive_extend_public_key;
use keystore::errors::KeystoreError;

use ed25519_bip32_core::XPub;
use ur_registry::crypto_account::CryptoAccount;
use ur_registry::crypto_hd_key::CryptoHDKey;
use ur_registry::crypto_key_path::CryptoKeyPath;
use ur_registry::error::URError;
use ur_registry::extend::crypto_multi_accounts::CryptoMultiAccounts;
use ur_registry::extend::qr_hardware_call::QRHardwareCall;
use ur_registry::traits::RegistryItem;

use crate::common::errors::RustCError;
use crate::common::ffi::CSliceFFI;
use crate::common::structs::{ExtendedPublicKey, Response};
use crate::common::types::{Ptr, PtrBytes, PtrString, PtrT, PtrUR};
use crate::common::ur::{UREncodeResult, FRAGMENT_MAX_LENGTH_DEFAULT};
use crate::common::utils::{recover_c_array, recover_c_char};
use crate::{extract_array, extract_ptr_with_type};
use structs::QRHardwareCallData;

#[no_mangle]
pub unsafe extern "C" fn parse_qr_hardware_call(ur: PtrUR) -> Ptr<Response<QRHardwareCallData>> {
    let qr_hardware_call = extract_ptr_with_type!(ur, QRHardwareCall);
    let data = QRHardwareCallData::try_from(qr_hardware_call);
    match data {
        Ok(_data) => Response::success(_data).c_ptr(),
        Err(_e) => Response::from(_e).c_ptr(),
    }
}

#[no_mangle]
pub unsafe extern "C" fn check_hardware_call_path(path: PtrString) -> *mut Response<bool> {
    let mut path = recover_c_char(path).to_lowercase();
    if !path.starts_with('m') {
        path = format!("m/{path}");
    }
    if DerivationPath::from_str(path.as_str()).is_err() {
        return Response::success(false).c_ptr();
    }

    Response::success(is_supported_hardware_call_path(path.as_str())).c_ptr()
}

fn is_supported_hardware_call_path(path: &str) -> bool {
    const SUPPORTED_PATH_PREFIXES: &[&str] = &[
        "m/44'/0'",
        "m/84'",
        "m/86'",
        "m/49'",
        "m/44'/60'",
        "m/44'/501'",
        "m/44'/144'",
        "m/1852'/1815'",
        "m/1853'/1815'",
        "m/1854'/1815'",
        "m/44'/195'",
        "m/44'/145'",
        "m/44'/637'",
        "m/44'/784'",
        "m/44'/5'",
        "m/44'/472'",
        "m/44'/148'",
        "m/44'/118'",
        "m/44'/394'",
        "m/44'/459'",
        "m/44'/330'",
        "m/44'/529'",
        "m/44'/564'",
        "m/44'/234'",
        "m/44'/931'",
        "m/44'/4218'",
    ];

    SUPPORTED_PATH_PREFIXES
        .iter()
        .any(|prefix| path_matches_prefix(path, prefix))
}

fn path_matches_prefix(path: &str, prefix: &str) -> bool {
    path == prefix
        || path
            .strip_prefix(prefix)
            .map_or(false, |suffix| suffix.starts_with('/'))
}

#[no_mangle]
pub unsafe extern "C" fn generate_key_derivation_ur(
    master_fingerprint: PtrBytes,
    master_fingerprint_length: uint32_t,
    xpubs: Ptr<CSliceFFI<ExtendedPublicKey>>,
    device_version: PtrString,
) -> Ptr<UREncodeResult> {
    let mfp = extract_array!(master_fingerprint, u8, master_fingerprint_length);
    let mfp = match <&[u8; 4]>::try_from(mfp) {
        Ok(mfp) => *mfp,
        Err(e) => return UREncodeResult::from(URError::UrEncodeError(e.to_string())).c_ptr(),
    };
    let public_keys = recover_c_array(xpubs);
    let device_version = recover_c_char(device_version);
    let keys = public_keys
        .iter()
        .map(|v| {
            let xpub = recover_c_char(v.xpub);
            let path = recover_c_char(v.path);
            let path = match CryptoKeyPath::from_path(path, None) {
                Ok(path) => path,
                Err(e) => return Err(URError::UrEncodeError(e)),
            };
            let xpub_decode = hex::decode(xpub);

            match xpub_decode {
                Ok(v) => {
                    if v.len() >= 78 {
                        // sec256k1 xpub
                        let xpub = bitcoin::bip32::Xpub::decode(&v);
                        match xpub {
                            Ok(xpub) => {
                                let chain_code = xpub.chain_code.as_bytes().to_vec();
                                Ok(CryptoHDKey::new_extended_key(
                                    None,
                                    xpub.public_key.serialize().to_vec(),
                                    Some(chain_code),
                                    None,
                                    Some(path),
                                    None,
                                    Some(xpub.parent_fingerprint.to_bytes()),
                                    None,
                                    None,
                                ))
                            }
                            Err(e) => Err(URError::UrEncodeError(e.to_string())),
                        }
                    } else if v.len() == 32 {
                        //  ed25519
                        Ok(CryptoHDKey::new_extended_key(
                            None,
                            v,
                            None,
                            None,
                            Some(path),
                            None,
                            None,
                            None,
                            None,
                        ))
                    } else {
                        let xpub = XPub::from_slice(&v);
                        match xpub {
                            Ok(xpub) => {
                                let chain_code = xpub.chain_code().to_vec();
                                Ok(CryptoHDKey::new_extended_key(
                                    None,
                                    xpub.public_key().to_vec(),
                                    Some(chain_code),
                                    None,
                                    Some(path),
                                    None,
                                    None,
                                    None,
                                    None,
                                ))
                            }
                            Err(e) => Err(URError::UrEncodeError(e.to_string())),
                        }
                    }
                }
                Err(e) => Err(URError::UrEncodeError(e.to_string())),
            }
        })
        .collect::<Result<Vec<CryptoHDKey>, URError>>();
    let keys = match keys {
        Ok(keys) => keys,
        Err(e) => return UREncodeResult::from(e).c_ptr(),
    };
    let accounts = CryptoMultiAccounts::new(
        mfp,
        keys,
        Some(DEVICE_TYPE.to_string()),
        None,
        Some(device_version),
    );
    match accounts.try_into() {
        Ok(v) => {
            UREncodeResult::encode(v, CryptoMultiAccounts::get_registry_type().get_type(), 240)
                .c_ptr()
        }
        Err(_e) => UREncodeResult::from(_e).c_ptr(),
    }
}

#[no_mangle]
pub unsafe extern "C" fn generate_common_crypto_multi_accounts_ur(
    master_fingerprint: PtrBytes,
    master_fingerprint_length: u32,
    public_keys: Ptr<CSliceFFI<ExtendedPublicKey>>,
    account_prefix: PtrString,
) -> Ptr<UREncodeResult> {
    if master_fingerprint_length != 4 {
        return UREncodeResult::from(URError::UrEncodeError(format!(
            "master fingerprint length must be 4, current is {master_fingerprint_length}"
        )))
        .c_ptr();
    }
    let account_prefix = recover_c_char(account_prefix);
    unsafe {
        let mfp = slice::from_raw_parts(master_fingerprint, master_fingerprint_length as usize);
        let public_keys = recover_c_array(public_keys);
        let master_fingerprint = bitcoin::bip32::Fingerprint::from_str(hex::encode(mfp).as_str())
            .map_err(|_e| RustCError::InvalidMasterFingerprint);
        match master_fingerprint {
            Ok(fp) => {
                let mut keys = BTreeMap::new();
                for x in public_keys {
                    let pubkey = recover_c_char(x.xpub);
                    let path = recover_c_char(x.path);
                    match DerivationPath::from_str(path.to_lowercase().as_str()) {
                        Ok(path) => {
                            keys.insert(path, pubkey);
                        }
                        Err(_e) => {
                            return UREncodeResult::from(RustCError::InvalidHDPath).c_ptr();
                        }
                    }
                }
                let result = generate_crypto_multi_accounts_sync_ur(
                    fp.as_ref(),
                    keys,
                    account_prefix.as_str(),
                );
                match result.map(|v| v.try_into()) {
                    Ok(v) => match v {
                        Ok(data) => UREncodeResult::encode(
                            data,
                            CryptoMultiAccounts::get_registry_type().get_type(),
                            FRAGMENT_MAX_LENGTH_DEFAULT,
                        )
                        .c_ptr(),
                        Err(e) => UREncodeResult::from(e).c_ptr(),
                    },
                    Err(e) => UREncodeResult::from(e).c_ptr(),
                }
            }
            Err(e) => UREncodeResult::from(e).c_ptr(),
        }
    }
}
