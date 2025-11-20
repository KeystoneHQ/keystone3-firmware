#![no_std]

#[allow(unused_imports)] //stupid compiler
#[macro_use]
extern crate alloc;

#[cfg(test)]
#[macro_use]
extern crate std;

use crate::errors::{NearError, Result};
use alloc::string::{String, ToString};
use alloc::vec::Vec;

use cryptoxide::hashing::sha256;

mod account_id;
mod crypto;
pub mod errors;
pub mod parser;
mod primitives_core;

pub fn sign(message: &Vec<u8>, hd_path: &String, seed: &[u8]) -> Result<[u8; 64]> {
    let message = sha256(message);
    keystore::algorithms::ed25519::slip10_ed25519::sign_message_by_seed(seed, hd_path, &message)
        .map_err(|e| NearError::KeystoreError(format!("sign failed {:?}", e.to_string())))
}

pub fn parse(data: &Vec<u8>) -> Result<parser::structs::ParsedNearTx> {
    parser::structs::ParsedNearTx::build(data)
}

#[cfg(test)]
mod tests {
    use super::*;

    use hex::ToHex;
    use ur_registry::near::near_sign_request::NearSignRequest;
    #[test]
    fn test_near_sign() {
        let hd_path = "m/44'/397'/0'".to_string();
        let cbor_str="a301d82550ed2ab5bb9cc24571b7dcbec9e5bc7b79028159016040000000333138323466626632343335666231656361346466633339373734313833636232356631336231303335326435643533323736313662353963333565616539660031824fbf2435fb1eca4dfc39774183cb25f13b10352d5d5327616b59c35eae9f442d16f48e3f00003c000000613062383639393163363231386233366331643139643461326539656230636533363036656234382e666163746f72792e6272696467652e6e6561728db41252bff5ecc0b28f55fc078d1b2de23990f728fc762502688d340bb42c9301000000020b00000066745f7472616e73666572630000007b22616d6f756e74223a223232303030222c2272656365697665725f6964223a2230346361353938333132633631656464646165656531333035333938303366323931313464363534353537643330333066313561623533316335653730613264227d00e057eb481b00000100000000000000000000000000000003d90130a20186182cf519018df500f5021a707eed6c";
        let near_sign_request = NearSignRequest::try_from(hex::decode(cbor_str).unwrap()).unwrap();
        let seed = hex::decode("5eb00bbddcf069084889a8ab9155568165f5c453ccb85e70811aaed6f6da5fc19a5ac40b389cd370d086206dec8aa6c43daea6690f20ad3d8d48b2d2ce9e38e4").unwrap();
        let sign_data = near_sign_request.get_sign_data();
        let signature = sign(&sign_data[0], &hd_path, seed.as_slice()).unwrap();
        assert_eq!("e93a63ffb66009874463fabec3d0097e2480a0dccecfb8d5ce1da34c45df28f7af9eced2bc731baa2ffdad44dd3ca2c80cec9c7b69b72991ebf08a790c378a0e", signature.encode_hex::<String>());
    }
}
