#![no_std]
extern crate alloc;

use alloc::boxed::Box;
use alloc::string::{String, ToString};
use alloc::{format, slice};

use crate::structs::{
    DisplayETH, DisplayETHPersonalMessage, DisplayETHTypedData, EthParsedErc20Transaction,
    TransactionType,
};
use app_ethereum::erc20::parse_erc20;
use app_ethereum::errors::EthereumError;
use app_ethereum::{
    parse_fee_market_tx, parse_legacy_tx, parse_personal_message, parse_typed_data_message,
};
use common_rust_c::errors::RustCError;
use common_rust_c::structs::{TransactionCheckResult, TransactionParseResult};
use common_rust_c::types::{PtrBytes, PtrString, PtrT, PtrUR};
use common_rust_c::ur::{UREncodeResult, FRAGMENT_MAX_LENGTH_DEFAULT, FRAGMENT_UNLIMITED_LENGTH};
use common_rust_c::utils::{convert_c_char, recover_c_char};
use common_rust_c::{extract_ptr_with_type, KEYSTONE};
use keystore::algorithms::secp256k1::derive_public_key;
use third_party::ur_registry::ethereum::eth_sign_request::EthSignRequest;
use third_party::ur_registry::ethereum::eth_signature::EthSignature;
use third_party::ur_registry::traits::RegistryItem;

mod abi;
pub mod address;
pub mod structs;

#[no_mangle]
pub extern "C" fn eth_check(
    ptr: PtrUR,
    master_fingerprint: PtrBytes,
    length: u32,
) -> PtrT<TransactionCheckResult> {
    if length != 4 {
        return TransactionCheckResult::from(RustCError::InvalidMasterFingerprint).c_ptr();
    }
    let eth_sign_request = extract_ptr_with_type!(ptr, EthSignRequest);

    let mfp = unsafe { core::slice::from_raw_parts(master_fingerprint, 4) };
    let mfp: [u8; 4] = match mfp.try_into() {
        Ok(mfp) => mfp,
        Err(_) => {
            return TransactionCheckResult::from(RustCError::InvalidMasterFingerprint).c_ptr();
        }
    };
    let derivation_path: third_party::ur_registry::crypto_key_path::CryptoKeyPath =
        eth_sign_request.get_derivation_path();
    let ur_mfp: [u8; 4] = match derivation_path
        .get_source_fingerprint()
        .ok_or(RustCError::InvalidMasterFingerprint)
    {
        Ok(ur_mfp) => ur_mfp,
        Err(e) => {
            return TransactionCheckResult::from(e).c_ptr();
        }
    };
    if ur_mfp == mfp {
        return TransactionCheckResult::new().c_ptr();
    } else {
        return TransactionCheckResult::from(RustCError::MasterFingerprintMismatch).c_ptr();
    }
}

#[no_mangle]
pub extern "C" fn eth_get_root_path(ptr: PtrUR) -> PtrString {
    let eth_sign_request = extract_ptr_with_type!(ptr, EthSignRequest);
    let derivation_path: third_party::ur_registry::crypto_key_path::CryptoKeyPath =
        eth_sign_request.get_derivation_path();
    if let Some(path) = derivation_path.get_path() {
        if let Some(root_path) = parse_eth_root_path(path) {
            return convert_c_char(root_path);
        }
    }
    return convert_c_char("".to_string());
}

fn parse_eth_root_path(path: String) -> Option<String> {
    let root_path = "44'/60'/";
    match path.strip_prefix(root_path) {
        Some(path) => {
            if let Some(index) = path.find("/") {
                let sub_path = &path[..index];
                Some(format!("{}{}", root_path, sub_path))
            } else {
                None
            }
        }
        None => None,
    }
}

fn parse_eth_sub_path(path: String) -> Option<String> {
    let root_path = "44'/60'/";
    match path.strip_prefix(root_path) {
        Some(path) => {
            if let Some(index) = path.find("/") {
                Some(path[index + 1..].to_string())
            } else {
                None
            }
        }
        None => None,
    }
}

fn try_get_eth_public_key(
    xpub: String,
    eth_sign_request: EthSignRequest,
) -> Result<third_party::secp256k1::PublicKey, RustCError> {
    match eth_sign_request.get_derivation_path().get_path() {
        None => Err(RustCError::InvalidHDPath),
        Some(path) => {
            let _path = path.clone();
            if let Some(sub_path) = parse_eth_sub_path(_path) {
                derive_public_key(&xpub, &format!("m/{}", sub_path))
                    .map_err(|e| RustCError::UnexpectedError(format!("unable to derive pubkey")))
            } else {
                Err(RustCError::InvalidHDPath)
            }
        }
    }
}

#[no_mangle]
pub extern "C" fn eth_parse(
    ptr: PtrUR,
    xpub: PtrString,
) -> PtrT<TransactionParseResult<DisplayETH>> {
    let crypto_eth = extract_ptr_with_type!(ptr, EthSignRequest);
    let xpub = recover_c_char(xpub);
    let pubkey = try_get_eth_public_key(xpub, crypto_eth.clone());
    let transaction_type = TransactionType::from(crypto_eth.get_data_type());

    match (pubkey, transaction_type) {
        (Err(e), _) => TransactionParseResult::from(e).c_ptr(),
        (Ok(key), ty) => {
            match ty {
                TransactionType::Legacy => {
                    let tx = parse_legacy_tx(crypto_eth.get_sign_data(), key);
                    match tx {
                        Ok(t) => TransactionParseResult::success(
                            DisplayETH::from(t).c_ptr()
                        )
                        .c_ptr(),
                        Err(e) => TransactionParseResult::from(e).c_ptr(),
                    }
                }
                TransactionType::TypedTransaction => {
                    match crypto_eth.get_sign_data().get(0) {
                        Some(02) => {
                            //remove envelop
                            let payload = crypto_eth.get_sign_data()[1..].to_vec();
                            let tx = parse_fee_market_tx(payload, key);
                            match tx {
                                Ok(t) => TransactionParseResult::success(
                                    DisplayETH::from(t).c_ptr(),
                                )
                                .c_ptr(),
                                Err(e) => TransactionParseResult::from(e).c_ptr(),
                            }
                        }
                        Some(x) => TransactionParseResult::from(
                            RustCError::UnsupportedTransaction(format!("ethereum tx type:{}", x)),
                        )
                        .c_ptr(),
                        None => {
                            TransactionParseResult::from(EthereumError::InvalidTransaction).c_ptr()
                        }
                    }
                }
                _ => TransactionParseResult::from(RustCError::UnsupportedTransaction(
                    "PersonalMessage or TypedData".to_string(),
                ))
                .c_ptr(),
            }
        }
    }
}

#[no_mangle]
pub extern "C" fn eth_parse_personal_message(
    ptr: PtrUR,
    xpub: PtrString,
) -> PtrT<TransactionParseResult<DisplayETHPersonalMessage>> {
    let crypto_eth = extract_ptr_with_type!(ptr, EthSignRequest);
    let xpub = recover_c_char(xpub);
    let pubkey = try_get_eth_public_key(xpub, crypto_eth.clone());

    let transaction_type = TransactionType::from(crypto_eth.get_data_type());

    match (pubkey, transaction_type) {
        (Err(e), _) => TransactionParseResult::from(e).c_ptr(),
        (Ok(key), ty) => match ty {
            TransactionType::PersonalMessage => {
                let tx = parse_personal_message(crypto_eth.get_sign_data(), key);
                match tx {
                    Ok(t) => {
                        TransactionParseResult::success(DisplayETHPersonalMessage::from(t).c_ptr())
                            .c_ptr()
                    }
                    Err(e) => TransactionParseResult::from(e).c_ptr(),
                }
            }
            _ => TransactionParseResult::from(RustCError::UnsupportedTransaction(
                "Legacy or TypedTransaction or TypedData".to_string(),
            ))
            .c_ptr(),
        },
    }
}

#[no_mangle]
pub extern "C" fn eth_parse_typed_data(
    ptr: PtrUR,
    xpub: PtrString,
) -> PtrT<TransactionParseResult<DisplayETHTypedData>> {
    let crypto_eth = extract_ptr_with_type!(ptr, EthSignRequest);
    let xpub = recover_c_char(xpub);
    let pubkey = try_get_eth_public_key(xpub, crypto_eth.clone());

    let transaction_type = TransactionType::from(crypto_eth.get_data_type());

    match (pubkey, transaction_type) {
        (Err(e), _) => TransactionParseResult::from(e).c_ptr(),
        (Ok(key), ty) => match ty {
            TransactionType::TypedData => {
                let tx = parse_typed_data_message(crypto_eth.get_sign_data(), key);
                match tx {
                    Ok(t) => TransactionParseResult::success(DisplayETHTypedData::from(t).c_ptr())
                        .c_ptr(),
                    Err(e) => TransactionParseResult::from(e).c_ptr(),
                }
            }
            _ => TransactionParseResult::from(RustCError::UnsupportedTransaction(
                "Legacy or TypedTransaction or PersonalMessage".to_string(),
            ))
            .c_ptr(),
        },
    }
}

#[no_mangle]
pub extern "C" fn eth_sign_tx_dynamic(
    ptr: PtrUR,
    seed: PtrBytes,
    seed_len: u32,
    fragment_length: usize,
) -> PtrT<UREncodeResult> {
    let crypto_eth = extract_ptr_with_type!(ptr, EthSignRequest);
    let seed = unsafe { slice::from_raw_parts(seed, seed_len as usize) };
    let mut path = match crypto_eth.get_derivation_path().get_path() {
        Some(v) => v,
        None => return UREncodeResult::from(EthereumError::InvalidTransaction).c_ptr(),
    };
    if !path.starts_with("m/") {
        path = format!("m/{}", path);
    }

    let signature = match TransactionType::from(crypto_eth.get_data_type()) {
        TransactionType::Legacy => {
            app_ethereum::sign_legacy_tx(crypto_eth.get_sign_data().to_vec(), seed, &path)
        }
        TransactionType::TypedTransaction => match crypto_eth.get_sign_data().get(0) {
            Some(0x02) => {
                app_ethereum::sign_fee_markey_tx(crypto_eth.get_sign_data().to_vec(), seed, &path)
            }
            Some(x) => {
                return UREncodeResult::from(RustCError::UnsupportedTransaction(format!(
                    "ethereum tx type: {}",
                    x
                )))
                .c_ptr();
            }
            None => {
                return UREncodeResult::from(EthereumError::InvalidTransaction).c_ptr();
            }
        },
        TransactionType::PersonalMessage => {
            app_ethereum::sign_personal_message(crypto_eth.get_sign_data().to_vec(), seed, &path)
        }
        TransactionType::TypedData => {
            app_ethereum::sign_typed_data_message(crypto_eth.get_sign_data().to_vec(), seed, &path)
        }
    };
    match signature {
        Err(e) => UREncodeResult::from(e).c_ptr(),
        Ok(sig) => {
            let eth_signature = EthSignature::new(
                crypto_eth.get_request_id(),
                sig.serialize(),
                Some(format!("{}", KEYSTONE)),
            );
            match eth_signature.try_into() {
                Err(e) => UREncodeResult::from(e).c_ptr(),
                Ok(v) => UREncodeResult::encode(
                    v,
                    EthSignature::get_registry_type().get_type(),
                    fragment_length,
                )
                .c_ptr(),
            }
        }
    }
}

#[no_mangle]
pub extern "C" fn eth_sign_tx(ptr: PtrUR, seed: PtrBytes, seed_len: u32) -> PtrT<UREncodeResult> {
    eth_sign_tx_dynamic(ptr, seed, seed_len, FRAGMENT_MAX_LENGTH_DEFAULT.clone())
}

// _unlimited
#[no_mangle]
pub extern "C" fn eth_sign_tx_unlimited(
    ptr: PtrUR,
    seed: PtrBytes,
    seed_len: u32,
) -> PtrT<UREncodeResult> {
    eth_sign_tx_dynamic(ptr, seed, seed_len, FRAGMENT_UNLIMITED_LENGTH.clone())
}

#[no_mangle]
pub extern "C" fn eth_parse_erc20(
    input: PtrString,
    decimal: u32,
) -> PtrT<TransactionParseResult<EthParsedErc20Transaction>> {
    let input = recover_c_char(input);
    let tx = parse_erc20(&input, decimal);
    match tx {
        Ok(t) => {
            TransactionParseResult::success(EthParsedErc20Transaction::from(t).c_ptr()).c_ptr()
        }
        Err(_) => TransactionParseResult::from(EthereumError::DecodeContractDataError(
            String::from("invalid input data"),
        ))
        .c_ptr(),
    }
}
#[cfg(test)]
mod tests {
    extern crate std;

    use std::println;

    #[test]
    fn test() {
        let p = "m/44'/60'/0'/0/0";
        let prefix = "m/44'/60'/0'/";
        println!("{:?}", p.strip_prefix(prefix))
    }

    #[test]
    fn test_test() {
        let _path = "44'/60'/1'/0/0";
        let root_path = "44'/60'/";
        let sub_path = match _path.strip_prefix(root_path) {
            Some(path) => {
                if let Some(index) = path.find("/") {
                    println!("{}", &path[index..]);
                }
            }
            None => {}
        };
    }
}
