use crate::impl_base_transaction;
use crate::normalizer::{normalize_price, normalize_value};
use crate::structs::TransactionAction;
use crate::traits::BaseTransaction;
use alloc::string::{String, ToString};
use alloc::{format, vec};

use core::ops::{Deref, Mul};
use ethereum_types::{H256, U256};

use rlp::{Decodable, DecoderError, Rlp};
use third_party::hex;

type Bytes = vec::Vec<u8>;

#[derive(Clone)]
pub struct TransactionRecoveryId(pub u64);

impl Deref for TransactionRecoveryId {
    type Target = u64;

    fn deref(&self) -> &u64 {
        &self.0
    }
}

impl TransactionRecoveryId {
    pub fn standard(self) -> u8 {
        if self.0 == 27 || self.0 == 28 || self.0 > 36 {
            ((self.0 - 1) % 2) as u8
        } else {
            4
        }
    }

    pub fn chain_id(self) -> Option<u64> {
        if self.0 > 36 {
            Some((self.0 - 35) / 2)
        } else {
            None
        }
    }
}

struct TransactionSignature {
    v: TransactionRecoveryId,
    r: H256,
    s: H256,
}

impl TransactionSignature {
    #[must_use]
    pub fn new(v: u64, r: H256, s: H256) -> Self {
        Self {
            v: TransactionRecoveryId(v),
            r,
            s,
        }
    }

    pub fn v(&self) -> u64 {
        self.v.0
    }

    #[allow(dead_code)]
    pub fn r(&self) -> &H256 {
        &self.r
    }

    pub fn s(&self) -> &H256 {
        &self.s
    }

    #[allow(dead_code)]
    pub fn is_low_s(&self) -> bool {
        const LOWER: H256 = H256([
            0x7f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
            0xff, 0xff, 0x5d, 0x57, 0x6e, 0x73, 0x57, 0xa4, 0x50, 0x1d, 0xdf, 0xe9, 0x2f, 0x46,
            0x68, 0x1b, 0x20, 0xa0,
        ]);

        self.s <= LOWER
    }
}

pub struct LegacyTransaction {
    nonce: U256,
    gas_price: U256,
    gas_limit: U256,
    action: TransactionAction,
    value: U256,
    input: Bytes,
    signature: Option<TransactionSignature>,
}

impl LegacyTransaction {
    pub fn decode_raw(bytes: &[u8]) -> Result<LegacyTransaction, DecoderError> {
        rlp::decode(bytes)
    }

    pub fn get_gas_price(&self) -> U256 {
        self.gas_price
    }

    pub fn get_gas_limit(&self) -> U256 {
        self.gas_limit
    }

    pub fn get_value(&self) -> U256 {
        self.value
    }

    pub fn get_data(&self) -> Bytes {
        self.input.clone()
    }

    fn decode_rsv(rlp: &Rlp) -> Result<Option<TransactionSignature>, DecoderError> {
        if rlp.item_count()? == 6 {
            return Ok(None);
        } else if rlp.item_count()? == 9 {
            let v = rlp.val_at(6)?;
            let r = {
                let mut rarr = [0_u8; 32];
                rlp.val_at::<U256>(7)?.to_big_endian(&mut rarr);
                H256::from(rarr)
            };
            let s = {
                let mut sarr = [0_u8; 32];
                rlp.val_at::<U256>(8)?.to_big_endian(&mut sarr);
                H256::from(sarr)
            };
            return Ok(Some(TransactionSignature::new(v, r, s)));
        }
        Err(DecoderError::RlpIncorrectListLen)
    }

    pub fn chain_id(&self) -> u64 {
        match &self.signature {
            Some(signature) => {
                if signature.s().is_zero() {
                    // unsigned eip155 compatible transaction
                    signature.v()
                } else {
                    // signed eip 155 compatible transaction
                    // v = {0, 1} + chain_id * 2 + 27 + 8
                    (signature.v() - 35) / 2
                }
            }
            None => 1,
        }
    }

    pub fn is_eip155_compatible(&self) -> bool {
        match &self.signature {
            Some(signature) => {
                if signature.s().is_zero() {
                    // unsigned eip155 compatible transaction
                    true
                } else {
                    // signed eip 155 compatible transaction
                    // v = {0, 1} + chain_id * 2 + 27 + 8
                    signature.v() > 35
                }
            }
            None => false,
        }
    }
}

impl_base_transaction!(LegacyTransaction);

impl Decodable for LegacyTransaction {
    fn decode(rlp: &Rlp) -> Result<Self, DecoderError> {
        if rlp.item_count()? < 6 {
            return Err(DecoderError::RlpIncorrectListLen);
        }
        let signature = LegacyTransaction::decode_rsv(rlp)?;
        Ok(Self {
            nonce: rlp.val_at(0)?,
            gas_price: rlp.val_at(1)?,
            gas_limit: rlp.val_at(2)?,
            action: rlp.val_at(3)?,
            value: rlp.val_at(4)?,
            input: rlp.val_at(5)?,
            signature,
        })
    }
}

pub struct ParsedLegacyTransaction {
    pub(crate) nonce: u32,
    pub(crate) gas_price: String,
    pub(crate) gas_limit: String,
    pub(crate) to: String,
    pub(crate) value: String,
    pub(crate) input: String,
    pub(crate) chain_id: u64,
    pub(crate) max_txn_fee: String,
}

impl From<LegacyTransaction> for ParsedLegacyTransaction {
    fn from(value: LegacyTransaction) -> Self {
        Self {
            nonce: value.nonce.as_u32(),
            gas_price: normalize_price(value.gas_price.as_u64()),
            gas_limit: value.gas_limit.to_string(),
            to: format!("0x{}", hex::encode(value.get_to())),
            value: normalize_value(value.get_value()),
            input: hex::encode(value.get_data()),
            chain_id: value.chain_id(),
            max_txn_fee: normalize_value(value.gas_price.mul(value.gas_limit)),
        }
    }
}
