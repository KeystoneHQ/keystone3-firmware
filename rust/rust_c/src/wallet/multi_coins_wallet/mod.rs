pub mod aptos;
pub mod arconnect;
pub mod backpack;
pub mod bitget;
mod imtoken;
pub mod keplr;
pub mod keystone;
pub mod keystone_connect;
pub mod okx;
pub mod solana;
pub mod structs;
pub mod sui;
pub mod tonkeeper;
mod utils;
pub mod xbull;
pub mod xrp_toolkit;

pub mod core_wallet;
pub mod thor_wallet;

use alloc::format;
use alloc::string::String;
use alloc::string::ToString;
use alloc::vec::Vec;

use app_wallets::metamask::ETHAccountTypeApp;
use app_wallets::DEVICE_TYPE;
use cty::uint32_t;
use keystore::algorithms::secp256k1::derive_extend_public_key;
use keystore::errors::KeystoreError;

use ed25519_bip32_core::XPub;
use hex;
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
use crate::common::ur::{UREncodeResult, FRAGMENT_MAX_LENGTH_DEFAULT, FRAGMENT_UNLIMITED_LENGTH};
use crate::common::utils::{recover_c_array, recover_c_char};
use crate::{extract_array, extract_ptr_with_type};

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
pub unsafe extern "C" fn get_connect_metamask_ur_dynamic(
    master_fingerprint: PtrBytes,
    master_fingerprint_length: uint32_t,
    account_type: ETHAccountType,
    public_keys: PtrT<CSliceFFI<ExtendedPublicKey>>,
    fragment_max_length_default: usize,
    fragment_max_length_other: usize,
) -> *mut UREncodeResult {
    if master_fingerprint_length != 4 {
        return UREncodeResult::from(URError::UrEncodeError(format!(
            "master fingerprint length must be 4, current is {master_fingerprint_length}"
        )))
        .c_ptr();
    }
    let mfp = extract_array!(master_fingerprint, u8, master_fingerprint_length);
    let mfp = match <&[u8; 4]>::try_from(mfp) {
        Ok(mfp) => mfp,
        Err(e) => return UREncodeResult::from(URError::UrEncodeError(e.to_string())).c_ptr(),
    };

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
                    let result = app_wallets::metamask::generate_ledger_live_account(mfp, &value);
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
                Err(e) => UREncodeResult::from(e).c_ptr(),
            }
        }
        _ => {
            let key = keys.first().ok_or(RustCError::InvalidXPub);
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
                Err(e) => UREncodeResult::from(e).c_ptr(),
            }
        }
    }
}

#[no_mangle]
pub unsafe extern "C" fn get_connect_metamask_ur_unlimited(
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
        FRAGMENT_UNLIMITED_LENGTH,
        FRAGMENT_UNLIMITED_LENGTH,
    )
}

#[no_mangle]
pub unsafe extern "C" fn get_connect_metamask_ur(
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
        FRAGMENT_MAX_LENGTH_DEFAULT,
        240,
    )
}
