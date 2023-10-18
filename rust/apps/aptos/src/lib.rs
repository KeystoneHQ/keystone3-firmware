#![no_std]
#![feature(error_in_core)]

#[allow(unused_imports)]
#[macro_use]
extern crate alloc;
extern crate core;

#[cfg(test)]
#[macro_use]
extern crate std;

use crate::parser::Parser;
use alloc::format;
use alloc::string::{String, ToString};
use alloc::vec::Vec;
use parser::AptosTx;
use third_party::cryptoxide::hashing::sha3::Sha3_256;
use third_party::hex;

use crate::errors::Result;

mod aptos_type;
pub mod errors;
pub mod parser;

pub fn generate_address(pub_key: &str) -> Result<String> {
    let mut buf: Vec<u8> = hex::decode(pub_key)?;
    buf.push(0);
    let addr = Sha3_256::new().update(&buf).finalize();
    Ok(format!("0x{}", hex::encode(addr)))
}

pub fn parse_tx(data: &Vec<u8>) -> crate::errors::Result<AptosTx> {
    Ok(Parser::parse_tx(data)?)
}

pub fn parse_msg(data: &Vec<u8>) -> crate::errors::Result<String> {
    Ok(Parser::parse_msg(data)?)
}

pub fn sign(message: Vec<u8>, hd_path: &String, seed: &[u8]) -> errors::Result<[u8; 64]> {
    keystore::algorithms::ed25519::slip10_ed25519::sign_message_by_seed(&seed, hd_path, &message)
        .map_err(|e| errors::AptosError::KeystoreError(format!("sign failed {:?}", e.to_string())))
}

#[cfg(test)]
mod tests {
    extern crate std;

    use super::*;

    use third_party::hex::FromHex;
    use third_party::hex::ToHex;

    #[test]
    fn test() {
        let json_str = r#"{"formatted_json":"{\n  \"sender\": \"8bbbb70ae8b90a8686b2a27f10e21e44f2fb64ffffcaa4bb0242e9f1ea698659\",\n  \"sequence_number\": 1,\n  \"payload\": {\n    \"EntryFunction\": {\n      \"module\": {\n        \"address\": \"0000000000000000000000000000000000000000000000000000000000000001\",\n        \"name\": \"coin\"\n      },\n      \"function\": \"transfer\",\n      \"ty_args\": [\n        {\n          \"struct\": {\n            \"address\": \"0000000000000000000000000000000000000000000000000000000000000001\",\n            \"module\": \"aptos_coin\",\n            \"name\": \"AptosCoin\",\n            \"type_args\": []\n          }\n        }\n      ],\n      \"args\": [\n        [\n          131,\n          79,\n          75,\n          117,\n          220,\n          170,\n          203,\n          215,\n          197,\n          73,\n          169,\n          147,\n          205,\n          211,\n          20,\n          6,\n          118,\n          225,\n          114,\n          209,\n          254,\n          224,\n          96,\n          155,\n          246,\n          135,\n          108,\n          116,\n          170,\n          167,\n          17,\n          96\n        ],\n        [\n          64,\n          13,\n          3,\n          0,\n          0,\n          0,\n          0,\n          0\n        ]\n      ]\n    }\n  },\n  \"max_gas_amount\": 3738,\n  \"gas_unit_price\": 100,\n  \"expiration_timestamp_secs\": 1665644470,\n  \"chain_id\": 33\n}","raw_json":{"chain_id":33,"expiration_timestamp_secs":1665644470,"gas_unit_price":100,"max_gas_amount":3738,"payload":{"EntryFunction":{"args":[[131,79,75,117,220,170,203,215,197,73,169,147,205,211,20,6,118,225,114,209,254,224,96,155,246,135,108,116,170,167,17,96],[64,13,3,0,0,0,0,0]],"function":"transfer","module":{"address":"0000000000000000000000000000000000000000000000000000000000000001","name":"coin"},"ty_args":[{"struct":{"address":"0000000000000000000000000000000000000000000000000000000000000001","module":"aptos_coin","name":"AptosCoin","type_args":[]}}]}},"sender":"8bbbb70ae8b90a8686b2a27f10e21e44f2fb64ffffcaa4bb0242e9f1ea698659","sequence_number":1}}"#;
        let data = "8bbbb70ae8b90a8686b2a27f10e21e44f2fb64ffffcaa4bb0242e9f1ea698659010000000000000002000000000000000000000000000000000000000000000000000000000000000104636f696e087472616e73666572010700000000000000000000000000000000000000000000000000000000000000010a6170746f735f636f696e094170746f73436f696e000220834f4b75dcaacbd7c549a993cdd3140676e172d1fee0609bf6876c74aaa7116008400d0300000000009a0e0000000000006400000000000000b6b747630000000021";
        let buf_message = Vec::from_hex(data).unwrap();
        let parse_result = parse_tx(&buf_message).unwrap();
        assert_eq!(json_str, parse_result.get_result().unwrap());
    }

    #[test]
    fn test_aptos_sign() {
        let hd_path = "m/44'/637'/0'/0'/0'".to_string();
        let tx_hex =  Vec::from_hex("b5e97db07fa0bd0e5598aa3643a9bc6f6693bddc1a9fec9e674a461eaa00b193f007dbb60994463db95b80fad4259ec18767a5bb507f9e048da84b75ea793ef500000000000000000200000000000000000000000000000000000000000000000000000000000000010d6170746f735f6163636f756e740e7472616e736665725f636f696e73010700000000000000000000000000000000000000000000000000000000000000010a6170746f735f636f696e094170746f73436f696e000220f007dbb60994463db95b80fad4259ec18767a5bb507f9e048da84b75ea793ef50800000000000000000a00000000000000640000000000000061242e650000000002").unwrap();
        let seed = hex::decode("5eb00bbddcf069084889a8ab9155568165f5c453ccb85e70811aaed6f6da5fc19a5ac40b389cd370d086206dec8aa6c43daea6690f20ad3d8d48b2d2ce9e38e4").unwrap();
        let signature = sign(tx_hex, &hd_path, seed.as_slice()).unwrap();
        assert_eq!("ff2c5e05557c30d1cddd505b26836747eaf28f25b2816b1e702bd40236be674eaaef10e4bd940b85317bede537cad22365eb7afca7456b90dcc2807cbbdcaa0a", signature.encode_hex::<String>());
    }
}
