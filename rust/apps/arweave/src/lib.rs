#![no_std]
#![feature(error_in_core)]

mod errors;

#[macro_use]
extern crate alloc;

#[cfg(test)]
#[macro_use]
extern crate std;

use crate::errors::{ArweaveError, Result};
use alloc::string::{String, ToString};
use alloc::vec::Vec;
use keystore::algorithms::rsa::get_rsa_secret_from_seed;
use sha2;
use sha2::{Digest, Sha256};
use third_party::base64;
use third_party::rsa::{BigUint, RsaPrivateKey};

pub fn generate_address(owner: Vec<u8>) -> Result<String> {
    let mut hasher = Sha256::new();
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

// pub fn sign(message: &[u8], seed: &[u8], signing_option: &SigningOption) -> Result<Vec<u8>> {
//     keystore::algorithms::rsa::sign_message(message, seed, signing_option)
//         .map_err(|e| ArweaveError::KeystoreError(format!("sign failed {:?}", e.to_string())))
// }

// pub fn verify(signature: &[u8], message: &[u8]) -> Result<()> {
//     keystore::algorithms::rsa::verify(signature, message).map_err(|e| {
//         ArweaveError::KeystoreError(format!(
//             "signature verification failed e {:?}",
//             e.to_string()
//         ))
//     })
// }

#[cfg(test)]
mod tests {
    use super::*;
    use alloc::borrow::ToOwned;
    use third_party::{hex, rsa::PublicKeyParts};

    // #[test]
    // fn test_sign_verify_salt_zero() {
    //     let message = hex::decode("00f41cfa7bfad3d7b097fcc28ed08cb4ca7d0c544ec760cc6cc5c4f3780d0ec43cc011eaaab0868393c3c813ab8c04df").unwrap();
    //     let signing_option = SigningOption::PSS { salt_len: 0 };
    //     let seed = hex::decode("5eb00bbddcf069084889a8ab9155568165f5c453ccb85e70811aaed6f6da5fc19a5ac40b389cd370d086206dec8aa6c43daea6690f20ad3d8d48b2d2ce9e38e4").unwrap();
    //     let signature = sign(message.as_slice(), seed.as_slice(), &signing_option).unwrap();
    //     assert_eq!(hex::encode(signature.clone()), "a8e58c9aa9a74039f239f49adca18ea5d54b9d28852b7d39b098a96230ebe4b07bf1f66eea2ef3ee29ab912f90508917703ca9838f228b0f75014ea5d41101f7dff194d8086010aa92b6e6d04a56ed6cb7bd63c3dc15f833c0fcbeb03a16892ed715f7b178c20dbb6cd9923ddd0ab4b1c8753a554a8165ff34224fb630445582d3b588581deca41dbcf2144dcf10a362510178af9923e9f6cdf30dfaafa5642a20f777a4a9bff7170517d9a4347a2f0e360a38bf90a8b5d10f80f2581422798aa7b77d959f237a77d71b35558349e35f9c1193154bcf252d79171abeec6f37858584f878503af44a3553eb218b86dc31dfcca66dea947364580515bb2543d2403d53866ee16bba1b8e51ba060a5ecfef3ef4617d96fa3a3f67176621e638ad7e33bf08c56409f0ce01ef345ac4b49ba4fd94dbaf11b544f4ce089d9adcebf5b592afd2f8cecf22f21539975e50441fe3bf5f77d7d0fcfa2bd3c6e2cbf1bb59ed141b5c0f257be5958c5b46c9f08ec1e912b7fa6ff7182aa9010ce9f0cd6fc4845760a37f97197ea8ad3fa8a75b742e9ad61f877acd5771e7c43e0c75a422eb7d96153d4c561469c0f6011d0fe74f718b2db26894e3c5daf72784d34374c4dab78c3ff7619f883085a45efe1781cfcdb80b64b4c8aa96f86225144ca9430a499e96c607a77538ad7fb920fdd1126cdc8c5574ed3c2b1fb1dadac51ad4e13fdd9d");
    //     let result = verify(signature.as_slice(), message.as_slice());
    //     println!("verify result {:?}", result);
    //     assert_eq!(result.ok(), Some(()));
    // }

    // #[test]
    // fn test_sign_verify_salt_32() {
    //     let signing_option = SigningOption::PSS { salt_len: 32 };
    //     let message = hex::decode("00f41cfa7bfad3d7b097fcc28ed08cb4ca7d0c544ec760cc6cc5c4f3780d0ec43cc011eaaab0868393c3c813ab8c04df").unwrap();
    //     let seed = hex::decode("5eb00bbddcf069084889a8ab9155568165f5c453ccb85e70811aaed6f6da5fc19a5ac40b389cd370d086206dec8aa6c43daea6690f20ad3d8d48b2d2ce9e38e4").unwrap();
    //     let signature = sign(message.as_slice(), seed.as_slice(), &signing_option).unwrap();
    //     let result = verify(&signature.as_ref(), message.as_slice());
    //     assert_eq!(result.ok(), Some(()));
    // }

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
}
