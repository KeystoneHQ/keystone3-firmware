#![no_std]

extern crate alloc;
use alloc::slice;
use alloc::boxed::Box;
use alloc::string::ToString;
use app_monero::address::Address;
use app_monero::key::PublicKey;
use app_monero::structs::{AddressType, Network};
use app_monero::utils::constants::{PRVKEY_LEH, OUTPUT_EXPORT_MAGIC, UNSIGNED_TX_PREFIX};
use common_rust_c::structs::{SimpleResponse, TransactionCheckResult, TransactionParseResult};
use common_rust_c::errors::RustCError;
use common_rust_c::ur::{UREncodeResult, FRAGMENT_MAX_LENGTH_DEFAULT};
// use crate::free::Free;
use common_rust_c::free::Free;
use common_rust_c::types::{PtrBytes, PtrString, PtrT, PtrUR};
use common_rust_c::extract_ptr_with_type;
use common_rust_c::utils::{convert_c_char, recover_c_char};
use ur_registry::monero::xmr_output::XmrOutput;
use ur_registry::monero::xmr_keyimage::XmrKeyImage;
use ur_registry::monero::xmr_txunsigned::XmrTxUnsigned;
use ur_registry::monero::xmr_txsigned::XmrTxSigned;
use cty::c_char;
use ur_registry::traits::{RegistryItem, To};
use crate::structs::{DisplayMoneroOutput, DisplayMoneroUnsignedTx};

mod structs;

#[no_mangle]
pub extern "C" fn monero_get_address(
    pub_spend_key: PtrString,
    pub_view_key: PtrString,
) -> *mut SimpleResponse<c_char> {
    let spend_key = recover_c_char(pub_spend_key);
    let view_key = recover_c_char(pub_view_key);

    let address = Address::new(
        Network::Mainnet,
        AddressType::Standard,
        PublicKey::from_str(spend_key.as_str()).unwrap(),
        PublicKey::from_str(view_key.as_str()).unwrap(),
    );

    SimpleResponse::success(convert_c_char(address.to_string()) as *mut c_char).simple_c_ptr()
}

fn safe_parse_pvk(pvk: PtrBytes) -> Result<[u8; PRVKEY_LEH], RustCError> {
    let pvk = unsafe { slice::from_raw_parts(pvk, PRVKEY_LEH) };

    if pvk.len() != PRVKEY_LEH {
        return Err(RustCError::InvalidMasterFingerprint);
    }

    Ok(pvk.try_into().unwrap())
}

#[no_mangle]
pub extern "C" fn monero_output_request_check(ptr: PtrUR, pvk: PtrBytes) -> PtrT<TransactionCheckResult> {
    let request = extract_ptr_with_type!(ptr, XmrOutput);
    let pvk = match safe_parse_pvk(pvk) {
        Ok(pvk) => pvk,
        _ => return TransactionCheckResult::from(RustCError::InvalidMasterFingerprint).c_ptr(),
    };

    let payload = request.get_payload();

    match app_monero::utils::decrypt_data_with_pvk(pvk, payload, OUTPUT_EXPORT_MAGIC) {
        Ok(_) => TransactionCheckResult::new().c_ptr(),
        _ => TransactionCheckResult::from(RustCError::InvalidMasterFingerprint).c_ptr(),
    }
}

#[no_mangle]
pub extern "C" fn monero_unsigned_request_check(ptr: PtrUR, pvk: PtrBytes) -> PtrT<TransactionCheckResult> {
    let request = extract_ptr_with_type!(ptr, XmrTxUnsigned);
    let pvk = match safe_parse_pvk(pvk) {
        Ok(pvk) => pvk,
        _ => return TransactionCheckResult::from(RustCError::InvalidMasterFingerprint).c_ptr(),
    };

    let payload = request.get_payload();

    match app_monero::utils::decrypt_data_with_pvk(pvk, payload, UNSIGNED_TX_PREFIX) {
        Ok(_) => TransactionCheckResult::new().c_ptr(),
        Err(_) => return TransactionCheckResult::from(RustCError::InvalidMasterFingerprint).c_ptr(),
    }
}

#[no_mangle]
pub extern "C" fn monero_get_pvk(seed: PtrBytes, seed_len: u32, major: u32) -> *mut SimpleResponse<u8> {
    let seed = unsafe { slice::from_raw_parts(seed, seed_len as usize) };
    match app_monero::key::generate_keypair(seed, major) {
        Ok(keypair) => {
            let pvk = keypair.view.to_bytes();
            return SimpleResponse::success(Box::into_raw(Box::new(pvk)) as *mut u8).simple_c_ptr()
        },
        _ => return SimpleResponse::from(RustCError::InvalidData("invalid seed".to_string()))
            .simple_c_ptr(),
    }
}


#[no_mangle]
pub extern "C" fn monero_parse_output(ptr: PtrUR) -> PtrT<TransactionParseResult<DisplayMoneroOutput>> {
    let request = extract_ptr_with_type!(ptr, XmrOutput);
    // TODO: ...
    let display_data = DisplayMoneroOutput {
        raw: convert_c_char("DisplayMoneroOutput".to_string()),
    };
    TransactionParseResult::success(Box::into_raw(Box::new(display_data)) as *mut DisplayMoneroOutput)
        .c_ptr()
}

#[no_mangle]
pub extern "C" fn monero_parse_unsigned_tx(ptr: PtrUR) -> PtrT<TransactionParseResult<DisplayMoneroUnsignedTx>> {
    let request = extract_ptr_with_type!(ptr, XmrTxUnsigned);
    // TODO: ...
    let display_data = DisplayMoneroUnsignedTx {
        raw: convert_c_char("DisplayMoneroUnsignedTx".to_string()),
    };
    TransactionParseResult::success(Box::into_raw(Box::new(display_data)) as *mut DisplayMoneroUnsignedTx)
        .c_ptr()
}

#[no_mangle]
pub extern "C" fn monero_generate_keyimage(ptr: PtrUR, seed: PtrBytes, seed_len: u32, major: u32) -> PtrT<UREncodeResult> {
    let request = extract_ptr_with_type!(ptr, XmrOutput);
    let seed = unsafe { slice::from_raw_parts(seed, seed_len as usize) };
    let keypair = app_monero::key::generate_keypair(seed, major).unwrap();
    match app_monero::key_images::generate_export_ur_data(keypair, request.get_payload()) {
        Ok(data) => {
            let data = XmrKeyImage::new(data);

            UREncodeResult::encode(
                data.try_into().unwrap(),
                XmrKeyImage::get_registry_type().get_type(),
                FRAGMENT_MAX_LENGTH_DEFAULT.clone(),
            )
            .c_ptr()
        },
        Err(_) => UREncodeResult::from(RustCError::InvalidData("invalid data".to_string())).c_ptr(),
    }
}

#[no_mangle]
pub extern "C" fn monero_generate_signature(ptr: PtrUR, seed: PtrBytes, seed_len: u32, major: u32) -> PtrT<UREncodeResult> {
    let request = extract_ptr_with_type!(ptr, XmrTxUnsigned);
    let seed = unsafe { slice::from_raw_parts(seed, seed_len as usize) };
    let keypair = app_monero::key::generate_keypair(seed, major).unwrap();
    match app_monero::transfer::sign_tx(keypair, request.get_payload()) {
        Ok(data) => {
            let data = XmrTxSigned::new(data);

            UREncodeResult::encode(
                data.try_into().unwrap(),
                XmrTxSigned::get_registry_type().get_type(),
                FRAGMENT_MAX_LENGTH_DEFAULT.clone(),
            )
            .c_ptr()
        },
        Err(_) => UREncodeResult::from(RustCError::InvalidData("invalid data".to_string())).c_ptr(),
    }
}
