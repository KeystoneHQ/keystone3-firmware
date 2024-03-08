#![no_std]

extern crate alloc;

use alloc::string::String;
use alloc::string::ToString;
use alloc::vec::Vec;
use alloc::{format, vec};
use app_utils::normalize_path;
use app_wallets::DEVICE_TYPE;
use app_wallets::DEVICE_VERSION;
use common_rust_c::errors::RustCError;
use common_rust_c::extract_array;
use common_rust_c::ffi::CSliceFFI;
use common_rust_c::structs::ExtendedPublicKey;
use common_rust_c::types::{Ptr, PtrBytes, PtrString, PtrT};
use common_rust_c::ur::{UREncodeResult, FRAGMENT_MAX_LENGTH_DEFAULT};
use common_rust_c::utils::{recover_c_array, recover_c_char};
use core::slice;
use core::str::FromStr;
use cty::{int32_t, uint32_t};
use third_party::bitcoin::bip32::{DerivationPath, Xpub};
use third_party::hex;
use third_party::ur_registry::bytes::Bytes;
use third_party::ur_registry::crypto_account::CryptoAccount;
use third_party::ur_registry::error::URError;
use third_party::ur_registry::extend::crypto_multi_accounts::CryptoMultiAccounts;
use third_party::ur_registry::traits::RegistryItem;

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
            let k1_x_pub = recover_c_char(k1.xpub);
            let k1_x_pub_path = recover_c_char(k1.path);
            let k2_x_pub = recover_c_char(k2.xpub);
            let k2_x_pub_path = recover_c_char(k2.path);
            let k3_x_pub = recover_c_char(k3.xpub);
            let k3_x_pub_path = recover_c_char(k3.path);

            let extended_public_keys = [k1_x_pub.trim(), k2_x_pub.trim(), k3_x_pub.trim()];
            let extend_public_key_paths = [
                k1_x_pub_path.trim(),
                k2_x_pub_path.trim(),
                k3_x_pub_path.trim(),
            ];
            let mfp = match <&[u8; 4]>::try_from(mfp) {
                Ok(mfp) => mfp,
                Err(e) => {
                    return UREncodeResult::from(URError::UrEncodeError(e.to_string())).c_ptr();
                }
            };

            let result = app_wallets::blue_wallet::generate_crypto_account(
                mfp,
                &extended_public_keys,
                &extend_public_key_paths,
            );
            match result.map(|v| v.try_into()) {
                Ok(v) => match v {
                    Ok(data) => UREncodeResult::encode(
                        data,
                        CryptoAccount::get_registry_type().get_type(),
                        FRAGMENT_MAX_LENGTH_DEFAULT.clone(),
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

#[no_mangle]
pub extern "C" fn get_connect_sparrow_wallet_ur(
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
        let key4 = keys.get(3);
        return if let (Some(k1), Some(k2), Some(k3), Some(k4)) = (key1, key2, key3, key4) {
            let k1_x_pub = recover_c_char(k1.xpub);
            let k1_x_pub_path = recover_c_char(k1.path);
            let k2_x_pub = recover_c_char(k2.xpub);
            let k2_x_pub_path = recover_c_char(k2.path);
            let k3_x_pub = recover_c_char(k3.xpub);
            let k3_x_pub_path = recover_c_char(k3.path);
            let k4_x_pub = recover_c_char(k4.xpub);
            let k4_x_pub_path = recover_c_char(k4.path);
            let extended_public_keys = [
                k1_x_pub.trim(),
                k2_x_pub.trim(),
                k3_x_pub.trim(),
                k4_x_pub.trim(),
            ];
            let extend_public_key_paths = [
                k1_x_pub_path.trim(),
                k2_x_pub_path.trim(),
                k3_x_pub_path.trim(),
                k4_x_pub_path.trim(),
            ];
            let mfp = match <&[u8; 4]>::try_from(mfp) {
                Ok(mfp) => mfp,
                Err(e) => {
                    return UREncodeResult::from(URError::UrEncodeError(e.to_string())).c_ptr();
                }
            };

            let result = app_wallets::blue_wallet::generate_crypto_account(
                mfp,
                &extended_public_keys,
                &extend_public_key_paths,
            );
            match result.map(|v| v.try_into()) {
                Ok(v) => match v {
                    Ok(data) => UREncodeResult::encode(
                        data,
                        CryptoAccount::get_registry_type().get_type(),
                        FRAGMENT_MAX_LENGTH_DEFAULT.clone(),
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

#[no_mangle]
pub extern "C" fn get_connect_specter_wallet_ur(
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
        return if let (Some(k1), Some(k2)) = (key1, key2) {
            let k1_x_pub = recover_c_char(k1.xpub);
            let k1_x_pub_path = recover_c_char(k1.path);
            let k2_x_pub = recover_c_char(k2.xpub);
            let k2_x_pub_path = recover_c_char(k2.path);

            let extended_public_keys = [k1_x_pub.trim(), k2_x_pub.trim()];
            let extend_public_key_paths = [k1_x_pub_path.trim(), k2_x_pub_path.trim()];
            let mfp = match <&[u8; 4]>::try_from(mfp) {
                Ok(mfp) => mfp,
                Err(e) => {
                    return UREncodeResult::from(URError::UrEncodeError(e.to_string())).c_ptr();
                }
            };

            let result = app_wallets::blue_wallet::generate_crypto_account(
                mfp,
                &extended_public_keys,
                &extend_public_key_paths,
            );
            match result.map(|v| v.try_into()) {
                Ok(v) => match v {
                    Ok(data) => UREncodeResult::encode(
                        data,
                        CryptoAccount::get_registry_type().get_type(),
                        FRAGMENT_MAX_LENGTH_DEFAULT.clone(),
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
                FRAGMENT_MAX_LENGTH_DEFAULT.clone(),
            )
            .c_ptr(),
            Err(e) => UREncodeResult::from(e).c_ptr(),
        },
        Err(e) => UREncodeResult::from(e).c_ptr(),
    }
}

#[no_mangle]
pub extern "C" fn get_okx_wallet_ur_btc_only(
    master_fingerprint: PtrBytes,
    master_fingerprint_length: u32,
    serial_number: PtrString,
    public_keys: Ptr<CSliceFFI<ExtendedPublicKey>>,
    device_type: PtrString,
    device_version: PtrString,
) -> Ptr<UREncodeResult> {
    if master_fingerprint_length != 4 {
        return UREncodeResult::from(URError::UrEncodeError(format!(
            "master fingerprint length must be 4, current is {}",
            master_fingerprint_length
        )))
        .c_ptr();
    }
    let mfp = extract_array!(master_fingerprint, u8, master_fingerprint_length);
    let mfp = match <[u8; 4]>::try_from(mfp) {
        Ok(mfp) => mfp,
        Err(e) => return UREncodeResult::from(URError::UrEncodeError(e.to_string())).c_ptr(),
    };
    unsafe {
        let keys = recover_c_array(public_keys);
        let serial_number = recover_c_char(serial_number);
        let device_version = recover_c_char(device_version);
        let device_type = recover_c_char(device_type);
        match normalize_xpub(keys) {
            Ok(_keys) => {
                match app_wallets::okx::generate_crypto_multi_accounts(
                    mfp,
                    &serial_number,
                    _keys,
                    &device_type,
                    &device_version,
                ) {
                    Ok(data) => match data.try_into() {
                        Ok(_v) => UREncodeResult::encode(
                            _v,
                            CryptoMultiAccounts::get_registry_type().get_type(),
                            FRAGMENT_MAX_LENGTH_DEFAULT.clone(),
                        )
                        .c_ptr(),
                        Err(_e) => UREncodeResult::from(_e).c_ptr(),
                    },
                    Err(_e) => UREncodeResult::from(_e).c_ptr(),
                }
            }
            Err(_e) => UREncodeResult::from(_e).c_ptr(),
        }
    }
}

fn normalize_xpub(
    keys: &[ExtendedPublicKey],
) -> Result<Vec<app_wallets::ExtendedPublicKey>, RustCError> {
    let mut result = vec![];
    for ele in keys {
        let xpub = recover_c_char(ele.xpub);
        let path = recover_c_char(ele.path);
        let path = normalize_path(&path);
        let derivation_path =
            DerivationPath::from_str(&path).map_err(|_e| RustCError::InvalidHDPath)?;
        let key = match xpub.len() {
            //32 bytes ed25519 public key or 64 bytes bip32-ed25519 xpub;
            64 | 128 => hex::decode(&xpub).map_err(|_e| RustCError::InvalidXPub)?,
            _ => Xpub::from_str(&xpub)
                .map_err(|_e| RustCError::InvalidXPub)?
                .encode()
                .to_vec(),
        };
        result.push(app_wallets::ExtendedPublicKey::new(derivation_path, key));
    }
    Ok(result)
}
