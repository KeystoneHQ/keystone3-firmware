pub mod btc_only_wallet;
use core::str::FromStr;

use app_utils::normalize_path;
use bitcoin::bip32::DerivationPath;
use bitcoin::bip32::Xpub;
pub use btc_only_wallet::*;
#[cfg(feature = "cypherpunk")]
pub mod cypherpunk_wallet;
#[cfg(feature = "multi-coins")]
pub mod multi_coins_wallet;
mod structs;

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
use structs::QRHardwareCallData;

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
        "ADA_CIP_1853" => "m/1853'/1815'",
        "ADA_CIP_1854" => "m/1854'/1815'",
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
        "THOR" => "m/44'/931'",
        _ => return Response::success(false).c_ptr(),
    };
    let mut path = recover_c_char(path).to_lowercase();
    if !path.starts_with('m') {
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
    device_version: PtrString,
) -> Ptr<UREncodeResult> {
    let mfp = extract_array!(master_fingerprint, u8, master_fingerprint_length);
    let mfp = match <&[u8; 4]>::try_from(mfp) {
        Ok(mfp) => *mfp,
        Err(e) => return UREncodeResult::from(URError::UrEncodeError(e.to_string())).c_ptr(),
    };
    let public_keys = unsafe { recover_c_array(xpubs) };
    let device_version = unsafe { recover_c_char(device_version) };
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

pub fn normalize_xpub(
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
