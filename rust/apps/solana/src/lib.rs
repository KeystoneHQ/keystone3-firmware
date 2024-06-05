#![no_std]
#![feature(error_in_core)]
#![allow(dead_code)] // add for solana use a lot of external code

extern crate alloc;
extern crate core;

#[cfg(test)]
#[macro_use]
extern crate std;

use crate::read::Read;
use alloc::format;
use alloc::string::{String, ToString};
use alloc::vec::Vec;

mod address;
use crate::parser::structs::ParsedSolanaTx;
use crate::structs::SolanaMessage;
pub use address::get_address;

mod compact;
pub mod errors;
mod instruction;
pub mod message;
pub mod parser;
pub mod read;
mod resolvers;
mod solana_lib;
pub mod structs;

pub fn parse_message(tx_hex: Vec<u8>, from_key: &String) -> errors::Result<SolanaMessage> {
    let raw_message = hex::encode(tx_hex.clone());
    let mut utf8_message =
        String::from_utf8(tx_hex).map_or_else(|_| "".to_string(), |utf8_msg| utf8_msg);
    if app_utils::is_cjk(&utf8_message) {
        utf8_message = "".to_string();
    }
    SolanaMessage::from(raw_message, utf8_message, from_key)
}

pub fn validate_tx(message: &mut Vec<u8>) -> bool {
    message::Message::validate(message)
}

pub fn parse(data: &Vec<u8>) -> errors::Result<ParsedSolanaTx> {
    ParsedSolanaTx::build(data)
}

pub fn sign(message: Vec<u8>, hd_path: &String, seed: &[u8]) -> errors::Result<[u8; 64]> {
    keystore::algorithms::ed25519::slip10_ed25519::sign_message_by_seed(&seed, hd_path, &message)
        .map_err(|e| errors::SolanaError::KeystoreError(format!("sign failed {:?}", e.to_string())))
}

#[cfg(test)]
mod tests {
    use super::*;
    use third_party::hex::{FromHex, ToHex};
    use third_party::ur_registry::solana::sol_sign_request::SolSignRequest;

    #[test]
    fn test_solana_sign() {
        let hd_path = "m/44'/501'/0'".to_string();
        let tx_hex =  Vec::from_hex("010002041a93fffb26ce645adeae58f0f414c320bcec30ce12a66bd263a91ec9b3958ff46f345144d352e4190c2dec43e1d3e0296a49bdfc2594eed9d8a5902e22d0af8b00000000000000000000000000000000000000000000000000000000000000000306466fe5211732ffecadba72c39be7bc8ce5bbc5f7126b2c439b3a40000000f70a9d4448ef435c5beab6cbc4211e00ddb4b9ad84886385f8b7ccfb9d9e7ca40303000903d8d600000000000003000502400d0300020200010c020000008096980000000000").unwrap();
        let seed = hex::decode("5eb00bbddcf069084889a8ab9155568165f5c453ccb85e70811aaed6f6da5fc19a5ac40b389cd370d086206dec8aa6c43daea6690f20ad3d8d48b2d2ce9e38e4").unwrap();
        let signature = sign(tx_hex, &hd_path, seed.as_slice()).unwrap();
        assert_eq!("9625b26df39be0a392cd2f0db075a238fe7bd98d181b8705bcc6c1c64f652294c54760af911cca245769489c30c12e44cf5e139ca71f1acc834eea4b63017b00", signature.encode_hex::<String>());
    }

    #[test]
    fn test_solana_validate() {
        let mut tx_hex =  Vec::from_hex("010002041a93fffb26ce645adeae58f0f414c320bcec30ce12a66bd263a91ec9b3958ff46f345144d352e4190c2dec43e1d3e0296a49bdfc2594eed9d8a5902e22d0af8b00000000000000000000000000000000000000000000000000000000000000000306466fe5211732ffecadba72c39be7bc8ce5bbc5f7126b2c439b3a40000000f70a9d4448ef435c5beab6cbc4211e00ddb4b9ad84886385f8b7ccfb9d9e7ca40303000903d8d600000000000003000502400d0300020200010c020000008096980000000000").unwrap();
        assert_eq!(true, validate_tx(&mut tx_hex));
    }

    #[test]
    fn test_solana_parse_message() {
        let cbor_hex = "a401d82550316ddac29df74514b9d8f53ebd847a750259021d6d616769636564656e2e696f2077616e747320796f7520746f207369676e20696e207769746820796f757220536f6c616e61206163636f756e743a0a47575a567a6353324d58664671486d50373832517833527a6b6b5158324b666763685a504c703341455a726d0a0a436c69636b205369676e206f7220417070726f7665206f6e6c79206d65616e7320796f7520686176652070726f76656420746869732077616c6c6574206973206f776e656420627920796f752e205468697320726571756573742077696c6c206e6f74207472696767657220616e7920626c6f636b636861696e207472616e73616374696f6e206f7220636f737420616e7920676173206665652e20557365206f66206f7572207765627369746520616e64207365727669636520617265207375626a65637420746f206f7572205465726d73206f6620536572766963653a2068747470733a2f2f6d616769636564656e2e696f2f7465726d732d6f662d736572766963652e70646620616e64205072697661637920506f6c6963793a2068747470733a2f2f6d616769636564656e2e696f2f707269766163792d706f6c6963792e7064660a0a5552493a2068747470733a2f2f6d616769636564656e2e696f0a56657273696f6e3a20310a436861696e2049443a206d61696e6e65740a4e6f6e63653a2076706478336e476662390a4973737565642041743a20323032332d30372d32375430323a31303a34392e3136315a03d90130a20188182cf51901f5f500f500f5021a527447030601";
        let pubkey = "e671e524ef43ccc5ef0006876f9a2fd66681d5abc5871136b343a3e4b073efde".to_string();
        let sol_sign_request = SolSignRequest::try_from(hex::decode(cbor_hex).unwrap()).unwrap();
        let parsed = parse_message(sol_sign_request.get_sign_data(), &pubkey).unwrap();
        assert_eq!("GWZVzcS2MXfFqHmP782Qx3RzkkQX2KfgchZPLp3AEZrm", parsed.from);
    }

    #[test]
    fn test_solana_version_message() {
        let mut buffer = hex::decode("8001000308ad7dba70a9559a85961a00a0d00e7caf6967ff28c5861360cd8e22abcbb9b3eb080a39da2d35d3c666ff3d5195bb9065ed97a2208b4245bc70e69ff4b1d2b73fff21b6871e314c96e0848ba15117ec8cef8433b5a594ff2b88798b7e6d37b84e0a0cc6b26d4216a826e48cbb719136037e76cc273efb63ec73a6f2cb56cfa6015f376f7759fe1a4270a9b568d86110869cb073c8580adc986a178c876c18ee3d0306466fe5211732ffecadba72c39be7bc8ce5bbc5f7126b2c439b3a4000000004b2acb11258cce3682c418ba872ff3df91102712f15af12b6be69b3435b0008eaa020c61cc479712813461ce153894a96a6c00b21ed0cfc2798d1f9a9e9c94a5b6bfd38a1d64b54c65490c10b1be53496be5ede8d18086335aade667915e9e90705000502400d03000500090310010a00000000000606080d0706060e0802da8aeb4fc9196606030d010808218493e497c04859060b0001090802030d0f101112098c90fd150a4af80300060c00010d09080a0b0c04061113104b5d5ddc2296dac4ffffffffffffffff060b0001090802030d0f101112098c90fd150a4af803000110aaf654f15c22d8c9aa80622becb4872b0f5a52cc8f467d8a0da58b19197d690540024544420701757403060704").unwrap();
        let pubkey = "e671e524ef43ccc5ef0006876f9a2fd66681d5abc5871136b343a3e4b073efde".to_string();
        let parsed = validate_tx(&mut buffer);
    }
}
