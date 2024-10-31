use crate::address::generate_address;
use crate::eip1559_transaction::ParsedEIP1559Transaction;
use crate::errors::Result;
use crate::{Bytes, ParsedLegacyTransaction};
use alloc::string::String;

use alloc::vec::Vec;
use bitcoin::secp256k1::PublicKey;
use ethereum_types::H160;
use rlp::{Decodable, DecoderError, Encodable, Rlp};

#[derive(Clone)]
pub enum TransactionAction {
    Call(H160),
    Create,
}

impl Decodable for TransactionAction {
    fn decode(rlp: &Rlp) -> core::result::Result<Self, DecoderError> {
        if rlp.is_empty() {
            if rlp.is_data() {
                Ok(TransactionAction::Create)
            } else {
                Err(DecoderError::RlpExpectedToBeData)
            }
        } else {
            Ok(TransactionAction::Call(rlp.as_val()?))
        }
    }
}

impl Encodable for TransactionAction {
    fn rlp_append(&self, s: &mut rlp::RlpStream) {
        match self {
            TransactionAction::Call(address) => {
                s.append(address);
            }
            TransactionAction::Create => {
                s.append_empty_data();
            }
        }
        ()
    }
}

#[derive(Clone, Debug)]
pub struct ParsedEthereumTransaction {
    pub nonce: u32,
    pub chain_id: u64,
    pub from: String,
    pub to: String,
    pub value: String,
    pub input: String,

    pub gas_price: Option<String>,
    pub max_fee_per_gas: Option<String>,
    pub max_priority_fee_per_gas: Option<String>,
    pub max_fee: Option<String>,
    pub max_priority: Option<String>,

    pub gas_limit: String,
    pub max_txn_fee: String,
}

impl ParsedEthereumTransaction {
    pub(crate) fn from_legacy(tx: ParsedLegacyTransaction, from: PublicKey) -> Result<Self> {
        Ok(Self {
            nonce: tx.nonce,
            gas_limit: tx.gas_limit,
            gas_price: Some(tx.gas_price),
            from: generate_address(from)?,
            to: tx.to,
            value: tx.value,
            chain_id: tx.chain_id,
            input: tx.input,
            max_txn_fee: tx.max_txn_fee,

            max_fee_per_gas: None,
            max_priority_fee_per_gas: None,
            max_fee: None,
            max_priority: None,
        })
    }

    pub(crate) fn from_eip1559(tx: ParsedEIP1559Transaction, from: PublicKey) -> Result<Self> {
        Ok(Self {
            nonce: tx.nonce,
            gas_limit: tx.gas_limit,
            from: generate_address(from)?,
            to: tx.to,
            value: tx.value,
            chain_id: tx.chain_id,
            input: tx.input,
            max_fee_per_gas: Some(tx.max_fee_per_gas),
            max_priority_fee_per_gas: Some(tx.max_priority_fee_per_gas),
            max_txn_fee: tx.max_txn_fee,
            max_fee: Some(tx.max_fee),
            max_priority: Some(tx.max_priority),
            gas_price: None,
        })
    }
}

pub struct EthereumSignature(pub u64, pub [u8; 64]);

impl EthereumSignature {
    pub fn serialize(&self) -> Bytes {
        let mut s = Vec::from(self.1);
        let v_bytes = self.0.to_be_bytes();
        let mut index = 0;
        for x in v_bytes {
            if x == 0 {
                index += 1
            } else {
                break;
            }
        }
        let stripped_v = &v_bytes[index..];
        if stripped_v.is_empty() {
            s.push(0)
        } else {
            s.extend(stripped_v)
        }
        s
    }
}

#[derive(Clone, Debug)]
pub struct PersonalMessage {
    pub raw_message: String,
    pub utf8_message: String,
    pub from: String,
}

impl PersonalMessage {
    pub fn from(raw_message: String, utf8_message: String, from: PublicKey) -> Result<Self> {
        Ok(Self {
            raw_message,
            utf8_message,
            from: generate_address(from)?,
        })
    }
}

#[derive(Clone, Debug)]
pub struct TypedData {
    pub name: String,
    pub version: String,
    pub chain_id: String,
    pub verifying_contract: String,
    pub salt: String,
    pub primary_type: String,
    pub message: String,
    pub from: String,
}

impl TypedData {
    pub fn from(data: TypedData, from: PublicKey) -> Result<Self> {
        Ok(Self {
            from: generate_address(from)?,
            ..data
        })
    }
}

#[cfg(test)]
pub mod tests {
    use crate::structs::EthereumSignature;
    use hex;

    extern crate std;

    #[test]
    fn test_signature() {
        {
            let signature = EthereumSignature(1440123, [0u8; 64]);
            assert_eq!("0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000015f97b", hex::encode(signature.serialize()))
        }
        {
            let signature = EthereumSignature(0, [0u8; 64]);
            assert_eq!("0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000", hex::encode(signature.serialize()))
        }
        {
            let signature = EthereumSignature(1, [0u8; 64]);
            assert_eq!("0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000001", hex::encode(signature.serialize()))
        }
    }
}
