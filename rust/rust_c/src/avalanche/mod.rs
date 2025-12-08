#![no_std]
#![feature(vec_into_raw_parts)]

extern crate alloc;

pub mod address;
pub mod structs;

use crate::common::{
    errors::RustCError,
    ffi::CSliceFFI,
    structs::{ExtendedPublicKey, TransactionCheckResult, TransactionParseResult},
    types::{PtrBytes, PtrT, PtrUR},
    ur::{UREncodeResult, FRAGMENT_MAX_LENGTH_DEFAULT, FRAGMENT_UNLIMITED_LENGTH},
    utils::{recover_c_array, recover_c_char},
};
use crate::{extract_array, extract_ptr_with_type};
use alloc::{format, string::String, string::ToString};
use app_avalanche::{
    constants::{
        C_BLOCKCHAIN_ID, C_CHAIN_PREFIX, C_TEST_BLOCKCHAIN_ID, P_BLOCKCHAIN_ID, X_BLOCKCHAIN_ID,
        X_P_CHAIN_PREFIX, X_TEST_BLOCKCHAIN_ID,
    },
    errors::AvaxError,
    get_avax_tx_header, get_avax_tx_type_id, parse_avax_tx,
    transactions::{
        base_tx::{avax_base_sign, BaseTx},
        c_chain::{evm_export::ExportTx as CchainExportTx, evm_import::ImportTx as CchainImportTx},
        export::ExportTx,
        import::ImportTx,
        p_chain::{
            add_permissionless_delegator::AddPermissionlessDelegatorTx,
            add_permissionless_validator::AddPermissionlessValidatorTx,
        },
        type_id::{self, TypeId},
    },
};
use structs::DisplayAvaxTx;
use {
    hex,
    ur_registry::{
        avalanche::{avax_sign_request::AvaxSignRequest, avax_signature::AvaxSignature},
        traits::RegistryItem,
    },
};

#[no_mangle]
pub unsafe extern "C" fn avax_parse_transaction(
    ptr: PtrUR,
    mfp: PtrBytes,
    mfp_len: u32,
    public_keys: PtrT<CSliceFFI<ExtendedPublicKey>>,
) -> PtrT<TransactionParseResult<DisplayAvaxTx>> {
    parse_transaction_by_type(extract_ptr_with_type!(ptr, AvaxSignRequest), public_keys)
}

unsafe fn parse_transaction_by_type(
    sign_request: &mut AvaxSignRequest,
    public_keys: PtrT<CSliceFFI<ExtendedPublicKey>>,
) -> PtrT<TransactionParseResult<DisplayAvaxTx>> {
    let tx_data = sign_request.get_tx_data();
    let type_id = match get_avax_tx_type_id(sign_request.get_tx_data()) {
        Ok(type_id) => type_id,
        Err(_) => {
            return TransactionParseResult::from(RustCError::InvalidData(
                "invalid avax tx type id".to_string(),
            ))
            .c_ptr()
        }
    };

    // Get derivation path from sign_request
    let derivation_path = sign_request.get_derivation_path();
    let full_path = match derivation_path.get_path() {
        Some(p) => format!("m/{}", p),
        None => {
            return TransactionParseResult::from(RustCError::InvalidData(
                "invalid derivation path".to_string(),
            ))
            .c_ptr()
        }
    };

    // Derive address by matching full_path with available keys
    let mut address = String::new();
    for key in recover_c_array(public_keys).iter() {
        let key_path = recover_c_char(key.path).to_lowercase();
        if full_path.starts_with(&key_path) {
            address = match key_path.as_str() {
                "m/44'/60'/0'" => app_ethereum::address::derive_address(
                    full_path.as_str(),
                    &recover_c_char(key.xpub),
                    &key_path,
                )
                .unwrap_or("no address".to_string()),
                _ => app_avalanche::get_address(
                    app_avalanche::network::Network::AvaxMainNet,
                    full_path.as_str(),
                    &recover_c_char(key.xpub),
                    &key_path,
                )
                .unwrap_or("no address".to_string()),
            };
            break;
        }
    }

    // Helper macro: given a concrete tx type `$tx_type`, parse raw tx bytes (`tx_data`)
    // into that type with `parse_avax_tx::<$tx_type>`, then convert it to the
    // UI-friendly `DisplayAvaxTx` and wrap it into `TransactionParseResult` (C pointer).
    // On parse error, returns a unified `InvalidData` result.
    macro_rules! parse_tx {
        ($tx_type:ty) => {
            parse_avax_tx::<$tx_type>(tx_data)
                .map(|parse_data| {
                    TransactionParseResult::success(
                        DisplayAvaxTx::from_tx_info(
                            parse_data,
                            full_path.clone(),
                            address.clone(),
                            type_id,
                        )
                        .c_ptr(),
                    )
                    .c_ptr()
                })
                .unwrap_or_else(|_| {
                    TransactionParseResult::from(RustCError::InvalidData(
                        "invalid data".to_string(),
                    ))
                    .c_ptr()
                })
        };
    }

    match type_id {
        TypeId::BaseTx => {
            let header = get_avax_tx_header(tx_data.clone()).unwrap();
            if header.get_blockchain_id() == C_BLOCKCHAIN_ID
                || header.get_blockchain_id() == C_TEST_BLOCKCHAIN_ID
            {
                // For C-chain import, use empty path
                parse_avax_tx::<CchainImportTx>(tx_data)
                    .map(|parse_data| {
                        TransactionParseResult::success(
                            DisplayAvaxTx::from_tx_info(
                                parse_data,
                                "".to_string(),
                                address.clone(),
                                type_id,
                            )
                            .c_ptr(),
                        )
                        .c_ptr()
                    })
                    .unwrap_or_else(|_| {
                        TransactionParseResult::from(RustCError::InvalidData(
                            "invalid data".to_string(),
                        ))
                        .c_ptr()
                    })
            } else {
                parse_tx!(BaseTx)
            }
        }
        TypeId::PchainExportTx | TypeId::XchainExportTx => parse_tx!(ExportTx),
        TypeId::XchainImportTx | TypeId::PchainImportTx => parse_tx!(ImportTx),
        TypeId::CchainExportTx => parse_tx!(CchainExportTx),
        TypeId::AddPermissionlessValidator => parse_tx!(AddPermissionlessValidatorTx),
        TypeId::AddPermissionlessDelegator => parse_tx!(AddPermissionlessDelegatorTx),
        _ => TransactionParseResult::from(RustCError::InvalidData(format!(
            "{type_id:?} not support"
        )))
        .c_ptr(),
    }
}

#[no_mangle]
unsafe fn avax_sign_dynamic(
    ptr: PtrUR,
    seed: PtrBytes,
    seed_len: u32,
    fragment_length: usize,
) -> PtrT<UREncodeResult> {
    let seed = extract_array!(seed, u8, seed_len as usize);
    build_sign_result(ptr, seed)
        .map(|v: AvaxSignature| v.try_into())
        .map_or_else(
            |e| UREncodeResult::from(e).c_ptr(),
            |v| {
                v.map_or_else(
                    |e| UREncodeResult::from(e).c_ptr(),
                    |data| {
                        UREncodeResult::encode(
                            data,
                            AvaxSignature::get_registry_type().get_type(),
                            fragment_length,
                        )
                        .c_ptr()
                    },
                )
            },
        )
}

unsafe fn build_sign_result(ptr: PtrUR, seed: &[u8]) -> Result<AvaxSignature, AvaxError> {
    let sign_request = extract_ptr_with_type!(ptr, AvaxSignRequest);

    // Get full path from derivation_path
    let derivation_path = sign_request.get_derivation_path();
    let path = match derivation_path.get_path() {
        Some(p) => format!("m/{}", p),
        None => return Err(AvaxError::InvalidInput),
    };

    avax_base_sign(seed, path, sign_request.get_tx_data())
        .map(|signature| AvaxSignature::new(sign_request.get_request_id(), signature.to_vec()))
}

#[no_mangle]
pub unsafe extern "C" fn avax_sign(
    ptr: PtrUR,
    seed: PtrBytes,
    seed_len: u32,
) -> PtrT<UREncodeResult> {
    avax_sign_dynamic(ptr, seed, seed_len, FRAGMENT_MAX_LENGTH_DEFAULT)
}

#[no_mangle]
pub unsafe extern "C" fn avax_sign_unlimited(
    ptr: PtrUR,
    seed: PtrBytes,
    seed_len: u32,
) -> PtrT<UREncodeResult> {
    avax_sign_dynamic(ptr, seed, seed_len, FRAGMENT_UNLIMITED_LENGTH)
}

#[no_mangle]
pub unsafe extern "C" fn avax_check_transaction(
    ptr: PtrUR,
    mfp: PtrBytes,
    mfp_len: u32,
) -> PtrT<TransactionCheckResult> {
    let avax_tx = extract_ptr_with_type!(ptr, AvaxSignRequest);
    let mfp: [u8; 4] = match extract_array!(mfp, u8, mfp_len as usize).try_into() {
        Ok(mfp) => mfp,
        Err(_) => {
            return TransactionCheckResult::from(RustCError::InvalidMasterFingerprint).c_ptr();
        }
    };

    match avax_tx.get_derivation_path().get_source_fingerprint() {
        Some(fingerprint) if fingerprint == mfp => TransactionCheckResult::new().c_ptr(),
        _ => TransactionCheckResult::from(RustCError::MasterFingerprintMismatch).c_ptr(),
    }
}
