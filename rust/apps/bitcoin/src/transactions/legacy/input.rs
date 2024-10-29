use crate::collect;
use crate::errors::{BitcoinError, Result};
use crate::transactions::script_type::ScriptType;
use alloc::string::{String, ToString};
use alloc::vec::Vec;
use bitcoin;
use bitcoin::script::{Builder, PushBytesBuf};
use bitcoin::secp256k1::ecdsa::Signature;
use bitcoin::WPubkeyHash;
use bitcoin::{PublicKey, ScriptBuf, Sequence};
use bitcoin_hashes::Hash;
use core::iter;
use core::str::FromStr;
use ur_registry::pb::protoc;
use ur_registry::pb::protoc::sign_transaction::Transaction::{BchTx, BtcTx, DashTx, LtcTx};
use ur_registry::pb::protoc::{bch_tx, dash_tx, Input};

macro_rules! negative_check {
    ($t: expr, $v: expr) => {{
        let mut result: Result<()> = Ok(());
        if $v < 0 {
            result = Err(BitcoinError::InvalidRawTxCryptoBytes(format!(
                "negative value {:?} for {:?}",
                $v, $t
            )));
        };
        result
    }};
}

#[derive(Debug, Clone)]
pub struct TxIn {
    pub(crate) previous_output: String,
    pub(crate) vout: u32,
    pub(crate) value: u64,
    pub(crate) pubkey: String,
    pub(crate) hd_path: String,
}

impl TxIn {
    #[inline]
    pub fn script_sig(
        &self,
        sig: Signature,
        sig_hash_type: u8,
        script_type: &ScriptType,
    ) -> Result<ScriptBuf> {
        let pubkey = PublicKey::from_str(self.pubkey.as_str())
            .map_err(|_| BitcoinError::SignLegacyTxError("invalid public key".to_string()))?;
        match script_type {
            ScriptType::P2PKH => {
                let sig_bytes = sig
                    .serialize_der()
                    .iter()
                    .copied()
                    .chain(iter::once(sig_hash_type))
                    .collect::<Vec<u8>>();
                let mut sig_buf = PushBytesBuf::new();
                sig_buf.extend_from_slice(&sig_bytes)?;
                let builder = Builder::new().push_slice(sig_buf).push_key(&pubkey);
                Ok(builder.into_script())
            }
            ScriptType::P2SHP2WPKH => {
                let redeem_script = ScriptBuf::new_p2wpkh(&WPubkeyHash::hash(&pubkey.to_bytes()));
                let mut script_sig_bytes = PushBytesBuf::new();
                script_sig_bytes.extend_from_slice(redeem_script.as_bytes())?;
                let builder = Builder::new().push_slice(script_sig_bytes);
                Ok(builder.into_script())
            }
            _ => Err(BitcoinError::SignLegacyTxError(
                "script type is not supported".to_string(),
            )),
        }
    }
}

impl TryInto<bitcoin::TxIn> for TxIn {
    type Error = BitcoinError;

    fn try_into(self) -> Result<bitcoin::TxIn> {
        let tx_id = bitcoin::Txid::from_str(self.previous_output.as_str())?;
        Ok(bitcoin::TxIn {
            previous_output: bitcoin::OutPoint {
                txid: tx_id,
                vout: self.vout,
            },
            script_sig: Default::default(),
            sequence: Sequence::ENABLE_RBF_NO_LOCKTIME,
            witness: Default::default(),
        })
    }
}

impl TryFrom<bch_tx::Input> for TxIn {
    type Error = BitcoinError;
    fn try_from(value: bch_tx::Input) -> Result<Self> {
        let _ = negative_check!("utxo_value".to_string(), value.value);
        let utxo_value = value.value as u64;
        let _ = negative_check!("utxo_index".to_string(), value.index);
        let index = value.index as u32;
        Ok(Self {
            previous_output: value.hash,
            pubkey: value.pubkey,
            vout: index,
            value: utxo_value,
            hd_path: value.owner_key_path,
        })
    }
}

impl TryFrom<dash_tx::Input> for TxIn {
    type Error = BitcoinError;
    fn try_from(value: dash_tx::Input) -> Result<Self> {
        let _ = negative_check!("utxo_value".to_string(), value.value);
        let utxo_value = value.value as u64;
        let _ = negative_check!("utxo_index".to_string(), value.index);
        let index = value.index as u32;
        Ok(Self {
            previous_output: value.hash,
            vout: index,
            pubkey: value.pubkey,
            value: utxo_value,
            hd_path: value.owner_key_path,
        })
    }
}

impl TryFrom<Input> for TxIn {
    type Error = BitcoinError;

    fn try_from(value: protoc::Input) -> Result<Self> {
        let utxo = value
            .utxo
            .ok_or(BitcoinError::InvalidRawTxCryptoBytes(format!("empty utxo")))?;
        let _ = negative_check!("utxo value".to_string(), utxo.value)?;
        let utxo_value = utxo.value as u64;
        let _ = negative_check!("utxo index".to_string(), value.index)?;
        let index = value.index as u32;
        Ok(Self {
            previous_output: value.hash,
            vout: index,
            pubkey: utxo.public_key,
            value: utxo_value,
            hd_path: value.owner_key_path,
        })
    }
}

pub trait InputConverter {
    fn to_tx_in(&self) -> Result<Vec<TxIn>>;
}
impl InputConverter for protoc::sign_transaction::Transaction {
    fn to_tx_in(&self) -> Result<Vec<TxIn>> {
        let inputs = match self {
            LtcTx(tx) => collect!(&tx.inputs),
            BtcTx(tx) => collect!(&tx.inputs),
            BchTx(tx) => collect!(&tx.inputs),
            DashTx(tx) => collect!(&tx.inputs),
            _ => Err(BitcoinError::InvalidRawTxCryptoBytes(
                "invalid tx inputs".to_string(),
            )),
        }?;
        Ok(inputs)
    }
}
