use crate::aptos_type::RawTransaction;
use crate::errors::{AptosError, Result};
use alloc::format;
use alloc::string::{String, ToString};
use alloc::vec::Vec;

use bcs;
use serde_json::{json, Value};

pub struct Parser;

impl Parser {
    pub fn parse_tx(data: &Vec<u8>) -> Result<AptosTx> {
        // prefix bytes is sha3_256("APTOS::RawTransaction")
        let tx_prefix = [
            0xb5, 0xe9, 0x7d, 0xb0, 0x7f, 0xa0, 0xbd, 0x0e, 0x55, 0x98, 0xaa, 0x36, 0x43, 0xa9,
            0xbc, 0x6f, 0x66, 0x93, 0xbd, 0xdc, 0x1a, 0x9f, 0xec, 0x9e, 0x67, 0x4a, 0x46, 0x1e,
            0xaa, 0x00, 0xb1, 0x93,
        ];
        let mut data_parse = data.clone();
        if data[..32] == tx_prefix {
            data_parse = data[32..].to_vec();
        }
        let tx: RawTransaction = bcs::from_bytes(&data_parse).map_err(|err| {
            AptosError::ParseTxError(format!("bcs deserialize failed {}", err.to_string()))
        })?;
        Ok(AptosTx::new(tx))
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
            Ok(v) => Ok(json!(v)),
            Err(e) => Err(AptosError::ParseTxError(format!(
                "to json failed {}",
                e.to_string()
            ))),
        }
    }

    fn to_json_value(&self) -> Result<Value> {
        let value = serde_json::to_value(&self.tx)
            .map_err(|e| AptosError::ParseTxError(format!("to json failed {}", e.to_string())))?;
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
