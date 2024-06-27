#![no_std]

extern crate alloc;

use crate::structs::{ArweaveRequestType, DisplayArweaveMessage, DisplayArweaveTx};
use alloc::boxed::Box;
use alloc::fmt::format;
use alloc::string::{String, ToString};
use alloc::vec::Vec;
use alloc::{format, slice};
use app_arweave::parse_data_item;
use app_arweave::{
    aes256_decrypt, aes256_encrypt, errors::ArweaveError, fix_address,
    generate_public_key_from_primes, generate_secret, parse,
};
use common_rust_c::errors::{ErrorCodes, RustCError};
use common_rust_c::extract_ptr_with_type;
use common_rust_c::structs::{SimpleResponse, TransactionCheckResult, TransactionParseResult};
use common_rust_c::types::{PtrBytes, PtrString, PtrT, PtrUR};
use common_rust_c::ur::{UREncodeResult, FRAGMENT_MAX_LENGTH_DEFAULT};
use common_rust_c::utils::{convert_c_char, recover_c_char};
use cty::c_char;
use keystore::algorithms::ed25519::slip10_ed25519::get_private_key_by_seed;
use keystore::algorithms::rsa::{sign_message, SigningOption};
use third_party::hex;
use third_party::serde_json;
use third_party::serde_json::{json, Value};
use third_party::ur_registry::arweave::arweave_sign_request::{
    ArweaveSignRequest, SaltLen, SignType,
};
use third_party::ur_registry::arweave::arweave_signature::ArweaveSignature;
use third_party::ur_registry::traits::{RegistryItem, To};

pub mod data_item;
pub mod structs;

fn generate_aes_key_iv(seed: &[u8]) -> ([u8; 32], [u8; 16]) {
    // The number 1557192335 is derived from the ASCII representation of "keystone" hashed with SHA-256, taking the first 32 bits with the highest bit set to 0.
    let key_path = "m/44'/1557192335'/0'/0'/0'".to_string();
    let iv_path = "m/44'/1557192335'/0'/1'/0'".to_string();
    let key = get_private_key_by_seed(seed, &key_path).unwrap();
    let (_, key_bytes) = third_party::cryptoxide::ed25519::keypair(&key);
    let iv = get_private_key_by_seed(seed, &iv_path).unwrap();
    let (_, iv) = third_party::cryptoxide::ed25519::keypair(&iv);
    let mut iv_bytes: [u8; 16] = [0; 16];
    iv_bytes.copy_from_slice(&iv[..16]);
    (key_bytes, iv_bytes)
}

#[no_mangle]
pub extern "C" fn generate_arweave_secret(
    seed: PtrBytes,
    seed_len: u32,
) -> *mut SimpleResponse<u8> {
    let seed = unsafe { slice::from_raw_parts(seed, seed_len as usize) };
    let secret = generate_secret(seed).unwrap();
    let mut secret_bytes: [u8; 512] = [0; 512];
    secret_bytes[..256].copy_from_slice(&secret.primes()[0].to_bytes_be());
    secret_bytes[256..].copy_from_slice(&secret.primes()[1].to_bytes_be());

    SimpleResponse::success(Box::into_raw(Box::new(secret_bytes)) as *mut u8).simple_c_ptr()
}

#[no_mangle]
pub extern "C" fn generate_arweave_public_key_from_primes(
    p: PtrBytes,
    p_len: u32,
    q: PtrBytes,
    q_len: u32,
) -> *mut SimpleResponse<u8> {
    let p = unsafe { slice::from_raw_parts(p, p_len as usize) };
    let q = unsafe { slice::from_raw_parts(q, q_len as usize) };
    let public = generate_public_key_from_primes(p, q).unwrap();
    return SimpleResponse::success(Box::into_raw(Box::new(public)) as *mut u8).simple_c_ptr();
}

#[no_mangle]
pub extern "C" fn generate_rsa_public_key(
    p: PtrBytes,
    p_len: u32,
    q: PtrBytes,
    q_len: u32,
) -> *mut SimpleResponse<c_char> {
    let p = unsafe { slice::from_raw_parts(p, p_len as usize) };
    let q = unsafe { slice::from_raw_parts(q, q_len as usize) };
    let public = generate_public_key_from_primes(p, q).unwrap();
    return SimpleResponse::success(convert_c_char(hex::encode(public))).simple_c_ptr();
}

#[no_mangle]
pub extern "C" fn aes256_encrypt_primes(
    seed: PtrBytes,
    seed_len: u32,
    data: PtrBytes,
) -> *mut SimpleResponse<u8> {
    let seed = unsafe { slice::from_raw_parts(seed, seed_len as usize) };
    let data = unsafe { slice::from_raw_parts(data, 512) };
    let (key, iv) = generate_aes_key_iv(seed);
    let encrypted_data = aes256_encrypt(&key, &iv, data).unwrap();
    let mut result_bytes: [u8; 528] = [0; 528];
    result_bytes.copy_from_slice(&encrypted_data);
    return SimpleResponse::success(Box::into_raw(Box::new(result_bytes)) as *mut u8)
        .simple_c_ptr();
}

#[no_mangle]
pub extern "C" fn aes256_decrypt_primes(
    seed: PtrBytes,
    seed_len: u32,
    data: PtrBytes,
) -> *mut SimpleResponse<u8> {
    let seed = unsafe { slice::from_raw_parts(seed, seed_len as usize) };
    let data = unsafe { slice::from_raw_parts(data, 528) };
    let (key, iv) = generate_aes_key_iv(seed);
    match aes256_decrypt(&key, &iv, data) {
        Ok(decrypted_data) => {
            let mut result_bytes: [u8; 512] = [0; 512];
            result_bytes.copy_from_slice(&decrypted_data);
            SimpleResponse::success(Box::into_raw(Box::new(result_bytes)) as *mut u8).simple_c_ptr()
        }
        Err(e) => SimpleResponse::from(e).simple_c_ptr(),
    }
}

#[no_mangle]
pub extern "C" fn arweave_get_address(xpub: PtrString) -> *mut SimpleResponse<c_char> {
    let xpub = recover_c_char(xpub);
    let address = app_arweave::generate_address(hex::decode(xpub).unwrap()).unwrap();
    return SimpleResponse::success(convert_c_char(address)).simple_c_ptr();
}

#[no_mangle]
pub extern "C" fn fix_arweave_address(address: PtrString) -> *mut SimpleResponse<c_char> {
    let address = recover_c_char(address);
    let fixed_address = fix_address(&address);
    return SimpleResponse::success(convert_c_char(fixed_address)).simple_c_ptr();
}

#[no_mangle]
pub extern "C" fn ar_check_tx(
    ptr: PtrUR,
    master_fingerprint: PtrBytes,
    length: u32,
) -> PtrT<TransactionCheckResult> {
    if length != 4 {
        return TransactionCheckResult::from(RustCError::InvalidMasterFingerprint).c_ptr();
    }
    let mfp = unsafe { slice::from_raw_parts(master_fingerprint, 4) };
    let sign_request = extract_ptr_with_type!(ptr, ArweaveSignRequest);
    let ur_mfp = sign_request.get_master_fingerprint();

    if let Ok(mfp) = mfp.try_into() as Result<[u8; 4], _> {
        if hex::encode(mfp) == hex::encode(ur_mfp) {
            return TransactionCheckResult::new().c_ptr();
        } else {
            return TransactionCheckResult::from(RustCError::MasterFingerprintMismatch).c_ptr();
        }
    }
    return TransactionCheckResult::from(RustCError::InvalidMasterFingerprint).c_ptr();
}

#[no_mangle]
pub extern "C" fn ar_request_type(ptr: PtrUR) -> *mut SimpleResponse<ArweaveRequestType> {
    let sign_request = extract_ptr_with_type!(ptr, ArweaveSignRequest);
    let sign_type = sign_request.get_sign_type();
    let sign_type_str = match sign_type {
        SignType::Transaction => ArweaveRequestType::ArweaveRequestTypeTransaction,
        SignType::Message => ArweaveRequestType::ArweaveRequestTypeMessage,
        SignType::DataItem => ArweaveRequestType::ArweaveRequestTypeDataItem,
        _ => ArweaveRequestType::ArweaveRequestTypeUnknown,
    };
    SimpleResponse::success(Box::into_raw(Box::new(sign_type_str)) as *mut ArweaveRequestType)
        .simple_c_ptr()
}

#[no_mangle]
pub extern "C" fn ar_message_parse(
    ptr: PtrUR,
) -> PtrT<TransactionParseResult<DisplayArweaveMessage>> {
    let sign_request = extract_ptr_with_type!(ptr, ArweaveSignRequest);
    let sign_data = sign_request.get_sign_data();
    let raw_message = hex::encode(sign_request.get_sign_data());
    let message = String::from_utf8(sign_data).unwrap_or_default();
    let display_message = DisplayArweaveMessage {
        message: convert_c_char(message),
        raw_message: convert_c_char(raw_message),
    };
    TransactionParseResult::success(
        Box::into_raw(Box::new(display_message)) as *mut DisplayArweaveMessage
    )
    .c_ptr()
}

fn get_value(raw_json: &Value, key: &str) -> String {
    raw_json["formatted_json"][key.to_string()]
        .as_str()
        .unwrap()
        .to_string()
}

#[no_mangle]
pub extern "C" fn ar_parse(ptr: PtrUR) -> PtrT<TransactionParseResult<DisplayArweaveTx>> {
    let sign_request = extract_ptr_with_type!(ptr, ArweaveSignRequest);
    let sign_data = sign_request.get_sign_data();
    let raw_tx = parse(&sign_data).unwrap();
    let raw_json: Value = serde_json::from_str(&raw_tx).unwrap();
    let value = get_value(&raw_json, "quantity");
    let fee = get_value(&raw_json, "reward");
    let from = get_value(&raw_json, "from");
    let to = get_value(&raw_json, "target");
    let detail = get_value(&raw_json, "detail");
    let display_tx = DisplayArweaveTx {
        value: convert_c_char(value),
        fee: convert_c_char(fee),
        from: convert_c_char(from),
        to: convert_c_char(to),
        detail: convert_c_char(detail),
    };
    TransactionParseResult::success(Box::into_raw(Box::new(display_tx)) as *mut DisplayArweaveTx)
        .c_ptr()
}

fn parse_sign_data(ptr: PtrUR) -> Result<Vec<u8>, ArweaveError> {
    let sign_request = extract_ptr_with_type!(ptr, ArweaveSignRequest);
    let sign_data = sign_request.get_sign_data();
    match sign_request.get_sign_type() {
        SignType::Transaction => {
            let raw_tx = parse(&sign_data)?;
            let raw_json: Value = serde_json::from_str(&raw_tx).unwrap();
            let signature_data = raw_json["formatted_json"]["signature_data"]
                .as_str()
                .unwrap();
            let signature_data = hex::decode(signature_data).unwrap();
            Ok(signature_data)
        }
        SignType::DataItem => {
            let data_item = parse_data_item(&sign_data)?;
            Ok(data_item.deep_hash()?)
        }
        SignType::Message => {
            return Ok(sign_data);
        }
    }
}

fn build_sign_result(ptr: PtrUR, p: &[u8], q: &[u8]) -> Result<ArweaveSignature, ArweaveError> {
    let sign_request = extract_ptr_with_type!(ptr, ArweaveSignRequest);
    let salt_len = match sign_request.get_salt_len() {
        SaltLen::Zero => 0,
        SaltLen::Digest => 32,
    };
    let signature_data = parse_sign_data(ptr)?;
    let sign_type = sign_request.get_sign_type();
    let signing_option = match sign_type {
        SignType::Transaction | SignType::DataItem => SigningOption::PSS { salt_len },
        SignType::Message => SigningOption::RSA { salt_len },
    };
    let signature = sign_message(&signature_data, p, q, &signing_option)?;

    Ok(ArweaveSignature::new(
        sign_request.get_request_id(),
        signature,
    ))
}

#[no_mangle]
pub extern "C" fn ar_sign_tx(
    ptr: PtrUR,
    p: PtrBytes,
    p_len: u32,
    q: PtrBytes,
    q_len: u32,
) -> PtrT<UREncodeResult> {
    let p = unsafe { slice::from_raw_parts(p, p_len as usize) };
    let q = unsafe { slice::from_raw_parts(q, q_len as usize) };

    build_sign_result(ptr, p, q)
        .map(|v: ArweaveSignature| v.try_into())
        .map_or_else(
            |e| UREncodeResult::from(e).c_ptr(),
            |v| {
                v.map_or_else(
                    |e| UREncodeResult::from(e).c_ptr(),
                    |data| {
                        UREncodeResult::encode(
                            data,
                            ArweaveSignature::get_registry_type().get_type(),
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
    use super::*;
    extern crate std;

    #[test]
    fn test_generate_arweave_secret() {
        let seed = hex::decode("96063C45132C840F7E1665A3B97814D8EB2586F34BD945F06FA15B9327EEBE355F654E81C6233A52149D7A95EA7486EB8D699166F5677E507529482599624CDC").unwrap();
        let p = hex::decode("EA8E3612876ED1433E5909D25F699F7C5D4984CF0D2F268B185141F0E29CE65237EAD8236C94A0A9547F1FEABD4F54399C626C0FB813249BC74A3082F8637A9E9A3C9D4F6E1858ED29770FE95418ED66F07A9F2F378D43D31ED37A0E6942727394A87B93540E421742ADE9630E26500FD2C01502DF8E3F869C70DAA97D4583048DD367E2977851052F6A991182318015557EC81B58E81B668E3A715212C807A1D7835FCB2B87B5DEFAC0948B220D340D6B2DA0DCFC7123DE1F1424F6F5B8EAFA719B3DE8B9B6FEC196E2E393CE30204267A586625541C7B1433F8FA7873B51B3E65462831BF34A4E5912297A06B2E91B31657DFA3CCFDB5F94D438D9904CFD27").unwrap();
        let q = hex::decode("E360BFD757FF6FCF844AF226DCA7CFBD353A89112079C9C5A17C4F354DE0B1BE38BBFD73EAA77C4E2FFC681A79CEC8C8E79A5A00E32113A77748F435717BE6AD04AEF473BCE05DC3B742AAB853C02C565847133AFFD451B472B13031300978606F74BE8761A69733BEF8C2CCD6F396A0CCE23CDC73A8FF4609F47C18FE4626B788C4BFB73B9CF10BC5D6F80E9B9847530973CF5212D8EB142EAA155D774417D7BF89E1F229472926EA539AC3BAF42CF63EF18A6D85915727E9A77B4EA31B577B1E4A35C40CCCE72F5ECE426709E976DAEDBE7B76291F89EB85903182035CA98EB156563E392D0D1E427C59657B9EDF1DDB049BBB9620B881D8715982AD257D29").unwrap();
        let secret = generate_secret(&seed).unwrap();
        let mut secret_bytes: [u8; 512] = [0; 512];
        secret_bytes[..256].copy_from_slice(&secret.primes()[0].to_bytes_be());
        secret_bytes[256..].copy_from_slice(&secret.primes()[1].to_bytes_be());

        assert_eq!(secret_bytes[..256], p[..]);
        assert_eq!(secret_bytes[256..], q[..]);
    }

    #[test]
    fn test_aes256() {
        let seed = hex::decode("99391fa4462a92eba7661f54acf54af5b60410cd407b82598b675ede5bd4d9ebf1a5b70f66376fbf777562be5eb4e20c9cba31b505dd0ed8ca0c47cec80d31fb").unwrap();
        let data = hex::decode("cfbe18586b3ed63813e05ba65f606a6bf936c358285162dae3c123fb657f3327284ceacada59cf64c1bf9b8f55a96575815b6904dda565a786a7222d563f3e70729a3cc6d46a3916083b6dd4c97ab67a599a8d5b382a6d7d4ea04bedb37fa8856bdcc871ca1a6ed0916de02eb6b200ca150ed730cbb73ce31eac3f84a3e10e208195df9c33d933bebb64a42ccec3744dd9a9fbde484a993ac17cb027acbc0c2948af4a3c3cce1f64ed724fbcc9360cadfa3bbcc73f2798ea95ed6994a7c9c76ae112a96f75040dcceabf49b6546a0ca869a08a58a9befcc82fc7d973cb8e8a0c8f9659c66b3de614d5ff531a130a1d4e3e06a3c4f8957b612913d9597f6a12a5f794c6cd8a066b819c537fd9cc2a79de7d39069d9fd1ad79652e199845ae4cea68350c06e0af43bd71b302a1d578a9df7c9a351d5d23d5104deef986f326cac564628f4e8fbc2b83be05b288434eb99cfdf0e57b755714b93b16ee1eab7ae2cb4cdec24ec350fa8a2d20a71feb3a7b1ceaecda03479cbd1a1614c64bc2e4d09586204dde7525a077a00632c71fa1771a8f8164beed3d02bb4f47a53733a9540b4a1cd0e7aabe09b1d3b1d331004533aaac75e48e12ba3b1bc2f6e5fec9fb6942da3d113eeb8ac3b67f6ca67fc4f5be98d19f8551ab25af29ac0c0ba5790d930515a76878bf2dbd34653b3311ce2fdcd4ea74ffe1cea687bf45ee7baab1987aaf").unwrap();
        let encoded_data_test = hex::decode("85113aedb4f44eb56cf113557fecf91afe908a9869cca1ab4c22b50eb01fcb7081e6d687533617d9451c062f15ab32fac5558fe4f56e4fd66415cb2904e82fd207f206fd2067c4c553b05bdd663523209e1940e8ffdee3621c2c79ae0e3c1eece83824f22565eb8112063dc23d0f0609ef4669c59ae117c0c4fb2136bc74d98f29e7903d59e520106f4d0281025afeba9ea2ebfdde99d7ac8bdb22eedc569e867f18629a5639fd51d46b0caa798ee6b84c20e4c15a112bc7005a433bc8850d83df2ca10588930b8151200fe68c43183b16a64d76bd4b46c429bf3e45f954efa28040e3edfaf942f7ddcdf573c10f0952ca0f7b8d932f5ddf6f5ce4d0092e399dfa485cfdd19fcd118bb814d6d321bd114a1e8c2314926c41b1d0fcaec33222c53b02b24081dce8cbe25154f9d9ad195955ab7dada82ca3642afc39a746dcb0fa2647334533272d2abe6d99f400451a2b1f1dfc6341b45fedf0d68abaa210c0203233fb0aa7e6503d3c6e385c64299277005316f8bca38fc1bc8d82b2575ff80d24d8ed2efd07179219143c3acba5a2df1778228aaeb2f44d5f03b25cdc51c08e76039dae0b33f1aa23f48a27a6a5342259e5b90ee3927d07e9982ac46ebe66fe416d7745a3ba15931aa7f1ce0a2ef0a1aee9f2f6d3bf1b7485889d55305e7ccbbc5b0fc33a8843f4c1a3518c659275009c47fb2d3cd53bd4c9feba630816bc9f96101d9cd94087d7392674b735d379c2").unwrap();
        assert_eq!(encoded_data_test.len(), 528);
        let (key, iv) = generate_aes_key_iv(&seed);
        let encrypted_data = aes256_encrypt(&key, &iv, &data).unwrap();
        let decrypted_data = aes256_decrypt(&key, &iv, &encrypted_data).unwrap();
        assert_eq!(data, decrypted_data);
        assert_eq!(encrypted_data.len(), 528);
        assert_eq!(decrypted_data.len(), 512);

        match aes256_decrypt(&key, &iv, &encoded_data_test) {
            Ok(decrypted_data2) => {
                assert_eq!(decrypted_data2.len(), 512);
                assert_eq!(decrypted_data, decrypted_data2);
            }
            Err(_) => {}
        }
    }
}
