pub mod structs;

use crate::interfaces::structs::{SimpleResponse, TransactionParseResult};
use crate::interfaces::types::{PtrBytes, PtrString, PtrT, PtrUR};
use crate::interfaces::utils::{convert_c_char, recover_c_array, recover_c_char};
use alloc::format;
use alloc::string::String;
use alloc::vec::Vec;
use core::slice;
use core::str::FromStr;
use cty::c_char;
use third_party::{hex, secp256k1};

use crate::extract_ptr_with_type;
use crate::interfaces::errors::RustCError;
use crate::interfaces::ffi::CSliceFFI;
use crate::interfaces::ur::{UREncodeResult, FRAGMENT_MAX_LENGTH_DEFAULT};
use crate::interfaces::xrp::structs::{DisplayXrpTx, XRPHDPath};
use app_xrp::errors::XRPError;
use third_party::bitcoin::bip32::{DerivationPath, ExtendedPubKey};
use third_party::ur_registry::bytes::Bytes;
use third_party::ur_registry::traits::RegistryItem;

use super::errors::ErrorCodes;

#[no_mangle]
pub extern "C" fn xrp_get_address(
    hd_path: PtrString,
    root_x_pub: PtrString,
    root_path: PtrString,
) -> *mut SimpleResponse<c_char> {
    let hd_path = recover_c_char(hd_path);
    let root_x_pub = recover_c_char(root_x_pub);
    let root_path = recover_c_char(root_path);
    if !hd_path.starts_with(root_path.as_str()) {
        return SimpleResponse::from(XRPError::InvalidHDPath(format!(
            "{} does not match {}",
            hd_path, root_path
        )))
        .simple_c_ptr();
    }
    let address = app_xrp::address::get_address(&hd_path, &root_x_pub, &root_path);
    match address {
        Ok(result) => SimpleResponse::success(convert_c_char(result) as *mut c_char).simple_c_ptr(),
        Err(e) => SimpleResponse::from(e).simple_c_ptr(),
    }
}

fn build_sign_result(
    ptr: PtrUR,
    hd_paths: PtrT<CSliceFFI<XRPHDPath>>,
    seed: &[u8],
) -> Result<Vec<u8>, XRPError> {
    let crypto_bytes = extract_ptr_with_type!(ptr, Bytes);
    let mut derivation_paths = Vec::new();
    unsafe {
        let hd_paths = recover_c_array(hd_paths);
        for x in hd_paths {
            let path = recover_c_char(x.path);
            let derivation_path =
                DerivationPath::from_str(path.as_str()).map_err(|_e| RustCError::InvalidHDPath);
            if let Ok(v) = derivation_path {
                derivation_paths.push(v);
            }
        }
    }
    app_xrp::sign_tx(crypto_bytes.get_bytes().as_slice(), derivation_paths, seed)
}

#[no_mangle]
pub extern "C" fn xrp_parse_tx(ptr: PtrUR) -> PtrT<TransactionParseResult<DisplayXrpTx>> {
    let crypto_bytes = extract_ptr_with_type!(ptr, Bytes);
    match app_xrp::parse(crypto_bytes.get_bytes().as_slice()) {
        Ok(v) => TransactionParseResult::success(DisplayXrpTx::from(v).c_ptr()).c_ptr(),
        Err(e) => TransactionParseResult::from(e).c_ptr(),
    }
}

#[no_mangle]
pub extern "C" fn xrp_sign_tx(
    ptr: PtrUR,
    hd_paths: PtrT<CSliceFFI<XRPHDPath>>,
    seed: PtrBytes,
    seed_len: u32,
) -> PtrT<UREncodeResult> {
    let seed = unsafe { slice::from_raw_parts(seed, seed_len as usize) };
    let result = build_sign_result(ptr, hd_paths, seed);
    match result.map(|v| Bytes::new(v).try_into()) {
        Ok(v) => match v {
            Ok(data) => UREncodeResult::encode(
                data,
                Bytes::get_registry_type().get_type(),
                FRAGMENT_MAX_LENGTH_DEFAULT.clone(),
            )
            .c_ptr(),
            Err(e) => UREncodeResult::from(e).c_ptr(),
        },
        Err(e) => UREncodeResult::from(e).c_ptr(),
    }
}

#[no_mangle]
pub extern "C" fn xrp_check_pub_key(
    pub_key: PtrString,
    root_xpub: PtrString,
) -> *mut SimpleResponse<c_char> {
    let pub_key_bytes = match hex::decode(recover_c_char(pub_key)) {
        Ok(v) => v,
        Err(_) => {
            return SimpleResponse::error(ErrorCodes::InvalidData, String::from("Invalid public key"))
                .simple_c_ptr()
        }
    };
    let root_xpub = recover_c_char(root_xpub);
    let xpub = match ExtendedPubKey::from_str(&root_xpub) {
        Ok(v) => v,
        Err(_) => {
            return SimpleResponse::error(ErrorCodes::InvalidData, String::from("Invalid xpub"))
                .simple_c_ptr()
        }
    };
    let k1 = secp256k1::Secp256k1::new();
    let a_xpub = xpub.derive_pub(&k1, &DerivationPath::from_str("m/0").unwrap()).unwrap();
    for i in 0..1001 {
        let pk = a_xpub.derive_pub(
            &k1,
            &DerivationPath::from_str(&format!("m/{}", i)).unwrap(),
        );
        if let Ok(pk) = pk {
            if pub_key_bytes.eq(&pk.public_key.serialize()) {
                return SimpleResponse::success(convert_c_char(format!("m/0/{}", i)) as *mut c_char)
                    .simple_c_ptr();
            }
        }
    }
    SimpleResponse::error(ErrorCodes::InvalidData, String::from("Invalid public key"))
        .simple_c_ptr()
}
