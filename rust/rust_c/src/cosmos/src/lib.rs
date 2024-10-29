#![no_std]

extern crate alloc;

pub mod structs;

use crate::structs::DisplayCosmosTx;
use alloc::format;
use alloc::string::{String, ToString};
use alloc::vec::Vec;
use app_cosmos::errors::CosmosError;
use app_cosmos::transaction::structs::SignMode;
use app_utils::normalize_path;
use common_rust_c::errors::RustCError;
use common_rust_c::extract_ptr_with_type;
use common_rust_c::structs::{SimpleResponse, TransactionCheckResult, TransactionParseResult};
use common_rust_c::types::{PtrBytes, PtrString, PtrT, PtrUR};
use common_rust_c::ur::{QRCodeType, UREncodeResult, FRAGMENT_MAX_LENGTH_DEFAULT};
use common_rust_c::utils::{convert_c_char, recover_c_char};
use core::slice;
use cty::c_char;
use either::Either;
use ur_registry::cosmos::cosmos_sign_request::{CosmosSignRequest, DataType};
use ur_registry::cosmos::cosmos_signature::CosmosSignature;
use ur_registry::cosmos::evm_sign_request::{EvmSignRequest, SignDataType};
use ur_registry::cosmos::evm_signature::EvmSignature;
use ur_registry::traits::RegistryItem;

fn get_public_key(seed: &[u8], path: &String) -> Result<Vec<u8>, CosmosError> {
    let path = normalize_path(path);
    let extended_key =
        keystore::algorithms::secp256k1::get_extended_public_key_by_seed(seed, &path);
    let public_key = match extended_key {
        Ok(xpub) => xpub.public_key,
        Err(e) => {
            return Err(CosmosError::SignFailure(format!(
                "derive public key failed {:?}",
                e
            )))
        }
    };
    Ok(public_key.serialize().to_vec())
}

fn build_sign_result(
    ptr: PtrUR,
    ur_type: QRCodeType,
    seed: &[u8],
) -> Result<Either<CosmosSignature, EvmSignature>, CosmosError> {
    match ur_type {
        QRCodeType::CosmosSignRequest => {
            let sign_request = extract_ptr_with_type!(ptr, CosmosSignRequest);
            let path = sign_request.get_derivation_paths()[0].get_path().ok_or(
                CosmosError::SignFailure("invalid derivation path".to_string()),
            )?;
            let signature = app_cosmos::sign_tx(
                sign_request.get_sign_data().to_vec(),
                &path,
                SignMode::COSMOS,
                &seed,
            )?;
            Ok(Either::Left(CosmosSignature::new(
                sign_request.get_request_id(),
                signature.to_vec(),
                get_public_key(seed, &path)?,
            )))
        }
        QRCodeType::EvmSignRequest => {
            let sign_request = extract_ptr_with_type!(ptr, EvmSignRequest);
            let path =
                sign_request
                    .get_derivation_path()
                    .get_path()
                    .ok_or(CosmosError::SignFailure(
                        "invalid derivation path".to_string(),
                    ))?;
            let signature = app_cosmos::sign_tx(
                sign_request.get_sign_data().to_vec(),
                &path,
                SignMode::EVM,
                &seed,
            )?;
            Ok(Either::Right(EvmSignature::new(
                sign_request.get_request_id(),
                signature.to_vec(),
            )))
        }
        _ => return Err(CosmosError::SignFailure("invalid ur type".to_string())),
    }
}

#[no_mangle]
pub extern "C" fn cosmos_check_tx(
    ptr: PtrUR,
    ur_type: QRCodeType,
    master_fingerprint: PtrBytes,
    length: u32,
) -> PtrT<TransactionCheckResult> {
    if length != 4 {
        return TransactionCheckResult::from(RustCError::InvalidMasterFingerprint).c_ptr();
    }
    let mfp = unsafe { slice::from_raw_parts(master_fingerprint, 4) };
    let ur_mfp = match ur_type {
        QRCodeType::CosmosSignRequest => {
            let sign_request = extract_ptr_with_type!(ptr, CosmosSignRequest);
            sign_request.get_derivation_paths()[0].get_source_fingerprint()
        }
        QRCodeType::EvmSignRequest => {
            let sign_request = extract_ptr_with_type!(ptr, EvmSignRequest);
            sign_request.get_derivation_path().get_source_fingerprint()
        }
        _ => {
            return TransactionCheckResult::from(RustCError::UnsupportedTransaction(
                "invalid ur type".to_string(),
            ))
            .c_ptr()
        }
    };

    if let Ok(mfp) = mfp.try_into() as Result<[u8; 4], _> {
        if let Some(ur_mfp) = ur_mfp {
            return if mfp == ur_mfp {
                TransactionCheckResult::new().c_ptr()
            } else {
                TransactionCheckResult::from(RustCError::MasterFingerprintMismatch).c_ptr()
            };
        }
        TransactionCheckResult::from(RustCError::MasterFingerprintMismatch).c_ptr()
    } else {
        TransactionCheckResult::from(RustCError::InvalidMasterFingerprint).c_ptr()
    }
}

#[no_mangle]
pub extern "C" fn cosmos_get_address(
    hd_path: PtrString,
    root_x_pub: PtrString,
    root_path: PtrString,
    prefix: PtrString,
) -> *mut SimpleResponse<c_char> {
    let hd_path = recover_c_char(hd_path);
    let root_x_pub = recover_c_char(root_x_pub);
    let root_path = recover_c_char(root_path);
    let prefix = recover_c_char(prefix);
    if !hd_path.starts_with(root_path.as_str()) {
        return SimpleResponse::from(CosmosError::InvalidHDPath(format!(
            "{} does not match {}",
            hd_path, root_path
        )))
        .simple_c_ptr();
    }
    let address = app_cosmos::derive_address(&hd_path, &root_x_pub, &root_path, &prefix);
    match address {
        Ok(result) => SimpleResponse::success(convert_c_char(result) as *mut c_char).simple_c_ptr(),
        Err(e) => SimpleResponse::from(e).simple_c_ptr(),
    }
}

#[no_mangle]
pub extern "C" fn cosmos_sign_tx(
    ptr: PtrUR,
    ur_type: QRCodeType,
    seed: PtrBytes,
    seed_len: u32,
) -> PtrT<UREncodeResult> {
    let seed = unsafe { slice::from_raw_parts(seed, seed_len as usize) };
    let ur_tag = match ur_type {
        QRCodeType::CosmosSignRequest => CosmosSignature::get_registry_type().get_type(),
        QRCodeType::EvmSignRequest => EvmSignature::get_registry_type().get_type(),
        _ => {
            return UREncodeResult::from(CosmosError::SignFailure(
                "unsupported ur type".to_string(),
            ))
            .c_ptr()
        }
    };
    build_sign_result(ptr, ur_type, seed)
        .map(|v| match v {
            Either::Left(sig) => sig.try_into(),
            Either::Right(sig) => sig.try_into(),
        })
        .map_or_else(
            |e| UREncodeResult::from(e).c_ptr(),
            |v| {
                v.map_or_else(
                    |e| UREncodeResult::from(e).c_ptr(),
                    |data| {
                        UREncodeResult::encode(data, ur_tag, FRAGMENT_MAX_LENGTH_DEFAULT.clone())
                            .c_ptr()
                    },
                )
            },
        )
}

#[no_mangle]
pub extern "C" fn cosmos_parse_tx(
    ptr: PtrUR,
    ur_type: QRCodeType,
) -> PtrT<TransactionParseResult<DisplayCosmosTx>> {
    let (sign_data, data_type) = match ur_type {
        QRCodeType::CosmosSignRequest => {
            let sign_request = extract_ptr_with_type!(ptr, CosmosSignRequest);
            let sign_data = sign_request.get_sign_data();
            let data_type = match sign_request.get_data_type() {
                DataType::Amino => app_cosmos::transaction::structs::DataType::Amino,
                DataType::Direct => app_cosmos::transaction::structs::DataType::Direct,
                DataType::Message => app_cosmos::transaction::structs::DataType::Amino,
                _ => {
                    return TransactionParseResult::from(CosmosError::SignFailure(
                        "unsupported cosmos sign request data type".to_string(),
                    ))
                    .c_ptr()
                }
            };
            (sign_data, data_type)
        }
        QRCodeType::EvmSignRequest => {
            let sign_request = extract_ptr_with_type!(ptr, EvmSignRequest);
            let sign_data = sign_request.get_sign_data();
            let data_type = match sign_request.get_data_type() {
                SignDataType::CosmosAmino => app_cosmos::transaction::structs::DataType::Amino,
                SignDataType::CosmosDirect => app_cosmos::transaction::structs::DataType::Direct,
                _ => {
                    return TransactionParseResult::from(CosmosError::SignFailure(
                        "unsupported cosmos sign request data type".to_string(),
                    ))
                    .c_ptr()
                }
            };
            (sign_data, data_type)
        }
        _ => {
            return TransactionParseResult::from(CosmosError::SignFailure(
                "unsupported ur type".to_string(),
            ))
            .c_ptr()
        }
    };
    match app_cosmos::parse(&sign_data.to_vec(), data_type) {
        Ok(v) => TransactionParseResult::success(DisplayCosmosTx::from(v).c_ptr()).c_ptr(),
        Err(e) => TransactionParseResult::from(e).c_ptr(),
    }
}
