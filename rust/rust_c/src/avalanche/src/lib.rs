#![no_std]
#![feature(vec_into_raw_parts)]

extern crate alloc;

pub mod address;
pub mod structs;

use alloc::{
    format, slice,
    string::{String, ToString},
    vec::Vec,
};
use app_avalanche::{
    constants::{C_BLOCKCHAIN_ID, P_BLOCKCHAIN_ID, X_BLOCKCHAIN_ID},
    errors::AvaxError,
    get_avax_tx_type_id, parse_avax_tx,
    transactions::{
        base_tx::{avax_base_sign, BaseTx},
        export::ExportTx,
        import::ImportTx,
        type_id::TypeId,
        P_chain::{
            add_permissionless_delegator::AddPermissLessionDelegatorTx,
            add_permissionless_validator::AddPermissLessionValidatorTx,
        },
    },
};
use bitcoin::ecdsa::Signature;
use common_rust_c::{
    errors::RustCError,
    extract_ptr_with_type,
    ffi::{CSliceFFI, VecFFI},
    impl_c_ptr,
    structs::{ExtendedPublicKey, SimpleResponse, TransactionCheckResult, TransactionParseResult},
    types::{Ptr, PtrBytes, PtrString, PtrT, PtrUR},
    ur::{UREncodeResult, FRAGMENT_MAX_LENGTH_DEFAULT, FRAGMENT_UNLIMITED_LENGTH},
    utils::recover_c_char,
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
pub extern "C" fn avax_parse_transaction(
    ptr: PtrUR,
    mfp: PtrBytes,
    mfp_len: u32,
    public_keys: PtrT<CSliceFFI<ExtendedPublicKey>>,
) -> PtrT<TransactionParseResult<DisplayAvaxTx>> {
    let avax_sign_request = extract_ptr_with_type!(ptr, AvaxSignRequest);

    match get_avax_tx_type_id(avax_sign_request.get_tx_data()) {
        Ok(type_id) => parse_transaction_by_type(type_id, avax_sign_request.get_tx_data()),
        Err(_) => {
            TransactionParseResult::from(RustCError::InvalidData("Invalid tx type".to_string()))
                .c_ptr()
        }
    }
}

fn parse_transaction_by_type(
    type_id: TypeId,
    tx_data: Vec<u8>,
) -> PtrT<TransactionParseResult<DisplayAvaxTx>> {
    macro_rules! parse_tx {
        ($tx_type:ty) => {
            parse_avax_tx::<$tx_type>(tx_data)
                .map(|parse_data| {
                    TransactionParseResult::success(DisplayAvaxTx::from(parse_data).c_ptr()).c_ptr()
                })
                .unwrap_or_else(|_| {
                    TransactionParseResult::from(RustCError::InvalidMasterFingerprint).c_ptr()
                })
        };
    }

    match type_id {
        TypeId::BaseTx => parse_tx!(BaseTx),
        TypeId::PchainExportTx | TypeId::XchainExportTx => parse_tx!(ExportTx),
        TypeId::XchainImportTx | TypeId::PchainImportTx => parse_tx!(ImportTx),
        TypeId::AddPermissLessionValidator => parse_tx!(AddPermissLessionValidatorTx),
        TypeId::AddPermissLessionDelegator => parse_tx!(AddPermissLessionDelegatorTx),
        _ => TransactionParseResult::from(RustCError::InvalidData(format!(
            "{:?} not support",
            type_id
        )))
        .c_ptr(),
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

fn build_sign_result(ptr: PtrUR, seed: &[u8]) -> Result<AvaxSignature, AvaxError> {
    let sign_request = extract_ptr_with_type!(ptr, AvaxSignRequest);
    let wallet_index = sign_request.get_wallet_index();
    let path = match get_avax_tx_type_id(sign_request.get_tx_data()) {
        Ok(type_id) => match type_id {
            TypeId::CchainExportTx => format!("m/44'/60'/0'/0/{}", wallet_index),
            TypeId::BaseTx => {
                let base_tx = parse_avax_tx::<BaseTx>(sign_request.get_tx_data()).unwrap();
                if base_tx.tx_header.get_blockchain_id() == C_BLOCKCHAIN_ID {
                    format!("m/44'/60'/0'/0/{}", wallet_index)
                } else {
                    format!("m/44'/9000'/0'/0/{}", wallet_index)
                }
            }
            _ => format!("m/44'/9000'/0'/0/{}", wallet_index),
        },
        Err(_) => {
            return Err(AvaxError::InvalidInput);
        }
    };

    extern crate std;
    use std::println;
    println!("sign_request  get_request_id{:?}", sign_request.get_request_id());

    Ok(AvaxSignature::new(
        sign_request.get_request_id(),
        avax_base_sign(seed, path, sign_request.get_tx_data())?.to_vec(),
    ))
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
