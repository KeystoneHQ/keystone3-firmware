pub mod structs;

use crate::common::errors::{KeystoneError, RustCError};
use crate::common::keystone;
use crate::common::structs::{SimpleResponse, TransactionCheckResult, TransactionParseResult};
use crate::common::types::{PtrBytes, PtrString, PtrT, PtrUR};
use crate::common::ur::{QRCodeType, UREncodeResult};
use crate::common::utils::{convert_c_char, recover_c_char};
use crate::{extract_array, extract_array_mut};
use alloc::boxed::Box;
use alloc::slice;
use alloc::string::{String, ToString};
use alloc::vec::Vec;
use cty::c_char;
use structs::{DisplayTron, TransactionType};

use crate::extract_ptr_with_type;
use alloc::format;
use structs::DisplayTRONPersonalMessage;
use ur_registry::traits::{RegistryItem, To};
use ur_registry::tron::tron_sign_request::TronSignRequest;
use ur_registry::tron::tron_signature::TronSignature;
use zeroize::Zeroize;

fn request_path(req: &TronSignRequest) -> Option<String> {
    req.get_derivation_path()
        .get_path()
        .map(|path| format!("m/{path}"))
        .filter(|path| app_tron::validate_derivation_path(path).is_ok())
}

#[no_mangle]
pub unsafe extern "C" fn tron_check_sign_request(
    ptr: PtrUR,
    x_pub: PtrString,
    master_fingerprint: PtrBytes,
    length: u32,
) -> PtrT<TransactionCheckResult> {
    if length != 4 {
        return TransactionCheckResult::from(RustCError::InvalidMasterFingerprint).c_ptr();
    }
    let req = extract_ptr_with_type!(ptr, TronSignRequest);
    let mfp = extract_array!(master_fingerprint, u8, 4);
    if let Ok(mfp) = (mfp.try_into() as Result<[u8; 4], _>) {
        let derivation_path = req.get_derivation_path();
        if let Some(ur_mfp) = derivation_path.get_source_fingerprint() {
            if mfp != ur_mfp {
                return TransactionCheckResult::from(RustCError::MasterFingerprintMismatch).c_ptr();
            }
        } else {
            return TransactionCheckResult::from(RustCError::MasterFingerprintMismatch).c_ptr();
        }
    } else {
        return TransactionCheckResult::from(RustCError::InvalidMasterFingerprint).c_ptr();
    }

    let x_pub_recovered = recover_c_char(x_pub);
    let xpub_str = x_pub_recovered.as_str();
    let sign_data = req.get_sign_data();
    if let Err(e) = app_tron::validate_sign_data_len(sign_data.len()) {
        return TransactionCheckResult::from(e).c_ptr();
    }
    let path = match request_path(req) {
        Some(p) => p,
        None => return TransactionCheckResult::from(RustCError::InvalidHDPath).c_ptr(),
    };

    let transaction_type = TransactionType::from(req.get_data_type());
    match transaction_type {
        TransactionType::Transaction => {
            match app_tron::check_tx_request(&sign_data, &path, xpub_str) {
                Ok(_) => TransactionCheckResult::new().c_ptr(),
                Err(e) => TransactionCheckResult::from(e).c_ptr(),
            }
        }
        TransactionType::PersonalMessage => match try_get_trx_public_key(xpub_str, req) {
            Ok(_) => TransactionCheckResult::new().c_ptr(),
            Err(e) => TransactionCheckResult::from(e).c_ptr(),
        },
    }
}

#[no_mangle]
pub unsafe extern "C" fn tron_parse_sign_request(
    ptr: PtrUR,
) -> *mut TransactionParseResult<DisplayTron> {
    let req = extract_ptr_with_type!(ptr, TronSignRequest);
    let json_bytes = req.get_sign_data();
    let path = match request_path(req) {
        Some(path) => path,
        None => return TransactionParseResult::from(RustCError::InvalidHDPath).c_ptr(),
    };

    app_tron::parse_tx_request(&json_bytes, &path).map_or_else(
        |e| TransactionParseResult::from(e).c_ptr(),
        |parsed_tx| {
            let display_tx = DisplayTron::from(parsed_tx);

            TransactionParseResult::success(Box::into_raw(Box::new(display_tx))).c_ptr()
        },
    )
}

#[no_mangle]
pub unsafe extern "C" fn tron_sign_request(
    ptr: PtrUR,
    seed: PtrBytes,
    seed_len: u32,
    fragment_len: usize,
) -> *mut UREncodeResult {
    if seed.is_null() || !matches!(seed_len, 16 | 32 | 64) {
        return UREncodeResult::from(KeystoneError::SignTxFailed(
            "invalid seed pointer or length; expected 16, 32, or 64 bytes".into(),
        ))
        .c_ptr();
    }
    let seed_slice = extract_array_mut!(seed, u8, seed_len as usize);

    let sign_res = (|| -> Result<Vec<u8>, KeystoneError> {
        if ptr.is_null() {
            return Err(KeystoneError::SignTxFailed("null TRON request".into()));
        }
        let req = extract_ptr_with_type!(ptr, TronSignRequest);
        let sign_data = req.get_sign_data();
        app_tron::validate_sign_data_len(sign_data.len())
            .map_err(|e| KeystoneError::SignTxFailed(e.to_string()))?;
        let request_id = req.get_request_id();
        let path = request_path(req).ok_or(KeystoneError::SignTxFailed(
            "invalid or missing TRON derivation path".to_string(),
        ))?;
        let expected = req
            .get_derivation_path()
            .get_source_fingerprint()
            .ok_or_else(|| KeystoneError::SignTxFailed("missing TRON source fingerprint".into()))?;
        let actual = keystore::algorithms::secp256k1::get_master_fingerprint_by_seed(seed_slice)
            .map_err(|e| KeystoneError::SignTxFailed(e.to_string()))?;
        if actual != expected.into() {
            return Err(KeystoneError::SignTxFailed(
                "TRON fingerprint does not match signing seed".into(),
            ));
        }

        let signed_tx_hex = match TransactionType::from(req.get_data_type()) {
            TransactionType::Transaction => {
                app_tron::sign_tx_request(&sign_data, &path, seed_slice)
                    .map_err(|e| KeystoneError::SignTxFailed(e.to_string()))?
            }
            TransactionType::PersonalMessage => {
                app_tron::sign_personal_message(&sign_data, &path, seed_slice)
                    .map_err(|e| KeystoneError::SignTxFailed(e.to_string()))?
            }
        };

        let signed_tx_bytes = hex::decode(signed_tx_hex)
            .map_err(|_| KeystoneError::SignTxFailed("Invalid Hex output".to_string()))?;
        let sig_obj = TronSignature::new(request_id, signed_tx_bytes);
        sig_obj
            .to_bytes()
            .map_err(|e| KeystoneError::SignTxFailed(e.to_string()))
    })();

    seed_slice.zeroize();

    match sign_res {
        Ok(data) => UREncodeResult::encode(
            data,
            TronSignature::get_registry_type().get_type(),
            fragment_len,
        )
        .c_ptr(),
        Err(e) => UREncodeResult::from(e).c_ptr(),
    }
}

#[no_mangle]
pub unsafe extern "C" fn tron_check_keystone(
    ptr: PtrUR,
    ur_type: QRCodeType,
    master_fingerprint: PtrBytes,
    length: u32,
    x_pub: PtrString,
) -> PtrT<TransactionCheckResult> {
    keystone::check(ptr, ur_type, master_fingerprint, length, x_pub)
}

#[no_mangle]
pub unsafe extern "C" fn tron_parse_keystone(
    ptr: PtrUR,
    ur_type: QRCodeType,
    master_fingerprint: PtrBytes,
    length: u32,
    x_pub: PtrString,
) -> *mut TransactionParseResult<DisplayTron> {
    if length != 4 {
        return TransactionParseResult::from(RustCError::InvalidMasterFingerprint).c_ptr();
    }
    keystone::build_payload(ptr, ur_type).map_or_else(
        |e| TransactionParseResult::from(e).c_ptr(),
        |payload| {
            keystone::build_parse_context(master_fingerprint, x_pub).map_or_else(
                |e| TransactionParseResult::from(e).c_ptr(),
                |context| {
                    app_tron::parse_raw_tx(payload, context).map_or_else(
                        |e| TransactionParseResult::from(e).c_ptr(),
                        |res| {
                            TransactionParseResult::success(Box::into_raw(Box::new(
                                DisplayTron::from(res),
                            )))
                            .c_ptr()
                        },
                    )
                },
            )
        },
    )
}

#[no_mangle]
pub unsafe extern "C" fn tron_sign_keystone(
    ptr: PtrUR,
    ur_type: QRCodeType,
    master_fingerprint: PtrBytes,
    length: u32,
    x_pub: PtrString,
    cold_version: i32,
    seed: PtrBytes,
    seed_len: u32,
) -> *mut UREncodeResult {
    let seed = extract_array!(seed, u8, seed_len as usize);
    keystone::sign(
        ptr,
        ur_type,
        master_fingerprint,
        length,
        x_pub,
        cold_version,
        seed,
    )
}

#[no_mangle]
pub unsafe extern "C" fn tron_get_address(
    hd_path: PtrString,
    x_pub: PtrString,
) -> *mut SimpleResponse<c_char> {
    let x_pub = recover_c_char(x_pub);
    let hd_path = recover_c_char(hd_path);
    let address = app_tron::get_address(hd_path, &x_pub);
    match address {
        Ok(result) => SimpleResponse::success(convert_c_char(result) as *mut c_char).simple_c_ptr(),
        Err(e) => SimpleResponse::from(e).simple_c_ptr(),
    }
}

fn try_get_trx_public_key(
    xpub: &str,
    trx_sign_request: &TronSignRequest,
) -> Result<bitcoin::secp256k1::PublicKey, RustCError> {
    let path = request_path(trx_sign_request).ok_or(RustCError::InvalidHDPath)?;
    app_tron::get_public_key(&path, xpub).map_err(|e| RustCError::UnexpectedError(e.to_string()))
}

#[no_mangle]
pub unsafe extern "C" fn tron_parse_personal_message(
    ptr: PtrUR,
    xpub: PtrString,
) -> PtrT<TransactionParseResult<DisplayTRONPersonalMessage>> {
    let crypto_trx = extract_ptr_with_type!(ptr, TronSignRequest);
    let xpub = recover_c_char(xpub);

    let pubkey = match try_get_trx_public_key(&xpub, crypto_trx) {
        Ok(pubkey) => pubkey,
        Err(e) => return TransactionParseResult::from(e).c_ptr(),
    };
    let transaction_type = TransactionType::from(crypto_trx.get_data_type());

    match transaction_type {
        TransactionType::PersonalMessage => {
            match app_tron::parse_personal_message(&crypto_trx.get_sign_data(), Some(pubkey)) {
                Ok(tx) => {
                    TransactionParseResult::success(DisplayTRONPersonalMessage::from(tx).c_ptr())
                        .c_ptr()
                }
                Err(e) => TransactionParseResult::from(e).c_ptr(),
            }
        }
        _ => TransactionParseResult::from(RustCError::UnsupportedTransaction(
            "TypedTransaction or TypedData".to_string(),
        ))
        .c_ptr(),
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::common::free::free_ur_encode_result;
    use crate::common::ur::FRAGMENT_UNLIMITED_LENGTH;
    use ur_registry::crypto_key_path::CryptoKeyPath;
    use ur_registry::tron::tron_sign_request::DataType;

    fn request(path: &str, data_type: DataType) -> TronSignRequest {
        TronSignRequest::new(
            None,
            b"Hello TRON".to_vec(),
            data_type,
            CryptoKeyPath::from_path(path.to_string(), Some([0x73, 0xc5, 0xda, 0x0a])).unwrap(),
            None,
            None,
        )
    }

    fn test_seed() -> Vec<u8> {
        hex::decode("5eb00bbddcf069084889a8ab9155568165f5c453ccb85e70811aaed6f6da5fc19a5ac40b389cd370d086206dec8aa6c43daea6690f20ad3d8d48b2d2ce9e38e4").unwrap()
    }

    fn signable_request(data_type: DataType, fingerprint: Option<[u8; 4]>) -> TronSignRequest {
        let mut req = request("m/44'/195'/0'/0/0", data_type.clone());
        req.set_derivation_path(
            CryptoKeyPath::from_path("m/44'/195'/0'/0/0".into(), fingerprint).unwrap(),
        );
        if data_type == DataType::Transaction {
            req.set_sign_data(hex::decode(concat!(
                "0a0200012208020202020202020240d00f5a65080112610a2d",
                "747970652e676f6f676c65617069732e636f6d2f70726f746f636f6c2e",
                "5472616e73666572436f6e747261637412300a1541c8599111f29c1e1e061265b4af93ea1f274ad78a",
                "121541b6e708a39781c96bd399c7657780ff9fe9f052a8180170e807",
            )).unwrap());
        }
        req
    }

    fn sign_via_ffi(req: &mut TronSignRequest, seed: &mut [u8]) -> Option<Vec<u8>> {
        assert!(seed.iter().any(|byte| *byte != 0));
        let encoded = unsafe {
            let result = tron_sign_request(
                req as *mut TronSignRequest as PtrUR,
                seed.as_mut_ptr(),
                seed.len() as u32,
                FRAGMENT_UNLIMITED_LENGTH,
            );
            assert!(!result.is_null());
            let encoded = if (*result).data.is_null() {
                None
            } else {
                Some(recover_c_char((*result).data))
            };
            free_ur_encode_result(result);
            encoded
        };
        assert!(
            seed.iter().all(|byte| *byte == 0),
            "FFI retained seed bytes"
        );
        encoded.map(|encoded| {
            let (_, cbor) = ur::decode(&encoded.to_lowercase()).unwrap();
            minicbor::decode::<TronSignature>(&cbor)
                .unwrap()
                .get_signature()
        })
    }

    #[test]
    fn final_signing_rejects_null_seed_and_invalid_lengths_without_memory_access() {
        let mut req = signable_request(DataType::PersonalMessage, Some([0x73, 0xc5, 0xda, 0x0a]));
        for len in [0, 1, 15, 17, 31, 33, 63, 65, u32::MAX] {
            let mut storage = [0x42; 66];
            unsafe {
                let result = tron_sign_request(
                    &mut req as *mut TronSignRequest as PtrUR,
                    storage.as_mut_ptr().add(1),
                    len,
                    FRAGMENT_UNLIMITED_LENGTH,
                );
                assert!((*result).data.is_null());
                free_ur_encode_result(result);
            }
            assert_eq!(
                storage, [0x42; 66],
                "invalid length {len} wrote caller memory"
            );
        }
        for len in [0, 16, 32, 64, u32::MAX] {
            unsafe {
                let result = tron_sign_request(
                    &mut req as *mut TronSignRequest as PtrUR,
                    core::ptr::null_mut(),
                    len,
                    FRAGMENT_UNLIMITED_LENGTH,
                );
                assert!((*result).data.is_null());
                free_ur_encode_result(result);
            }
        }
    }

    #[test]
    fn final_signing_accepts_supported_seed_lengths_and_preserves_canaries() {
        for len in [16, 32, 64] {
            let mut storage = [0x42; 66];
            let seed = &mut storage[1..1 + len];
            let fingerprint =
                keystore::algorithms::secp256k1::get_master_fingerprint_by_seed(seed).unwrap();
            let mut req =
                signable_request(DataType::PersonalMessage, Some(*fingerprint.as_bytes()));
            assert!(sign_via_ffi(&mut req, seed).is_some());
            assert_eq!(storage[0], 0x42);
            assert!(storage[1 + len..].iter().all(|byte| *byte == 0x42));
        }
    }

    #[test]
    fn final_signing_rejects_null_request_and_clears_valid_seed() {
        let mut seed = test_seed();
        unsafe {
            let result = tron_sign_request(
                core::ptr::null_mut(),
                seed.as_mut_ptr(),
                seed.len() as u32,
                FRAGMENT_UNLIMITED_LENGTH,
            );
            assert!((*result).data.is_null());
            free_ur_encode_result(result);
        }
        assert!(seed.iter().all(|byte| *byte == 0));
    }

    #[test]
    fn final_signing_rejects_requests_over_16_kib_and_clears_seed() {
        for data_type in [DataType::Transaction, DataType::PersonalMessage] {
            let mut req = signable_request(data_type, Some([0x73, 0xc5, 0xda, 0x0a]));
            req.set_sign_data(alloc::vec![b'a'; app_tron::MAX_SIGN_DATA_LEN + 1]);
            assert!(sign_via_ffi(&mut req, &mut test_seed()).is_none());
        }
        let mut req = signable_request(DataType::PersonalMessage, Some([0x73, 0xc5, 0xda, 0x0a]));
        req.set_sign_data(alloc::vec![b'a'; app_tron::MAX_SIGN_DATA_LEN]);
        assert!(sign_via_ffi(&mut req, &mut test_seed()).is_some());
    }

    #[test]
    fn final_signing_rejects_missing_or_wrong_fingerprint_and_clears_seed() {
        for data_type in [DataType::Transaction, DataType::PersonalMessage] {
            for fingerprint in [None, Some([0xde, 0xad, 0xbe, 0xef])] {
                let mut req = signable_request(data_type.clone(), fingerprint);
                assert!(
                    sign_via_ffi(&mut req, &mut test_seed()).is_none(),
                    "accepted {data_type:?} with fingerprint {fingerprint:?}"
                );
            }
        }
    }

    #[test]
    fn final_signing_succeeds_with_matching_fingerprint_and_clears_seed() {
        for data_type in [DataType::Transaction, DataType::PersonalMessage] {
            let mut seed = test_seed();
            let fingerprint =
                keystore::algorithms::secp256k1::get_master_fingerprint_by_seed(&seed).unwrap();
            let mut req = signable_request(data_type.clone(), Some(*fingerprint.as_bytes()));
            let path = request_path(&req).unwrap();
            let expected = match data_type {
                DataType::Transaction => {
                    app_tron::sign_tx_request(&req.get_sign_data(), &path, &seed)
                }
                DataType::PersonalMessage => {
                    app_tron::sign_personal_message(&req.get_sign_data(), &path, &seed)
                }
            }
            .unwrap();
            assert_eq!(
                sign_via_ffi(&mut req, &mut seed).unwrap(),
                hex::decode(expected).unwrap()
            );
        }
    }

    #[test]
    fn final_signing_clears_seed_on_invalid_paths_and_seed_errors() {
        for data_type in [DataType::Transaction, DataType::PersonalMessage] {
            let mut req = signable_request(data_type.clone(), Some([0x73, 0xc5, 0xda, 0x0a]));
            let mut invalid_seed = [0xff; 16];
            assert!(sign_via_ffi(&mut req, &mut invalid_seed).is_none());
            for path in [
                "m/44'/194'/0'/0/0",
                "m/44'/195'/0'/0",
                "m/44'/195'/0'/0/0/0",
            ] {
                req.set_derivation_path(
                    CryptoKeyPath::from_path(path.into(), Some([0x73, 0xc5, 0xda, 0x0a])).unwrap(),
                );
                assert!(sign_via_ffi(&mut req, &mut test_seed()).is_none());
            }
        }
    }

    #[test]
    fn final_signing_clears_seed_on_transaction_parse_and_owner_errors() {
        let mut req = signable_request(DataType::Transaction, Some([0x73, 0xc5, 0xda, 0x0a]));
        req.set_derivation_path(
            CryptoKeyPath::from_path("m/44'/195'/0'/0/1".into(), Some([0x73, 0xc5, 0xda, 0x0a]))
                .unwrap(),
        );
        assert!(sign_via_ffi(&mut req, &mut test_seed()).is_none());
        req.set_sign_data(alloc::vec![0xff]);
        assert!(sign_via_ffi(&mut req, &mut test_seed()).is_none());
    }

    #[test]
    fn requests_reject_wrong_coin_account_and_path_shape() {
        for path in [
            "m/44'/194'/0'/0/0",
            "m/44'/60'/0'/0/0",
            "m/44'/195'/1'/0/0",
            "m/44'/195'/0'/1/0",
            "m/44'/195'/0'/0",
            "m/44'/195'/0'/0/0/0",
            "m/44'/195'/0'/0/0'",
        ] {
            for data_type in [DataType::Transaction, DataType::PersonalMessage] {
                let req = request(path, data_type);
                assert!(request_path(&req).is_none(), "accepted {path}");
                assert!(matches!(
                    try_get_trx_public_key("unused", &req),
                    Err(RustCError::InvalidHDPath)
                ));
            }
        }
        assert!(request_path(&TronSignRequest::default()).is_none());
    }

    #[test]
    fn personal_message_requires_derivable_public_key() {
        let xpub = "xpub6D1AabNHCupeiLM65ZR9UStMhJ1vCpyV4XbZdyhMZBiJXALQtmn9p42VTQckoHVn8WNqS7dqnJokZHAHcHGoaQgmv8D45oNUKx6DZMNZBCd";
        for index in [0, 1, 2147483647] {
            let path = format!("m/44'/195'/0'/0/{index}");
            let req = request(&path, DataType::PersonalMessage);
            assert_eq!(request_path(&req), Some(path.clone()));
            assert_eq!(
                try_get_trx_public_key(xpub, &req).unwrap(),
                app_tron::get_public_key(&path, xpub).unwrap()
            );
            assert!(try_get_trx_public_key("invalid_xpub", &req).is_err());
        }
    }
}
