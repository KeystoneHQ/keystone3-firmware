pub mod structs;

use crate::common::{
    errors::RustCError,
    free::Free,
    structs::{SimpleResponse, TransactionCheckResult, TransactionParseResult},
    types::{Ptr, PtrBytes, PtrString, PtrT, PtrUR},
    ur::{UREncodeResult, FRAGMENT_MAX_LENGTH_DEFAULT},
    utils::{convert_c_char, recover_c_char},
};
use crate::{extract_array_mut, extract_array};
use crate::{extract_ptr_with_type, make_free_method};
use alloc::{boxed::Box, format, string::String, string::ToString};
use app_zcash::get_address;
use core::slice;
use cryptoxide::hashing::sha256;
use cty::c_char;
use keystore::algorithms::{
    ed25519::slip10_ed25519::get_private_key_by_seed,
    zcash::{calculate_seed_fingerprint, derive_ufvk},
};
use structs::DisplayPczt;
use ur_registry::{traits::RegistryItem, zcash::zcash_pczt::ZcashPczt};
use zcash_vendor::zcash_protocol::consensus::MainNetwork;
use zeroize::Zeroize;

#[no_mangle]
pub unsafe extern "C" fn derive_zcash_ufvk(
    seed: PtrBytes,
    seed_len: u32,
    account_path: PtrString,
) -> *mut SimpleResponse<c_char> {
    let mut seed = extract_array_mut!(seed, u8, seed_len as usize);
    let account_path = unsafe { recover_c_char(account_path) };
    let ufvk_text = derive_ufvk(&MainNetwork, seed, &account_path);
    let result = match ufvk_text {
        Ok(text) => SimpleResponse::success(convert_c_char(text)).simple_c_ptr(),
        Err(e) => SimpleResponse::from(e).simple_c_ptr(),
    };
    seed.zeroize();
    result
}

#[no_mangle]
pub unsafe extern "C" fn calculate_zcash_seed_fingerprint(
    seed: PtrBytes,
    seed_len: u32,
) -> *mut SimpleResponse<u8> {
    let mut seed = extract_array_mut!(seed, u8, seed_len as usize);
    let sfp = calculate_seed_fingerprint(seed);
    let result = match sfp {
        Ok(bytes) => {
            SimpleResponse::success(Box::into_raw(Box::new(bytes)) as *mut u8).simple_c_ptr()
        }
        Err(e) => SimpleResponse::from(e).simple_c_ptr(),
    };
    seed.zeroize();
    result
}

#[no_mangle]
pub unsafe extern "C" fn generate_zcash_default_address(
    ufvk_text: PtrString,
) -> *mut SimpleResponse<c_char> {
    let ufvk_text = unsafe { recover_c_char(ufvk_text) };
    let address = get_address(&MainNetwork, &ufvk_text);
    match address {
        Ok(text) => SimpleResponse::success(convert_c_char(text)).simple_c_ptr(),
        Err(e) => SimpleResponse::from(e).simple_c_ptr(),
    }
}

#[no_mangle]
pub unsafe extern "C" fn check_zcash_tx(
    tx: PtrUR,
    ufvk: PtrString,
    seed_fingerprint: PtrBytes,
    account_index: u32,
    disabled: bool,
) -> *mut TransactionCheckResult {
    if disabled {
        return TransactionCheckResult::from(RustCError::UnsupportedTransaction(
            "zcash is not supported for slip39 and passphrase wallet now".to_string(),
        ))
        .c_ptr();
    }
    let pczt = extract_ptr_with_type!(tx, ZcashPczt);
    let ufvk_text = unsafe { recover_c_char(ufvk) };
    let seed_fingerprint = extract_array!(seed_fingerprint, u8, 32);
    let seed_fingerprint = seed_fingerprint.try_into().unwrap();
    match app_zcash::check_pczt(
        &MainNetwork,
        &pczt.get_data(),
        &ufvk_text,
        seed_fingerprint,
        account_index,
    ) {
        Ok(_) => TransactionCheckResult::new().c_ptr(),
        Err(e) => TransactionCheckResult::from(e).c_ptr(),
    }
}

#[no_mangle]
pub unsafe extern "C" fn parse_zcash_tx(
    tx: PtrUR,
    ufvk: PtrString,
    seed_fingerprint: PtrBytes,
) -> Ptr<TransactionParseResult<DisplayPczt>> {
    let pczt = extract_ptr_with_type!(tx, ZcashPczt);
    let ufvk_text = unsafe { recover_c_char(ufvk) };
    let seed_fingerprint = extract_array!(seed_fingerprint, u8, 32);
    let seed_fingerprint = seed_fingerprint.try_into().unwrap();
    match app_zcash::parse_pczt(&MainNetwork, &pczt.get_data(), &ufvk_text, seed_fingerprint) {
        Ok(pczt) => TransactionParseResult::success(DisplayPczt::from(&pczt).c_ptr()).c_ptr(),
        Err(e) => TransactionParseResult::from(e).c_ptr(),
    }
}

#[no_mangle]
pub unsafe extern "C" fn sign_zcash_tx(
    tx: PtrUR,
    seed: PtrBytes,
    seed_len: u32,
) -> *mut UREncodeResult {
    let pczt = extract_ptr_with_type!(tx, ZcashPczt);
    let mut seed = extract_array_mut!(seed, u8, seed_len as usize);
    let result = match app_zcash::sign_pczt(&pczt.get_data(), seed) {
        Ok(pczt) => match ZcashPczt::new(pczt).try_into() {
            Err(e) => UREncodeResult::from(e).c_ptr(),
            Ok(v) => UREncodeResult::encode(
                v,
                ZcashPczt::get_registry_type().get_type(),
                FRAGMENT_MAX_LENGTH_DEFAULT,
            )
            .c_ptr(),
        },
        Err(e) => UREncodeResult::from(e).c_ptr(),
    };
    seed.zeroize();
    result
}

make_free_method!(TransactionParseResult<DisplayPczt>);

use aes::cipher::block_padding::Pkcs7;
use aes::cipher::generic_array::GenericArray;
use aes::cipher::{BlockDecryptMut, BlockEncryptMut, KeyIvInit};

type Aes256CbcEnc = cbc::Encryptor<aes::Aes256>;
type Aes256CbcDec = cbc::Decryptor<aes::Aes256>;

#[no_mangle]
pub unsafe extern "C" fn rust_aes256_cbc_encrypt(
    data: PtrString,
    password: PtrString,
    iv: PtrBytes,
    iv_len: u32,
) -> *mut SimpleResponse<c_char> {
    let data = unsafe { recover_c_char(data) };
    let data = data.as_bytes();
    let password = unsafe { recover_c_char(password) };
    let iv = extract_array!(iv, u8, iv_len as usize);
    let key = sha256(password.as_bytes());
    let iv = GenericArray::from_slice(iv);
    let key = GenericArray::from_slice(&key);
    let ct = Aes256CbcEnc::new(key, iv).encrypt_padded_vec_mut::<Pkcs7>(data);
    SimpleResponse::success(convert_c_char(hex::encode(ct))).simple_c_ptr()
}

#[no_mangle]
pub unsafe extern "C" fn rust_aes256_cbc_decrypt(
    hex_data: PtrString,
    password: PtrString,
    iv: PtrBytes,
    iv_len: u32,
) -> *mut SimpleResponse<c_char> {
    let hex_data = unsafe { recover_c_char(hex_data) };
    let data = hex::decode(hex_data).unwrap();
    let password = unsafe { recover_c_char(password) };
    let iv = extract_array!(iv, u8, iv_len as usize);
    let key = sha256(password.as_bytes());
    let iv = GenericArray::from_slice(iv);
    let key = GenericArray::from_slice(&key);

    match Aes256CbcDec::new(key, iv).decrypt_padded_vec_mut::<Pkcs7>(&data) {
        Ok(pt) => {
            SimpleResponse::success(convert_c_char(String::from_utf8(pt).unwrap())).simple_c_ptr()
        }
        Err(_e) => SimpleResponse::from(RustCError::InvalidHex("decrypt failed".to_string()))
            .simple_c_ptr(),
    }
}

#[no_mangle]
pub unsafe extern "C" fn rust_derive_iv_from_seed(
    seed: PtrBytes,
    seed_len: u32,
) -> *mut SimpleResponse<u8> {
    let mut seed = extract_array_mut!(seed, u8, seed_len as usize);
    let iv_path = "m/44'/1557192335'/0'/2'/0'".to_string();
    let iv = get_private_key_by_seed(seed, &iv_path).unwrap();
    let mut iv_bytes = [0; 16];
    iv_bytes.copy_from_slice(&iv[..16]);
    seed.zeroize();
    SimpleResponse::success(Box::into_raw(Box::new(iv_bytes)) as *mut u8).simple_c_ptr()
}

#[cfg(test)]
mod tests {
    use alloc::{string::String, vec::Vec};

    use super::*;

    #[test]
    fn test_aes256_cbc_encrypt() {
        let mut data = convert_c_char("hello world".to_string());
        let mut password = convert_c_char("password".to_string());
        let mut seed = hex::decode("00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000").unwrap();
        let iv = unsafe { rust_derive_iv_from_seed(seed.as_mut_ptr(), 64) };
        let mut iv = unsafe { slice::from_raw_parts_mut((*iv).data, 16) };
        let iv_len = 16;
        let ct = unsafe { rust_aes256_cbc_encrypt(data, password, iv.as_mut_ptr(), iv_len as u32) };
        let ct_vec = unsafe { (*ct).data };
        let value = unsafe { recover_c_char(ct_vec) };
        assert_eq!(value, "639194f4bf964e15d8ea9c9bd9d96918");
    }

    #[test]
    fn test_aes256_cbc_decrypt() {
        //8dd387c3b2656d9f24ace7c3daf6fc26a1c161098460f8dddd37545fc951f9cd7da6c75c71ae52f32ceb8827eca2169ef4a643d2ccb9f01389d281a85850e2ddd100630ab1ca51310c3e6ccdd3029d0c48db18cdc971dba8f0daff9ad281b56221ffefc7d32333ea310a1f74f99dea444f8a089002cf1f0cd6a4ddf608a7b5388dc09f9417612657b9bf335a466f951547f9707dd129b3c24c900a26010f51c543eba10e9aabef7062845dc6969206b25577a352cb4d984db67c54c7615fe60769726bffa59fd8bd0b66fe29ee3c358af13cf0796c2c062bc79b73271eb0366f0536e425f8e42307ead4c695804fd3281aca5577d9a621e3a8047b14128c280c45343b5bbb783a065d94764e90ad6820fe81a200637401c256b1fb8f58a9d412d303b89c647411662907cdc55ed93adb
        //73e6ca87d5cd5622cdc747367905efe7
        //68487dc295052aa79c530e283ce698b8c6bb1b42ff0944252e1910dbecdc5425
        let data = convert_c_char("639194f4bf964e15d8ea9c9bd9d96918".to_string());
        let password = convert_c_char("password".to_string());
        let mut seed = hex::decode("00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000").unwrap();
        let iv = unsafe { rust_derive_iv_from_seed(seed.as_mut_ptr(), 64) };
        let iv = unsafe { slice::from_raw_parts_mut((*iv).data, 16) };
        let iv_len = 16;
        let pt = unsafe { rust_aes256_cbc_decrypt(data, password, iv.as_mut_ptr(), iv_len as u32) };
        assert!(!pt.is_null());
        let ct_vec = unsafe { (*pt).data };
        let value = unsafe { recover_c_char(ct_vec) };
        assert_eq!(value, "hello world");
    }

    #[test]
    fn test_dep_aes256() {
        let mut data = b"hello world";
        let seed = hex::decode("00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000").unwrap();
        let iv_path = "m/44'/1557192335'/0'/2'/0'".to_string();
        let iv = get_private_key_by_seed(&seed, &iv_path).unwrap();
        let mut iv_bytes = [0; 16];
        iv_bytes.copy_from_slice(&iv[..16]);
        let key = sha256(b"password");
        let iv = GenericArray::from_slice(&iv_bytes);
        let key = GenericArray::from_slice(&key);

        let encrypter = Aes256CbcEnc::new(key, iv);
        let decrypter = Aes256CbcDec::new(key, iv);

        let ct = encrypter.encrypt_padded_vec_mut::<Pkcs7>(data);
        let pt = decrypter.decrypt_padded_vec_mut::<Pkcs7>(&ct).unwrap();

        assert_eq!(String::from_utf8(pt).unwrap(), "hello world");
    }
}
