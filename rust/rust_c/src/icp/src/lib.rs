#![no_std]

extern crate alloc;

use alloc::format;
use alloc::string::ToString;

use common_rust_c::extract_ptr_with_type;
use common_rust_c::structs::SimpleResponse;
use common_rust_c::types::{PtrBytes, PtrString, PtrT, PtrUR};
use common_rust_c::ur::{UREncodeResult, FRAGMENT_MAX_LENGTH_DEFAULT};
use common_rust_c::utils::{convert_c_char, recover_c_char};
use cty::c_char;

use third_party::secp256k1::Message;
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

fn build_sign_result(
    ptr: PtrUR,
    seed: &[u8],
    pub_key: PtrString,
) -> app_icp::errors::Result<IcpSignature> {
    let sign_request = extract_ptr_with_type!(ptr, IcpSignRequest);

    let path = "m/44'/223'/0'/0/0".to_string();
    let message = Message::from_digest_slice(sign_request.get_sign_data().to_vec().as_ref())
        .map_err(|e| {
            ICPError::KeystoreError(format!(
                "Could not convert message from digest slice: `{}`",
                e
            ))
        })?;
    let signature = app_icp::sign(message, &path, seed)?;
    Ok(IcpSignature::new(
        sign_request.get_request_id(),
        signature.1.to_vec(),
    ))
}

#[no_mangle]
pub extern "C" fn icp_sign_tx(
    ptr: PtrUR,
    seed: PtrBytes,
    seed_len: u32,
    pub_key: PtrString,
) -> PtrT<UREncodeResult> {
    let seed = unsafe { alloc::slice::from_raw_parts(seed, seed_len as usize) };

    build_sign_result(ptr, seed, pub_key)
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

#[cfg(test)]
mod tests {
    use alloc::vec;
    use third_party::secp256k1::PublicKey;

    extern crate std;
    use third_party::ur_registry::crypto_key_path::{CryptoKeyPath, PathComponent};
    use third_party::ur_registry::icp::icp_sign_request::SaltLen;
    use third_party::ur_registry::traits::To;
    use third_party::hex;
    use alloc::vec::Vec;
    use super::*;
    #[test]
    fn test_generate_address() {
        let public_key_bytes = [
            3, 59, 101, 243, 36, 181, 142, 146, 174, 252, 195, 17, 7, 16, 168, 230, 66, 167, 206,
            106, 94, 101, 65, 150, 148, 1, 255, 96, 112, 12, 5, 244, 57,
        ];
        let public_key = PublicKey::from_slice(&public_key_bytes).unwrap();
        let address = icp_generate_address(public_key.to_string().as_ptr() as *mut c_char);
        let simple_resp_addres = extract_ptr_with_type!(address, SimpleResponse<c_char>);
        let address = recover_c_char(simple_resp_addres.data);
        assert_eq!(
            "33a807e6078195d2bbe1904b0ed0fc65b8a3a437b43831ccebba2b7b6d393bd6",
            address.as_str()
        );
    }

    #[test]
    fn test_extract_icp_sign_request() {
        let master_fingerprint: [u8; 4] = [233, 24, 28, 243];
        let request_id: Option<Vec<u8>> = Some(
            [
                155, 29, 235, 77, 59, 125, 75, 173, 155, 221, 43, 13, 123, 61, 203, 109,
            ]
            .to_vec(),
        );
        let sign_data =
            hex::decode("af78f85b29d88a61ee49d36e84139ec8511c558f14612413f1503b8e6959adca")
                .unwrap();
        let sign_type = SignType::Transaction;
        let salt_len = SaltLen::Zero;
        let origin = Some("plugwallet".to_string());
        let path1 = PathComponent::new(Some(44), true).unwrap();
        let path2 = PathComponent::new(Some(223), true).unwrap();
        let path3 = PathComponent::new(Some(0), true).unwrap();

        let source_fingerprint: [u8; 4] = [242, 63, 159, 210];
        let components = vec![path1, path2, path3];
        let crypto_key_path = CryptoKeyPath::new(components, Some(source_fingerprint), None);

        let mut sign_request = IcpSignRequest::new(
            master_fingerprint,
            request_id,
            sign_data,
            sign_type,
            salt_len,
            None,
            origin,
            Some(crypto_key_path),
        );

        assert_eq!(
            "a6011ae9181cf302d825509b1deb4d3b7d4bad9bdd2b0d7b3dcb6d035820af78f85b29d88a61ee49d36e84139ec8511c558f14612413f1503b8e6959adca0401050008d90130a20186182cf518dff500f5021af23f9fd2066a706c756777616c6c6574",
            hex::encode(sign_request.to_bytes().unwrap()).to_lowercase()
        );
        let icp_sign_req_ptr = &mut sign_request;

        let icp_sign_req_ptr_ur = icp_sign_req_ptr as *mut IcpSignRequest as PtrUR;

        let sign_request = extract_ptr_with_type!(icp_sign_req_ptr_ur, IcpSignRequest);
        let request_id = Some(
            [
                155, 29, 235, 77, 59, 125, 75, 173, 155, 221, 43, 13, 123, 61, 203, 109,
            ]
            .to_vec(),
        );
        assert_eq!(request_id, sign_request.get_request_id());
        assert_eq!(master_fingerprint, sign_request.get_master_fingerprint());
    }

    #[test]
    fn test_sign_tx() {
        let master_fingerprint: [u8; 4] = [233, 24, 28, 243];
        let request_id: Option<Vec<u8>> = Some(
            [
                155, 29, 235, 77, 59, 125, 75, 173, 155, 221, 43, 13, 123, 61, 203, 109,
            ]
            .to_vec(),
        );
        let sign_data =
            hex::decode("af78f85b29d88a61ee49d36e84139ec8511c558f14612413f1503b8e6959adca")
                .unwrap();
        let sign_type = SignType::Transaction;
        let salt_len = SaltLen::Zero;
        let origin = Some("plugwallet".to_string());
        let path1 = PathComponent::new(Some(44), true).unwrap();
        let path2 = PathComponent::new(Some(223), true).unwrap();
        let path3 = PathComponent::new(Some(0), true).unwrap();

        let source_fingerprint: [u8; 4] = [242, 63, 159, 210];
        let components = vec![path1, path2, path3];
        let crypto_key_path = CryptoKeyPath::new(components, Some(source_fingerprint), None);

        let mut sign_request = IcpSignRequest::new(
            master_fingerprint,
            request_id,
            sign_data,
            sign_type,
            salt_len,
            None,
            origin,
            Some(crypto_key_path),
        );

        let icp_sign_req_ptr = &mut sign_request;

        let icp_sign_req_ptr_ur = icp_sign_req_ptr as *mut IcpSignRequest as PtrUR;
        let seed = "8AC41B663FC02C7E6BCD961989A5E03B23325509089C3F9984F9D86169B918E0967AB3CC894D10C8A66824B0163E445EBBE639CD126E37AAC16591C278425FF9".to_string();
        let seed_ptr_bytes = seed.as_ptr() as *mut u8;
        let seed_len = seed.len() as u32;

        let public_key_bytes = [
            3, 59, 101, 243, 36, 181, 142, 146, 174, 252, 195, 17, 7, 16, 168, 230, 66, 167, 206,
            106, 94, 101, 65, 150, 148, 1, 255, 96, 112, 12, 5, 244, 57,
        ];
        let public_key = PublicKey::from_slice(&public_key_bytes).unwrap();
        let public_key_ptr = public_key.to_string().as_ptr() as *mut c_char;
        let sign_encoded_ur_result = icp_sign_tx(
            icp_sign_req_ptr_ur,
            seed_ptr_bytes,
            seed_len,
            public_key_ptr,
        );
        let ur_encoded_result = extract_ptr_with_type!(sign_encoded_ur_result, UREncodeResult);
        let sig = recover_c_char(ur_encoded_result.data);
        assert_eq!(
            "UR:ICP-SIGNATURE/OEADTPDAGDNDCAWMGTFRKIGRPMNDUTDNBTKGFSSBJNAOHDFZDMTPSGFLVSQDMTGMTSDKFXOEWEOEBKSKTTHDJZKSUTLRZTEMUOCWFDFXKNKSBBRKDNKIRDDKMYCLFWLODLNBWNWFMDIHAMJZYKMUHYMUYKGEDPIDWPFTFTHTUOBSMEYARTTNVAVA".to_string(),
            sig
        )
    }
}
