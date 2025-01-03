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
    ur::{UREncodeResult, FRAGMENT_MAX_LENGTH_DEFAULT},
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

// #[no_mangle]
// fn avax_sign_dynamic(
//     ptr: PtrUR,
//     seed: PtrBytes,
//     seed_len: u32,
//     master_fingerprint: PtrBytes,
//     master_fingerprint_len: u32,
//     fragment_length: usize,
// ) -> *mut UREncodeResult {
//     if master_fingerprint_len != 4 {
//         return UREncodeResult::from(RustCError::InvalidMasterFingerprint).c_ptr();
//     }
//     let master_fingerprint = unsafe { core::slice::from_raw_parts(master_fingerprint, 4) };
//     let master_fingerprint = match bitcoin::bip32::Fingerprint::from_str(
//         hex::encode(master_fingerprint.to_vec()).as_str(),
//     )
//     .map_err(|_e| RustCError::InvalidMasterFingerprint)
//     {
//         Ok(mfp) => mfp,
//         Err(e) => {
//             return UREncodeResult::from(e).c_ptr();
//         }
//     };

//     let crypto_psbt = extract_ptr_with_type!(ptr, CryptoPSBT);
//     let psbt = crypto_psbt.get_psbt();

//     let seed = unsafe { slice::from_raw_parts(seed, seed_len as usize) };

//     let result = app_bitcoin::sign_psbt(psbt, seed, master_fingerprint);
//     match result.map(|v| CryptoPSBT::new(v).try_into()) {
//         Ok(v) => match v {
//             Ok(data) => UREncodeResult::encode(
//                 data,
//                 CryptoPSBT::get_registry_type().get_type(),
//                 fragment_length.clone(),
//             )
//             .c_ptr(),
//             Err(e) => UREncodeResult::from(e).c_ptr(),
//         },
//         Err(e) => UREncodeResult::from(e).c_ptr(),
//     }
// }

fn build_sign_result(ptr: PtrUR, seed: &[u8]) -> Result<AvaxSignature, AvaxError> {
    let sign_request = extract_ptr_with_type!(ptr, AvaxSignRequest);
    let signature = avax_base_sign(
        seed,
        format!("m/44'/9000'/0'/0/{}", sign_request.get_wallet_index()),
        sign_request.get_tx_data(),
    )
    .unwrap();
    Ok(AvaxSignature::new(
        sign_request.get_request_id(),
        signature.to_vec(),
    ))
}

#[no_mangle]
pub extern "C" fn avax_sign(ptr: PtrUR, seed: PtrBytes, seed_len: u32) -> PtrT<UREncodeResult> {
    // let seed = unsafe { alloc::slice::from_raw_parts(seed, seed_len as usize) };
    let seed = hex::decode("b75a396d4965e5352b6c2c83e4a59ad3d243fbd58133ea9fe0631e5c1576808cb7c1a578099f35278ba00fccd2709a2ef73d7e31380898a63a15b5b3f4532010").unwrap();
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
                            FRAGMENT_MAX_LENGTH_DEFAULT.clone(),
                        )
                        .c_ptr()
                    },
                )
            },
        )
}

// #[no_mangle]
// pub extern "C" fn avax_sign_unlimited(
//     ptr: PtrUR,
//     seed: PtrBytes,
//     seed_len: u32,
//     master_fingerprint: PtrBytes,
//     master_fingerprint_len: u32,
// ) -> *mut UREncodeResult {
//     btc_sign_psbt_dynamic(
//         ptr,
//         seed,
//         seed_len,
//         master_fingerprint,
//         master_fingerprint_len,
//         FRAGMENT_UNLIMITED_LENGTH.clone(),
//     )
// }

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
