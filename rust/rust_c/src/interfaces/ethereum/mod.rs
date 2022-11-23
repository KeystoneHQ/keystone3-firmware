use alloc::boxed::Box;
use alloc::string::ToString;
use alloc::{format, slice};

use app_ethereum::errors::EthereumError;
use app_ethereum::{
    parse_fee_market_tx, parse_legacy_tx, parse_personal_message, parse_typed_data_message,
};
use keystore::algorithms::secp256k1::derive_public_key;
use third_party::hex;
use third_party::ur_registry::ethereum::eth_sign_request::EthSignRequest;
use third_party::ur_registry::ethereum::eth_signature::EthSignature;
use third_party::ur_registry::traits::RegistryItem;

use crate::extract_ptr_with_type;
use crate::interfaces::errors::RustCError;
use crate::interfaces::ethereum::structs::{
    DisplayETH, DisplayETHPersonalMessage, DisplayETHTypedData, TransactionType,
};

use crate::interfaces::structs::{TransactionCheckResult, TransactionParseResult};
use crate::interfaces::types::{PtrBytes, PtrString, PtrT, PtrUR};
use crate::interfaces::ur::{UREncodeResult, FRAGMENT_MAX_LENGTH_DEFAULT};
use crate::interfaces::utils::recover_c_char;
use crate::interfaces::KEYSTONE;

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
pub extern "C" fn eth_parse(
    ptr: PtrUR,
    xpub: PtrString,
) -> PtrT<TransactionParseResult<DisplayETH>> {
    let crypto_eth = extract_ptr_with_type!(ptr, EthSignRequest);
    let pubkey = match crypto_eth.get_derivation_path().get_path() {
        None => {
            return TransactionParseResult::from(RustCError::InvalidHDPath).c_ptr();
        }
        Some(path) => {
            let xpub = recover_c_char(xpub);
            let mut _path = path.clone();
            let root_path = "44'/60'/0'/";
            let sub_path = match _path.strip_prefix(root_path) {
                Some(path) => path,
                None => {
                    return TransactionParseResult::from(RustCError::InvalidHDPath).c_ptr();
                }
            };
            derive_public_key(&xpub, &format!("m/{}", sub_path))
        }
    };

    let transaction_type = TransactionType::from(crypto_eth.get_data_type());

    match (pubkey, transaction_type) {
        (Err(e), _) => TransactionParseResult::from(e).c_ptr(),
        (Ok(key), ty) => {
            match ty {
                TransactionType::Legacy => {
                    let tx = parse_legacy_tx(crypto_eth.get_sign_data(), key);
                    match tx {
                        Ok(t) => TransactionParseResult::success(Box::into_raw(Box::new(
                            DisplayETH::from(t),
                        )))
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
                                Ok(t) => TransactionParseResult::success(Box::into_raw(Box::new(
                                    DisplayETH::from(t),
                                )))
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
    let pubkey = match crypto_eth.get_derivation_path().get_path() {
        None => {
            return TransactionParseResult::from(RustCError::InvalidHDPath).c_ptr();
        }
        Some(path) => {
            let xpub = recover_c_char(xpub);
            let mut _path = path.clone();
            let root_path = "44'/60'/0'/";
            let sub_path = match _path.strip_prefix(root_path) {
                Some(path) => path,
                None => {
                    return TransactionParseResult::from(RustCError::InvalidHDPath).c_ptr();
                }
            };
            derive_public_key(&xpub, &format!("m/{}", sub_path))
        }
    };

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
    let pubkey = match crypto_eth.get_derivation_path().get_path() {
        None => {
            return TransactionParseResult::from(RustCError::InvalidHDPath).c_ptr();
        }
        Some(path) => {
            let xpub = recover_c_char(xpub);
            let mut _path = path.clone();
            let root_path = "44'/60'/0'/";
            let sub_path = match _path.strip_prefix(root_path) {
                Some(path) => path,
                None => {
                    return TransactionParseResult::from(RustCError::InvalidHDPath).c_ptr();
                }
            };
            derive_public_key(&xpub, &format!("m/{}", sub_path))
        }
    };

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
pub extern "C" fn eth_sign_tx(ptr: PtrUR, seed: PtrBytes, seed_len: u32) -> PtrT<UREncodeResult> {
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
                    FRAGMENT_MAX_LENGTH_DEFAULT,
                )
                .c_ptr(),
            }
        }
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
}
