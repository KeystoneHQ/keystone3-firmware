use crate::errors::{Result, TonError};
use crate::messages::traits::ParseCell;
use crate::messages::{jetton, Comment, Operation, SigningMessage};
use crate::vendor::cell::BagOfCells;
use alloc::string::{String, ToString};
use serde::Serialize;
use third_party::hex;
use third_party::serde_json::{self, Value};

#[derive(Debug, Clone, Serialize, Default)]
pub struct TonTransaction {
    pub to: String,
    pub amount: String,
    pub action: String,
    pub comment: Option<String>,
    pub data_view: Option<String>,
    pub raw_data: String,
    
}

impl TonTransaction {
    pub fn parse(boc: BagOfCells) -> Result<Self> {
        let root = boc.single_root()?;
        let signing_message = SigningMessage::parse(&root)?;
        Self::try_from(&signing_message)
    }

    pub fn parse_hex(serial: &[u8]) -> Result<Self> {
        let boc = BagOfCells::parse(serial)?;
        Self::parse(boc).map(|mut v| {
            v.raw_data = hex::encode(serial);
            v
        })
    }

    pub fn to_json(&self) -> Result<Value> {
        serde_json::to_value(self).map_err(|e| TonError::TransactionJsonError(e.to_string()))
    }
}

impl TryFrom<&SigningMessage> for TonTransaction {
    type Error = TonError;

    fn try_from(signing_message: &SigningMessage) -> Result<Self> {
        if let None = signing_message.messages.get(0) {
            return Err(TonError::InvalidTransaction(
                "transaction does not contain transfer info".to_string(),
            ));
        };
        let message = signing_message.messages[0].clone();
        let to = message.dest_addr.clone();
        let amount = message.value.clone();
        match message.data {
            None => Ok(Self {
                to,
                amount,
                action: "Ton Transfer".to_string(),
                ..Default::default()
            }),
            Some(data) => {
                let action = data.action.clone().unwrap_or("Ton Transfer".to_string());
                match data.operation {
                    Operation::Comment(comment) => Ok(Self {
                        to,
                        amount,
                        action,
                        comment: Some(comment),
                        ..Default::default()
                    }),
                    Operation::JettonMessage(jetton_message) => Ok(Self {
                        to,
                        amount,
                        action,
                        data_view: Some(
                            serde_json::to_value(jetton_message)
                                .map_err(|e| TonError::TransactionJsonError(e.to_string()))?
                                .to_string(),
                        ),
                        ..Default::default()
                    }),
                    Operation::NFTMessage(nft_message) => Ok(Self {
                        to,
                        amount,
                        action,
                        data_view: Some(
                            serde_json::to_value(nft_message)
                                .map_err(|e| TonError::TransactionJsonError(e.to_string()))?
                                .to_string(),
                        ),
                        ..Default::default()
                    }),
                    Operation::OtherMessage(other_message) => Ok(Self {
                        to,
                        amount,
                        action,
                        ..Default::default()
                    }),
                }
            }
        }
    }
}
