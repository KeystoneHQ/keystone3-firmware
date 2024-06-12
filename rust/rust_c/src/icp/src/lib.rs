#![no_std]

extern crate alloc;

use alloc::boxed::Box;
use alloc::format;
use alloc::string::ToString;
use core::slice;

use common_rust_c::extract_ptr_with_type;
use common_rust_c::structs::{SimpleResponse, TransactionCheckResult, TransactionParseResult};
use common_rust_c::types::{PtrBytes, PtrString, PtrT, PtrUR};
use common_rust_c::ur::{UREncodeResult, FRAGMENT_MAX_LENGTH_DEFAULT};
use common_rust_c::utils::{convert_c_char, recover_c_char};
use cty::c_char;
use third_party::hex;

use third_party::secp256k1::Message;
use third_party::ur_registry::icp::icp_sign_request::{IcpSignRequest, SignType};
use third_party::ur_registry::icp::icp_signature::IcpSignature;
use third_party::ur_registry::traits::RegistryItem;

use app_icp::errors::ICPError;
use common_rust_c::errors::RustCError;
use crate::structs::DisplayIcpTx;

pub mod structs;
fn build_sign_result(
    ptr: PtrUR,
    seed: &[u8]
) -> app_icp::errors::Result<IcpSignature> {
    let sign_request = extract_ptr_with_type!(ptr, IcpSignRequest);
    let derive_path = sign_request.get_derivation_path().get_path().unwrap_or("m/44'/223'/0'/0/0".to_string());
    let message = Message::from_digest_slice(sign_request.get_sign_data().to_vec().as_ref())
        .map_err(|e| {
            ICPError::KeystoreError(format!(
                "Could not convert message from digest slice: `{}`",
                e
            ))
        })?;
    let signature = app_icp::sign(message, &derive_path, seed)?;
    Ok(IcpSignature::new(
        signature.1.to_vec()
    ))
}



#[no_mangle]
pub extern "C" fn icp_generate_address(hd_path:PtrString,x_pub:PtrString) -> *mut SimpleResponse<c_char> {
    let x_pub = recover_c_char(x_pub);
    let hd_path = recover_c_char(hd_path);
    let address = app_icp::address::get_address(hd_path, &x_pub);
    match address {
        Ok(result) => SimpleResponse::success(convert_c_char(result.1) as *mut c_char).simple_c_ptr(),
        Err(e) => SimpleResponse::from(e).simple_c_ptr(),
    }
}

#[no_mangle]
pub extern "C" fn icp_generate_principal_id(hd_path:PtrString,x_pub:PtrString) -> *mut SimpleResponse<c_char> {
    let x_pub = recover_c_char(x_pub);
    let hd_path = recover_c_char(hd_path);
    let address = app_icp::address::get_address(hd_path, &x_pub);
    match address {
        Ok(result) => SimpleResponse::success(convert_c_char(result.0) as *mut c_char).simple_c_ptr(),
        Err(e) => SimpleResponse::from(e).simple_c_ptr(),
    }
}

#[no_mangle]
pub extern "C" fn icp_sign_tx(
    ptr: PtrUR,
    seed: PtrBytes,
    seed_len: u32,
) -> PtrT<UREncodeResult> {
    let seed = unsafe { alloc::slice::from_raw_parts(seed, seed_len as usize) };

    build_sign_result(ptr, seed)
        .map(|v| v.try_into())
        .map_or_else(
            |e| UREncodeResult::from(e).c_ptr(),
            |v| {
                v.map_or_else(
                    |e| UREncodeResult::from(e).c_ptr(),
                    |data| {
                        UREncodeResult::encode(
                            data,
                            IcpSignature::get_registry_type().get_type(),
                            FRAGMENT_MAX_LENGTH_DEFAULT.clone(),
                        )
                        .c_ptr()
                    },
                )
            },
        )
}



#[no_mangle]
pub extern "C" fn icp_parse(ptr: PtrUR) -> PtrT<TransactionParseResult<DisplayIcpTx>> {
    let sign_request = extract_ptr_with_type!(ptr, IcpSignRequest);
    let raw_message = match sign_request.get_sign_type() {
        SignType::Transaction => hex::encode(&sign_request.get_sign_data()),
        _ => {
            return TransactionParseResult::from(RustCError::UnsupportedTransaction(
                "Transaction".to_string(),
            ))
                .c_ptr();
        }
    };
    let display_data = DisplayIcpTx {
        raw_message: convert_c_char(raw_message),
    };
    TransactionParseResult::success(Box::into_raw(Box::new(display_data)) as *mut DisplayIcpTx)
        .c_ptr()
}


#[no_mangle]
pub extern "C" fn icp_check_tx(
    ptr: PtrUR,
    master_fingerprint: PtrBytes,
    length: u32,
) -> PtrT<TransactionCheckResult> {
    if length != 4 {
        return TransactionCheckResult::from(RustCError::InvalidMasterFingerprint).c_ptr();
    }
    let mfp = unsafe { slice::from_raw_parts(master_fingerprint, 4) };
    let sign_request = extract_ptr_with_type!(ptr, IcpSignRequest);
    rust_tools::debug!(format!("Master Fingerprint: {:?}", mfp));
    rust_tools::debug!(format!("Master Fingerprint: {:?}", sign_request.get_master_fingerprint()));
    if let Some(mfp) = (mfp.try_into() as Result<[u8; 4], _>).ok() {
        let derivation_path: third_party::ur_registry::crypto_key_path::CryptoKeyPath =
            sign_request.get_derivation_path();
            rust_tools::debug!(format!("Derivation Path: {:?}", derivation_path));
        if let Some(ur_mfp) = derivation_path.get_source_fingerprint() {
            return if mfp == ur_mfp {
                TransactionCheckResult::new().c_ptr()
            } else {
                TransactionCheckResult::from(RustCError::MasterFingerprintMismatch).c_ptr()
            };
        }
        return TransactionCheckResult::from(RustCError::MasterFingerprintMismatch).c_ptr();
    };
    TransactionCheckResult::from(RustCError::InvalidMasterFingerprint).c_ptr()
}


#[cfg(test)]
mod tests {
    use alloc::vec;
    use third_party::secp256k1::PublicKey;

    extern crate std;
    use super::*;
    use third_party::hex;
    use third_party::ur_registry::crypto_key_path::{CryptoKeyPath, PathComponent};
    use third_party::ur_registry::traits::To;

    #[test]
    fn test_extract_icp_sign_request() {

        let master_fingerprint: [u8; 4] = [18, 80, 182, 188];
        let sign_data =
            hex::decode("af78f85b29d88a61ee49d36e84139ec8511c558f14612413f1503b8e6959adca")
                .unwrap();
        let sign_type = SignType::Transaction;
        let origin = Some("plugwallet".to_string());
        let path1 = PathComponent::new(Some(44), true).unwrap();
        let path2 = PathComponent::new(Some(223), true).unwrap();
        let path3 = PathComponent::new(Some(0), true).unwrap();

        let source_fingerprint: [u8; 4] = [18, 80, 182, 188];
        let components = vec![path1, path2, path3];
        let crypto_key_path = CryptoKeyPath::new(components, Some(source_fingerprint), None);

        let mut sign_request = IcpSignRequest::new(
            master_fingerprint,
            sign_data,
            sign_type,
            origin,
            crypto_key_path,
        );

        assert_eq!(
            "a5011a1250b6bc025820af78f85b29d88a61ee49d36e84139ec8511c558f14612413f1503b8e6959adca030105d90130a20186182cf518dff500f5021af23f9fd2046a706c756777616c6c6574",
            hex::encode(sign_request.to_bytes().unwrap()).to_lowercase()
        );
        let icp_sign_req_ptr = &mut sign_request;

        let icp_sign_req_ptr_ur = icp_sign_req_ptr as *mut IcpSignRequest as PtrUR;

        let sign_request = extract_ptr_with_type!(icp_sign_req_ptr_ur, IcpSignRequest);
        assert_eq!(master_fingerprint, sign_request.get_master_fingerprint());
    }

    #[test]
    fn test_sign_tx() {
        let master_fingerprint: [u8; 4] = [233, 24, 28, 243];
        let sign_data =
            hex::decode("af78f85b29d88a61ee49d36e84139ec8511c558f14612413f1503b8e6959adca")
                .unwrap();
        let sign_type = SignType::Transaction;
        let origin = Some("plugwallet".to_string());
        let path1 = PathComponent::new(Some(44), true).unwrap();
        let path2 = PathComponent::new(Some(223), true).unwrap();
        let path3 = PathComponent::new(Some(0), true).unwrap();

        let source_fingerprint: [u8; 4] = [242, 63, 159, 210];
        let components = vec![path1, path2, path3];
        let crypto_key_path = CryptoKeyPath::new(components, Some(source_fingerprint), None);

        let mut sign_request = IcpSignRequest::new(
            master_fingerprint,
            sign_data,
            sign_type,
            origin,
            crypto_key_path,
        );

        let icp_sign_req_ptr = &mut sign_request;

        let icp_sign_req_ptr_ur = icp_sign_req_ptr as *mut IcpSignRequest as PtrUR;
        let seed = "8AC41B663FC02C7E6BCD961989A5E03B23325509089C3F9984F9D86169B918E0967AB3CC894D10C8A66824B0163E445EBBE639CD126E37AAC16591C278425FF9".to_string();
        let seed_ptr_bytes = seed.as_ptr() as *mut u8;
        let seed_len = seed.len() as u32;

        let sign_encoded_ur_result = icp_sign_tx(
            icp_sign_req_ptr_ur,
            seed_ptr_bytes,
            seed_len,
        );
        let ur_encoded_result = extract_ptr_with_type!(sign_encoded_ur_result, UREncodeResult);
        let sig = recover_c_char(ur_encoded_result.data);
        assert_eq!(
            "UR:ICP-SIGNATURE/OYADHDFZOSYNLDGSGMDAHFDSVEJZGSFRVYRKMHEOTEFEAHISUTGOLPZTISRNLGHLLEBSTEGRFHFDBBLRLBWEFLJKHHRDGDGRWLLGIENNJEDKFNVANNMHFMGALEPKCTBTBKFTOLCSSEATWSLT".to_string(),
            sig
        )
    }
}
