#![no_std]
#![feature(vec_into_raw_parts)]

extern crate alloc;

pub mod address;
pub mod structs;
pub mod transactions;

use alloc::{
    format,
    string::{String, ToString},
    vec::Vec,
};
use app_avalanche::{parse_base_tx, transactions::base_tx::BaseTx};
use common_rust_c::{
    errors::RustCError,
    extract_ptr_with_type,
    ffi::VecFFI,
    impl_c_ptr,
    structs::{SimpleResponse, TransactionCheckResult, TransactionParseResult},
    types::{Ptr, PtrBytes, PtrString, PtrT, PtrUR},
    ur::{UREncodeResult, FRAGMENT_MAX_LENGTH_DEFAULT},
    utils::recover_c_char,
};
use structs::DisplayAvaxTx;
use ur_registry::pb::protoc::Base;
use {
    hex,
    ur_registry::{avalanche::avax_sign_request::AvaxSignRequest, traits::RegistryItem},
};

#[no_mangle]
pub extern "C" fn avax_parse_transaction(
    ptr: PtrUR,
    // master_fingerprint: PtrBytes,
    // length: u32,
    // public_keys: PtrT<CSliceFFI<ExtendedPublicKey>>,
    // multisig_wallet_config: PtrString,
) -> PtrT<TransactionParseResult<DisplayAvaxTx>> {
    // if length != 4 {
    //     return TransactionParseResult::from(RustCError::InvalidMasterFingerprint).c_ptr();
    // }
    extern crate std;
    use std::println;
    println!(".............................\n");
    let unsigned_data = extract_ptr_with_type!(ptr, AvaxSignRequest);
    let parse_data = parse_base_tx(unsigned_data.get_tx_data()).unwrap();
    println!("prase_data = {:?}", parse_data);

    unsafe {
        // let mfp = core::slice::from_raw_parts(master_fingerprint, 4);
        // let public_keys = recover_c_array(public_keys);
        TransactionParseResult::success(DisplayAvaxTx::from(tx_data).c_ptr()).c_ptr()
    }
}

fn parse_tx(tx_data: BaseTx) -> PtrT<TransactionParseResult<DisplayAvaxTx>> { 
    // let master_fingerprint =
    //     bitcoin::bip32::Fingerprint::from_str(hex::encode(mfp.to_vec()).as_str())
    //         .map_err(|_e| RustCError::InvalidMasterFingerprint);
    TransactionParseResult::success(DisplayAvaxTx::from(tx_data).c_ptr()).c_ptr()
    // let data = TransactionParseResult::success(DisplayAvaxTx::from(tx_data).c_ptr()).c_ptr();
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

// #[no_mangle]
// pub extern "C" fn avax_sign(
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
//         FRAGMENT_MAX_LENGTH_DEFAULT.clone(),
//     )
// }

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
    public_key: PtrString,
) -> PtrT<TransactionCheckResult> {
    TransactionCheckResult::new().c_ptr()
    // let ton_tx = extract_ptr_with_type!(ptr, AvaxSignRequest);
    // let pk = recover_c_char(public_key);
    // match hex::decode(pk) {
    // Ok(pk) => {}
    // Err(e) => TransactionCheckResult::from(RustCError::InvalidHex(e.to_string())).c_ptr(),
    // }
}
