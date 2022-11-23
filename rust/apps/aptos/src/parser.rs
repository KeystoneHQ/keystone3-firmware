use crate::aptos_type::RawTransaction;
use crate::errors::{AptosError, Result};
use alloc::format;
use alloc::string::{String, ToString};
use alloc::vec::Vec;

use bcs;
use serde_json::{json, Value};

pub struct Parser;

impl Parser {
    pub fn parse(data: &Vec<u8>) -> Result<AptosTx> {
        let tx: RawTransaction = bcs::from_bytes(data).map_err(|err| {
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

    fn get_raw_json(&self) -> Result<Value> {
        self.to_json_value()
    }

    fn get_formatted_json(&self) -> Result<Value> {
        self.to_json_value()
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
