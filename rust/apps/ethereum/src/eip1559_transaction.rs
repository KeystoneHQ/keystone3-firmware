use crate::normalizer::{normalize_price, normalize_value};
use crate::structs::TransactionAction;
use crate::traits::BaseTransaction;
use crate::{impl_base_transaction, Bytes};
use alloc::format;
use alloc::string::{String, ToString};

use core::ops::Mul;
use ethereum_types::U256;
use hex;
use rlp::{Decodable, DecoderError, Rlp};

pub struct EIP1559Transaction {
    pub chain_id: u64,
    pub nonce: U256,
    pub max_priority_fee_per_gas: U256,
    pub max_fee_per_gas: U256,
    pub gas_limit: U256,
    pub action: TransactionAction,
    pub value: U256,
    pub input: Bytes,
    // we do not care access_list currently
    // pub access_list: AccessList,
}

impl EIP1559Transaction {
    pub fn decode_raw(bytes: &[u8]) -> Result<EIP1559Transaction, DecoderError> {
        rlp::decode(bytes)
    }
}

impl_base_transaction!(EIP1559Transaction);

impl Decodable for EIP1559Transaction {
    fn decode(rlp: &Rlp) -> Result<Self, DecoderError> {
        Ok(Self {
            chain_id: rlp.val_at(0)?,
            nonce: rlp.val_at(1)?,
            max_priority_fee_per_gas: rlp.val_at(2)?,
            max_fee_per_gas: rlp.val_at(3)?,
            gas_limit: rlp.val_at(4)?,
            action: rlp.val_at(5)?,
            value: rlp.val_at(6)?,
            input: rlp.val_at(7)?,
        })
    }
}

pub struct ParsedEIP1559Transaction {
    pub(crate) chain_id: u64,
    pub(crate) nonce: u32,
    pub(crate) max_priority_fee_per_gas: String,
    pub(crate) max_fee_per_gas: String,
    pub(crate) gas_limit: String,
    pub(crate) to: String,
    pub(crate) value: String,
    pub(crate) input: String,
    pub(crate) max_txn_fee: String,
    pub(crate) max_fee: String,
    pub(crate) max_priority: String,
}

impl From<EIP1559Transaction> for ParsedEIP1559Transaction {
    fn from(value: EIP1559Transaction) -> Self {
        Self {
            chain_id: value.chain_id,
            nonce: value.nonce.as_u32(),
            max_priority_fee_per_gas: normalize_price(value.max_priority_fee_per_gas.as_u64()),
            max_fee_per_gas: normalize_price(value.max_fee_per_gas.as_u64()),
            gas_limit: value.gas_limit.to_string(),
            to: format!("0x{}", hex::encode(value.get_to())),
            value: normalize_value(value.value),
            input: hex::encode(value.input),
            max_txn_fee: normalize_value(value.max_fee_per_gas.mul(value.gas_limit)),
            max_priority: normalize_value(value.max_priority_fee_per_gas.mul(value.gas_limit)),
            max_fee: normalize_value(value.max_fee_per_gas.mul(value.gas_limit)),
        }
    }
}
