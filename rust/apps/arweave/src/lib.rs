#![no_std]
#![feature(error_in_core)]

pub mod errors;

#[macro_use]
extern crate alloc;
extern crate aes;

#[cfg(test)]
#[macro_use]
extern crate std;

use crate::errors::{ArweaveError, Result};
use alloc::string::{String, ToString};
use alloc::vec::Vec;
use sha2;
use sha2::Digest;
use aes::cipher::{
    BlockDecryptMut, generic_array::GenericArray,
    BlockEncryptMut, KeyIvInit
};
use aes::cipher::block_padding::Pkcs7;
use keystore::algorithms::rsa::get_rsa_secret_from_seed;
use third_party::base64;
use third_party::rsa::{BigUint, RsaPrivateKey};

type Aes256CbcEnc = cbc::Encryptor<aes::Aes256>;
type Aes256CbcDec = cbc::Decryptor<aes::Aes256>;

pub fn aes256_encrypt(key: &[u8], iv: &[u8], data: &[u8]) -> Result<Vec<u8>> {
    let iv = GenericArray::from_slice(iv);
    let key = GenericArray::from_slice(key);

    let ct = Aes256CbcEnc::new(key, iv)
        .encrypt_padded_vec_mut::<Pkcs7>(data);
    Ok(ct)
}

pub fn aes256_decrypt(key: &[u8], iv: &[u8], data: &[u8]) -> Result<Vec<u8>> {
    let iv = GenericArray::from_slice(iv);
    let key = GenericArray::from_slice(key);

    match Aes256CbcDec::new(key, iv)
    .decrypt_padded_vec_mut::<Pkcs7>(data) {
        Ok(pt) => Ok(pt),
        Err(e) => Err(ArweaveError::KeystoreError(format!(
            "aes256_decrypt failed {:?}",
            e.to_string()
        )))
    }
}

pub fn generate_address(owner: Vec<u8>) -> Result<String> {
    let mut hasher = sha2::Sha256::new();
    hasher.update(owner);
    let owner_base64url_sha256 = hasher.finalize();
    let address = base64::encode_config(owner_base64url_sha256.as_slice(), base64::URL_SAFE_NO_PAD);
    Ok(address)
}

pub fn generate_public_key_from_primes(p: &[u8], q: &[u8]) -> Result<Vec<u8>> {
    let p = BigUint::from_bytes_be(p);
    let q = BigUint::from_bytes_be(q);
    let n = p * q;
    Ok(n.to_bytes_be())
}

pub fn generate_secret(seed: &[u8]) -> Result<RsaPrivateKey> {
    match get_rsa_secret_from_seed(seed) {
        Ok(secret) => Ok(secret),
        Err(e) => Err(ArweaveError::KeystoreError(format!(
            "generate secret failed {:?}",
            e.to_string()
        ))),
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use alloc::borrow::ToOwned;
    use third_party::{hex, rsa::PublicKeyParts};

    #[test]
    fn test_generate_address() {
        let seed = hex::decode("cfd803c49799c014c239ff2e6a986575c360269927c715ee275a4f21f336eb342c3e3659ccd65385c3ba9017a3e4aee721ad4310b131fe98eb50e8944acd2ad5").unwrap();
        let result = generate_secret(seed.as_slice()).unwrap();
        let address = generate_address(result.n().to_bytes_be()).unwrap();
        assert_eq!(address, "cV_M3Zdqq9_hWOqfSLGezrKz4slXjWYOLn_lrN0ouLE");
    }

    #[test]
    fn test_generate_secret() {
        let seed = hex::decode("5eb00bbddcf069084889a8ab9155568165f5c453ccb85e70811aaed6f6da5fc19a5ac40b389cd370d086206dec8aa6c43daea6690f20ad3d8d48b2d2ce9e38e4").unwrap();
        let p = hex::decode("fdec3a1aee520780ca4058402d0422b5cd5950b715728f532499dd4bbcb68e5d44650818b43656782237316c4b0e2faa2b15c245fb82d10cf4f5b420f1f293ba75b2c8d8cef6ad899c34ce9de482cb248cc5ab802fd93094a63577590d812d5dd781846ef7d4f5d9018199c293966371c2349b0f847c818ec99caad800116e02085d35a39a913bc735327705161761ae30a4ec775f127fbb5165418c0fe08e54ae0aff8b2dab2b82d3b4b9c807de5fae116096075cf6d5b77450d743d743e7dcc56e7cafdcc555f228e57b363488e171d099876993e93e37a94983ccc12dba894c58ca84ac154c1343922c6a99008fabd0fa7010d3cc34f69884fec902984771").unwrap();
        let q = hex::decode("c5b50031ba31ab7c8b76453ce771f048b84fb89a3e4d44c222c3d8c823c683988b0dbf354d8b8cbf65f3db53e1365d3c5e043f0155b41d1ebeca6e20b2d6778600b5c98ffdba33961dae73b018307ef2bce9d217bbdf32964080f8db6f0cf7ef27ac825fcaf98d5143690a5d7e138f4875280ed6de581e66ed17f83371c268a073e4594814bcc88a33cbb4ec8819cc722ea15490312b85fed06e39274c4f73ac91c7f4d1b899729691cce616fb1a5feee1972456addcb51ac830e947fcc1b823468f0eefbaf195ac3b34f0baf96afc6fa77ee2e176081d6d91ce8c93c3d0f3547e48d059c9da447ba05ee3984703bebfd6d704b7f327ffaea7d0f63d0d3c6d65").unwrap();
        let result = generate_secret(seed.as_slice()).unwrap();

        let public_key = generate_public_key_from_primes(p.as_slice(), q.as_slice()).unwrap();

        assert_eq!(result.primes()[0], BigUint::from_bytes_be(&p));
        assert_eq!(result.primes()[1], BigUint::from_bytes_be(&q));
        assert_eq!(
            result.n().to_owned().to_bytes_be(),
            BigUint::from_bytes_be(public_key.as_slice()).to_bytes_be()
        );
    }

    #[test]
    fn test_aes128() {
        let key = hex::decode("5eb00bbddcf069084889a8ab915556815eb00bbddcf069084889a8ab91555681").unwrap();
        let iv = hex::decode("65f5c453ccb85e70811aaed6f6da5fc1").unwrap();
        let data = hex::decode("fdec3a1aee520780ca4058402d0422b5cd5950b715728f532499dd4bbcb68e5d44650818b43656782237316c4b0e2faa2b15c245fb82d10cf4f5b420f1f293ba75b2c8d8cef6ad899c34ce9de482cb248cc5ab802fd93094a63577590d812d5dd781846ef7d4f5d9018199c293966371c2349b0f847c818ec99caad800116e02085d35a39a913bc735327705161761ae30a4ec775f127fbb5165418c0fe08e54ae0aff8b2dab2b82d3b4b9c807de5fae116096075cf6d5b77450d743d743e7dcc56e7cafdcc555f228e57b363488e171d099876993e93e37a94983ccc12dba894c58ca84ac154c1343922c6a99008fabd0fa7010d3cc34f69884fec902984771c5b50031ba31ab7c8b76453ce771f048b84fb89a3e4d44c222c3d8c823c683988b0dbf354d8b8cbf65f3db53e1365d3c5e043f0155b41d1ebeca6e20b2d6778600b5c98ffdba33961dae73b018307ef2bce9d217bbdf32964080f8db6f0cf7ef27ac825fcaf98d5143690a5d7e138f4875280ed6de581e66ed17f83371c268a073e4594814bcc88a33cbb4ec8819cc722ea15490312b85fed06e39274c4f73ac91c7f4d1b899729691cce616fb1a5feee1972456addcb51ac830e947fcc1b823468f0eefbaf195ac3b34f0baf96afc6fa77ee2e176081d6d91ce8c93c3d0f3547e48d059c9da447ba05ee3984703bebfd6d704b7f327ffaea7d0f63d0d3c6d65").unwrap();
        let encrypted_data = aes256_encrypt(key.as_slice(), iv.as_slice(), data.as_slice()).unwrap();
        assert_eq!(encrypted_data.len(), 528);
        let decrypted_data = aes256_decrypt(key.as_slice(), iv.as_slice(), encrypted_data.as_slice()).unwrap();
        assert_eq!(data, decrypted_data);
    }
}
