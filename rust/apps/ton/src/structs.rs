use crate::errors::{Result, TonError};
use crate::messages::traits::ParseCell;
use crate::messages::SigningMessage;
use crate::vendor::cell::{BagOfCells, Cell};
use alloc::string::ToString;
use alloc::vec::Vec;
use serde::Serialize;
use third_party::serde_json::{self, Value};

#[derive(Debug, Clone, Serialize)]
pub struct TonTransaction {
    pub signing_message: SigningMessage,
    pub buffer_to_sign: Vec<u8>,
}

impl TonTransaction {
    pub fn parse(boc: BagOfCells) -> Result<Self> {
        let root = boc.single_root()?;
        let buffer_to_sign = root.cell_hash()?;
        let signing_message = SigningMessage::parse(&root)?;
        Ok(Self {
            signing_message,
            buffer_to_sign,
        })
    }

    pub fn parse_hex(serial: &[u8]) -> Result<Self> {
        let boc = BagOfCells::parse(serial)?;
        Self::parse(boc)
    }

    pub fn to_json(&self) -> Result<Value> {
        serde_json::to_value(self).map_err(|e| TonError::TransactionJsonError(e.to_string()))
    }
}
