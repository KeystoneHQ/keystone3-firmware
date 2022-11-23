#![no_std]
#![feature(error_in_core)]

extern crate alloc;
extern crate core;

#[cfg(test)]
#[macro_use]
extern crate std;

use crate::parser::Parser;
use alloc::string::String;
use alloc::vec::Vec;

mod aptos_type;
pub mod errors;
mod parser;

pub fn parse(data: &Vec<u8>) -> crate::errors::Result<String> {
    let tx = Parser::parse(data)?;
    tx.get_result()
}

#[cfg(test)]
mod tests {
    extern crate std;

    use super::*;

    use std::println;
    use third_party::hex::FromHex;

    #[test]
    fn test() {
        let json = r#"{"formatted_json":{"chain_id":33,"expiration_timestamp_secs":1665644470,"gas_unit_price":100,"max_gas_amount":3738,"payload":{"EntryFunction":{"args":[[131,79,75,117,220,170,203,215,197,73,169,147,205,211,20,6,118,225,114,209,254,224,96,155,246,135,108,116,170,167,17,96],[64,13,3,0,0,0,0,0]],"function":"transfer","module":{"address":"0000000000000000000000000000000000000000000000000000000000000001","name":"coin"},"ty_args":[{"struct":{"address":"0000000000000000000000000000000000000000000000000000000000000001","module":"aptos_coin","name":"AptosCoin","type_args":[]}}]}},"sender":"8bbbb70ae8b90a8686b2a27f10e21e44f2fb64ffffcaa4bb0242e9f1ea698659","sequence_number":1},"raw_json":{"chain_id":33,"expiration_timestamp_secs":1665644470,"gas_unit_price":100,"max_gas_amount":3738,"payload":{"EntryFunction":{"args":[[131,79,75,117,220,170,203,215,197,73,169,147,205,211,20,6,118,225,114,209,254,224,96,155,246,135,108,116,170,167,17,96],[64,13,3,0,0,0,0,0]],"function":"transfer","module":{"address":"0000000000000000000000000000000000000000000000000000000000000001","name":"coin"},"ty_args":[{"struct":{"address":"0000000000000000000000000000000000000000000000000000000000000001","module":"aptos_coin","name":"AptosCoin","type_args":[]}}]}},"sender":"8bbbb70ae8b90a8686b2a27f10e21e44f2fb64ffffcaa4bb0242e9f1ea698659","sequence_number":1}}"#;
        let data = "8bbbb70ae8b90a8686b2a27f10e21e44f2fb64ffffcaa4bb0242e9f1ea698659010000000000000002000000000000000000000000000000000000000000000000000000000000000104636f696e087472616e73666572010700000000000000000000000000000000000000000000000000000000000000010a6170746f735f636f696e094170746f73436f696e000220834f4b75dcaacbd7c549a993cdd3140676e172d1fee0609bf6876c74aaa7116008400d0300000000009a0e0000000000006400000000000000b6b747630000000021";
        let buf_message = Vec::from_hex(data).unwrap();
        let parse_result = parse(&buf_message).unwrap();
        println!("json is {}", parse_result);
        assert_eq!(json, parse_result);
    }
}
