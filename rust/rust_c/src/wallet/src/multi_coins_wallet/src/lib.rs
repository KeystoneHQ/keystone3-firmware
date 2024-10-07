#![no_std]

extern crate alloc;
pub mod aptos;
pub mod arconnect;
pub mod backpack;
pub mod bitget;
mod imtoken;
pub mod keplr;
pub mod keystone;
pub mod okx;
pub mod solana;
pub mod structs;
pub mod sui;
pub mod tonkeeper;
mod utils;
pub mod xbull;
pub mod xrp_toolkit;

pub mod thor_wallet;

use alloc::format;
use alloc::string::String;
use alloc::string::ToString;
use alloc::vec::Vec;

use app_wallets::metamask::ETHAccountTypeApp;
use app_wallets::DEVICE_TYPE;
use app_wallets::DEVICE_VERSION;
use cty::uint32_t;
use keystore::algorithms::secp256k1::derive_extend_public_key;
use keystore::errors::KeystoreError;
use third_party::bitcoin::hex::DisplayHex;
use third_party::core2::io::Read;
use third_party::ed25519_bip32_core::XPub;
use third_party::hex;
use third_party::ur_registry::crypto_account::CryptoAccount;
use third_party::ur_registry::crypto_hd_key::CryptoHDKey;
use third_party::ur_registry::crypto_key_path::CryptoKeyPath;
use third_party::ur_registry::error::URError;
use third_party::ur_registry::extend::crypto_multi_accounts::CryptoMultiAccounts;
use third_party::ur_registry::extend::qr_hardware_call::QRHardwareCall;
use third_party::ur_registry::traits::RegistryItem;

use common_rust_c::errors::RustCError;
use common_rust_c::ffi::CSliceFFI;
use common_rust_c::structs::{ExtendedPublicKey, Response};
use common_rust_c::types::{Ptr, PtrBytes, PtrString, PtrT, PtrUR};
use common_rust_c::ur::{UREncodeResult, FRAGMENT_MAX_LENGTH_DEFAULT, FRAGMENT_UNLIMITED_LENGTH};
use common_rust_c::utils::{recover_c_array, recover_c_char};
use common_rust_c::{extract_array, extract_ptr_with_type};

use crate::structs::QRHardwareCallData;

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
pub extern "C" fn check_hardware_call_path(
    path: PtrString,
    chain_type: PtrString,
) -> *mut Response<bool> {
    let chain_type_str = recover_c_char(chain_type);
    let prefix = match chain_type_str.as_str() {
        "BTC_LEGACY" => "m/44'/0'",
        "BTC_NATIVE_SEGWIT" => "m/84'",
        "BTC_TAPROOT" => "m/86'",
        "BTC" => "m/49'",
        "ETH" => "m/44'/60'",
        "SOL" => "m/44'/501'",
        "XRP" => "m/44'/144'",
        "ADA" => "m/1852'/1815'",
        "TRX" => "m/44'/195'",
        "LTC" => "m/49'/2'",
        "BCH" => "m/44'/145'",
        "APT" => "m/44'/637'",
        "SUI" => "m/44'/784'",
        "DASH" => "m/44'/5'",
        "AR" => "m/44'/472'",
        "XLM" => "m/44'/148'",
        "TIA" => "m/44'/118'",
        "ATOM" => "m/44'/118'",
        "DYM" => "m/44'/118'",
        "OSMO" => "m/44'/118'",
        "INJ" => "m/44'/60'",
        "CRO" => "m/44'/394'",
        "KAVA" => "m/44'/459'",
        "LUNC" => "m/44'/330'",
        "AXL" => "m/44'/118'",
        "LUNA" => "m/44'/330'",
        "AKT" => "m/44'/118'",
        "STRD" => "m/44'/118'",
        "SCRT" => "m/44'/529'",
        "BLD" => "m/44'/564'",
        "CTK" => "m/44'/118'",
        "EVMOS" => "m/44'/60'",
        "STARS" => "m/44'/118'",
        "XPRT" => "m/44'/118'",
        "SOMM" => "m/44'/118'",
        "JUNO" => "m/44'/118'",
        "IRIS" => "m/44'/118'",
        "DVPN" => "m/44'/118'",
        "ROWAN" => "m/44'/118'",
        "REGEN" => "m/44'/118'",
        "BOOT" => "m/44'/118'",
        "GRAV" => "m/44'/118'",
        "IXO" => "m/44'/118'",
        "NGM" => "m/44'/118'",
        "IOV" => "m/44'/234'",
        "UMEE" => "m/44'/118'",
        "QCK" => "m/44'/118'",
        "TGD" => "m/44'/118'",
        _ => return Response::success(false).c_ptr(),
    };
    let mut path = recover_c_char(path).to_lowercase();
    if !path.starts_with("m") {
        path = format!("m/{}", path);
    }
    let result = path.starts_with(prefix);
    Response::success(result).c_ptr()
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
            let xpub_decode = hex::decode(xpub);

            match xpub_decode {
                Ok(v) => {
                    if v.len() >= 78 {
                        // sec256k1 xpub
                        let xpub = third_party::bitcoin::bip32::Xpub::decode(&v);
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
                    } else if (v.len() == 32) {
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
                Err(e) => {
                    rust_tools::debug!(format!("cant decode xpub error: {:?}", e));
                    Err(URError::UrEncodeError(e.to_string()))
                }
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
