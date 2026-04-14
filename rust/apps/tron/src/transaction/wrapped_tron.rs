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
use core::str::FromStr;
use cryptoxide::hashing;
use ethabi::{
    ethereum_types::{H160, U256},
    Function, Param, ParamType, StateMutability, Token,
};
use hex;
use prost::Message;
use prost_types::Any;
use ur_registry::pb::protoc;
use ur_registry::pb::protoc::sign_transaction::Transaction::TronTx;
use ur_registry::pb::protoc::{LatestBlock, Payload, SignTransaction};

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
    pub(crate) fee_limit: u64,
    pub(crate) memo: String,
    pub(crate) expiration: String,
}

#[macro_export]
macro_rules! derivation_account_path {
    ($t: expr) => {{
        let parts = $t.split("/").collect::<Vec<&str>>();
        let result: Result<String> = match $crate::check_hd_path!(parts) {
            Ok(_) => {
                let path = parts.as_slice()[1..parts.len() - 2].to_vec().join("/");
                Ok(format!("{}{}", "m/", path))
            }
            Err(e) => Err(e),
        };
        result
    }};
}

const KNOWN_TOKENS: &[(&str, &str, f64)] = &[
    ("TR7NHqjeKQxGTCi8q8ZY4pL8otSzgjLj6t", "USDT", 1_000_000.0), // 6 decimals
    ("TEkxiTehnzSmSe2XqrBj4w32RUN966rdz8", "USDC", 1_000_000.0), // 6 decimals
    (
        "TUpMhErZL2fhh4sVNULAbNKLokS4GjC1F4",
        "TUSD",
        1_000_000_000_000_000_000.0,
    ), // 18 decimals
    (
        "TAFjULxiVgT4qWk6UZwjqwZXTSaGaqnVp4",
        "BTT",
        1_000_000_000_000_000_000.0,
    ), // 18 decimals
    (
        "TCFLL5dx5ZJdKnWuesXxi1VPwjLVmWZZy9",
        "JST",
        1_000_000_000_000_000_000.0,
    ), // 18 decimals
    ("TFczxzPhnThNSqr5by8tvxsdCFRRz6cPNq", "NFT", 1_000_000.0),  // 6 decimals
    (
        "TSSMHYeV2uE9qYH95DqyoCuNCzEL1NvU3S",
        "SUN",
        1_000_000_000_000_000_000.0,
    ), // 18 decimals
    ("TYhWwKpw43ENFWBTGpzLHn3882f2au7SMi", "WBTC", 100_000_000.0), // 8 decimals
    ("TNUC9Qb1rRpS5CbWLmNMxXBjyFoydXjWFR", "WTRX", 1_000_000.0), // 6 decimals
    ("TLa2f6VPqDgRE67v1736s7bJ8Ray5wYjU7", "WIN", 1_000_000.0),  // 6 decimals
    (
        "TXDk8mbtRbXeYuMNS83CfKPaYYT8XWv9Hz",
        "USDD",
        1_000_000_000_000_000_000.0,
    ), // 18 decimals
];

impl WrappedTron {
    pub fn from_raw_transaction(raw_tx: Transaction, path: String) -> Result<Self> {
        let mut instance = Self {
            tron_tx: raw_tx,
            hd_path: path,
            extended_pubkey: String::new(),
            xfp: String::new(),
            token: "TRX".to_string(),
            contract_address: String::new(),
            from: String::new(),
            to: String::new(),
            value: "0".to_string(),
            divider: DIVIDER,
            token_short_name: None,
            fee_limit: 0,
            memo: String::new(),
            expiration: String::new(),
        };

        if let Some(raw) = &instance.tron_tx.raw_data {
            instance.fee_limit = raw.fee_limit as u64;
            instance.memo = String::from_utf8_lossy(&raw.data).to_string();
            if let Some(contract) = raw.contract.get(0) {
                use crate::pb::protocol::transaction::contract::ContractType;
                let c_type = ContractType::from_i32(contract.r#type)
                    .unwrap_or(ContractType::TransferContract);

                if let Some(param) = &contract.parameter {
                    match c_type {
                        // A. TRX Transfer
                        ContractType::TransferContract => {
                            let ct =
                                TransferContract::decode(param.value.as_slice()).map_err(|_| {
                                    TronError::InvalidRawTxCryptoBytes(
                                        "TransferContract decode failed".to_string(),
                                    )
                                })?;
                            instance.from = bitcoin::base58::encode_check(&ct.owner_address);
                            instance.to = bitcoin::base58::encode_check(&ct.to_address);
                            instance.value = ct.amount.to_string();
                            instance.token = "TRX".to_string();
                            instance.divider = DIVIDER;
                        }

                        // B. TRC-20 Transfer
                        ContractType::TriggerSmartContract => {
                            let ct = TriggerSmartContract::decode(param.value.as_slice()).map_err(
                                |_| {
                                    TronError::InvalidRawTxCryptoBytes(
                                        "TriggerSmartContract decode failed".to_string(),
                                    )
                                },
                            )?;
                            instance.from = bitcoin::base58::encode_check(&ct.owner_address);
                            instance.contract_address =
                                bitcoin::base58::encode_check(&ct.contract_address);

                            if ct.data.len() >= 68 && &ct.data[0..4] == &[0xa9, 0x05, 0x9c, 0xbb] {
                                let mut to_addr_bytes = vec![0x41u8];
                                to_addr_bytes.extend_from_slice(&ct.data[16..36]);
                                instance.to = bitcoin::base58::encode_check(&to_addr_bytes);

                                let amount_bytes = &ct.data[36..68];
                                instance.value =
                                    ethabi::ethereum_types::U256::from_big_endian(amount_bytes)
                                        .to_string();

                                if let Some(token_info) = KNOWN_TOKENS
                                    .iter()
                                    .find(|t| t.0 == instance.contract_address)
                                {
                                    instance.token = token_info.1.to_string();
                                    instance.divider = token_info.2;
                                } else {
                                    instance.token = "TRC20 Token".to_string();
                                    instance.divider = 10u64.pow(6) as f64;
                                }
                            }
                        }

                        // C. TRC-10 Transfer
                        ContractType::TransferAssetContract => {
                            let ct = TransferAssetContract::decode(param.value.as_slice())
                                .map_err(|_| {
                                    TronError::InvalidRawTxCryptoBytes(
                                        "TransferAssetContract decode failed".to_string(),
                                    )
                                })?;
                            instance.from = bitcoin::base58::encode_check(&ct.owner_address);
                            instance.to = bitcoin::base58::encode_check(&ct.to_address);
                            instance.value = ct.amount.to_string();
                            instance.token = String::from_utf8_lossy(&ct.asset_name).to_string();
                            instance.divider = DIVIDER;
                        }
                        _ => {}
                    }
                }
            }
        }
        Ok(instance)
    }

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
        if token.to_uppercase() == "TRX" {
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
        }
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
        Ok(transaction)
    }

    fn ref_with_latest_block(
        tx: &mut Transaction,
        latest_block: LatestBlock,
        is_trc_20: bool,
    ) -> Result<Transaction> {
        let number_buf: [u8; 8] = (latest_block.number as u64).to_be_bytes();
        let hash_buf = hex::decode(&latest_block.hash).map_err(|_e| {
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
        Ok(tx.to_owned())
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
        Self::ref_with_latest_block(tx, latest_block, false)
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
            .map_err(|_| TronError::InvalidRawTxCryptoBytes(format!("invalid token {tokens:?}")))
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
        let content: SignTransaction = sign_tx_content?;
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
                let mut tron_tx = if tx.contract_address.is_empty() {
                    Self::build_transfer_tx(tx)
                } else {
                    Self::generate_trc20_tx(tx)
                }?;
                let fee_limit = if let Some(raw) = &tron_tx.raw_data {
                    raw.fee_limit as u64
                } else {
                    0
                };
                let mut expiration = "".to_string();

                if !tx.memo.is_empty() {
                    if let Some(ref mut raw) = tron_tx.raw_data {
                        raw.data = tx.memo.as_bytes().to_vec();
                        expiration = (raw.expiration).to_string();
                    }
                }

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
                    fee_limit,
                    memo: tx.memo.clone(),
                    expiration,
                })
            }
            _ => Err(TronError::InvalidRawTxCryptoBytes(
                "invalid transaction type".to_string(),
            )),
        }
    }

    pub fn format_amount(&self) -> Result<String> {
        let raw_val = f64::from_str(self.value.as_str())?;
        let amount = raw_val / self.divider;
        let unit = self.format_unit()?;

        // Calculate precision from divider (power of 10)
        let precision = if self.divider <= 1.0 {
            0
        } else {
            let mut count = 0;
            let mut d = self.divider;
            while d >= 9.99999 {
                // Account for float precision
                d /= 10.0;
                count += 1;
            }
            count
        };

        let formatted = format!("{:.*}", precision, amount);
        let trimmed = if formatted.contains('.') {
            formatted
                .trim_end_matches('0')
                .trim_end_matches('.')
                .to_string()
        } else {
            formatted
        };

        let final_value = if trimmed == "0" && raw_val > 0.0 {
            format!("{:.*}", precision, amount)
        } else {
            trimmed
        };

        Ok(format!("{} {}", final_value, unit))
    }

    pub fn format_method(&self) -> Result<String> {
        if !self.contract_address.is_empty() {
            Ok("TRC-20 Transfer".to_string())
        } else if !self.token.is_empty() && self.token != "TRX" {
            Ok("TRC-10 Transfer".to_string())
        } else {
            Ok("TRX Transfer".to_string())
        }
    }
    pub fn format_unit(&self) -> Result<String> {
        if let Some(ref name) = self.token_short_name {
            return Ok(name.clone());
        }
        if !self.token.is_empty() {
            Ok(self.token.clone())
        } else {
            Ok("TRX".to_string())
        }
    }
}

pub const DIVIDER: f64 = 1000000_f64;
pub const NETWORK: &str = "Tron Mainnet";

#[cfg(test)]
mod tests {
    extern crate std;
    use super::*;
    use crate::test::{prepare_parse_context, prepare_payload};
    use alloc::string::ToString;
    use bitcoin::bip32::Fingerprint;
    use core::str::FromStr;
    use ur_registry::pb::protoc::{payload, Payload, SignMessage};

    #[test]
    fn test_from_raw_transaction_decode_failures() {
        let mut tron_tx = Transaction::default();
        let mut raw = transaction::Raw::default();
        let mut contract = transaction::Contract::default();
        contract.r#type = 1;
        contract.parameter = Some(prost_types::Any {
            type_url: "type.googleapis.com/protocol.TransferContract".to_string(),
            value: vec![0xff, 0xff],
        });
        raw.contract = vec![contract];
        tron_tx.raw_data = Some(raw);

        let result = WrappedTron::from_raw_transaction(tron_tx, "m/44'/195'/0'/0/0".to_string());
        assert!(result.is_err());
    }

    #[test]
    fn test_from_payload_invalid_content() {
        let context = prepare_parse_context("xpub6D1AabNHCupeiLM65ZR9UStMhJ1vCpyV4XbZdyhMZBiJXALQtmn9p42VTQckoHVn8WNqS7dqnJokZHAHcHGoaQgmv8D45oNUKx6DZMNZBCd");
        let payload_empty = Payload {
            r#type: payload::Type::SignTx as i32,
            xfp: "12345678".to_string(),
            content: None,
        };
        let result = WrappedTron::from_payload(payload_empty, &context);
        assert!(result.is_err());

        let payload_wrong = Payload {
            r#type: payload::Type::SignMsg as i32,
            xfp: "12345678".to_string(),
            content: Some(payload::Content::SignMsg(SignMessage::default())),
        };
        let result = WrappedTron::from_payload(payload_wrong, &context);
        assert!(result.is_err());
    }

    #[test]
    fn test_signature_hash_error_cases() {
        let tx_no_raw = WrappedTron {
            tron_tx: Transaction::default(),
            hd_path: "".to_string(),
            extended_pubkey: "".to_string(),
            xfp: "".to_string(),
            token: "".to_string(),
            contract_address: "".to_string(),
            from: "".to_string(),
            to: "".to_string(),
            value: "".to_string(),
            token_short_name: None,
            divider: 1.0,
            fee_limit: 0,
            memo: String::new(),
            expiration: String::new(),
        };
        assert!(tx_no_raw.signature_hash().is_err());
    }

    #[test]
    fn test_check_input_failure_paths() {
        let hex = "1f8b08000000000000030dcfbd4ac34000c071220ea58bdaa9742a41a84bc87d27270e9ab61890c4268d54bb5dee2e26607b508b4a9fa26fe01bf8b128f812be82b383b8161703ffe9bffd1a5bad9d64d1374a77470bb334d2dc7436567d1b1e96540920ec6fabb99da5e7716b5f4a4e58ae91e36b221d8272ed088ca04399a058f8b2a09075f62297909e0b39edb9a0ce05dde79faf8f0d3868048f56c7ce2e86d3b13abb35833089f4f4be2a97ca04554cd8eaa13c9d5ca9d0b6b3315d8d4c9f5c0e83597837884fe6f309ba0e719494328d5995ce90050fe3e671c17c0ab9d2bc904011a031a502f202e414032e19c60c78be209e409aab1cfa9041e603c204821ad588ddd7f5baddfefd7c7aff03e1cbdbd13f2aab0f710f010000";
        let pubkey_str = "xpub6D1AabNHCupeiLM65ZR9UStMhJ1vCpyV4XbZdyhMZBiJXALQtmn9p42VTQckoHVn8WNqS7dqnJokZHAHcHGoaQgmv8D45oNUKx6DZMNZBCd";
        let payload = prepare_payload(hex);
        let context = prepare_parse_context(pubkey_str);
        let mut tx = WrappedTron::from_payload(payload, &context).unwrap();

        tx.from = "TAddressNotMine".to_string();
        assert!(matches!(
            tx.check_input(&context),
            Err(TronError::NoMyInputs)
        ));
    }

    #[test]
    fn test_check_input_address_mismatch() {
        let hex = "1f8b08000000000000030dcfbd4ac34000c071220ea58bdaa9742a41a84bc87d27270e9ab61890c4268d54bb5dee2e26607b508b4a9fa26fe01bf8b128f812be82b383b8161703ffe9bffd1a5bad9d64d1374a77470bb334d2dc7436567d1b1e96540920ec6fabb99da5e7716b5f4a4e58ae91e36b221d8272ed088ca04399a058f8b2a09075f62297909e0b39edb9a0ce05dde79faf8f0d3868048f56c7ce2e86d3b13abb35833089f4f4be2a97ca04554cd8eaa13c9d5ca9d0b6b3315d8d4c9f5c0e83597837884fe6f309ba0e719494328d5995ce90050fe3e671c17c0ab9d2bc904011a031a502f202e414032e19c60c78be209e409aab1cfa9041e603c204821ad588ddd7f5baddfefd7c7aff03e1cbdbd13f2aab0f710f010000";
        let pubkey_str = "xpub6D1AabNHCupeiLM65ZR9UStMhJ1vCpyV4XbZdyhMZBiJXALQtmn9p42VTQckoHVn8WNqS7dqnJokZHAHcHGoaQgmv8D45oNUKx6DZMNZBCd";
        let payload = prepare_payload(hex);
        let context = prepare_parse_context(pubkey_str);
        let mut tx = WrappedTron::from_payload(payload, &context).unwrap();

        tx.xfp = hex::encode(context.master_fingerprint);

        tx.from = "TUEZSdKsoDHQMeZwihtdoBiN46zxhGWYdX".to_string();

        let result = tx.check_input(&context);

        assert!(matches!(result, Err(TronError::NoMyInputs)));
    }

    #[test]
    fn test_signature_hash_empty_raw() {
        let tx = WrappedTron {
            tron_tx: Transaction::default(),
            hd_path: "".to_string(),
            extended_pubkey: "".to_string(),
            xfp: "".to_string(),
            token: "".to_string(),
            contract_address: "".to_string(),
            from: "".to_string(),
            to: "".to_string(),
            value: "".to_string(),
            token_short_name: None,
            divider: 1.0,
            fee_limit: 0,
            memo: String::new(),
            expiration: String::new(),
        };
        let result = tx.signature_hash();
        assert!(result.is_err());
    }

    #[test]
    fn test_signature_hash() {
        let hex = "1f8b08000000000000030dcfbd4ac34000c071220ea58bdaa9742a41a84bc87d27270e9ab61890c4268d54bb5dee2e26607b508b4a9fa26fe01bf8b128f812be82b383b8161703ffe9bffd1a5bad9d64d1374a77470bb334d2dc7436567d1b1e96540920ec6fabb99da5e7716b5f4a4e58ae91e36b221d8272ed088ca04399a058f8b2a09075f62297909e0b39edb9a0ce05dde79faf8f0d3868048f56c7ce2e86d3b13abb35833089f4f4be2a97ca04554cd8eaa13c9d5ca9d0b6b3315d8d4c9f5c0e83597837884fe6f309ba0e719494328d5995ce90050fe3e671c17c0ab9d2bc904011a031a502f202e414032e19c60c78be209e409aab1cfa9041e603c204821ad588ddd7f5baddfefd7c7aff03e1cbdbd13f2aab0f710f010000";
        let pubkey_str = "xpub6D1AabNHCupeiLM65ZR9UStMhJ1vCpyV4XbZdyhMZBiJXALQtmn9p42VTQckoHVn8WNqS7dqnJokZHAHcHGoaQgmv8D45oNUKx6DZMNZBCd";
        let payload = prepare_payload(hex);
        let context = prepare_parse_context(pubkey_str);
        let tx = WrappedTron::from_payload(payload, &context).unwrap();
        let hash = tx.signature_hash().unwrap();
        assert_eq!(32, hash.len());
    }

    #[test]
    fn test_format_amount_trx() {
        let hex = "1f8b08000000000000030dcfbd4ac34000c071220ea58bdaa9742a41a84bc87d27270e9ab61890c4268d54bb5dee2e26607b508b4a9fa26fe01bf8b128f812be82b383b8161703ffe9bffd1a5bad9d64d1374a77470bb334d2dc7436567d1b1e96540920ec6fabb99da5e7716b5f4a4e58ae91e36b221d8272ed088ca04399a058f8b2a09075f62297909e0b39edb9a0ce05dde79faf8f0d3868048f56c7ce2e86d3b13abb35833089f4f4be2a97ca04554cd8eaa13c9d5ca9d0b6b3315d8d4c9f5c0e83597837884fe6f309ba0e719494328d5995ce90050fe3e671c17c0ab9d2bc904011a031a502f202e414032e19c60c78be209e409aab1cfa9041e603c204821ad588ddd7f5baddfefd7c7aff03e1cbdbd13f2aab0f710f010000";
        let pubkey_str = "xpub6D1AabNHCupeiLM65ZR9UStMhJ1vCpyV4XbZdyhMZBiJXALQtmn9p42VTQckoHVn8WNqS7dqnJokZHAHcHGoaQgmv8D45oNUKx6DZMNZBCd";
        let payload = prepare_payload(hex);
        let context = prepare_parse_context(pubkey_str);
        let tx = WrappedTron::from_payload(payload, &context).unwrap();
        let formatted = tx.format_amount().unwrap();
        assert!(formatted.contains("TRX"));
    }

    #[test]
    fn test_format_method_trx() {
        let hex = "1f8b08000000000000030dcfbd4ac34000c071220ea58bdaa9742a41a84bc87d27270e9ab61890c4268d54bb5dee2e26607b508b4a9fa26fe01bf8b128f812be82b383b8161703ffe9bffd1a5bad9d64d1374a77470bb334d2dc7436567d1b1e96540920ec6fabb99da5e7716b5f4a4e58ae91e36b221d8272ed088ca04399a058f8b2a09075f62297909e0b39edb9a0ce05dde79faf8f0d3868048f56c7ce2e86d3b13abb35833089f4f4be2a97ca04554cd8eaa13c9d5ca9d0b6b3315d8d4c9f5c0e83597837884fe6f309ba0e719494328d5995ce90050fe3e671c17c0ab9d2bc904011a031a502f202e414032e19c60c78be209e409aab1cfa9041e603c204821ad588ddd7f5baddfefd7c7aff03e1cbdbd13f2aab0f710f010000";
        let pubkey_str = "xpub6D1AabNHCupeiLM65ZR9UStMhJ1vCpyV4XbZdyhMZBiJXALQtmn9p42VTQckoHVn8WNqS7dqnJokZHAHcHGoaQgmv8D45oNUKx6DZMNZBCd";
        let payload = prepare_payload(hex);
        let context = prepare_parse_context(pubkey_str);
        let tx = WrappedTron::from_payload(payload, &context).unwrap();
        let method = tx.format_method().unwrap();
        assert_eq!("TRX Transfer", method);
    }

    #[test]
    fn test_format_method_trc20() {
        let hex = "1f8b08000000000000031590bf4ac3501c46359452bba871299d4a102a42c8bffbbbf7c6499b1403b6b14d52b42e92e426b5c53636462979029d7d01477707279f40147c0007df41707130856f3870a6f355387ebd9f1a098b1abd34c99230b9acbf70158eaf1099b4db26368427ae5af29c639bdf0e98a652d50fc4500922110121a21efb548c028010142d8814bdbed995106a4a8a0e4d492e26c98defb78ffb3f79a7dcfa5ae505cf21b6359f4447fdc5a1678ce99c9e0dd1558726999b8f269d09ceea82e7b96408dab58bd23c358deccc1fdf38f97cc114ec6746a40e1c41f05cc87b89814edbada9756bda07b3d9893ab2b46eff22746c3c76a6bb2b6a49d129d9b3abfb3e8be3400335f4090d3506818c303042402f0c669851888160504286502c2b408b001d01f5fd40d6286c3c7f3ed46a773fef45486bab5a1ab8a6c7af2d6f395f62ad6c3dfee2c66bef1f257dc3fe50010000";
        let pubkey_str = "xpub6C3ndD75jvoARyqUBTvrsMZaprs2ZRF84kRTt5r9oxKQXn5oFChRRgrP2J8QhykhKACBLF2HxwAh4wccFqFsuJUBBcwyvkyqfzJU5gfn5pY";
        let payload = prepare_payload(hex);
        let context = prepare_parse_context(pubkey_str);
        let tx = WrappedTron::from_payload(payload, &context).unwrap();
        let method = tx.format_method().unwrap();
        assert_eq!("TRC-20 Transfer", method);
    }

    #[test]
    fn test_format_method_trc10() {
        // Create a transaction with non-empty token but empty contract_address
        let hex = "1f8b08000000000000030dcfbd4ac34000c071220ea58bdaa9742a41a84bc87d27270e9ab61890c4268d54bb5dee2e26607b508b4a9fa26fe01bf8b128f812be82b383b8161703ffe9bffd1a5bad9d64d1374a77470bb334d2dc7436567d1b1e96540920ec6fabb99da5e7716b5f4a4e58ae91e36b221d8272ed088ca04399a058f8b2a09075f62297909e0b39edb9a0ce05dde79faf8f0d3868048f56c7ce2e86d3b13abb35833089f4f4be2a97ca04554cd8eaa13c9d5ca9d0b6b3315d8d4c9f5c0e83597837884fe6f309ba0e719494328d5995ce90050fe3e671c17c0ab9d2bc904011a031a502f202e414032e19c60c78be209e409aab1cfa9041e603c204821ad588ddd7f5baddfefd7c7aff03e1cbdbd13f2aab0f710f010000";
        let pubkey_str = "xpub6D1AabNHCupeiLM65ZR9UStMhJ1vCpyV4XbZdyhMZBiJXALQtmn9p42VTQckoHVn8WNqS7dqnJokZHAHcHGoaQgmv8D45oNUKx6DZMNZBCd";
        let payload = prepare_payload(hex);
        let context = prepare_parse_context(pubkey_str);
        let mut tx = WrappedTron::from_payload(payload, &context).unwrap();
        // Modify to simulate TRC-10
        tx.contract_address = String::new();
        tx.token = "1002000".to_string();
        let method = tx.format_method().unwrap();
        assert_eq!("TRC-10 Transfer", method);
    }

    #[test]
    fn test_format_unit_with_token_short_name() {
        let hex = "1f8b08000000000000030dcfbd4ac34000c071220ea58bdaa9742a41a84bc87d27270e9ab61890c4268d54bb5dee2e26607b508b4a9fa26fe01bf8b128f812be82b383b8161703ffe9bffd1a5bad9d64d1374a77470bb334d2dc7436567d1b1e96540920ec6fabb99da5e7716b5f4a4e58ae91e36b221d8272ed088ca04399a058f8b2a09075f62297909e0b39edb9a0ce05dde79faf8f0d3868048f56c7ce2e86d3b13abb35833089f4f4be2a97ca04554cd8eaa13c9d5ca9d0b6b3315d8d4c9f5c0e83597837884fe6f309ba0e719494328d5995ce90050fe3e671c17c0ab9d2bc904011a031a502f202e414032e19c60c78be209e409aab1cfa9041e603c204821ad588ddd7f5baddfefd7c7aff03e1cbdbd13f2aab0f710f010000";
        let pubkey_str = "xpub6D1AabNHCupeiLM65ZR9UStMhJ1vCpyV4XbZdyhMZBiJXALQtmn9p42VTQckoHVn8WNqS7dqnJokZHAHcHGoaQgmv8D45oNUKx6DZMNZBCd";
        let payload = prepare_payload(hex);
        let context = prepare_parse_context(pubkey_str);
        let mut tx = WrappedTron::from_payload(payload, &context).unwrap();
        tx.token_short_name = Some("USDT".to_string());
        let unit = tx.format_unit().unwrap();
        assert_eq!("USDT", unit);
    }

    #[test]
    fn test_format_unit_without_token_short_name() {
        let hex = "1f8b08000000000000030dcfbd4ac34000c071220ea58bdaa9742a41a84bc87d27270e9ab61890c4268d54bb5dee2e26607b508b4a9fa26fe01bf8b128f812be82b383b8161703ffe9bffd1a5bad9d64d1374a77470bb334d2dc7436567d1b1e96540920ec6fabb99da5e7716b5f4a4e58ae91e36b221d8272ed088ca04399a058f8b2a09075f62297909e0b39edb9a0ce05dde79faf8f0d3868048f56c7ce2e86d3b13abb35833089f4f4be2a97ca04554cd8eaa13c9d5ca9d0b6b3315d8d4c9f5c0e83597837884fe6f309ba0e719494328d5995ce90050fe3e671c17c0ab9d2bc904011a031a502f202e414032e19c60c78be209e409aab1cfa9041e603c204821ad588ddd7f5baddfefd7c7aff03e1cbdbd13f2aab0f710f010000";
        let pubkey_str = "xpub6D1AabNHCupeiLM65ZR9UStMhJ1vCpyV4XbZdyhMZBiJXALQtmn9p42VTQckoHVn8WNqS7dqnJokZHAHcHGoaQgmv8D45oNUKx6DZMNZBCd";
        let payload = prepare_payload(hex);
        let context = prepare_parse_context(pubkey_str);
        let tx = WrappedTron::from_payload(payload, &context).unwrap();
        let unit = tx.format_unit().unwrap();
        assert_eq!("TRX", unit);
    }

    #[test]
    fn test_check_input_invalid_xfp() {
        let hex = "1f8b08000000000000030dcfbd4ac34000c071220ea58bdaa9742a41a84bc87d27270e9ab61890c4268d54bb5dee2e26607b508b4a9fa26fe01bf8b128f812be82b383b8161703ffe9bffd1a5bad9d64d1374a77470bb334d2dc7436567d1b1e96540920ec6fabb99da5e7716b5f4a4e58ae91e36b221d8272ed088ca04399a058f8b2a09075f62297909e0b39edb9a0ce05dde79faf8f0d3868048f56c7ce2e86d3b13abb35833089f4f4be2a97ca04554cd8eaa13c9d5ca9d0b6b3315d8d4c9f5c0e83597837884fe6f309ba0e719494328d5995ce90050fe3e671c17c0ab9d2bc904011a031a502f202e414032e19c60c78be209e409aab1cfa9041e603c204821ad588ddd7f5baddfefd7c7aff03e1cbdbd13f2aab0f710f010000";
        let pubkey_str = "xpub6D1AabNHCupeiLM65ZR9UStMhJ1vCpyV4XbZdyhMZBiJXALQtmn9p42VTQckoHVn8WNqS7dqnJokZHAHcHGoaQgmv8D45oNUKx6DZMNZBCd";
        let payload = prepare_payload(hex);
        let context = prepare_parse_context(pubkey_str);
        let tx = WrappedTron::from_payload(payload, &context).unwrap();
        // Create context with different fingerprint
        let wrong_fp = Fingerprint::from_str("00000000").unwrap();
        let wrong_context = keystone::ParseContext::new(wrong_fp, context.extended_public_key);
        let result = tx.check_input(&wrong_context);
        assert!(result.is_err());
        assert!(matches!(
            result.unwrap_err(),
            TronError::InvalidParseContext(_)
        ));
    }

    #[test]
    fn test_format_amount_small_values() {
        let hex = "1f8b08000000000000030dcfbd4ac34000c071220ea58bdaa9742a41a84bc87d27270e9ab61890c4268d54bb5dee2e26607b508b4a9fa26fe01bf8b128f812be82b383b8161703ffe9bffd1a5bad9d64d1374a77470bb334d2dc7436567d1b1e96540920ec6fabb99da5e7716b5f4a4e58ae91e36b221d8272ed088ca04399a058f8b2a09075f62297909e0b39edb9a0ce05dde79faf8f0d3868048f56c7ce2e86d3b13abb35833089f4f4be2a97ca04554cd8eaa13c9d5ca9d0b6b3315d8d4c9f5c0e83597837884fe6f309ba0e719494328d5995ce90050fe3e671c17c0ab9d2bc904011a031a502f202e414032e19c60c78be209e409aab1cfa9041e603c204821ad588ddd7f5baddfefd7c7aff03e1cbdbd13f2aab0f710f010000";
        let pubkey_str = "xpub6D1AabNHCupeiLM65ZR9UStMhJ1vCpyV4XbZdyhMZBiJXALQtmn9p42VTQckoHVn8WNqS7dqnJokZHAHcHGoaQgmv8D45oNUKx6DZMNZBCd";
        let payload = prepare_payload(hex);
        let context = prepare_parse_context(pubkey_str);
        let mut tx = WrappedTron::from_payload(payload, &context).unwrap();

        // Test with value 1 (smallest non-zero)
        tx.value = "1".to_string();
        let formatted = tx.format_amount().unwrap();
        assert!(formatted.contains("0.000001"));

        // Test with zero value
        tx.value = "0".to_string();
        let formatted = tx.format_amount().unwrap();
        assert!(formatted.starts_with("0"));
    }

    #[test]
    fn test_format_amount_various_decimals() {
        let hex = "1f8b08000000000000031590bf4ac3501c46359452bba871299d4a102a42c8bffbbbf7c6499b1403b6b14d52b42e92e426b5c53636462979029d7d01477707279f40147c0007df41707130856f3870a6f355387ebd9f1a098b1abd34c99230b9acbf70158eaf1099b4db26368427ae5af29c639bdf0e98a652d50fc4500922110121a21efb548c028010142d8814bdbed995106a4a8a0e4d492e26c98defb78ffb3f79a7dcfa5ae505cf21b6359f4447fdc5a1678ce99c9e0dd1558726999b8f269d09ceea82e7b96408dab58bd23c358deccc1fdf38f97cc114ec6746a40e1c41f05cc87b89814edbada9756bda07b3d9893ab2b46eff22746c3c76a6bb2b6a49d129d9b3abfb3e8be3400335f4090d3506818c303042402f0c669851888160504286502c2b408b001d01f5fd40d6286c3c7f3ed46a773fef45486bab5a1ab8a6c7af2d6f395f62ad6c3dfee2c66bef1f257dc3fe50010000";
        let pubkey_str = "xpub6C3ndD75jvoARyqUBTvrsMZaprs2ZRF84kRTt5r9oxKQXn5oFChRRgrP2J8QhykhKACBLF2HxwAh4wccFqFsuJUBBcwyvkyqfzJU5gfn5pY";
        let payload = prepare_payload(hex);
        let context = prepare_parse_context(pubkey_str);
        let mut tx = WrappedTron::from_payload(payload, &context).unwrap();

        // Test 18 decimals (like TUSD)
        tx.divider = 1_000_000_000_000_000_000.0;
        tx.value = "1000000000000000000".to_string();
        let formatted = tx.format_amount().unwrap();
        assert!(formatted.contains("1"));

        // Test 8 decimals (like WBTC)
        tx.divider = 100_000_000.0;
        tx.value = "100000000".to_string();
        let formatted = tx.format_amount().unwrap();
        assert!(formatted.contains("1"));
    }

    #[test]
    fn test_from_raw_transaction_unknown_contract_type() {
        let mut tron_tx = Transaction::default();
        let mut raw = transaction::Raw::default();
        let mut contract = transaction::Contract::default();
        contract.r#type = 99; // Unknown contract type
        contract.parameter = None; // No parameter for unknown type
        raw.contract = vec![contract];
        tron_tx.raw_data = Some(raw);

        let result = WrappedTron::from_raw_transaction(tron_tx, "m/44'/195'/0'/0/0".to_string());
        assert!(result.is_ok());
        let wrapped = result.unwrap();
        assert_eq!(wrapped.from, "");
        assert_eq!(wrapped.to, "");
    }

    #[test]
    fn test_from_raw_transaction_trc20_short_data() {
        let mut tron_tx = Transaction::default();
        let mut raw = transaction::Raw::default();
        let mut contract = transaction::Contract::default();
        contract.r#type = 31; // TriggerSmartContract

        let mut trigger_contract = TriggerSmartContract::default();
        trigger_contract.owner_address = vec![0x41; 21];
        trigger_contract.contract_address = vec![0x41; 21];
        trigger_contract.data = vec![0xa9, 0x05, 0x9c, 0xbb]; // transfer selector but data too short

        contract.parameter = Some(prost_types::Any {
            type_url: "type.googleapis.com/protocol.TriggerSmartContract".to_string(),
            value: trigger_contract.encode_to_vec(),
        });
        raw.contract = vec![contract];
        tron_tx.raw_data = Some(raw);

        let result = WrappedTron::from_raw_transaction(tron_tx, "m/44'/195'/0'/0/0".to_string());
        assert!(result.is_ok());
        let wrapped = result.unwrap();
        // Should not parse TRC-20 data if it's too short
        assert_eq!(wrapped.value, "0");
    }

    #[test]
    fn test_from_raw_transaction_trc20_wrong_selector() {
        let mut tron_tx = Transaction::default();
        let mut raw = transaction::Raw::default();
        let mut contract = transaction::Contract::default();
        contract.r#type = 31;

        let mut trigger_contract = TriggerSmartContract::default();
        trigger_contract.owner_address = vec![0x41; 21];
        trigger_contract.contract_address = vec![0x41; 21];
        trigger_contract.data = vec![0xff; 68]; // Wrong selector, but correct length

        contract.parameter = Some(prost_types::Any {
            type_url: "type.googleapis.com/protocol.TriggerSmartContract".to_string(),
            value: trigger_contract.encode_to_vec(),
        });
        raw.contract = vec![contract];
        tron_tx.raw_data = Some(raw);

        let result = WrappedTron::from_raw_transaction(tron_tx, "m/44'/195'/0'/0/0".to_string());
        assert!(result.is_ok());
        let wrapped = result.unwrap();
        assert_eq!(wrapped.value, "0");
    }

    #[test]
    fn test_from_raw_transaction_no_contract() {
        let mut tron_tx = Transaction::default();
        let mut raw = transaction::Raw::default();
        raw.contract = vec![]; // No contracts
        tron_tx.raw_data = Some(raw);

        let result = WrappedTron::from_raw_transaction(tron_tx, "m/44'/195'/0'/0/0".to_string());
        assert!(result.is_ok());
        let wrapped = result.unwrap();
        assert_eq!(wrapped.from, "");
        assert_eq!(wrapped.value, "0");
    }

    #[test]
    fn test_format_amount_edge_cases() {
        let hex = "1f8b08000000000000030dcfbd4ac34000c071220ea58bdaa9742a41a84bc87d27270e9ab61890c4268d54bb5dee2e26607b508b4a9fa26fe01bf8b128f812be82b383b8161703ffe9bffd1a5bad9d64d1374a77470bb334d2dc7436567d1b1e96540920ec6fabb99da5e7716b5f4a4e58ae91e36b221d8272ed088ca04399a058f8b2a09075f62297909e0b39edb9a0ce05dde79faf8f0d3868048f56c7ce2e86d3b13abb35833089f4f4be2a97ca04554cd8eaa13c9d5ca9d0b6b3315d8d4c9f5c0e83597837884fe6f309ba0e719494328d5995ce90050fe3e671c17c0ab9d2bc904011a031a502f202e414032e19c60c78be209e409aab1cfa9041e603c204821ad588ddd7f5baddfefd7c7aff03e1cbdbd13f2aab0f710f010000";
        let pubkey_str = "xpub6D1AabNHCupeiLM65ZR9UStMhJ1vCpyV4XbZdyhMZBiJXALQtmn9p42VTQckoHVn8WNqS7dqnJokZHAHcHGoaQgmv8D45oNUKx6DZMNZBCd";
        let payload = prepare_payload(hex);
        let context = prepare_parse_context(pubkey_str);
        let mut tx = WrappedTron::from_payload(payload, &context).unwrap();

        // Test with custom divider (not 6 or 18)
        tx.divider = 1_000.0; // 3 decimals
        tx.value = "1000".to_string();
        let formatted = tx.format_amount().unwrap();
        assert!(formatted.contains("1"));

        // Test with divider = 1 (no decimals)
        tx.divider = 1.0;
        tx.value = "100".to_string();
        let formatted = tx.format_amount().unwrap();
        assert!(formatted.contains("100"));
    }

    #[test]
    fn test_payload_with_memo() {
        let hex = "1f8b080000000000000365ce3f4fc2400005f0508d121695c93091c60463527bff7ad76b6222b4481310150e81c9f47a2501954b8988f11b39b93b38b96af8087e020775711357fce54def2d2f6be437ce26be5649f174a26f74acaf0aaf46d6c8671de824d82591f96ce45645eba499dfe112aa44126561c9944564a42c2e95b290048a49ac62495561ebda26a46443ee946cb0880d8a8f1fef4f3f6077adf295c9ad8856af608ad630ad77e96567380b6f1b32ad55d376ea87a1821d3e9d0d6aba6c9ae27caccb7d41fab3a847184cfc6697dc4f477d5e81ba4ebaf120dc6b1c785511ee77da81f0c01d238862aadca34a807ca80286602029e492f0240a9043b0eb048c290fbb9042006c6c03cfbab0c7b1871637d1faa2fbe31de70ec1321e0f8044903246204ee27ffb92cdefcfccf67cfef2f0867f012b09fb6163010000";
        let pubkey_str = "xpub6D1AabNHCupeiLM65ZR9UStMhJ1vCpyV4XbZdyhMZBiJXALQtmn9p42VTQckoHVn8WNqS7dqnJokZHAHcHGoaQgmv8D45oNUKx6DZMNZBCd";
        let payload = prepare_payload(hex);
        let context = prepare_parse_context(pubkey_str);
        let tx = WrappedTron::from_payload(payload, &context).unwrap();
        assert_eq!(
            "=:ETH.USDT:0x742636d8FBD2C1dD721Db619b49eaD254385D77d:3816100/3/0:-_/nc:20/0",
            tx.memo
        );
        let raw_data = tx.tron_tx.raw_data.as_ref().expect("raw_data should exist");
        assert_eq!(tx.memo.as_bytes().to_vec(), raw_data.data);
    }
}
