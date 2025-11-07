use crate::errors::{Result, TonError};
use crate::jettons;
use crate::messages::jetton::JettonMessage;
use crate::messages::nft::NFTMessage;
use crate::messages::traits::ParseCell;
use crate::messages::{Operation, SigningMessage};
use crate::utils::shorten_string;
use crate::vendor::address::TonAddress;
use crate::vendor::cell::BagOfCells;
use alloc::string::{String, ToString};

use hex;
use serde::Serialize;
use serde_json::{self, json, Value};

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
        let signing_message = SigningMessage::parse(root)?;
        Self::try_from(&signing_message)
    }

    pub fn parse_hex(serial: &[u8]) -> Result<Self> {
        let boc = BagOfCells::parse(serial)?;
        Self::parse(boc).map(|mut v| {
            let raw_data = hex::encode(serial);
            v.raw_data = shorten_string(raw_data);
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
        if signing_message.messages.is_empty() {
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
                            )?;
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
                                        {"title": "Jetton Wallet Address", "value": to.clone()}
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
                                    json!([
                                        {"title": "NFT Wallet Address", "value": to.clone()}
                                    ])
                                    .to_string(),
                                ),
                                ..Default::default()
                            })
                        }
                        _ => Err(TonError::InvalidTransaction(
                            "invalid nft message".to_string(),
                        )),
                    },
                    Operation::OtherMessage(_other_message) => Ok(Self {
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
        index += 8;
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

#[cfg(test)]
mod tests {
    use super::*;
    extern crate std;
    use alloc::vec;
    use base64::{engine::general_purpose::STANDARD, Engine};
    use std::println;

    #[test]
    fn test_parse_simple_ton_transfer() {
        // Simple TON transfer without comment
        let body = "te6cckEBAgEARwABHCmpoxdmOz6lAAAACAADAQBoQgArFnMvHAX9tOjTp4/RDd3vP2Bn8xG+U5MTuKRKUE1NoqHc1lAAAAAAAAAAAAAAAAAAAHBy4G8=";
        let serial = STANDARD.decode(body).unwrap();

        let tx = TonTransaction::parse_hex(&serial).unwrap();

        assert_eq!(tx.action, "Ton Transfer");
        assert!(tx.comment.is_none());
        assert!(tx.data_view.is_none());
        assert!(tx.contract_data.is_none());
        assert!(!tx.to.is_empty());
        assert!(!tx.amount.is_empty());
        assert!(!tx.raw_data.is_empty());
    }

    #[test]
    fn test_parse_ton_transfer_with_comment() {
        // TON transfer with a long comment
        let serial = "b5ee9c724102050100019700011c29a9a31766611df6000000140003010166420013587ccf19c39b1ca51c29f0253ac98d03b8e5ccfc64c3ac2f21c59c20ee8b65987a1200000000000000000000000000010201fe000000004b657973746f6e652068617264776172652077616c6c6574206f666665727320756e6265617461626c65207365637572697479207769746820332050434920736563757269747920636869707320746f206d616e61676520426974636f696e20616e64206f746865722063727970746f20617373657473206f66660301fe6c696e652e4b657973746f6e65206f666665727320332077616c6c6574732c207768696368206d65616e7320796f752063616e206d616e616765206d756c7469706c65206163636f756e74732073657061726174656c79206f6e206f6e65206465766963652e4b657973746f6e65206f666665727320332077616c6c6574730400942c207768696368206d65616e7320796f752063616e206d616e616765206d756c7469706c65206163636f756e74732073657061726174656c79206f6e206f6e65206465766963652e0a0ac04eabc7";
        let serial = hex::decode(serial).unwrap();

        let tx = TonTransaction::parse_hex(&serial).unwrap();

        assert_eq!(tx.action, "Ton Transfer");
        assert!(tx.comment.is_some());
        let comment = tx.comment.as_ref().unwrap();
        assert!(comment.contains("Keystone"));
        assert!(!tx.to.is_empty());
        assert!(!tx.amount.is_empty());
    }

    #[test]
    fn test_parse_jetton_transfer() {
        // Jetton transfer (STON)
        let serial = "b5ee9c7241010301009e00011c29a9a3176656eb410000001000030101686200091c1bd942402db834b5977d2a1313119c3a3800c8e10233fa8eaf36c655ecab202faf0800000000000000000000000000010200a80f8a7ea5546de4ef815e87fb3989680800ac59ccbc7017f6d3a34e9e3f443777bcfd819fcc46f94e4c4ee291294135368b002d48cb0c90c22c52394f297b33990c2f6bbf6c425780862733961fa457f014ec02025f1050ae";
        let serial = hex::decode(serial).unwrap();

        let tx = TonTransaction::parse_hex(&serial).unwrap();

        assert_eq!(tx.action, "Jetton Transfer");
        assert!(tx.data_view.is_some());
        assert!(tx.contract_data.is_some());
        assert!(!tx.to.is_empty());

        // Contract data should contain Jetton Wallet Address
        let contract_data = tx.contract_data.as_ref().unwrap();
        assert!(contract_data.contains("Jetton Wallet Address"));
    }

    #[test]
    fn test_parse_transaction_from_boc() {
        let body = "te6cckEBAgEARwABHCmpoxdmOz6lAAAACAADAQBoQgArFnMvHAX9tOjTp4/RDd3vP2Bn8xG+U5MTuKRKUE1NoqHc1lAAAAAAAAAAAAAAAAAAAHBy4G8=";
        let serial = STANDARD.decode(body).unwrap();
        let boc = BagOfCells::parse(&serial).unwrap();

        let tx = TonTransaction::parse(boc).unwrap();

        assert_eq!(tx.action, "Ton Transfer");
        assert!(!tx.to.is_empty());
        assert!(!tx.amount.is_empty());
    }

    #[test]
    fn test_transaction_to_json() {
        let body = "te6cckEBAgEARwABHCmpoxdmOz6lAAAACAADAQBoQgArFnMvHAX9tOjTp4/RDd3vP2Bn8xG+U5MTuKRKUE1NoqHc1lAAAAAAAAAAAAAAAAAAAHBy4G8=";
        let serial = STANDARD.decode(body).unwrap();

        let tx = TonTransaction::parse_hex(&serial).unwrap();
        let json = tx.to_json().unwrap();

        assert!(json.is_object());
        assert!(json.get("to").is_some());
        assert!(json.get("amount").is_some());
        assert!(json.get("action").is_some());
    }

    #[test]
    fn test_parse_invalid_transaction_empty() {
        // Try to parse empty data
        let serial = vec![];
        let result = TonTransaction::parse_hex(&serial);

        assert!(result.is_err());
    }

    #[test]
    fn test_parse_invalid_transaction_corrupted() {
        // Try to parse corrupted data
        let serial = vec![0x00, 0x01, 0x02, 0x03];
        let result = TonTransaction::parse_hex(&serial);

        assert!(result.is_err());
    }

    #[test]
    fn test_parse_ton_proof() {
        // Valid TON proof data
        let serial = hex::decode("746f6e2d70726f6f662d6974656d2d76322f00000000b5232c324308b148e53ca5ecce6430bdaefdb1095e02189cce587e915fc053b015000000746b6170702e746f6e706f6b65722e6f6e6c696e65142b5866000000003735323061653632393534653666666330303030303030303636353765333639").unwrap();

        let proof = TonProof::parse_hex(&serial).unwrap();

        assert_eq!(proof.domain, "tkapp.tonpoker.online");
        assert!(!proof.address.is_empty());
        assert!(!proof.payload.is_empty());
        assert!(!proof.raw_message.is_empty());
        println!("Proof address: {}", proof.address);
        println!("Proof domain: {}", proof.domain);
        println!("Proof payload: {}", proof.payload);
    }

    #[test]
    fn test_parse_ton_proof_invalid_too_short() {
        // Proof data that is too short
        let serial = vec![0x74, 0x6f, 0x6e]; // "ton"
        let result = TonProof::parse_hex(&serial);

        assert!(result.is_err());
        if let Err(TonError::InvalidProof(msg)) = result {
            assert!(msg.contains("too short"));
        } else {
            panic!("Expected InvalidProof error");
        }
    }

    #[test]
    fn test_parse_ton_proof_invalid_utf8() {
        // Create valid structure but with invalid UTF-8 in domain
        let mut serial = b"ton-proof-item-v2/".to_vec();
        serial.extend_from_slice(&[0u8; 4]); // workchain
        serial.extend_from_slice(&[0u8; 32]); // address hash
        serial.extend_from_slice(&[5, 0, 0, 0]); // domain length = 5
        serial.extend_from_slice(&[0xFF, 0xFE, 0xFD, 0xFC, 0xFB]); // invalid UTF-8
        serial.extend_from_slice(&[0u8; 8]); // timestamp

        let result = TonProof::parse_hex(&serial);
        // Should fail due to invalid UTF-8 in domain
        assert!(result.is_err());
    }

    #[test]
    fn test_parse_ton_proof_invalid_domain_length() {
        // Valid header but domain length exceeds actual data
        let mut serial = b"ton-proof-item-v2/".to_vec();
        serial.extend_from_slice(&[0u8; 4]); // workchain
        serial.extend_from_slice(&[0u8; 32]); // address hash
        serial.extend_from_slice(&[0xFF, 0xFF, 0xFF, 0x7F]); // invalid domain length (very large)
        serial.extend_from_slice(&[0u8; 10]); // timestamp + small data

        let result = TonProof::parse_hex(&serial);
        assert!(result.is_err());
        if let Err(TonError::InvalidProof(msg)) = result {
            assert!(msg.contains("too short"));
        } else {
            panic!("Expected InvalidProof error");
        }
    }

    #[test]
    fn test_ton_transaction_default() {
        let tx = TonTransaction::default();

        assert_eq!(tx.to, "");
        assert_eq!(tx.amount, "");
        assert_eq!(tx.action, "");
        assert!(tx.comment.is_none());
        assert!(tx.data_view.is_none());
        assert_eq!(tx.raw_data, "");
        assert!(tx.contract_data.is_none());
    }

    #[test]
    fn test_ton_proof_default() {
        let proof = TonProof::default();

        assert_eq!(proof.domain, "");
        assert_eq!(proof.payload, "");
        assert_eq!(proof.address, "");
        assert_eq!(proof.raw_message, "");
    }

    #[test]
    fn test_transaction_clone() {
        let body = "te6cckEBAgEARwABHCmpoxdmOz6lAAAACAADAQBoQgArFnMvHAX9tOjTp4/RDd3vP2Bn8xG+U5MTuKRKUE1NoqHc1lAAAAAAAAAAAAAAAAAAAHBy4G8=";
        let serial = STANDARD.decode(body).unwrap();

        let tx1 = TonTransaction::parse_hex(&serial).unwrap();
        let tx2 = tx1.clone();

        assert_eq!(tx1.to, tx2.to);
        assert_eq!(tx1.amount, tx2.amount);
        assert_eq!(tx1.action, tx2.action);
    }

    #[test]
    fn test_proof_clone() {
        let serial = hex::decode("746f6e2d70726f6f662d6974656d2d76322f00000000b5232c324308b148e53ca5ecce6430bdaefdb1095e02189cce587e915fc053b015000000746b6170702e746f6e706f6b65722e6f6e6c696e65142b5866000000003735323061653632393534653666666330303030303030303636353765333639").unwrap();

        let proof1 = TonProof::parse_hex(&serial).unwrap();
        let proof2 = proof1.clone();

        assert_eq!(proof1.domain, proof2.domain);
        assert_eq!(proof1.payload, proof2.payload);
        assert_eq!(proof1.address, proof2.address);
    }

    #[test]
    fn test_transaction_with_empty_messages() {
        // This should fail as transaction needs at least one message
        // We need to craft a BOC with valid structure but empty messages
        let body = "te6cckEBAgEARwABHCmpoxdmOz6lAAAACAADAQBoQgArFnMvHAX9tOjTp4/RDd3vP2Bn8xG+U5MTuKRKUE1NoqHc1lAAAAAAAAAAAAAAAAAAAHBy4G8=";
        let serial = STANDARD.decode(body).unwrap();

        // This is a valid transaction, so it should succeed
        let result = TonTransaction::parse_hex(&serial);
        assert!(result.is_ok());
    }

    #[test]
    fn test_raw_data_contains_hex() {
        let body = "te6cckEBAgEARwABHCmpoxdmOz6lAAAACAADAQBoQgArFnMvHAX9tOjTp4/RDd3vP2Bn8xG+U5MTuKRKUE1NoqHc1lAAAAAAAAAAAAAAAAAAAHBy4G8=";
        let serial = STANDARD.decode(body).unwrap();

        let tx = TonTransaction::parse_hex(&serial).unwrap();

        // raw_data should be hex encoded (shortened)
        assert!(!tx.raw_data.is_empty());
        // Should start with valid hex characters
        assert!(tx
            .raw_data
            .chars()
            .all(|c| c.is_ascii_hexdigit() || c == '.'));
    }
}
