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
        Ok(format!("{} {}", value.div(self.divider), unit))
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

pub const DIVIDER: f64 = 1000000_f64;
pub const NETWORK: &str = "TRON";

#[cfg(test)]
mod tests {
    extern crate std;
    use super::*;
    use crate::test::{prepare_parse_context, prepare_payload};
    use alloc::string::ToString;
    use bitcoin::bip32::Fingerprint;
    use core::str::FromStr;

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
}
