pub mod aptos;
pub mod keplr;
pub mod okx;
pub mod solana;
pub mod structs;
pub mod sui;
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
use third_party::itertools::Itertools;
use third_party::ur_registry::crypto_key_path::CryptoKeyPath;
use third_party::ur_registry::extend::crypto_multi_accounts::CryptoMultiAccounts;
use third_party::ur_registry::extend::qr_hardware_call::QRHardwareCall;

use app_wallets::metamask::ETHAccountTypeApp;
use cty::{int32_t, uint32_t};
use keystore::algorithms::secp256k1::derive_extend_public_key;
use keystore::errors::KeystoreError;
use third_party::ur_registry::bytes::Bytes;
use third_party::ur_registry::crypto_account::CryptoAccount;
use third_party::ur_registry::crypto_hd_key::CryptoHDKey;
use third_party::ur_registry::error::URError;
use third_party::ur_registry::traits::RegistryItem;

use crate::extract_array;
use crate::extract_ptr_with_type;
use crate::interfaces::errors::RustCError;
use crate::interfaces::ffi::CSliceFFI;
use crate::interfaces::structs::ExtendedPublicKey;
use crate::interfaces::types::{PtrBytes, PtrString, PtrT};
use crate::interfaces::ur::{UREncodeResult, FRAGMENT_MAX_LENGTH_DEFAULT};
use crate::interfaces::utils::{recover_c_array, recover_c_char};

use self::structs::QRHardwareCallData;

use super::structs::Response;
use super::types::Ptr;
use super::types::PtrUR;

#[no_mangle]
pub extern "C" fn get_connect_blue_wallet_ur(
    master_fingerprint: *mut u8,
    length: u32,
    public_keys: PtrT<CSliceFFI<ExtendedPublicKey>>,
) -> *mut UREncodeResult {
    if length != 4 {
        return UREncodeResult::from(URError::UrEncodeError(format!(
            "master fingerprint length must be 4, current is {}",
            length
        )))
        .c_ptr();
    }
    unsafe {
        let mfp = slice::from_raw_parts(master_fingerprint, length as usize);
        let keys = recover_c_array(public_keys);
        let key1 = keys.get(0);
        let key2 = keys.get(1);
        let key3 = keys.get(2);
        return if let (Some(k1), Some(k2), Some(k3)) = (key1, key2, key3) {
            let native_x_pub = recover_c_char(k1.xpub);
            let nested_x_pub = recover_c_char(k2.xpub);
            let legacy_x_pub = recover_c_char(k3.xpub);
            let extended_public_keys = [
                native_x_pub.trim(),
                legacy_x_pub.trim(),
                nested_x_pub.trim(),
            ];
            let mfp = match <&[u8; 4]>::try_from(mfp) {
                Ok(mfp) => mfp,
                Err(e) => {
                    return UREncodeResult::from(URError::UrEncodeError(e.to_string())).c_ptr();
                }
            };

            let result =
                app_wallets::blue_wallet::generate_crypto_account(mfp, extended_public_keys);
            match result.map(|v| v.try_into()) {
                Ok(v) => match v {
                    Ok(data) => UREncodeResult::encode(
                        data,
                        CryptoAccount::get_registry_type().get_type(),
                        FRAGMENT_MAX_LENGTH_DEFAULT,
                    )
                    .c_ptr(),
                    Err(e) => UREncodeResult::from(e).c_ptr(),
                },
                Err(e) => UREncodeResult::from(e).c_ptr(),
            }
        } else {
            UREncodeResult::from(URError::UrEncodeError(format!("getting key error"))).c_ptr()
        };
    }
}

#[repr(C)]
#[derive(Clone)]
pub struct AccountConfig {
    pub hd_path: PtrString,
    pub x_pub: PtrString,
    pub address_length: int32_t,
    pub is_multi_sign: bool,
}

impl AccountConfig {
    pub fn into(&self) -> app_wallets::companion_app::AccountConfig {
        app_wallets::companion_app::AccountConfig {
            hd_path: recover_c_char(self.hd_path),
            x_pub: recover_c_char(self.x_pub),
            address_length: self.address_length,
            is_multi_sign: self.is_multi_sign,
        }
    }
}

#[repr(C)]
#[derive(Clone)]
pub struct CoinConfig {
    pub is_active: bool,
    pub coin_code: PtrString,
    pub accounts: *mut AccountConfig,
    pub accounts_length: uint32_t,
}

impl CoinConfig {
    pub fn into(&self) -> app_wallets::companion_app::CoinConfig {
        app_wallets::companion_app::CoinConfig {
            is_active: self.is_active,
            coin_code: recover_c_char(self.coin_code),
            accounts: unsafe {
                core::slice::from_raw_parts(self.accounts, self.accounts_length as usize)
                    .to_vec()
                    .iter()
                    .map(|x| x.into())
                    .collect()
            },
        }
    }
}

#[no_mangle]
pub extern "C" fn get_connect_companion_app_ur(
    master_fingerprint: PtrBytes,
    master_fingerprint_length: uint32_t,
    cold_version: i32,
    coin_config: PtrT<CoinConfig>,
    coin_config_length: uint32_t,
) -> PtrT<UREncodeResult> {
    let mfp = extract_array!(master_fingerprint, u8, master_fingerprint_length);
    let coin_config = extract_array!(coin_config, CoinConfig, coin_config_length);
    let coin_config: Vec<app_wallets::companion_app::CoinConfig> =
        coin_config.to_vec().iter().map(|x| x.into()).collect();
    let mfp = match <&[u8; 4]>::try_from(mfp) {
        Ok(mfp) => mfp,
        Err(e) => return UREncodeResult::from(URError::UrEncodeError(e.to_string())).c_ptr(),
    };
    let result =
        app_wallets::companion_app::generate_companion_app_sync_ur(mfp, cold_version, coin_config);
    match result.map(|v| v.try_into()) {
        Ok(v) => match v {
            Ok(data) => UREncodeResult::encode(
                data,
                Bytes::get_registry_type().get_type(),
                FRAGMENT_MAX_LENGTH_DEFAULT,
            )
            .c_ptr(),
            Err(e) => UREncodeResult::from(e).c_ptr(),
        },
        Err(e) => UREncodeResult::from(e).c_ptr(),
    }
}

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
pub extern "C" fn get_connect_metamask_ur(
    master_fingerprint: PtrBytes,
    master_fingerprint_length: uint32_t,
    account_type: ETHAccountType,
    public_keys: PtrT<CSliceFFI<ExtendedPublicKey>>,
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
                                    FRAGMENT_MAX_LENGTH_DEFAULT,
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
                        );
                        match result.map(|v| v.try_into()) {
                            Ok(v) => match v {
                                Ok(data) => UREncodeResult::encode(
                                    data,
                                    CryptoHDKey::get_registry_type().get_type(),
                                    240,
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
