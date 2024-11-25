use crate::addresses::address::Address;
use crate::collect;
use crate::errors::{BitcoinError, Result};
use alloc::string::{String, ToString};
use alloc::vec::Vec;
use bitcoin::{self, Amount};
use core::str::FromStr;
use ur_registry::pb::protoc;
use ur_registry::pb::protoc::sign_transaction::Transaction::{BchTx, BtcTx, DashTx, LtcTx};

#[derive(Debug, Clone)]
pub struct TxOut {
    pub(crate) value: u64,
    pub(crate) address: String,
    pub(crate) is_change: bool,
    pub(crate) change_address_path: String,
}

impl TryInto<bitcoin::TxOut> for TxOut {
    type Error = BitcoinError;

    fn try_into(self) -> Result<bitcoin::TxOut> {
        let address = Address::from_str(self.address.as_str())?;
        let script_pubkey = address.payload.script_pubkey();
        Ok(bitcoin::TxOut {
            value: Amount::from_sat(self.value),
            script_pubkey,
        })
    }
}

impl TryFrom<protoc::Output> for TxOut {
    type Error = BitcoinError;

    fn try_from(value: protoc::Output) -> Result<Self> {
        if value.value < 0 {
            return Err(BitcoinError::InvalidRawTxCryptoBytes(format!(
                "negative output value {:?}",
                value.value
            )));
        }
        let output_value = value.value as u64;
        Ok(Self {
            value: output_value,
            is_change: value.is_change,
            address: value.address,
            change_address_path: value.change_address_path,
        })
    }
}

pub trait OutputConverter {
    fn to_tx_out(&self) -> Result<Vec<TxOut>>;
}

impl OutputConverter for protoc::sign_transaction::Transaction {
    fn to_tx_out(&self) -> Result<Vec<TxOut>> {
        let outputs = match self {
            LtcTx(tx) => collect!(&tx.outputs),
            BtcTx(tx) => collect!(&tx.outputs),
            BchTx(tx) => collect!(&tx.outputs),
            DashTx(tx) => collect!(&tx.outputs),
            _ => Err(BitcoinError::InvalidRawTxCryptoBytes(
                "invalid tx outputs".to_string(),
            )),
        }?;
        Ok(outputs)
    }
}
