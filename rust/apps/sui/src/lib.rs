#![no_std]
#![feature(error_in_core)]

#[allow(unused_imports)]
#[macro_use]
extern crate alloc;
extern crate core;
#[cfg(test)]
#[macro_use]
extern crate std;

use alloc::{
    string::{String, ToString},
    vec::Vec,
};
use core::str::FromStr;

use serde_derive::{Deserialize, Serialize};
use sui_types::{message::PersonalMessage, transaction::TransactionData};

use errors::SuiError;
use types::{intent::IntentMessage, msg::PersonalMessageUtf8};
use {bcs, hex};
use {
    blake2::{
        digest::{Update, VariableOutput},
        Blake2bVar,
    },
    serde_json,
};

use crate::{errors::Result, types::intent::IntentScope};

pub mod errors;
pub mod types;

#[derive(Debug, Serialize, Deserialize)]
pub enum Intent {
    TransactionData(IntentMessage<TransactionData>),
    PersonalMessage(IntentMessage<PersonalMessageUtf8>),
}

pub fn generate_address(pub_key: &str) -> Result<String> {
    let mut hasher = Blake2bVar::new(32).unwrap();
    let mut buf: Vec<u8> = hex::decode(pub_key)?;
    // insert flag, ed25519 is 0, secp256k1 is 1, secp256r1 is 2, multi sign is 3.
    buf.insert(0, 0);
    hasher.update(&buf);
    let mut addr = [0u8; 32];
    hasher.finalize_variable(&mut addr).unwrap();
    Ok(format!("0x{}", hex::encode(addr)))
}

pub fn parse_intent(intent: &[u8]) -> Result<Intent> {
    match IntentScope::try_from(intent[0])? {
        IntentScope::TransactionData => {
            let tx: IntentMessage<TransactionData> =
                bcs::from_bytes(&intent).map_err(|err| SuiError::from(err))?;
            Ok(Intent::TransactionData(tx))
        }
        IntentScope::PersonalMessage => {
            let msg: IntentMessage<PersonalMessage> = match bcs::from_bytes(&intent) {
                Ok(msg) => msg,
                Err(_) => {
                    if intent.len() < 4 {
                        return Err(SuiError::InvalidData(String::from("message too short")));
                    }
                    let intent_bytes = intent[..3].to_vec();
                    IntentMessage::<PersonalMessage>::new(
                        types::intent::Intent::from_str(hex::encode(&intent_bytes).as_str())?,
                        PersonalMessage {
                            message: intent[3..].to_vec(),
                        },
                    )
                }
            };
            let m = match decode_utf8(&msg.value.message) {
                Ok(m) => m,
                Err(_) => serde_json::to_string(&msg.value.message)?,
            };
            Ok(Intent::PersonalMessage(IntentMessage::<
                PersonalMessageUtf8,
            > {
                intent: msg.intent,
                value: PersonalMessageUtf8 { message: m },
            }))
        }
        _ => {
            return Err(SuiError::InvalidData(String::from("unsupported intent")));
        }
    }
}

pub fn decode_utf8(msg: &[u8]) -> Result<String> {
    match String::from_utf8(msg.to_vec()) {
        Ok(utf8_msg) => {
            if app_utils::is_cjk(&utf8_msg) {
                Err(errors::SuiError::InvalidData(String::from("contains CJK")))
            } else {
                Ok(utf8_msg)
            }
        }
        Err(e) => Err(errors::SuiError::InvalidData(e.to_string())),
    }
}

pub fn sign_intent(seed: &[u8], path: &String, intent: &[u8]) -> Result<[u8; 64]> {
    let mut hasher = Blake2bVar::new(32).unwrap();
    hasher.update(intent);
    let mut hash = [0u8; 32];
    hasher.finalize_variable(&mut hash).unwrap();
    let sig =
        keystore::algorithms::ed25519::slip10_ed25519::sign_message_by_seed(seed, path, &hash)
            .map_err(|e| errors::SuiError::SignFailure(e.to_string()))?;
    Ok(sig)
}

pub fn sign_hash(seed: &[u8], path: &String, hash: &[u8]) -> Result<[u8; 64]> {
    keystore::algorithms::ed25519::slip10_ed25519::sign_message_by_seed(seed, path, hash)
        .map_err(|e| errors::SuiError::SignFailure(e.to_string()))
}

#[cfg(test)]
mod tests {
    use alloc::string::ToString;

    use serde_json::json;

    use super::*;

    #[test]
    fn test_generate_address() {
        let pub_key = "edbe1b9b3b040ff88fbfa4ccda6f5f8d404ae7ffe35f9b220dec08679d5c336f";
        let address = generate_address(pub_key).unwrap();
        assert_eq!(
            "0x504886c9ec43bff70af37f55865094cc3a799cb54479f252d30cd3717f15ecdc",
            address
        );
    }

    #[test]
    fn test_parse_tx_transfer_objects() {
        let bytes = hex::decode("000000000002002086ac6179ca6ad9a7b1ccb47202d06ae09a131e66309944922af9c73d3c203b660100d833a8eabc697a0b2e23740aca7be9b0b9e1560a39d2f390cf2534e94429f91ced0c00000000000020190ca0d64215ac63f50dbffa47563404182304e0c10ea30b5e4d671b7173a34c0101010101000100000e4d9313fb5b3f166bb6f2aea587edbe21fb1c094472ccd002f34b9d0633c71901280f4809b93ed87cc06f3397cd42a800a1034316e80d05443bce08e810817a96f50c0000000000002051c8eb5d437fb66c8d296e1cdf446c91be29fbc89f8430a2407acb0179a503880e4d9313fb5b3f166bb6f2aea587edbe21fb1c094472ccd002f34b9d0633c719e803000000000000d00700000000000000").unwrap();

        let intent = parse_intent(&bytes);
        assert_eq!(json!(intent.unwrap()).to_string(), "{\"TransactionData\":{\"intent\":{\"app_id\":\"Sui\",\"scope\":\"TransactionData\",\"version\":\"V0\"},\"value\":{\"V1\":{\"expiration\":\"None\",\"gas_data\":{\"budget\":2000,\"owner\":\"0x0e4d9313fb5b3f166bb6f2aea587edbe21fb1c094472ccd002f34b9d0633c719\",\"payment\":[[\"0x280f4809b93ed87cc06f3397cd42a800a1034316e80d05443bce08e810817a96\",3317,\"6WFidAcGkUzUEPvSChbMXg9AZUHj3gzJL4U7HkxJA43H\"]],\"price\":1000},\"kind\":{\"ProgrammableTransaction\":{\"commands\":[{\"TransferObjects\":[[{\"Input\":1}],{\"Input\":0}]}],\"inputs\":[{\"Pure\":[134,172,97,121,202,106,217,167,177,204,180,114,2,208,106,224,154,19,30,102,48,153,68,146,42,249,199,61,60,32,59,102]},{\"Object\":{\"ImmOrOwnedObject\":[\"0xd833a8eabc697a0b2e23740aca7be9b0b9e1560a39d2f390cf2534e94429f91c\",3309,\"2gnMwEZqfMY1Q2Ree5iW3cAt7rhauevfBDY74SH3Ef1D\"]}}]}},\"sender\":\"0x0e4d9313fb5b3f166bb6f2aea587edbe21fb1c094472ccd002f34b9d0633c719\"}}}}");
    }

    #[test]
    fn test_parse_tx_move_call() {
        let bytes = hex::decode("0000000000020100d833a8eabc697a0b2e23740aca7be9b0b9e1560a39d2f390cf2534e94429f91ced0c00000000000020190ca0d64215ac63f50dbffa47563404182304e0c10ea30b5e4d671b7173a34c00090140420f000000000001000000000000000000000000000000000000000000000000000000000000000002037061790973706c69745f76656301070000000000000000000000000000000000000000000000000000000000000002037375690353554900020100000101000e4d9313fb5b3f166bb6f2aea587edbe21fb1c094472ccd002f34b9d0633c71901280f4809b93ed87cc06f3397cd42a800a1034316e80d05443bce08e810817a96f50c0000000000002051c8eb5d437fb66c8d296e1cdf446c91be29fbc89f8430a2407acb0179a503880e4d9313fb5b3f166bb6f2aea587edbe21fb1c094472ccd002f34b9d0633c719e803000000000000e80300000000000000").unwrap();

        let intent = parse_intent(&bytes);
        assert_eq!(json!(intent.unwrap()).to_string(), "{\"TransactionData\":{\"intent\":{\"app_id\":\"Sui\",\"scope\":\"TransactionData\",\"version\":\"V0\"},\"value\":{\"V1\":{\"expiration\":\"None\",\"gas_data\":{\"budget\":1000,\"owner\":\"0x0e4d9313fb5b3f166bb6f2aea587edbe21fb1c094472ccd002f34b9d0633c719\",\"payment\":[[\"0x280f4809b93ed87cc06f3397cd42a800a1034316e80d05443bce08e810817a96\",3317,\"6WFidAcGkUzUEPvSChbMXg9AZUHj3gzJL4U7HkxJA43H\"]],\"price\":1000},\"kind\":{\"ProgrammableTransaction\":{\"commands\":[{\"MoveCall\":{\"arguments\":[{\"Input\":0},{\"Input\":1}],\"function\":\"split_vec\",\"module\":\"pay\",\"package\":\"0x0000000000000000000000000000000000000000000000000000000000000002\",\"type_arguments\":[{\"struct\":{\"address\":\"0000000000000000000000000000000000000000000000000000000000000002\",\"module\":\"sui\",\"name\":\"SUI\",\"type_args\":[]}}]}}],\"inputs\":[{\"Object\":{\"ImmOrOwnedObject\":[\"0xd833a8eabc697a0b2e23740aca7be9b0b9e1560a39d2f390cf2534e94429f91c\",3309,\"2gnMwEZqfMY1Q2Ree5iW3cAt7rhauevfBDY74SH3Ef1D\"]}},{\"Pure\":[1,64,66,15,0,0,0,0,0]}]}},\"sender\":\"0x0e4d9313fb5b3f166bb6f2aea587edbe21fb1c094472ccd002f34b9d0633c719\"}}}}");
    }

    #[test]
    fn test_parse_tx_split_coins() {
        let bytes = hex::decode("00000000000200201ff915a5e9e32fdbe0135535b6c69a00a9809aaf7f7c0275d3239ca79db20d6400081027000000000000020200010101000101020000010000ebe623e33b7307f1350f8934beb3fb16baef0fc1b3f1b92868eec3944093886901a2e3e42930675d9571a467eb5d4b22553c93ccb84e9097972e02c490b4e7a22ab73200000000000020176c4727433105da34209f04ac3f22e192a2573d7948cb2fabde7d13a7f4f149ebe623e33b7307f1350f8934beb3fb16baef0fc1b3f1b92868eec39440938869e803000000000000640000000000000000").unwrap();

        let intent = parse_intent(&bytes);
        assert_eq!(json!(intent.unwrap()).to_string(), "{\"TransactionData\":{\"intent\":{\"app_id\":\"Sui\",\"scope\":\"TransactionData\",\"version\":\"V0\"},\"value\":{\"V1\":{\"expiration\":\"None\",\"gas_data\":{\"budget\":100,\"owner\":\"0xebe623e33b7307f1350f8934beb3fb16baef0fc1b3f1b92868eec39440938869\",\"payment\":[[\"0xa2e3e42930675d9571a467eb5d4b22553c93ccb84e9097972e02c490b4e7a22a\",12983,\"2aS93HVFS54TNKfAFunntFgoRMbMCzp1bDfqSTRPRYpg\"]],\"price\":1000},\"kind\":{\"ProgrammableTransaction\":{\"commands\":[{\"SplitCoins\":[\"GasCoin\",[{\"Input\":1}]]},{\"TransferObjects\":[[{\"Result\":0}],{\"Input\":0}]}],\"inputs\":[{\"Pure\":[31,249,21,165,233,227,47,219,224,19,85,53,182,198,154,0,169,128,154,175,127,124,2,117,211,35,156,167,157,178,13,100]},{\"Pure\":[16,39,0,0,0,0,0,0]}]}},\"sender\":\"0xebe623e33b7307f1350f8934beb3fb16baef0fc1b3f1b92868eec39440938869\"}}}}");
    }

    #[test]
    fn test_parse_msg() {
        let bytes = hex::decode("0300000a48656c6c6f2c20537569").unwrap();

        let intent = parse_intent(&bytes);
        assert_eq!(json!(intent.unwrap()).to_string(), "{\"PersonalMessage\":{\"intent\":{\"app_id\":\"Sui\",\"scope\":\"PersonalMessage\",\"version\":\"V0\"},\"value\":{\"message\":\"Hello, Sui\"}}}");
    }

    #[test]
    fn test_parse_msg_not_bcs() {
        let bytes = hex::decode("030000506C656173652C20766572696679206F776E657273686970206279207369676E696E672074686973206D6573736167652E").unwrap();

        let intent = parse_intent(&bytes);
        assert_eq!(json!(intent.unwrap()).to_string(), "{\"PersonalMessage\":{\"intent\":{\"app_id\":\"Sui\",\"scope\":\"PersonalMessage\",\"version\":\"V0\"},\"value\":{\"message\":\"Please, verify ownership by signing this message.\"}}}");
    }

    #[test]
    fn test_decode_utf8() {
        let bytes = hex::decode("48656c6c6f2c20537569").unwrap();

        let msg_str = decode_utf8(&bytes);
        assert_eq!(msg_str.unwrap(), "Hello, Sui");
    }

    #[test]
    fn test_sign() {
        let seed = hex::decode("a33b2bdc2dc3c53d24081e5ed2273e6e8e0e43f8b26c746fbd1db2b8f1d4d8faa033545d3ec9303d36e743a4574b80b124353d380535532bb69455dc0ee442c4").unwrap();
        let hd_path = "m/44'/784'/0'/0'/0'".to_string();
        let msg =  hex::decode("00000000000200201ff915a5e9e32fdbe0135535b6c69a00a9809aaf7f7c0275d3239ca79db20d6400081027000000000000020200010101000101020000010000ebe623e33b7307f1350f8934beb3fb16baef0fc1b3f1b92868eec3944093886901a2e3e42930675d9571a467eb5d4b22553c93ccb84e9097972e02c490b4e7a22ab73200000000000020176c4727433105da34209f04ac3f22e192a2573d7948cb2fabde7d13a7f4f149ebe623e33b7307f1350f8934beb3fb16baef0fc1b3f1b92868eec39440938869e803000000000000640000000000000000").unwrap();
        let signature = sign_intent(&seed, &hd_path, &msg).unwrap();
        let expected_signature = hex::decode("f4b79835417490958c72492723409289b444f3af18274ba484a9eeaca9e760520e453776e5975df058b537476932a45239685f694fc6362fe5af6ba714da6505").unwrap();
        assert_eq!(expected_signature, signature);
    }
}
