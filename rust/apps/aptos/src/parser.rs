use crate::aptos_type::RawTransaction;
use crate::errors::{self, AptosError, Result};
use alloc::format;
use alloc::string::{String, ToString};

use bcs;
use hex;
use serde_json::{json, Value};

pub struct Parser;

pub fn decode_utf8(msg: &[u8]) -> Result<String> {
    match core::str::from_utf8(msg) {
        Ok(s) => {
            if app_utils::is_cjk(s) {
                Err(errors::AptosError::InvalidData(String::from(
                    "contains CJK",
                )))
            } else {
                Ok(s.to_string())
            }
        }
        Err(e) => Err(errors::AptosError::InvalidData(e.to_string())),
    }
}

pub fn is_tx(data: &[u8]) -> bool {
    // prefix bytes is sha3_256("APTOS::RawTransaction")
    const TX_PREFIX: [u8; 32] = [
        0xb5, 0xe9, 0x7d, 0xb0, 0x7f, 0xa0, 0xbd, 0x0e, 0x55, 0x98, 0xaa, 0x36, 0x43, 0xa9, 0xbc,
        0x6f, 0x66, 0x93, 0xbd, 0xdc, 0x1a, 0x9f, 0xec, 0x9e, 0x67, 0x4a, 0x46, 0x1e, 0xaa, 0x00,
        0xb1, 0x93,
    ];
    data.len() > 32 && data.starts_with(&TX_PREFIX)
}

impl Parser {
    pub fn parse_tx(data: &[u8]) -> Result<AptosTx> {
        let data_slice = if is_tx(data) { &data[32..] } else { data };
        let tx: RawTransaction = bcs::from_bytes(data_slice)
            .map_err(|err| AptosError::ParseTxError(format!("bcs deserialize failed {err}")))?;
        tx.validate_for_review()?;
        Ok(AptosTx::new(tx))
    }
    pub fn parse_msg(data: &[u8]) -> Result<String> {
        match decode_utf8(data) {
            Ok(v) => Ok(v),
            Err(_) => Ok(hex::encode(data)),
        }
    }
}

#[derive(Debug)]
pub struct AptosTx {
    tx: RawTransaction,
}

impl AptosTx {
    pub fn new(tx: RawTransaction) -> Self {
        AptosTx { tx }
    }

    pub fn get_raw_json(&self) -> Result<Value> {
        self.to_json_value()
    }

    pub fn get_formatted_json(&self) -> Result<Value> {
        match serde_json::to_string_pretty(&self.tx) {
            Ok(v) => Ok(Value::String(v)),
            Err(e) => Err(AptosError::ParseTxError(format!("to json failed {e}"))),
        }
    }

    fn to_json_value(&self) -> Result<Value> {
        let value = serde_json::to_value(&self.tx)
            .map_err(|e| AptosError::ParseTxError(format!("to json failed {e}")))?;
        Ok(value)
    }

    pub fn get_result(&self) -> Result<String> {
        let raw_json = self.get_raw_json()?;
        let formatted_json = self.get_formatted_json()?;
        let result = json!({
            "raw_json" : raw_json,
            "formatted_json": formatted_json
        });
        Ok(result.to_string())
    }
}

#[cfg(test)]
mod tests {
    extern crate std;
    use super::*;
    use hex;

    // Synthetic vectors generated with @aptos-labs/ts-sdk 7.5.0:
    // RawTransaction(0x11..11, 7, MultiSig(0x22..22, EntryFunction(
    //   0x1::example::do_something, [], [U32(30378)])), 2000, 100, 2000000000, 1).
    // No real wallet addresses, signatures, or keys are used.
    const MULTISIG_ENTRY: &str = "1111111111111111111111111111111111111111111111111111111111111111070000000000000003222222222222222222222222222222222222222222222222222222222222222201000000000000000000000000000000000000000000000000000000000000000001076578616d706c650c646f5f736f6d657468696e67000104aa760000d0070000000000006400000000000000009435770000000001";
    const MULTISIG_WITHOUT_PAYLOAD: &str = "1111111111111111111111111111111111111111111111111111111111111111070000000000000003222222222222222222222222222222222222222222222222222222222222222200d0070000000000006400000000000000009435770000000001";
    const ORDINARY_ENTRY: &str = "11111111111111111111111111111111111111111111111111111111111111110700000000000000020000000000000000000000000000000000000000000000000000000000000001076578616d706c650c646f5f736f6d657468696e67000104aa760000d0070000000000006400000000000000009435770000000001";

    #[test]
    fn test_multisig_entry_function_review_and_roundtrip() {
        let bytes = hex::decode(MULTISIG_ENTRY).unwrap();
        let tx = Parser::parse_tx(&bytes).unwrap();
        assert_eq!(bcs::to_bytes(&tx.tx).unwrap(), bytes);
        let value = tx.get_raw_json().unwrap();
        assert_eq!(value["sender"], "11".repeat(32));
        assert_eq!(
            value["payload"]["Multisig"]["multisig_address"],
            "22".repeat(32)
        );
        let entry = &value["payload"]["Multisig"]["transaction_payload"]["EntryFunction"];
        assert_eq!(entry["module"]["address"], format!("{}1", "0".repeat(63)));
        assert_eq!(entry["module"]["name"], "example");
        assert_eq!(entry["function"], "do_something");
        assert_eq!(entry["args"], json!([[170, 118, 0, 0]]));
        assert_eq!(value["max_gas_amount"], 2000);
        assert_eq!(value["gas_unit_price"], 100);
        assert_eq!(value["chain_id"], 1);
        let formatted = tx.get_formatted_json().unwrap();
        let displayed: Value = serde_json::from_str(formatted.as_str().unwrap()).unwrap();
        assert_eq!(displayed, value);
    }

    #[test]
    fn test_multisig_with_signing_prefix() {
        let mut bytes =
            hex::decode("b5e97db07fa0bd0e5598aa3643a9bc6f6693bddc1a9fec9e674a461eaa00b193")
                .unwrap();
        bytes.extend(hex::decode(MULTISIG_ENTRY).unwrap());
        assert!(Parser::parse_tx(&bytes).is_ok());
    }

    #[test]
    fn test_multisig_without_payload_rejected() {
        let err = Parser::parse_tx(&hex::decode(MULTISIG_WITHOUT_PAYLOAD).unwrap()).unwrap_err();
        assert!(err
            .to_string()
            .contains("multisig execution requires an inline entry function"));
    }

    #[test]
    fn test_multisig_script_rejected() {
        let mut bytes = hex::decode(MULTISIG_WITHOUT_PAYLOAD).unwrap();
        // After sender, sequence, outer variant, vault: Some(Script(empty)).
        bytes.splice(73..74, [1, 1, 0, 0, 0]);
        let err = Parser::parse_tx(&bytes).unwrap_err();
        assert!(err
            .to_string()
            .contains("multisig execution requires an inline entry function"));
    }

    #[test]
    fn test_multisig_malformed_rejected() {
        let bytes = hex::decode(MULTISIG_ENTRY).unwrap();
        for end in 0..bytes.len() {
            assert!(Parser::parse_tx(&bytes[..end]).is_err());
        }
        let mut trailing = bytes.clone();
        trailing.push(0);
        assert!(Parser::parse_tx(&trailing).is_err());
        let mut invalid_option = bytes.clone();
        invalid_option[73] = 2;
        assert!(Parser::parse_tx(&invalid_option).is_err());
        let mut invalid_inner = bytes.clone();
        invalid_inner[74] = 2;
        assert!(Parser::parse_tx(&invalid_inner).is_err());
        let mut invalid_outer = bytes;
        invalid_outer[40] = 4;
        assert!(Parser::parse_tx(&invalid_outer).is_err());
    }

    #[test]
    fn test_existing_entry_function_encoding_unchanged() {
        let bytes = hex::decode(ORDINARY_ENTRY).unwrap();
        let tx = Parser::parse_tx(&bytes).unwrap();
        assert_eq!(bcs::to_bytes(&tx.tx).unwrap(), bytes);
        assert_eq!(
            tx.get_raw_json().unwrap()["payload"]["EntryFunction"]["function"],
            "do_something"
        );
    }

    #[test]
    fn test_decode_utf8_valid_ascii() {
        let ascii = b"hello, aptos";
        let result = decode_utf8(ascii);
        assert!(result.is_ok());
        assert_eq!(result.unwrap(), "hello, aptos");
    }

    #[test]
    fn test_decode_utf8_valid_utf8() {
        let utf8 = "hello, мир".as_bytes();
        let result = decode_utf8(utf8);
        assert!(result.is_ok());
        assert_eq!(result.unwrap(), "hello, мир");
    }

    #[test]
    fn test_decode_utf8_cjk_rejected() {
        let cjk = "中文".as_bytes();
        let result = decode_utf8(cjk);
        assert!(result.is_err());
        assert!(matches!(result.unwrap_err(), AptosError::InvalidData(_)));
    }

    #[test]
    fn test_decode_utf8_japanese_rejected() {
        let japanese = "日本語".as_bytes();
        let result = decode_utf8(japanese);
        assert!(result.is_err());
    }

    #[test]
    fn test_decode_utf8_korean_rejected() {
        let korean = "한국어".as_bytes();
        let result = decode_utf8(korean);
        assert!(result.is_err());
    }

    #[test]
    fn test_decode_utf8_invalid_utf8() {
        let invalid = vec![0xff, 0xfe, 0xfd];
        let result = decode_utf8(&invalid);
        assert!(result.is_err());
    }

    #[test]
    fn test_decode_utf8_empty() {
        let empty = vec![];
        let result = decode_utf8(&empty);
        assert!(result.is_ok());
        assert_eq!(result.unwrap(), "");
    }

    #[test]
    fn test_is_tx_with_prefix() {
        let tx_prefix: [u8; 32] = [
            0xb5, 0xe9, 0x7d, 0xb0, 0x7f, 0xa0, 0xbd, 0x0e, 0x55, 0x98, 0xaa, 0x36, 0x43, 0xa9,
            0xbc, 0x6f, 0x66, 0x93, 0xbd, 0xdc, 0x1a, 0x9f, 0xec, 0x9e, 0x67, 0x4a, 0x46, 0x1e,
            0xaa, 0x00, 0xb1, 0x93,
        ];
        let mut data = tx_prefix.to_vec();
        data.push(0x00);
        assert!(is_tx(&data));
    }

    #[test]
    fn test_is_tx_without_prefix() {
        let data = vec![0x00; 33];
        assert!(!is_tx(&data));
    }

    #[test]
    fn test_is_tx_too_short() {
        let data = vec![0xb5, 0xe9, 0x7d, 0xb0];
        assert!(!is_tx(&data));
    }

    #[test]
    fn test_is_tx_exact_length() {
        let tx_prefix: [u8; 32] = [
            0xb5, 0xe9, 0x7d, 0xb0, 0x7f, 0xa0, 0xbd, 0x0e, 0x55, 0x98, 0xaa, 0x36, 0x43, 0xa9,
            0xbc, 0x6f, 0x66, 0x93, 0xbd, 0xdc, 0x1a, 0x9f, 0xec, 0x9e, 0x67, 0x4a, 0x46, 0x1e,
            0xaa, 0x00, 0xb1, 0x93,
        ];
        // Exactly 32 bytes should return false (needs > 32)
        assert!(!is_tx(&tx_prefix));
    }

    #[test]
    fn test_aptos_tx_new() {
        let tx_data = hex::decode("8bbbb70ae8b90a8686b2a27f10e21e44f2fb64ffffcaa4bb0242e9f1ea698659010000000000000002000000000000000000000000000000000000000000000000000000000000000104636f696e087472616e73666572010700000000000000000000000000000000000000000000000000000000000000010a6170746f735f636f696e094170746f73436f696e000220834f4b75dcaacbd7c549a993cdd3140676e172d1fee0609bf6876c74aaa7116008400d0300000000009a0e0000000000006400000000000000b6b747630000000021").unwrap();
        let tx: RawTransaction = bcs::from_bytes(&tx_data).unwrap();
        let aptos_tx = AptosTx::new(tx);
        let result = aptos_tx.get_result();
        assert!(result.is_ok());
        let json_str = result.unwrap();
        assert!(json_str.contains("raw_json"));
        assert!(json_str.contains("formatted_json"));
    }
}
