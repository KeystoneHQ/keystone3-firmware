#![no_std]
#![feature(vec_into_raw_parts)]

extern crate alloc;

pub mod address;
pub mod structs;

use crate::common::{
    errors::RustCError,
    ffi::{CSliceFFI, VecFFI},
    structs::{ExtendedPublicKey, SimpleResponse, TransactionCheckResult, TransactionParseResult},
    types::{Ptr, PtrBytes, PtrString, PtrT, PtrUR},
    ur::{UREncodeResult, FRAGMENT_MAX_LENGTH_DEFAULT, FRAGMENT_UNLIMITED_LENGTH},
    utils::{recover_c_array, recover_c_char},
};
use crate::{extract_ptr_with_type, impl_c_ptr};
use alloc::{
    format, slice,
    string::{String, ToString},
    vec::Vec,
};
use app_avalanche::{
    constants::{
        C_BLOCKCHAIN_ID, C_CHAIN_PREFIX, C_TEST_BLOCKCHAIN_ID, P_BLOCKCHAIN_ID, X_BLOCKCHAIN_ID,
        X_P_CHAIN_PREFIX, X_TEST_BLOCKCHAIN_ID,
    },
    errors::AvaxError,
    get_avax_tx_header, get_avax_tx_type_id, parse_avax_tx,
    transactions::{
        base_tx::{avax_base_sign, BaseTx},
        export::ExportTx,
        import::ImportTx,
        type_id::{self, TypeId},
        C_chain::{evm_export::ExportTx as CchainExportTx, evm_import::ImportTx as CchainImportTx},
        P_chain::{
            add_permissionless_delegator::AddPermissLessionDelegatorTx,
            add_permissionless_validator::AddPermissLessionValidatorTx,
        },
    },
};
use bitcoin::ecdsa::Signature;
use structs::DisplayAvaxTx;
use {
    hex,
    ur_registry::{
        avalanche::{avax_sign_request::AvaxSignRequest, avax_signature::AvaxSignature},
        traits::RegistryItem,
    },
};
#[derive(Debug, Clone)]
pub struct DerivationPath {
    pub base_path: String,
    pub full_path: String,
}

#[no_mangle]
pub extern "C" fn avax_parse_transaction(
    ptr: PtrUR,
    mfp: PtrBytes,
    mfp_len: u32,
    public_keys: PtrT<CSliceFFI<ExtendedPublicKey>>,
) -> PtrT<TransactionParseResult<DisplayAvaxTx>> {
    parse_transaction_by_type(extract_ptr_with_type!(ptr, AvaxSignRequest), public_keys)
}

fn parse_transaction_by_type(
    sign_request: &mut AvaxSignRequest,
    public_keys: PtrT<CSliceFFI<ExtendedPublicKey>>,
) -> PtrT<TransactionParseResult<DisplayAvaxTx>> {
    let tx_data = sign_request.get_tx_data();
    let type_id = get_avax_tx_type_id(sign_request.get_tx_data()).unwrap();

    unsafe {
        let path = get_avax_tx_type_id(sign_request.get_tx_data())
            .map_err(|_| AvaxError::InvalidInput)
            .and_then(|type_id| {
                determine_derivation_path(type_id, &sign_request, sign_request.get_wallet_index())
            })
            .unwrap();

        let mut address = String::new();
        for key in recover_c_array(public_keys).iter() {
            if recover_c_char(key.path) == path.base_path {
                address = match (type_id, path.base_path.as_str()) {
                    (TypeId::CchainExportTx, "m/44'/60'/0'") => {
                        app_ethereum::address::derive_address(
                            &path.full_path.as_str(),
                            &recover_c_char(key.xpub),
                            &path.base_path.as_str(),
                        )
                        .unwrap()
                    }
                    _ => app_avalanche::get_address(
                        app_avalanche::network::Network::AvaxMainNet,
                        &path.full_path.as_str(),
                        &recover_c_char(key.xpub).as_str(),
                        &path.base_path.as_str(),
                    )
                    .unwrap(),
                }
            }
        }

        macro_rules! parse_tx {
            ($tx_type:ty) => {
                parse_avax_tx::<$tx_type>(tx_data)
                    .map(|parse_data| {
                        TransactionParseResult::success(
                            DisplayAvaxTx::from_tx_info(
                                parse_data,
                                path.full_path,
                                address,
                                sign_request.get_wallet_index(),
                                type_id,
                            )
                            .c_ptr(),
                        )
                        .c_ptr()
                    })
                    .unwrap_or_else(|_| {
                        TransactionParseResult::from(RustCError::InvalidMasterFingerprint).c_ptr()
                    })
            };
        }

        match type_id {
            TypeId::BaseTx => {
                let header = get_avax_tx_header(tx_data.clone()).unwrap();
                if header.get_blockchain_id() == C_BLOCKCHAIN_ID
                    || header.get_blockchain_id() == C_TEST_BLOCKCHAIN_ID
                {
                    return parse_tx!(CchainImportTx);
                } else {
                    return parse_tx!(BaseTx);
                }
            }
            TypeId::PchainExportTx | TypeId::XchainExportTx => parse_tx!(ExportTx),
            TypeId::XchainImportTx | TypeId::PchainImportTx => parse_tx!(ImportTx),
            TypeId::CchainExportTx => parse_tx!(CchainExportTx),
            TypeId::AddPermissLessionValidator => parse_tx!(AddPermissLessionValidatorTx),
            TypeId::AddPermissLessionDelegator => parse_tx!(AddPermissLessionDelegatorTx),
            _ => TransactionParseResult::from(RustCError::InvalidData(format!(
                "{:?} not support",
                type_id
            )))
            .c_ptr(),
        }
    }
}

#[no_mangle]
fn avax_sign_dynamic(
    ptr: PtrUR,
    seed: PtrBytes,
    seed_len: u32,
    fragment_length: usize,
) -> PtrT<UREncodeResult> {
    let seed = unsafe { alloc::slice::from_raw_parts(seed, seed_len as usize) };
    build_sign_result(ptr, &seed)
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

pub fn determine_derivation_path(
    type_id: TypeId,
    sign_request: &AvaxSignRequest,
    wallet_index: u64,
) -> Result<DerivationPath, AvaxError> {
    let wallet_suffix = format!("/0/{}", wallet_index);
    let blockchain_id = get_avax_tx_header(sign_request.get_tx_data())?.get_blockchain_id();
    let is_c_chain = |id: &[u8; 32]| *id == C_BLOCKCHAIN_ID || *id == C_TEST_BLOCKCHAIN_ID;

    let (base_path, full_path) = match type_id {
        TypeId::CchainExportTx => (
            C_CHAIN_PREFIX,
            format!("{}{}", C_CHAIN_PREFIX, wallet_suffix),
        ),
        TypeId::XchainImportTx | TypeId::PchainImportTx => {
            let source_chain_id =
                parse_avax_tx::<ImportTx>(sign_request.get_tx_data())?.get_source_chain_id();
            let prefix = if is_c_chain(&source_chain_id) {
                C_CHAIN_PREFIX
            } else {
                X_P_CHAIN_PREFIX
            };
            (prefix, format!("{}{}", prefix, wallet_suffix))
        }
        _ => {
            let prefix = if is_c_chain(&blockchain_id) {
                C_CHAIN_PREFIX
            } else {
                X_P_CHAIN_PREFIX
            };
            (prefix, format!("{}{}", prefix, wallet_suffix))
        }
    };

    Ok(DerivationPath {
        base_path: base_path.to_string(),
        full_path,
    })
}

fn build_sign_result(ptr: PtrUR, seed: &[u8]) -> Result<AvaxSignature, AvaxError> {
    let sign_request = extract_ptr_with_type!(ptr, AvaxSignRequest);

    let path = get_avax_tx_type_id(sign_request.get_tx_data())
        .map_err(|_| AvaxError::InvalidInput)
        .and_then(|type_id| {
            determine_derivation_path(type_id, &sign_request, sign_request.get_wallet_index())
        })?
        .full_path;

    avax_base_sign(seed, path, sign_request.get_tx_data())
        .map(|signature| AvaxSignature::new(sign_request.get_request_id(), signature.to_vec()))
}

#[no_mangle]
pub extern "C" fn avax_sign(ptr: PtrUR, seed: PtrBytes, seed_len: u32) -> PtrT<UREncodeResult> {
    avax_sign_dynamic(ptr, seed, seed_len, FRAGMENT_MAX_LENGTH_DEFAULT.clone())
}

#[no_mangle]
pub extern "C" fn avax_sign_unlimited(
    ptr: PtrUR,
    seed: PtrBytes,
    seed_len: u32,
) -> PtrT<UREncodeResult> {
    avax_sign_dynamic(ptr, seed, seed_len, FRAGMENT_UNLIMITED_LENGTH.clone())
}

#[no_mangle]
pub extern "C" fn avax_check_transaction(
    ptr: PtrUR,
    mfp: PtrBytes,
    mfp_len: u32,
) -> PtrT<TransactionCheckResult> {
    let avax_tx = extract_ptr_with_type!(ptr, AvaxSignRequest);
    let mfp: [u8; 4] = match unsafe { slice::from_raw_parts(mfp, mfp_len as usize) }.try_into() {
        Ok(mfp) => mfp,
        Err(_) => {
            return TransactionCheckResult::from(RustCError::InvalidMasterFingerprint).c_ptr();
        }
    };
    if avax_tx.get_master_fingerprint() == mfp {
        TransactionCheckResult::new().c_ptr()
    } else {
        TransactionCheckResult::from(RustCError::MasterFingerprintMismatch).c_ptr()
    }
}
