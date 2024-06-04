#![no_std]

extern crate alloc;

use alloc::format;
use alloc::slice;
use alloc::string::ToString;
use alloc::vec::Vec;
use common_rust_c::errors::RustCError;
use common_rust_c::extract_ptr_with_type;
use common_rust_c::structs::{SimpleResponse, TransactionCheckResult, TransactionParseResult};
use common_rust_c::types::{PtrBytes, PtrString, PtrT, PtrUR};
use common_rust_c::ur::{UREncodeResult, FRAGMENT_MAX_LENGTH_DEFAULT};
use common_rust_c::utils::{convert_c_char, recover_c_char};
use cty::c_char;
use third_party::hex;
use third_party::hex::FromHex;
use third_party::hex;
use third_party::hex::FromHex;
use third_party::ur_registry::icp::icp_sign_request::{IcpSignRequest, SignType};
use third_party::ur_registry::icp::icp_signature::IcpSignature;
use third_party::ur_registry::traits::RegistryItem;

use app_icp::errors::ICPError;

#[no_mangle]
fn icp_generate_address(pub_key: PtrString) -> *mut SimpleResponse<c_char> {
    let pub_key = recover_c_char(pub_key);
    let address = app_icp::generate_address(&pub_key);
    match address {
        Ok(result) => SimpleResponse::success(convert_c_char(result) as *mut c_char).simple_c_ptr(),
        Err(e) => SimpleResponse::from(e).simple_c_ptr(),
    }
}


// fn build_sign_result(
//     ptr: PtrUR,
//     seed: &[u8],
//     pub_key: PtrString,
// ) -> app_icp::errors::Result<IcpSignature> {
//     let sign_request = extract_ptr_with_type!(ptr, IcpSignRequest);
//     let pub_key = recover_c_char(pub_key);
//     let mut path = sign_request.get_authentication_key_derivation_paths()[0]
//         .get_path()
//         .ok_or(ICPError::InvalidData(
//             "invalid derivation path".to_string(),
//         ))?;
//     if !path.starts_with("m/") {
//         path = format!("m/{}", path);
//     }
//     let signature = app_icp::sign(sign_request.get_sign_data().to_vec(), &path, seed)?;
//     let buf: Vec<u8> = hex::decode(pub_key)?;
//     Ok(IcpSignature::new(
//         sign_request.get_request_id(),
//         signature.to_vec(),
//         buf,
//     ))
// }

// #[no_mangle]
// pub extern "C" fn icp_sign_tx(
//     ptr: PtrUR,
//     seed: PtrBytes,
//     seed_len: u32,
//     pub_key: PtrString,
// ) -> PtrT<UREncodeResult> {
//     let seed = unsafe { alloc::slice::from_raw_parts(seed, seed_len as usize) };
//     build_sign_result(ptr, seed, pub_key)
//         .map(|v| v.try_into())
//         .map_or_else(
//             |e| UREncodeResult::from(e).c_ptr(),
//             |v| {
//                 v.map_or_else(
//                     |e| UREncodeResult::from(e).c_ptr(),
//                     |data| {
//                         UREncodeResult::encode(
//                             data,
//                             AptosSignature::get_registry_type().get_type(),
//                             FRAGMENT_MAX_LENGTH_DEFAULT.clone(),
//                         )
//                         .c_ptr()
//                     },
//                 )
//             },
//         )
// }