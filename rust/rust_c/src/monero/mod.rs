#![no_std]

extern crate alloc;
use crate::common::errors::RustCError;
use crate::common::structs::{SimpleResponse, TransactionCheckResult, TransactionParseResult};
use crate::common::ur::{UREncodeResult, FRAGMENT_MAX_LENGTH_DEFAULT};
use alloc::boxed::Box;
use alloc::slice;
use alloc::string::ToString;
use app_monero::address::Address;
use app_monero::key::{PrivateKey, PublicKey};
use app_monero::structs::{AddressType, Network};
use app_monero::utils::constants::{OUTPUT_EXPORT_MAGIC, PRVKEY_LEH, UNSIGNED_TX_PREFIX};
// use crate::free::Free;
use crate::common::free::Free;
use crate::common::types::{PtrBytes, PtrString, PtrT, PtrUR};
use crate::common::utils::{convert_c_char, recover_c_char};
use crate::extract_ptr_with_type;
use cty::c_char;
use structs::{DisplayMoneroOutput, DisplayMoneroUnsignedTx};
use ur_registry::monero::xmr_keyimage::XmrKeyImage;
use ur_registry::monero::xmr_output::XmrOutput;
use ur_registry::monero::xmr_txsigned::XmrTxSigned;
use ur_registry::monero::xmr_txunsigned::XmrTxUnsigned;
use ur_registry::traits::{RegistryItem, To};

mod structs;

#[no_mangle]
pub extern "C" fn monero_get_address(
    pub_spend_key: PtrString,
    private_view_key: PtrString,
    major: u32,
    minor: u32,
    is_subaddress: bool,
) -> *mut SimpleResponse<c_char> {
    let spend_key = recover_c_char(pub_spend_key);
    let pvk = hex::decode(recover_c_char(private_view_key)).unwrap();

    match app_monero::address::generate_address(
        &PublicKey::from_str(spend_key.as_str()).unwrap(),
        &PrivateKey::from_bytes(&pvk),
        major,
        minor,
        is_subaddress,
    ) {
        Ok(address) => SimpleResponse::success(convert_c_char(address.to_string()) as *mut c_char)
            .simple_c_ptr(),
        _ => {
            SimpleResponse::from(RustCError::InvalidData("invalid data".to_string())).simple_c_ptr()
        }
    }
}

fn safe_parse_key(decrypt_key: PtrBytes) -> Result<[u8; PRVKEY_LEH], RustCError> {
    let decrypt_key = unsafe { slice::from_raw_parts(decrypt_key, PRVKEY_LEH) };

    if decrypt_key.len() != PRVKEY_LEH {
        return Err(RustCError::InvalidMasterFingerprint);
    }

    Ok(decrypt_key.try_into().unwrap())
}

#[no_mangle]
pub extern "C" fn monero_output_request_check(
    ptr: PtrUR,
    decrypt_key: PtrBytes,
    pvk: PtrString,
) -> PtrT<TransactionCheckResult> {
    let request = extract_ptr_with_type!(ptr, XmrOutput);
    let decrypt_key = match safe_parse_key(decrypt_key) {
        Ok(decrypt_key) => decrypt_key,
        _ => return TransactionCheckResult::from(RustCError::InvalidMasterFingerprint).c_ptr(),
    };
    let pvk = hex::decode(recover_c_char(pvk)).unwrap();

    let payload = request.get_payload();

    match app_monero::utils::decrypt_data_with_decrypt_key(
        decrypt_key,
        pvk.try_into().unwrap(),
        payload,
        OUTPUT_EXPORT_MAGIC,
    ) {
        Ok(_) => TransactionCheckResult::new().c_ptr(),
        _ => TransactionCheckResult::from(RustCError::InvalidMasterFingerprint).c_ptr(),
    }
}

#[no_mangle]
pub extern "C" fn monero_unsigned_request_check(
    ptr: PtrUR,
    decrypt_key: PtrBytes,
    pvk: PtrString,
) -> PtrT<TransactionCheckResult> {
    let request = extract_ptr_with_type!(ptr, XmrTxUnsigned);
    let decrypt_key = match safe_parse_key(decrypt_key) {
        Ok(decrypt_key) => decrypt_key,
        _ => return TransactionCheckResult::from(RustCError::InvalidMasterFingerprint).c_ptr(),
    };
    let pvk = hex::decode(recover_c_char(pvk)).unwrap();

    let payload = request.get_payload();

    match app_monero::utils::decrypt_data_with_decrypt_key(
        decrypt_key,
        pvk.try_into().unwrap(),
        payload,
        UNSIGNED_TX_PREFIX,
    ) {
        Ok(_) => TransactionCheckResult::new().c_ptr(),
        Err(_) => {
            return TransactionCheckResult::from(RustCError::InvalidMasterFingerprint).c_ptr()
        }
    }
}

#[no_mangle]
pub extern "C" fn get_monero_pvk_by_seed(
    seed: PtrBytes,
    seed_len: u32,
) -> *mut SimpleResponse<c_char> {
    let seed = unsafe { slice::from_raw_parts(seed, seed_len as usize) };

    match app_monero::key::generate_keypair(seed, 0) {
        Ok(keypair) => {
            let pvk = keypair.view.to_bytes();
            SimpleResponse::success(convert_c_char(hex::encode(pvk)) as *mut c_char).simple_c_ptr()
        }
        _ => {
            SimpleResponse::from(RustCError::InvalidData("invalid seed".to_string())).simple_c_ptr()
        }
    }
}

#[no_mangle]
pub extern "C" fn monero_parse_output(
    ptr: PtrUR,
    decrypt_key: PtrBytes,
    pvk: PtrString,
) -> PtrT<TransactionParseResult<DisplayMoneroOutput>> {
    let request = extract_ptr_with_type!(ptr, XmrOutput);
    let decrypt_key = match safe_parse_key(decrypt_key) {
        Ok(decrypt_key) => decrypt_key,
        _ => return TransactionParseResult::from(RustCError::InvalidMasterFingerprint).c_ptr(),
    };
    let pvk = hex::decode(recover_c_char(pvk)).unwrap();

    match app_monero::outputs::parse_display_info(
        &request.get_payload(),
        decrypt_key,
        pvk.try_into().unwrap(),
    ) {
        Ok(display) => TransactionParseResult::success(Box::into_raw(Box::new(
            DisplayMoneroOutput::from(display),
        )) as *mut DisplayMoneroOutput)
        .c_ptr(),
        Err(_) => TransactionParseResult::from(RustCError::InvalidData("invalid data".to_string()))
            .c_ptr(),
    }
}

#[no_mangle]
pub extern "C" fn monero_parse_unsigned_tx(
    ptr: PtrUR,
    decrypt_key: PtrBytes,
    pvk: PtrString,
) -> PtrT<TransactionParseResult<DisplayMoneroUnsignedTx>> {
    let request = extract_ptr_with_type!(ptr, XmrTxUnsigned);
    let decrypt_key = match safe_parse_key(decrypt_key) {
        Ok(decrypt_key) => decrypt_key,
        _ => return TransactionParseResult::from(RustCError::InvalidMasterFingerprint).c_ptr(),
    };
    let pvk = hex::decode(recover_c_char(pvk)).unwrap();

    match app_monero::transfer::parse_unsigned(
        request.get_payload(),
        decrypt_key,
        pvk.try_into().unwrap(),
    ) {
        Ok(display) => TransactionParseResult::success(Box::into_raw(Box::new(
            DisplayMoneroUnsignedTx::from(display),
        )) as *mut DisplayMoneroUnsignedTx)
        .c_ptr(),
        Err(_) => TransactionParseResult::from(RustCError::InvalidData("invalid data".to_string()))
            .c_ptr(),
    }
}

#[no_mangle]
pub extern "C" fn monero_generate_keyimage(
    ptr: PtrUR,
    seed: PtrBytes,
    seed_len: u32,
    major: u32,
) -> PtrT<UREncodeResult> {
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
        }
        Err(_) => UREncodeResult::from(RustCError::InvalidData("invalid data".to_string())).c_ptr(),
    }
}

#[no_mangle]
pub extern "C" fn monero_generate_signature(
    ptr: PtrUR,
    seed: PtrBytes,
    seed_len: u32,
    major: u32,
) -> PtrT<UREncodeResult> {
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
        }
        Err(_) => UREncodeResult::from(RustCError::InvalidData("invalid data".to_string())).c_ptr(),
    }
}

#[no_mangle]
pub extern "C" fn monero_generate_decrypt_key(pvk: PtrString) -> *mut SimpleResponse<u8> {
    let pvk = hex::decode(recover_c_char(pvk)).unwrap();

    let key = app_monero::utils::generate_decrypt_key(pvk.try_into().unwrap());

    SimpleResponse::success(Box::into_raw(Box::new(key)) as *mut u8).simple_c_ptr()
}

#[no_mangle]
pub extern "C" fn get_extended_monero_pubkeys_by_seed(
    seed: PtrBytes,
    seed_len: u32,
    path: PtrString,
) -> *mut SimpleResponse<c_char> {
    let path = recover_c_char(path);
    let major = path
        .split('/')
        .nth(3)
        .unwrap()
        .replace("'", "")
        .parse::<u32>()
        .unwrap();
    let seed = unsafe { slice::from_raw_parts(seed, seed_len as usize) };
    let keypair = app_monero::key::generate_keypair(seed, major).unwrap();
    let public_spend_key = keypair.spend.get_public_key();
    let result = hex::encode(public_spend_key.as_bytes());

    SimpleResponse::success(convert_c_char(result.to_string())).simple_c_ptr()
}
