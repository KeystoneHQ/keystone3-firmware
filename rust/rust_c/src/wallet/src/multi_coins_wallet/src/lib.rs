#![no_std]

extern crate alloc;
pub mod aptos;
pub mod arconnect;
mod imtoken;
pub mod keplr;
pub mod okx;
pub mod solana;
pub mod structs;
pub mod sui;
pub mod tonkeeper;
mod utils;
pub mod xrp_toolkit;

use alloc::format;
use alloc::string::String;
use alloc::string::ToString;
use alloc::vec::Vec;
use app_wallets::DEVICE_TYPE;
use app_wallets::DEVICE_VERSION;
use core::slice;
use third_party::ed25519_bip32_core::XPub;
use third_party::hex;
use third_party::ur_registry::bytes::Bytes;
use third_party::ur_registry::crypto_key_path::CryptoKeyPath;
use third_party::ur_registry::extend::crypto_multi_accounts::CryptoMultiAccounts;
use third_party::ur_registry::extend::qr_hardware_call::QRHardwareCall;

use crate::structs::QRHardwareCallData;
use app_wallets::metamask::ETHAccountTypeApp;
use common_rust_c::errors::RustCError;
use common_rust_c::ffi::CSliceFFI;
use common_rust_c::structs::{ExtendedPublicKey, Response};
use common_rust_c::types::{Ptr, PtrBytes, PtrString, PtrT, PtrUR};
use common_rust_c::ur::{UREncodeResult, FRAGMENT_MAX_LENGTH_DEFAULT, FRAGMENT_UNLIMITED_LENGTH};
use common_rust_c::utils::{recover_c_array, recover_c_char};
use common_rust_c::{extract_array, extract_ptr_with_type};
use cty::{int32_t, uint32_t};
use keystore::algorithms::secp256k1::derive_extend_public_key;
use keystore::errors::KeystoreError;
use third_party::ur_registry::crypto_account::CryptoAccount;
use third_party::ur_registry::crypto_hd_key::CryptoHDKey;
use third_party::ur_registry::error::URError;
use third_party::ur_registry::traits::RegistryItem;

#[repr(C)]
pub enum ETHAccountType {
    Bip44Standard,
    LedgerLive,
    LedgerLegacy,
}

impl From<ETHAccountType> for ETHAccountTypeApp {
    fn from(enum_value: ETHAccountType) -> Self {
        match enum_value {
            ETHAccountType::Bip44Standard => Self::Bip44Standard,
            ETHAccountType::LedgerLive => Self::LedgerLive,
            ETHAccountType::LedgerLegacy => Self::LedgerLegacy,
        }
    }
}

#[no_mangle]
pub extern "C" fn get_connect_metamask_ur_dynamic(
    master_fingerprint: PtrBytes,
    master_fingerprint_length: uint32_t,
    account_type: ETHAccountType,
    public_keys: PtrT<CSliceFFI<ExtendedPublicKey>>,
    fragment_max_length_default: usize,
    fragment_max_length_other: usize,
) -> *mut UREncodeResult {
    if master_fingerprint_length != 4 {
        return UREncodeResult::from(URError::UrEncodeError(format!(
            "master fingerprint length must be 4, current is {}",
            master_fingerprint_length
        )))
        .c_ptr();
    }
    let mfp = extract_array!(master_fingerprint, u8, master_fingerprint_length);
    let mfp = match <&[u8; 4]>::try_from(mfp) {
        Ok(mfp) => mfp,
        Err(e) => return UREncodeResult::from(URError::UrEncodeError(e.to_string())).c_ptr(),
    };
    unsafe {
        let keys = recover_c_array(public_keys);
        match account_type {
            ETHAccountType::LedgerLive => {
                let extended_public_keys = keys
                    .iter()
                    .map(|v: &ExtendedPublicKey| {
                        derive_extend_public_key(&recover_c_char(v.xpub), &String::from("m/0/0"))
                            .map(|e| e.to_string())
                    })
                    .collect::<Result<Vec<String>, KeystoreError>>();

                match extended_public_keys {
                    Ok(value) => {
                        let result =
                            app_wallets::metamask::generate_ledger_live_account(mfp, &value);
                        match result.map(|v| v.try_into()) {
                            Ok(v) => match v {
                                Ok(data) => UREncodeResult::encode(
                                    data,
                                    CryptoAccount::get_registry_type().get_type(),
                                    fragment_max_length_default,
                                )
                                .c_ptr(),
                                Err(e) => UREncodeResult::from(e).c_ptr(),
                            },
                            Err(e) => UREncodeResult::from(e).c_ptr(),
                        }
                    }
                    Err(e) => {
                        return UREncodeResult::from(e).c_ptr();
                    }
                }
            }
            _ => {
                let key = keys.get(0).ok_or(RustCError::InvalidXPub);
                match key {
                    Ok(k) => {
                        let result = app_wallets::metamask::generate_standard_legacy_hd_key(
                            mfp,
                            &recover_c_char(k.xpub),
                            account_type.into(),
                            None,
                        );
                        match result.map(|v| v.try_into()) {
                            Ok(v) => match v {
                                Ok(data) => UREncodeResult::encode(
                                    data,
                                    CryptoHDKey::get_registry_type().get_type(),
                                    fragment_max_length_other,
                                )
                                .c_ptr(),
                                Err(e) => UREncodeResult::from(e).c_ptr(),
                            },
                            Err(e) => UREncodeResult::from(e).c_ptr(),
                        }
                    }
                    Err(e) => return UREncodeResult::from(e).c_ptr(),
                }
            }
        }
    }
}

#[no_mangle]
pub extern "C" fn get_connect_metamask_ur_unlimited(
    master_fingerprint: PtrBytes,
    master_fingerprint_length: uint32_t,
    account_type: ETHAccountType,
    public_keys: PtrT<CSliceFFI<ExtendedPublicKey>>,
) -> *mut UREncodeResult {
    get_connect_metamask_ur_dynamic(
        master_fingerprint,
        master_fingerprint_length,
        account_type,
        public_keys,
        FRAGMENT_UNLIMITED_LENGTH.clone(),
        FRAGMENT_UNLIMITED_LENGTH.clone(),
    )
}

#[no_mangle]
pub extern "C" fn get_connect_metamask_ur(
    master_fingerprint: PtrBytes,
    master_fingerprint_length: uint32_t,
    account_type: ETHAccountType,
    public_keys: PtrT<CSliceFFI<ExtendedPublicKey>>,
) -> *mut UREncodeResult {
    get_connect_metamask_ur_dynamic(
        master_fingerprint,
        master_fingerprint_length,
        account_type,
        public_keys,
        FRAGMENT_MAX_LENGTH_DEFAULT.clone(),
        240,
    )
}

#[no_mangle]
pub extern "C" fn parse_qr_hardware_call(ur: PtrUR) -> Ptr<Response<QRHardwareCallData>> {
    let qr_hardware_call = extract_ptr_with_type!(ur, QRHardwareCall);
    let data = QRHardwareCallData::try_from(qr_hardware_call);
    match data {
        Ok(_data) => Response::success(_data).c_ptr(),
        Err(_e) => Response::from(_e).c_ptr(),
    }
}

#[no_mangle]
pub extern "C" fn generate_key_derivation_ur(
    master_fingerprint: PtrBytes,
    master_fingerprint_length: uint32_t,
    xpubs: Ptr<CSliceFFI<ExtendedPublicKey>>,
) -> Ptr<UREncodeResult> {
    let mfp = extract_array!(master_fingerprint, u8, master_fingerprint_length);
    let mfp = match <&[u8; 4]>::try_from(mfp) {
        Ok(mfp) => mfp.clone(),
        Err(e) => return UREncodeResult::from(URError::UrEncodeError(e.to_string())).c_ptr(),
    };
    let public_keys = unsafe { recover_c_array(xpubs) };
    let keys = public_keys
        .iter()
        .map(|v| {
            let xpub = recover_c_char(v.xpub);
            let path = recover_c_char(v.path);
            let path = match CryptoKeyPath::from_path(path, None) {
                Ok(path) => path,
                Err(e) => return Err(URError::UrEncodeError(e)),
            };
            match hex::decode(xpub) {
                Ok(v) => match XPub::from_slice(&v) {
                    Ok(xpub) => Ok(CryptoHDKey::new_extended_key(
                        None,
                        xpub.public_key().to_vec(),
                        Some(xpub.chain_code().to_vec()),
                        None,
                        Some(path),
                        None,
                        None,
                        None,
                        None,
                    )),
                    Err(e) => Err(URError::UrEncodeError(e.to_string())),
                },
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
        Some(DEVICE_VERSION.to_string()),
    );
    match accounts.try_into() {
        Ok(v) => {
            UREncodeResult::encode(v, CryptoMultiAccounts::get_registry_type().get_type(), 240)
                .c_ptr()
        }
        Err(_e) => UREncodeResult::from(_e).c_ptr(),
    }
}
