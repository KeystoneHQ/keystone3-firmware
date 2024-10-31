use crate::aptos_type::account_address::AccountAddress;
use crate::aptos_type::value::MoveValue;
use alloc::format;
use alloc::vec::Vec;
use core::{convert::TryFrom, fmt};
use hex;
use serde::{Deserialize, Serialize};

#[derive(Clone, Hash, Eq, PartialEq, Serialize, Deserialize)]
pub enum TransactionArgument {
    U8(u8),
    U64(u64),
    U128(u128),
    Address(AccountAddress),
    U8Vector(#[serde(with = "serde_bytes")] Vec<u8>),
    Bool(bool),
}

impl fmt::Debug for TransactionArgument {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self {
            TransactionArgument::U8(value) => write!(f, "{{U8: {}}}", value),
            TransactionArgument::U64(value) => write!(f, "{{U64: {}}}", value),
            TransactionArgument::U128(value) => write!(f, "{{U128: {}}}", value),
            TransactionArgument::Bool(boolean) => write!(f, "{{BOOL: {}}}", boolean),
            TransactionArgument::Address(address) => write!(f, "{{ADDRESS: {:?}}}", address),
            TransactionArgument::U8Vector(vector) => {
                write!(f, "{{U8Vector: 0x{}}}", hex::encode(vector))
            }
        }
    }
}

impl From<TransactionArgument> for MoveValue {
    fn from(val: TransactionArgument) -> Self {
        match val {
            TransactionArgument::U8(i) => MoveValue::U8(i),
            TransactionArgument::U64(i) => MoveValue::U64(i),
            TransactionArgument::U128(i) => MoveValue::U128(i),
            TransactionArgument::Address(a) => MoveValue::Address(a),
            TransactionArgument::Bool(b) => MoveValue::Bool(b),
            TransactionArgument::U8Vector(v) => MoveValue::vector_u8(v),
        }
    }
}

impl TryFrom<MoveValue> for TransactionArgument {
    type Error = crate::errors::AptosError;
    fn try_from(val: MoveValue) -> crate::errors::Result<Self> {
        Ok(match val {
            MoveValue::U8(i) => TransactionArgument::U8(i),
            MoveValue::U64(i) => TransactionArgument::U64(i),
            MoveValue::U128(i) => TransactionArgument::U128(i),
            MoveValue::Address(a) => TransactionArgument::Address(a),
            MoveValue::Bool(b) => TransactionArgument::Bool(b),
            MoveValue::Vector(v) => TransactionArgument::U8Vector(
                v.into_iter()
                    .map(|mv| {
                        if let MoveValue::U8(byte) = mv {
                            Ok(byte)
                        } else {
                            Err(crate::errors::AptosError::ParseTxError(format!(
                                "unexpected value in bytes: {:?}",
                                mv
                            )))
                        }
                    })
                    .collect::<crate::errors::Result<Vec<u8>>>()?,
            ),
            MoveValue::Signer(_) | MoveValue::Struct(_) => {
                return Err(crate::errors::AptosError::ParseTxError(format!(
                    "invalid transaction argument: {:?}",
                    val
                )))
            }
        })
    }
}

/// Convert the transaction arguments into Move values.
pub fn convert_txn_args(args: &[TransactionArgument]) -> Vec<Vec<u8>> {
    args.iter()
        .map(|arg| {
            MoveValue::from(arg.clone())
                .simple_serialize()
                .expect("transaction arguments must serialize")
        })
        .collect()
}

/// Struct for encoding vector<vector<u8>> arguments for script functions
#[derive(Clone, Hash, Eq, PartialEq, Deserialize)]
pub struct VecBytes(Vec<serde_bytes::ByteBuf>);

impl VecBytes {
    pub fn from(vec_bytes: Vec<Vec<u8>>) -> Self {
        VecBytes(
            vec_bytes
                .into_iter()
                .map(serde_bytes::ByteBuf::from)
                .collect(),
        )
    }

    pub fn into_vec(self) -> Vec<Vec<u8>> {
        self.0
            .into_iter()
            .map(|byte_buf| byte_buf.into_vec())
            .collect()
    }
}
