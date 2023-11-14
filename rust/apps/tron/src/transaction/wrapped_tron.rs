use crate::address::get_address;
use crate::errors::{Result, TronError};
use crate::pb::protocol::{
    transaction, Transaction, TransferAssetContract, TransferContract, TriggerSmartContract,
};
use crate::utils::base58check_to_u8_slice;
use alloc::borrow::ToOwned;
use alloc::string::{String, ToString};
use alloc::vec::Vec;
use alloc::{format, vec};
use app_utils::keystone;
use ascii::AsciiStr;
use core::ops::Div;
use core::str::FromStr;
use ethabi::{
    ethereum_types::{H160, U256},
    Function, Param, ParamType, StateMutability, Token,
};
use prost::Message;
use prost_types::Any;
use third_party::cryptoxide::hashing;
use third_party::hex;
use third_party::ur_registry::pb::protoc;
use third_party::ur_registry::pb::protoc::sign_transaction::Transaction::TronTx;
use third_party::ur_registry::pb::protoc::{LatestBlock, Payload, SignTransaction};

#[derive(Debug, Clone)]
pub struct WrappedTron {
    pub(crate) tron_tx: Transaction,
    pub(crate) hd_path: String,
    pub(crate) extended_pubkey: String,
    pub(crate) xfp: String,
    pub(crate) token: String,
    pub(crate) contract_address: String,
    pub(crate) from: String,
    pub(crate) to: String,
    pub(crate) value: String,
    pub(crate) token_short_name: Option<String>,
    pub(crate) divider: f64,
}

#[macro_export]
macro_rules! derivation_account_path {
    ($t: expr) => {{
        let parts = $t.split("/").collect::<Vec<&str>>();
        let result: Result<String> = match crate::check_hd_path!(parts) {
            Ok(_) => {
                let path = parts.as_slice()[1..parts.len() - 2].to_vec().join("/");
                Ok(format!("{}{}", "m/", path))
            }
            Err(e) => Err(e),
        };
        result
    }};
}

impl WrappedTron {
    pub fn check_input(&self, context: &keystone::ParseContext) -> Result<()> {
        // check master fingerprint
        if self.xfp.to_uppercase() != hex::encode(context.master_fingerprint).to_uppercase() {
            return Err(TronError::InvalidParseContext(format!(
                "invalid xfp, expected {}, got {}",
                hex::encode(context.master_fingerprint),
                self.xfp
            )));
        }
        let derived_address = get_address(self.hd_path.to_string(), &self.extended_pubkey)?;
        if derived_address == self.from {
            return Ok(());
        }
        Err(TronError::NoMyInputs)
    }

    pub fn signature_hash(&self) -> Result<Vec<u8>> {
        let raw_data =
            &self
                .tron_tx
                .raw_data
                .to_owned()
                .ok_or(TronError::InvalidRawTxCryptoBytes(
                    "empty raw data".to_string(),
                ))?;
        let raw_bytes = raw_data.encode_to_vec();
        let result = hashing::sha256(&raw_bytes);
        Ok(result.to_vec())
    }
    fn build_transfer_transaction(
        token: String,
        from: String,
        to: String,
        amount: i64,
    ) -> Result<Transaction> {
        let to_address = base58check_to_u8_slice(to)?;
        let owner_address = base58check_to_u8_slice(from)?;
        return if token.to_uppercase() == "TRX" {
            let mut transfer_contract = TransferContract::default();
            transfer_contract.owner_address = owner_address;
            transfer_contract.amount = amount;
            transfer_contract.to_address = to_address;
            WrappedTron::build_transfer_contract(
                transfer_contract,
                transaction::contract::ContractType::TransferContract,
                "TransferContract",
                None,
            )
        } else {
            let mut transfer_contract = TransferAssetContract::default();
            transfer_contract.owner_address = owner_address;
            transfer_contract.to_address = to_address;
            transfer_contract.amount = amount;
            let token = AsciiStr::from_ascii(token.as_str())
                .map(|r| r.as_bytes().to_vec())
                .map_err(|_e| TronError::InvalidRawTxCryptoBytes("invalid token".to_string()))?;
            transfer_contract.asset_name = token;
            WrappedTron::build_transfer_contract(
                transfer_contract,
                transaction::contract::ContractType::TransferAssetContract,
                "TransferAssetContract",
                None,
            )
        };
    }

    fn build_transfer_contract(
        message: impl Message,
        contract_type: transaction::contract::ContractType,
        type_name: &str,
        permission_id: Option<i32>,
    ) -> Result<Transaction> {
        let mut type_url_prefix: String = String::from("type.googleapis.com/protocol.");
        type_url_prefix.push_str(type_name);
        let mut any_value = Any::default();
        let msg_bytes = message.encode_to_vec();
        any_value.value = msg_bytes;
        any_value.type_url = type_url_prefix;
        let contract = transaction::Contract {
            r#type: i32::from(contract_type),
            parameter: Some(any_value),
            provider: vec![],
            contract_name: vec![],
            permission_id: permission_id.unwrap_or(0),
        };
        let mut raw = transaction::Raw::default();
        raw.contract = vec![contract];
        let mut transaction = Transaction::default();
        transaction.raw_data = Some(raw);
        return Ok(transaction);
    }

    fn ref_with_latest_block(
        tx: &mut Transaction,
        latest_block: LatestBlock,
        is_trc_20: bool,
    ) -> Result<Transaction> {
        let number_buf: [u8; 8] = (latest_block.number as u64).to_be_bytes();
        let hash_buf = hex::decode(latest_block.hash.to_string()).map_err(|_e| {
            TronError::InvalidRawTxCryptoBytes(format!(
                "invalid latest block hash {}",
                latest_block.hash
            ))
        })?;
        let mut generated_block_id = [0u8; 32];
        generated_block_id[..8].copy_from_slice(&number_buf);
        generated_block_id[8..31].copy_from_slice(&hash_buf[8..hash_buf.len() - 1]);
        if let Some(mut raw_data) = tx.raw_data.to_owned() {
            raw_data.ref_block_hash = generated_block_id[8..16].to_vec();
            raw_data.ref_block_bytes = generated_block_id[6..8].to_vec();
            if is_trc_20 {
                raw_data.fee_limit = 1000000000;
                raw_data.timestamp = latest_block.timestamp;
            }
            raw_data.expiration = latest_block.timestamp + 600 * 5 * 1000;
            tx.raw_data = Some(raw_data);
        };
        return Ok(tx.to_owned());
    }

    fn build_transfer_tx(tx_data: &protoc::TronTx) -> Result<Transaction> {
        let mut token = tx_data.token.to_string();
        if token.is_empty() {
            token = "TRX".to_string();
        }
        let value: i64 = tx_data.value.parse::<i64>().map_err(|_e| {
            TronError::InvalidRawTxCryptoBytes(format!(
                "invalid transfer value {:?}",
                tx_data.value
            ))
        })?;
        let tx = &mut WrappedTron::build_transfer_transaction(
            token,
            tx_data.from.to_owned(),
            tx_data.to.to_owned(),
            value,
        )?;
        let latest_block =
            tx_data
                .latest_block
                .to_owned()
                .ok_or(TronError::InvalidRawTxCryptoBytes(
                    "empty latest block".to_string(),
                ))?;
        return Self::ref_with_latest_block(tx, latest_block, false);
    }

    #[allow(deprecated)]
    fn parse_contract_params(tx_data: &protoc::TronTx) -> Result<Vec<u8>> {
        let address_params = Param {
            name: "to".to_string(),
            kind: ParamType::Address,
            internal_type: None,
        };
        let value_params = Param {
            name: "value".to_string(),
            kind: ParamType::Uint(256),
            internal_type: None,
        };
        let inputs = vec![address_params, value_params];

        let outputs: Vec<Param> = Vec::new();

        let fun = Function {
            name: "transfer".to_string(),
            inputs,
            outputs,
            constant: false,
            state_mutability: StateMutability::Payable,
        };
        let to = tx_data.to.to_string();
        let to_address = base58check_to_u8_slice(to)?;
        let value: U256 = U256::from_str_radix(tx_data.value.as_str(), 10).map_err(|_e| {
            TronError::InvalidRawTxCryptoBytes(format!("invalid value {:?}", tx_data.value))
        })?;
        let tokens = vec![
            Token::Address(H160::from_slice(&to_address[1..])),
            Token::Uint(value),
        ];
        fun.encode_input(&tokens)
            .map_err(|_| TronError::InvalidRawTxCryptoBytes(format!("invalid token {:?}", tokens)))
    }

    fn generate_trc20_tx(tx_data: &protoc::TronTx) -> Result<Transaction> {
        let mut contract = TriggerSmartContract::default();
        let contract_address = tx_data.contract_address.to_string();
        contract.owner_address = base58check_to_u8_slice(tx_data.from.to_string())?;
        contract.contract_address = base58check_to_u8_slice(contract_address)?;
        contract.call_value = 0;
        let data = WrappedTron::parse_contract_params(tx_data)?;
        contract.data = data;
        let tx = &mut WrappedTron::build_transfer_contract(
            contract,
            transaction::contract::ContractType::TriggerSmartContract,
            "TriggerSmartContract",
            None,
        )?;
        let latest_block =
            tx_data
                .latest_block
                .to_owned()
                .ok_or(TronError::InvalidRawTxCryptoBytes(
                    "empty latest block".to_string(),
                ))?;
        Self::ref_with_latest_block(tx, latest_block, true)
    }

    pub fn from_payload(payload: Payload, context: &keystone::ParseContext) -> Result<Self> {
        let sign_tx_content: Result<SignTransaction> = match payload.content {
            Some(protoc::payload::Content::SignTx(sign_tx_content)) => Ok(sign_tx_content),
            _ => {
                return Err(TronError::InvalidRawTxCryptoBytes(format!(
                    "invalid payload content {:?}",
                    payload.content
                )));
            }
        };
        let content = sign_tx_content?;
        let tx = &content
            .transaction
            .ok_or(TronError::InvalidRawTxCryptoBytes(
                "empty transaction field for payload content".to_string(),
            ))?;
        let mut token_short_name: Option<String> = None;
        let mut divider = DIVIDER;
        match tx {
            TronTx(tx) => {
                if let Some(value) = tx.to_owned().r#override {
                    token_short_name = Some(value.token_short_name);
                    divider = 10u64.pow(value.decimals as u32) as f64;
                }
                let tron_tx = if tx.contract_address.is_empty() {
                    Self::build_transfer_tx(tx)
                } else {
                    Self::generate_trc20_tx(tx)
                }?;
                Ok(Self {
                    hd_path: content.hd_path,
                    extended_pubkey: context.extended_public_key.to_string(),
                    tron_tx,
                    xfp: payload.xfp,
                    token: tx.token.to_string(),
                    contract_address: tx.contract_address.to_string(),
                    from: tx.from.to_string(),
                    to: tx.to.to_string(),
                    value: tx.value.to_string(),
                    divider,
                    token_short_name,
                })
            }
            _ => Err(TronError::InvalidRawTxCryptoBytes(
                "invalid transaction type".to_string(),
            )),
        }
    }

    pub fn format_amount(&self) -> Result<String> {
        let value = f64::from_str(self.value.as_str())?;
        let unit = self.format_unit()?;
        Ok(format!("{} {}", value.div(self.divider as f64), unit))
    }

    pub fn format_method(&self) -> Result<String> {
        if !self.contract_address.is_empty() {
            Ok("TRC-20 Transfer".to_string())
        } else if !self.token.is_empty() && self.token.to_uppercase() != "TRX" {
            Ok("TRC-10 Transfer".to_string())
        } else {
            Ok("TRX Transfer".to_string())
        }
    }
    pub fn format_unit(&self) -> Result<String> {
        match self.token_short_name.to_owned() {
            Some(name) => Ok(name),
            _ => Ok("TRX".to_string()),
        }
    }
}

pub const DIVIDER: f64 = 1000000 as f64;
pub const NETWORK: &str = "TRON";
