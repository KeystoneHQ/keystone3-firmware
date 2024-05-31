use crate::errors::{Result, TonError};
use crate::jettons;
use crate::messages::jetton::JettonMessage;
use crate::messages::nft::NFTMessage;
use crate::messages::traits::ParseCell;
use crate::messages::{jetton, Comment, Operation, SigningMessage};
use crate::vendor::address::TonAddress;
use crate::vendor::cell::BagOfCells;
use alloc::string::{String, ToString};
use alloc::{format, vec};
use serde::Serialize;
use third_party::hex;
use third_party::serde_json::{self, json, Value};

#[derive(Debug, Clone, Serialize, Default)]
pub struct TonTransaction {
    pub to: String,
    pub amount: String,
    pub action: String,
    pub comment: Option<String>,
    pub data_view: Option<String>,
    pub raw_data: String,
    pub contract_data: Option<String>,
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
                    Operation::JettonMessage(jetton_message) => match jetton_message {
                        JettonMessage::JettonTransferMessage(jetton_transfer_message) => {
                            let destination = jetton_transfer_message.destination.clone();
                            let to = message.dest_addr_legacy.clone();
                            let amount = jettons::get_jetton_amount_text(
                                jetton_transfer_message.amount.clone(),
                                to.clone(),
                            );
                            Ok(Self {
                                to: destination,
                                amount,
                                action,
                                data_view: Some(
                                    serde_json::to_value(&jetton_transfer_message)
                                        .map_err(|e| TonError::TransactionJsonError(e.to_string()))?
                                        .to_string(),
                                ),
                                contract_data: Some(
                                    json!([
                                        {"title": "Contract Address", "value": to.clone()}
                                    ])
                                    .to_string(),
                                ),
                                ..Default::default()
                            })
                        }
                        _ => Err(TonError::InvalidTransaction(
                            "invalid jetton message".to_string(),
                        )),
                    },
                    Operation::NFTMessage(nft_message) => match nft_message {
                        NFTMessage::NFTTransferMessage(nft_transfer_message) => {
                            let destination = nft_transfer_message.new_owner_address.clone();
                            Ok(Self {
                                to: destination,
                                amount,
                                action,
                                data_view: Some(
                                    serde_json::to_value(nft_transfer_message)
                                        .map_err(|e| TonError::TransactionJsonError(e.to_string()))?
                                        .to_string(),
                                ),
                                contract_data: Some(
                                    json!({
                                        "nft_contract_address": to.clone()
                                    })
                                    .to_string(),
                                ),
                                ..Default::default()
                            })
                        }
                        _ => Err(TonError::InvalidTransaction(
                            "invalid nft message".to_string(),
                        )),
                    },
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

#[derive(Debug, Clone, Serialize, Default)]
pub struct TonProof {
    pub domain: String,
    pub payload: String,
    pub address: String,
    pub raw_message: String,
}

impl TonProof {
    pub fn parse_hex(serial: &[u8]) -> Result<Self> {
        let header_len = "ton-proof-item-v2/".len();
        //header + workchain + address_hash_part + domainLen + timestamp;
        let min_len = header_len + 4 + 32 + 4 + 8;
        let serial_len = serial.len();
        if serial_len < min_len {
            return Err(TonError::InvalidProof("proof is too short".to_string()));
        }
        let remaining = &serial[header_len..];

        //parse address
        let mut workchain_bytes = [0u8; 4];
        for i in 0..4 {
            workchain_bytes[i] = remaining[i];
        }
        let workchain = i32::from_be_bytes(workchain_bytes);
        let mut address_hash_parts = [0u8; 32];
        for i in 0..32 {
            address_hash_parts[i] = remaining[i + 4];
        }
        let address =
            TonAddress::new(workchain, &address_hash_parts).to_base64_url_flags(true, false);

        //parse domain
        let mut domain_len_bytes = [0u8; 4];
        for i in 0..4 {
            domain_len_bytes[i] = remaining[i + 36];
        }
        let domain_len = i32::from_le_bytes(domain_len_bytes);

        if serial_len < 40 + domain_len as usize {
            return Err(TonError::InvalidProof("proof is too short".to_string()));
        }

        let domain_bytes = &remaining[40..(40 + domain_len as usize)];
        let domain = String::from_utf8(domain_bytes.to_vec())
            .map_err(|e| TonError::InvalidProof(e.to_string()))?;

        let mut index = 40 + domain_len;

        let mut timestamp_bytes = [0u8; 8];
        for i in 0..8 {
            timestamp_bytes[i] = remaining[i + 40 + domain_len as usize];
        }
        index = index + 8;
        // need to transform to date time to display
        let _timestamp = i64::from_le_bytes(timestamp_bytes);

        let payload_bytes = &remaining[index as usize..];

        let payload = String::from_utf8(payload_bytes.to_vec())
            .map_err(|e| TonError::InvalidProof(e.to_string()))?;

        Ok(Self {
            domain,
            payload,
            address,
            raw_message: hex::encode(serial),
        })
    }
}
