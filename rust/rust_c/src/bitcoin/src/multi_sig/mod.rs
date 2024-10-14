pub mod structs;

use alloc::string::ToString;
use alloc::vec::Vec;
use alloc::{format, slice, vec};

use app_bitcoin::errors::BitcoinError;
use app_bitcoin::multi_sig::address::create_multi_sig_address_for_wallet;
use app_bitcoin::multi_sig::wallet::{
    export_wallet_by_ur, parse_bsms_wallet_config, parse_wallet_config, strict_verify_wallet_config,
};
use app_bitcoin::multi_sig::{
    export_xpub_by_crypto_account, extract_xpub_info_from_crypto_account,
};
use core::str::FromStr;
use cty::c_char;
use structs::{MultiSigFormatType, MultiSigXPubInfoItem};

use third_party::cryptoxide::hashing::sha256;
use third_party::hex;

use third_party::ur_registry::bytes::Bytes;

use common_rust_c::errors::RustCError;
use common_rust_c::ffi::CSliceFFI;

use common_rust_c::structs::{ExtendedPublicKey, Response, SimpleResponse};
use common_rust_c::types::{Ptr, PtrBytes, PtrString, PtrT, PtrUR};
use common_rust_c::ur::{UREncodeResult, ViewType, FRAGMENT_MAX_LENGTH_DEFAULT};
use common_rust_c::utils::{convert_c_char, recover_c_array, recover_c_char};

use crate::multi_sig::structs::{MultiSigWallet, NetworkType};

use common_rust_c::extract_ptr_with_type;
use third_party::ur_registry::crypto_account::CryptoAccount;
use third_party::ur_registry::error::URError;
use third_party::ur_registry::traits::RegistryItem;

#[no_mangle]
pub extern "C" fn export_multi_sig_xpub_by_ur(
    master_fingerprint: *mut u8,
    length: u32,
    public_keys: PtrT<CSliceFFI<ExtendedPublicKey>>,
    network: NetworkType,
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

        let mut extended_public_keys = vec![];
        let mut extend_public_key_paths = vec![];
        for key in keys {
            extended_public_keys.push(recover_c_char(key.xpub));
            extend_public_key_paths.push(recover_c_char(key.path));
        }

        let mfp = match <&[u8; 4]>::try_from(mfp) {
            Ok(mfp) => mfp,
            Err(e) => {
                return UREncodeResult::from(URError::UrEncodeError(e.to_string())).c_ptr();
            }
        };

        let extended_public_keys = extended_public_keys
            .iter()
            .map(|x| x.trim())
            .collect::<Vec<_>>();

        let extend_public_key_paths = extend_public_key_paths
            .iter()
            .map(|x| x.trim())
            .collect::<Vec<_>>();

        let result = export_xpub_by_crypto_account(
            mfp,
            extended_public_keys.as_slice(),
            extend_public_key_paths.as_slice(),
            network.into(),
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
    }
}

#[no_mangle]
pub extern "C" fn export_multi_sig_wallet_by_ur_test(
    master_fingerprint: *mut u8,
    length: u32,
    multi_sig_wallet: PtrT<MultiSigWallet>,
) -> *mut UREncodeResult {
    if length != 4 {
        return UREncodeResult::from(URError::UrEncodeError(format!(
            "master fingerprint length must be 4, current is {}",
            length
        )))
        .c_ptr();
    }
    unsafe {
        let master_fingerprint = slice::from_raw_parts(master_fingerprint, length as usize);
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

        let multi_sig_wallet = extract_ptr_with_type!(multi_sig_wallet, MultiSigWallet);

        let result = export_wallet_by_ur(&multi_sig_wallet.into(), &master_fingerprint.to_string());

        result.map_or_else(
            |e| UREncodeResult::from(e).c_ptr(),
            |data| {
                data.try_into().map_or_else(
                    |e| UREncodeResult::from(e).c_ptr(),
                    |data| {
                        UREncodeResult::encode(
                            data,
                            Bytes::get_registry_type().get_type(),
                            FRAGMENT_MAX_LENGTH_DEFAULT.clone(),
                        )
                        .c_ptr()
                    },
                )
            },
        )
    }
}

#[no_mangle]
#[cfg(feature = "btc-only")]
pub extern "C" fn export_xpub_info_by_ur(
    ur: PtrUR,
    multi_sig_type: MultiSigFormatType,
    viewType: ViewType,
) -> Ptr<Response<MultiSigXPubInfoItem>> {
    match viewType {
        ViewType::MultisigCryptoImportXpub => {
            let crypto_account = extract_ptr_with_type!(ur, CryptoAccount);
            let result =
                extract_xpub_info_from_crypto_account(&crypto_account, multi_sig_type.into());
            match result {
                Ok(wallet) => {
                    Response::success_ptr(MultiSigXPubInfoItem::from(wallet).c_ptr()).c_ptr()
                }
                Err(e) => Response::from(e).c_ptr(),
            }
        }
        ViewType::MultisigBytesImportXpub => {
            let bytes = extract_ptr_with_type!(ur, Bytes).clone();
            let result = parse_bsms_wallet_config(bytes);
            match result {
                Ok(wallet) => {
                    Response::success_ptr(MultiSigXPubInfoItem::from(wallet).c_ptr()).c_ptr()
                }
                Err(e) => Response::from(e).c_ptr(),
            }
        }
        _ => Response::from(BitcoinError::InvalidInput).c_ptr(),
    }
}

#[no_mangle]
pub extern "C" fn export_multi_sig_wallet_by_ur(
    master_fingerprint: *mut u8,
    length: u32,
    config: PtrString,
) -> *mut UREncodeResult {
    if length != 4 {
        return UREncodeResult::from(URError::UrEncodeError(format!(
            "master fingerprint length must be 4, current is {}",
            length
        )))
        .c_ptr();
    }
    unsafe {
        let master_fingerprint = slice::from_raw_parts(master_fingerprint, length as usize);
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
        // let xfp = hex::encode(xfp);
        let config = recover_c_char(config);
        let multi_sig_wallet =
            match parse_wallet_config(config.as_str(), &master_fingerprint.to_string()) {
                Ok(wallet) => wallet,
                Err(e) => return UREncodeResult::from(e).c_ptr(),
            };

        let result = export_wallet_by_ur(&multi_sig_wallet, &master_fingerprint.to_string());

        result.map_or_else(
            |e| UREncodeResult::from(e).c_ptr(),
            |data| {
                data.try_into().map_or_else(
                    |e| UREncodeResult::from(e).c_ptr(),
                    |data| {
                        UREncodeResult::encode(
                            data,
                            Bytes::get_registry_type().get_type(),
                            FRAGMENT_MAX_LENGTH_DEFAULT.clone(),
                        )
                        .c_ptr()
                    },
                )
            },
        )
    }
}

#[no_mangle]
pub extern "C" fn import_multi_sig_wallet_by_ur(
    ur: PtrUR,
    master_fingerprint: PtrBytes,
    master_fingerprint_len: u32,
) -> Ptr<Response<MultiSigWallet>> {
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

    let bytes = extract_ptr_with_type!(ur, Bytes);

    let result =
        app_bitcoin::multi_sig::wallet::import_wallet_by_ur(bytes, &master_fingerprint.to_string());

    match result {
        Ok(wallet) => Response::success_ptr(MultiSigWallet::from(wallet).c_ptr()).c_ptr(),
        Err(e) => Response::from(e).c_ptr(),
    }
}

#[no_mangle]
pub extern "C" fn import_multi_sig_wallet_by_file(
    content: PtrString,
    master_fingerprint: PtrBytes,
    master_fingerprint_len: u32,
) -> Ptr<Response<MultiSigWallet>> {
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

    let content = recover_c_char(content);

    let result = parse_wallet_config(&content, &master_fingerprint.to_string());

    match result {
        Ok(wallet) => Response::success_ptr(MultiSigWallet::from(wallet).c_ptr()).c_ptr(),
        Err(e) => Response::from(e).c_ptr(),
    }
}

#[no_mangle]
pub extern "C" fn generate_address_for_multisig_wallet_config(
    wallet_config: PtrString,
    account: u32,
    index: u32,
    master_fingerprint: PtrBytes,
    master_fingerprint_len: u32,
) -> Ptr<SimpleResponse<c_char>> {
    if master_fingerprint_len != 4 {
        return SimpleResponse::from(RustCError::InvalidMasterFingerprint).simple_c_ptr();
    }
    let master_fingerprint = unsafe { core::slice::from_raw_parts(master_fingerprint, 4) };
    let master_fingerprint = match third_party::bitcoin::bip32::Fingerprint::from_str(
        hex::encode(master_fingerprint.to_vec()).as_str(),
    )
    .map_err(|_e| RustCError::InvalidMasterFingerprint)
    {
        Ok(mfp) => mfp,
        Err(e) => {
            return SimpleResponse::from(e).simple_c_ptr();
        }
    };
    let content = recover_c_char(wallet_config);
    match parse_wallet_config(&content, &master_fingerprint.to_string()) {
        Ok(config) => match create_multi_sig_address_for_wallet(&config, account, index) {
            Ok(result) => SimpleResponse::success(convert_c_char(result)).simple_c_ptr(),
            Err(e) => SimpleResponse::from(e).simple_c_ptr(),
        },
        Err(e) => SimpleResponse::from(e).simple_c_ptr(),
    }
}

#[no_mangle]
pub extern "C" fn generate_psbt_file_name(
    // origin_file_name: PtrString,
    // wallet_name: PtrString,
    psbt_hex: PtrBytes,
    psbt_len: u32,
    time_stamp: u32,
) -> Ptr<SimpleResponse<c_char>> {
    // let wallet_name = recover_c_char(wallet_name);
    let psbt_hex = unsafe { slice::from_raw_parts(psbt_hex, psbt_len as usize) };
    let hash = sha256(psbt_hex);
    let checksum = hex::encode(&hash[0..4]);
    let name = format!("tx_{checksum}_{time_stamp}_signed.psbt");
    SimpleResponse::success(convert_c_char(name)).simple_c_ptr()
    // if origin_file_name.is_null() {

    // } else {
    //     let origin_file_name = recover_c_char(origin_file_name);
    //     if origin_file_name.contains("signed") {
    //         return SimpleResponse::success(convert_c_char(origin_file_name)).simple_c_ptr()
    //     }
    //     else {

    //     }
    // }
}

#[no_mangle]
pub extern "C" fn parse_and_verify_multisig_config(
    seed: PtrBytes,
    seed_len: u32,
    wallet_config: PtrString,
    master_fingerprint: PtrBytes,
    master_fingerprint_len: u32,
) -> Ptr<Response<MultiSigWallet>> {
    if master_fingerprint_len != 4 {
        return Response::from(RustCError::InvalidMasterFingerprint).c_ptr();
    }
    let seed = unsafe { core::slice::from_raw_parts(seed, seed_len as usize) };
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
    let content = recover_c_char(wallet_config);
    match parse_wallet_config(&content, &master_fingerprint.to_string()) {
        Ok(mut config) => {
            match strict_verify_wallet_config(seed, &mut config, &master_fingerprint.to_string()) {
                Ok(()) => Response::success(MultiSigWallet::from(config)).c_ptr(),
                Err(e) => Response::from(e).c_ptr(),
            }
        }
        Err(e) => Response::from(e).c_ptr(),
    }
}
