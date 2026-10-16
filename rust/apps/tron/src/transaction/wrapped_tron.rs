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
    pub(crate) method: String,
    pub(crate) token_short_name: Option<String>,
    pub(crate) decimals: u8,
    pub(crate) fee_limit: u64,
    pub(crate) memo: String,
    pub(crate) memo_hex: String,
    pub(crate) memo_hash: String,
    pub(crate) timestamp: String,
    pub(crate) expiration: String,
    pub(crate) ref_block_num: String,
    pub(crate) ref_block_bytes: String,
    pub(crate) ref_block_hash: String,
    pub(crate) permission_id: String,
    pub(crate) raw_data_hash: String,
    pub(crate) call_value: String,
    pub(crate) call_token_value: String,
    pub(crate) token_id: String,
    pub(crate) call_data: String,
    pub(crate) contract_warning: String,
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

const KNOWN_TOKENS: &[(&str, &str, u8)] = &[
    ("TR7NHqjeKQxGTCi8q8ZY4pL8otSzgjLj6t", "USDT", 6),
    ("TEkxiTehnzSmSe2XqrBj4w32RUN966rdz8", "USDC", 6),
    ("TUpMhErZL2fhh4sVNULAbNKLokS4GjC1F4", "TUSD", 18),
    ("TAFjULxiVgT4qWk6UZwjqwZXTSaGaqnVp4", "BTT", 18),
    ("TCFLL5dx5ZJdKnWuesXxi1VPwjLVmWZZy9", "JST", 18),
    ("TFczxzPhnThNSqr5by8tvxsdCFRRz6cPNq", "NFT", 6),
    ("TSSMHYeV2uE9qYH95DqyoCuNCzEL1NvU3S", "SUN", 18),
    ("TYhWwKpw43ENFWBTGpzLHn3882f2au7SMi", "WBTC", 8),
    ("TNUC9Qb1rRpS5CbWLmNMxXBjyFoydXjWFR", "WTRX", 6),
    ("TLa2f6VPqDgRE67v1736s7bJ8Ray5wYjU7", "WIN", 6),
    ("TXDk8mbtRbXeYuMNS83CfKPaYYT8XWv9Hz", "USDD", 18),
];

fn known_token_metadata(contract_address: &str) -> Option<(&'static str, u8)> {
    KNOWN_TOKENS
        .iter()
        .find(|token| token.0 == contract_address)
        .map(|token| (token.1, token.2))
}

fn invalid_raw(message: &str) -> TronError {
    TronError::InvalidRawTxCryptoBytes(message.to_string())
}

fn validate_address_bytes(address: &[u8], field: &str) -> Result<()> {
    if address.len() != 21 || address[0] != 0x41 {
        return Err(invalid_raw(&format!("invalid {field} address")));
    }
    Ok(())
}

fn format_decimal_units(raw: &str, decimals: u8) -> Result<String> {
    if raw.is_empty() || !raw.bytes().all(|b| b.is_ascii_digit()) {
        return Err(invalid_raw("amount is not an unsigned decimal integer"));
    }
    let normalized = raw.trim_start_matches('0');
    let digits = if normalized.is_empty() {
        "0"
    } else {
        normalized
    };
    if decimals == 0 {
        return Ok(digits.to_string());
    }
    let decimals = decimals as usize;
    let mut formatted = if digits.len() <= decimals {
        format!("0.{:0>width$}", digits, width = decimals)
    } else {
        let split = digits.len() - decimals;
        format!("{}.{}", &digits[..split], &digits[split..])
    };
    while formatted.ends_with('0') {
        formatted.pop();
    }
    if formatted.ends_with('.') {
        formatted.pop();
    }
    Ok(formatted)
}

impl WrappedTron {
    pub fn from_raw_transaction(raw_tx: Transaction, path: String) -> Result<Self> {
        if let Some(raw) = raw_tx.raw_data.as_ref() {
            crate::validate_sign_data_len(raw.encoded_len())?;
            if !raw.auths.is_empty() || !raw.scripts.is_empty() {
                return Err(invalid_raw("unsupported TRON auths or scripts"));
            }
            if raw.ref_block_num < 0 {
                return Err(invalid_raw("negative ref block number"));
            }
        }
        let path_parts = path.split('/').collect::<Vec<&str>>();
        crate::check_hd_path!(path_parts)?;
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
            method: String::new(),
            decimals: DEFAULT_DECIMALS,
            token_short_name: None,
            fee_limit: 0,
            memo: String::new(),
            memo_hex: String::new(),
            memo_hash: String::new(),
            timestamp: String::new(),
            expiration: String::new(),
            ref_block_num: String::new(),
            ref_block_bytes: String::new(),
            ref_block_hash: String::new(),
            permission_id: String::new(),
            raw_data_hash: String::new(),
            call_value: String::new(),
            call_token_value: String::new(),
            token_id: String::new(),
            call_data: String::new(),
            contract_warning: String::new(),
        };

        let raw = instance
            .tron_tx
            .raw_data
            .as_ref()
            .ok_or_else(|| invalid_raw("empty raw data"))?;
        if raw.contract.len() != 1 {
            return Err(invalid_raw("exactly one contract is required"));
        }
        if raw.ref_block_bytes.len() != 2 || raw.ref_block_hash.len() != 8 {
            return Err(invalid_raw("invalid ref block fields"));
        }
        if raw.timestamp <= 0 || raw.expiration <= raw.timestamp || raw.fee_limit < 0 {
            return Err(invalid_raw("invalid timestamp, expiration, or fee limit"));
        }

        instance.fee_limit = raw.fee_limit as u64;
        instance.memo = String::from_utf8(raw.data.clone())
            .ok()
            .filter(|memo| !memo.contains('\0'))
            .unwrap_or_default();
        instance.memo_hex = hex::encode(&raw.data);
        instance.memo_hash = hex::encode(hashing::sha256(&raw.data));
        instance.timestamp = raw.timestamp.to_string();
        instance.expiration = raw.expiration.to_string();
        instance.ref_block_num = raw.ref_block_num.to_string();
        instance.ref_block_bytes = hex::encode(&raw.ref_block_bytes);
        instance.ref_block_hash = hex::encode(&raw.ref_block_hash);
        let raw_bytes = raw.encode_to_vec();
        instance.raw_data_hash = hex::encode(hashing::sha256(&raw_bytes));

        let contract = &raw.contract[0];
        if !contract.provider.is_empty()
            || !contract.contract_name.is_empty()
            || contract.permission_id < 0
        {
            return Err(invalid_raw("unsupported contract metadata"));
        }
        instance.permission_id = contract.permission_id.to_string();
        let param = contract
            .parameter
            .as_ref()
            .ok_or_else(|| invalid_raw("missing contract parameter"))?;
        use crate::pb::protocol::transaction::contract::ContractType;
        let c_type = ContractType::from_i32(contract.r#type)
            .ok_or_else(|| invalid_raw("unsupported contract type"))?;

        match c_type {
            ContractType::TransferContract => {
                if param.type_url != "type.googleapis.com/protocol.TransferContract" {
                    return Err(invalid_raw("TransferContract type_url mismatch"));
                }
                let ct = TransferContract::decode(param.value.as_slice())
                    .map_err(|_| invalid_raw("TransferContract decode failed"))?;
                if ct.encode_to_vec() != param.value {
                    return Err(invalid_raw("non-canonical TransferContract encoding"));
                }
                validate_address_bytes(&ct.owner_address, "owner")?;
                validate_address_bytes(&ct.to_address, "recipient")?;
                if ct.amount < 0 {
                    return Err(invalid_raw("negative transfer amount"));
                }
                instance.from = bitcoin::base58::encode_check(&ct.owner_address);
                instance.to = bitcoin::base58::encode_check(&ct.to_address);
                instance.value = ct.amount.to_string();
                instance.token = String::new();
                instance.decimals = DEFAULT_DECIMALS;
            }
            ContractType::TriggerSmartContract => {
                if param.type_url != "type.googleapis.com/protocol.TriggerSmartContract" {
                    return Err(invalid_raw("TriggerSmartContract type_url mismatch"));
                }
                let ct = TriggerSmartContract::decode(param.value.as_slice())
                    .map_err(|_| invalid_raw("TriggerSmartContract decode failed"))?;
                if ct.encode_to_vec() != param.value {
                    return Err(invalid_raw("non-canonical TriggerSmartContract encoding"));
                }
                validate_address_bytes(&ct.owner_address, "owner")?;
                validate_address_bytes(&ct.contract_address, "contract")?;
                if ct.call_value < 0 || ct.call_token_value < 0 || ct.token_id < 0 {
                    return Err(invalid_raw("negative smart contract value fields"));
                }
                if ct.call_token_value > 0 && ct.token_id == 0 {
                    return Err(invalid_raw("missing attached TRC-10 token ID"));
                }
                instance.from = bitcoin::base58::encode_check(&ct.owner_address);
                instance.contract_address = bitcoin::base58::encode_check(&ct.contract_address);
                instance.call_value = format!(
                    "{} TRX",
                    format_decimal_units(&ct.call_value.to_string(), DEFAULT_DECIMALS)?
                );
                if ct.token_id != 0 {
                    instance.call_token_value = ct.call_token_value.to_string();
                    instance.token_id = ct.token_id.to_string();
                }
                instance.call_data = format!("0x{}", hex::encode(&ct.data));
                let selector = ct.data.get(..4);
                let transfer = selector == Some(&[0xa9, 0x05, 0x9c, 0xbb][..]);
                let approve = selector == Some(&[0x09, 0x5e, 0xa7, 0xb3][..]);
                if transfer || approve {
                    if ct.data.len() != 68 || ct.data[4..16].iter().any(|byte| *byte != 0) {
                        return Err(invalid_raw("non-canonical TRC-20 calldata"));
                    }
                    instance.method = if transfer {
                        "TRC-20 Transfer"
                    } else {
                        "TRC-20 Approve"
                    }
                    .to_string();
                    let mut destination = vec![0x41u8];
                    destination.extend_from_slice(&ct.data[16..36]);
                    instance.to = bitcoin::base58::encode_check(&destination);
                    instance.value = U256::from_big_endian(&ct.data[36..68]).to_string();
                    if let Some((token, decimals)) =
                        known_token_metadata(&instance.contract_address)
                    {
                        instance.token = token.to_string();
                        instance.decimals = decimals;
                    } else {
                        instance.token = "TRC20 Token (raw)".to_string();
                        instance.decimals = 0;
                    }
                } else {
                    instance.method = "Contract Call".to_string();
                    instance.to = instance.contract_address.clone();
                    instance.value = ct.call_value.to_string();
                    instance.contract_warning = "Unrecognized contract call. The device cannot determine its effects. Verify the contract, attached assets and call data before signing.".to_string();
                }
            }
            ContractType::TransferAssetContract => {
                if param.type_url != "type.googleapis.com/protocol.TransferAssetContract" {
                    return Err(invalid_raw("TransferAssetContract type_url mismatch"));
                }
                let ct = TransferAssetContract::decode(param.value.as_slice())
                    .map_err(|_| invalid_raw("TransferAssetContract decode failed"))?;
                if ct.encode_to_vec() != param.value {
                    return Err(invalid_raw("non-canonical TransferAssetContract encoding"));
                }
                validate_address_bytes(&ct.owner_address, "owner")?;
                validate_address_bytes(&ct.to_address, "recipient")?;
                if ct.amount < 0 {
                    return Err(invalid_raw("negative asset transfer amount"));
                }
                instance.from = bitcoin::base58::encode_check(&ct.owner_address);
                instance.to = bitcoin::base58::encode_check(&ct.to_address);
                instance.value = ct.amount.to_string();
                instance.token = String::from_utf8(ct.asset_name)
                    .map_err(|_| invalid_raw("invalid TRC-10 asset id"))?;
                if instance.token.is_empty() || !instance.token.bytes().all(|b| b.is_ascii_digit())
                {
                    return Err(invalid_raw("invalid TRC-10 asset id"));
                }
                instance.decimals = DEFAULT_DECIMALS;
            }
            _ => return Err(invalid_raw("unsupported contract type")),
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
        if latest_block.number < 0 || latest_block.timestamp <= 0 {
            return Err(invalid_raw("invalid latest block number or timestamp"));
        }
        let number_buf = (latest_block.number as u64).to_be_bytes();
        let hash_buf = hex::decode(&latest_block.hash)
            .map_err(|_| invalid_raw("invalid latest block hash"))?;
        if hash_buf.len() != 32 {
            return Err(invalid_raw("latest block hash must be 32 bytes"));
        }
        let expiration = latest_block
            .timestamp
            .checked_add(600 * 5 * 1000)
            .ok_or_else(|| invalid_raw("latest block timestamp overflow"))?;
        let mut raw_data = tx
            .raw_data
            .clone()
            .ok_or_else(|| invalid_raw("empty raw data"))?;
        raw_data.ref_block_hash = hash_buf[8..16].to_vec();
        raw_data.ref_block_bytes = number_buf[6..8].to_vec();
        raw_data.timestamp = latest_block.timestamp;
        raw_data.expiration = expiration;
        if is_trc_20 {
            raw_data.fee_limit = 1_000_000_000;
        }
        tx.raw_data = Some(raw_data);
        Ok(tx.clone())
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
        if value < 0 {
            return Err(invalid_raw("negative transfer value"));
        }
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
        if content.coin_code.to_uppercase() != "TRON" {
            return Err(invalid_raw("invalid coin code"));
        }
        match tx {
            TronTx(tx) => {
                crate::validate_sign_data_len(tx.encoded_len())?;
                let mut tron_tx = if tx.contract_address.is_empty() {
                    Self::build_transfer_tx(tx)
                } else {
                    Self::generate_trc20_tx(tx)
                }?;
                if let Some(ref mut raw) = tron_tx.raw_data {
                    raw.data = tx.memo.as_bytes().to_vec();
                }
                let mut wrapped = Self::from_raw_transaction(tron_tx, content.hd_path)?;
                wrapped.extended_pubkey = context.extended_public_key.to_string();
                wrapped.xfp = payload.xfp;
                Ok(wrapped)
            }
            _ => Err(invalid_raw("invalid transaction type")),
        }
    }

    pub fn format_amount(&self) -> Result<String> {
        let amount = format_decimal_units(&self.value, self.decimals)?;
        Ok(format!("{} {}", amount, self.format_unit()?))
    }

    pub fn format_method(&self) -> Result<String> {
        if !self.method.is_empty() {
            return Ok(self.method.clone());
        }
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
        if self.token == "1002000" {
            Ok("BTT".to_string())
        } else if !self.token.is_empty() {
            Ok(self.token.clone())
        } else {
            Ok("TRX".to_string())
        }
    }
}

pub const DEFAULT_DECIMALS: u8 = 6;
pub const NETWORK: &str = "Tron Mainnet";

#[cfg(test)]
mod tests {
    extern crate std;
    use super::*;
    use crate::test::{prepare_parse_context, prepare_payload};
    use crate::transaction::parser::TxParser;
    use alloc::string::ToString;
    use bitcoin::bip32::Fingerprint;
    use core::str::FromStr;
    use ur_registry::pb::protoc::{payload, Payload, SignMessage};

    fn valid_trigger_transaction(data: Vec<u8>) -> Transaction {
        let trigger = TriggerSmartContract {
            owner_address: [vec![0x41], vec![0x11; 20]].concat(),
            contract_address: bitcoin::base58::decode_check("TR7NHqjeKQxGTCi8q8ZY4pL8otSzgjLj6t")
                .unwrap(),
            data,
            ..Default::default()
        };
        Transaction {
            raw_data: Some(transaction::Raw {
                ref_block_bytes: vec![0, 1],
                ref_block_hash: vec![2; 8],
                expiration: 2_000,
                contract: vec![transaction::Contract {
                    r#type: transaction::contract::ContractType::TriggerSmartContract as i32,
                    parameter: Some(Any {
                        type_url: "type.googleapis.com/protocol.TriggerSmartContract".to_string(),
                        value: trigger.encode_to_vec(),
                    }),
                    ..Default::default()
                }],
                timestamp: 1_000,
                ..Default::default()
            }),
            ..Default::default()
        }
    }

    fn canonical_transfer_calldata() -> Vec<u8> {
        let mut data = vec![0xa9, 0x05, 0x9c, 0xbb];
        data.extend_from_slice(&[0; 12]);
        data.extend_from_slice(&[0x22; 20]);
        data.extend_from_slice(&[0; 31]);
        data.push(1);
        data
    }

    fn review_limit_raw(data_len: usize, memo_len: usize) -> transaction::Raw {
        let mut tx = valid_trigger_transaction(vec![0xab; data_len]);
        let raw = tx.raw_data.as_mut().unwrap();
        raw.data = vec![b'a'; memo_len];
        let param = raw.contract[0].parameter.as_mut().unwrap();
        let mut ct = TriggerSmartContract::decode(param.value.as_slice()).unwrap();
        ct.owner_address =
            bitcoin::base58::decode_check("TUEZSdKsoDHQMeZwihtdoBiN46zxhGWYdH").unwrap();
        param.value = ct.encode_to_vec();
        tx.raw_data.unwrap()
    }

    fn assert_raw_rejected(raw: transaction::Raw, reason: &str) {
        let bytes = raw.encode_to_vec();
        let path = "m/44'/195'/0'/0/0".to_string();
        let xpub = "xpub6D1AabNHCupeiLM65ZR9UStMhJ1vCpyV4XbZdyhMZBiJXALQtmn9p42VTQckoHVn8WNqS7dqnJokZHAHcHGoaQgmv8D45oNUKx6DZMNZBCd";
        let seed = hex::decode("5eb00bbddcf069084889a8ab9155568165f5c453ccb85e70811aaed6f6da5fc19a5ac40b389cd370d086206dec8aa6c43daea6690f20ad3d8d48b2d2ce9e38e4").unwrap();
        assert!(crate::parse_tx_request(&bytes, &path)
            .unwrap_err()
            .to_string()
            .contains(reason));
        assert!(crate::check_tx_request(&bytes, &path, xpub)
            .unwrap_err()
            .to_string()
            .contains(reason));
        assert!(crate::sign_tx_request(&bytes, &path, &seed)
            .unwrap_err()
            .to_string()
            .contains(reason));
    }

    #[test]
    fn bounds_total_raw_bytes_including_memo_and_calldata() {
        let path = "m/44'/195'/0'/0/0".to_string();
        let raw = review_limit_raw(9 * 1024, 2 * 1024);
        let parsed = crate::parse_tx_request(&raw.encode_to_vec(), &path).unwrap();
        assert_eq!(parsed.detail.memo_hex.len(), 4 * 1024);
        assert_eq!(parsed.detail.call_data.len(), 2 + 18 * 1024);
        let mut raw = review_limit_raw(4, crate::MAX_SIGN_DATA_LEN - 256);
        while raw.encoded_len() < crate::MAX_SIGN_DATA_LEN {
            raw.data.push(b'a');
        }
        assert_eq!(raw.encoded_len(), crate::MAX_SIGN_DATA_LEN);
        assert!(crate::parse_tx_request(&raw.encode_to_vec(), &path).is_ok());
        let xpub = "xpub6D1AabNHCupeiLM65ZR9UStMhJ1vCpyV4XbZdyhMZBiJXALQtmn9p42VTQckoHVn8WNqS7dqnJokZHAHcHGoaQgmv8D45oNUKx6DZMNZBCd";
        let seed = hex::decode("5eb00bbddcf069084889a8ab9155568165f5c453ccb85e70811aaed6f6da5fc19a5ac40b389cd370d086206dec8aa6c43daea6690f20ad3d8d48b2d2ce9e38e4").unwrap();
        crate::check_tx_request(&raw.encode_to_vec(), &path, xpub).unwrap();
        crate::sign_tx_request(&raw.encode_to_vec(), &path, &seed).unwrap();
        raw.data.push(b'a');
        assert_raw_rejected(raw, "request exceeds");
        assert_raw_rejected(
            review_limit_raw(crate::MAX_SIGN_DATA_LEN, 0),
            "request exceeds",
        );
    }

    #[test]
    fn rejects_unreviewed_raw_auths_and_scripts() {
        let mut raw = review_limit_raw(4, 0);
        raw.auths.push(Default::default());
        assert_raw_rejected(raw, "unsupported TRON auths or scripts");
        let mut raw = review_limit_raw(4, 0);
        raw.scripts.push(1);
        assert_raw_rejected(raw, "unsupported TRON auths or scripts");
        let mut raw = review_limit_raw(4, 0);
        raw.ref_block_num = -1;
        assert_raw_rejected(raw, "negative ref block number");
    }

    #[test]
    fn displays_signed_ref_block_number_without_precision_loss() {
        let mut raw = review_limit_raw(4, 0);
        raw.ref_block_num = i64::MAX;
        let bytes = raw.encode_to_vec();
        let path = "m/44'/195'/0'/0/0".to_string();
        let parsed = crate::parse_tx_request(&bytes, &path).unwrap();
        assert_eq!(parsed.detail.ref_block_num, "9223372036854775807");
        assert_eq!(
            parsed.detail.raw_data_hash,
            hex::encode(hashing::sha256(&bytes))
        );
        let seed = hex::decode("5eb00bbddcf069084889a8ab9155568165f5c453ccb85e70811aaed6f6da5fc19a5ac40b389cd370d086206dec8aa6c43daea6690f20ad3d8d48b2d2ce9e38e4").unwrap();
        assert!(crate::sign_tx_request(&bytes, &path, &seed).is_ok());
    }

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
            method: "".to_string(),
            token_short_name: None,
            decimals: 0,
            fee_limit: 0,
            memo: String::new(),
            memo_hex: String::new(),
            memo_hash: String::new(),
            timestamp: String::new(),
            expiration: String::new(),
            ref_block_num: String::new(),
            ref_block_bytes: String::new(),
            ref_block_hash: String::new(),
            permission_id: String::new(),
            raw_data_hash: String::new(),
            call_value: String::new(),
            call_token_value: String::new(),
            token_id: String::new(),
            call_data: String::new(),
            contract_warning: String::new(),
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
            method: "".to_string(),
            token_short_name: None,
            decimals: 0,
            fee_limit: 0,
            memo: String::new(),
            memo_hex: String::new(),
            memo_hash: String::new(),
            timestamp: String::new(),
            expiration: String::new(),
            ref_block_num: String::new(),
            ref_block_bytes: String::new(),
            ref_block_hash: String::new(),
            permission_id: String::new(),
            raw_data_hash: String::new(),
            call_value: String::new(),
            call_token_value: String::new(),
            token_id: String::new(),
            call_data: String::new(),
            contract_warning: String::new(),
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
        tx.decimals = 18;
        tx.value = "1000000000000000000".to_string();
        let formatted = tx.format_amount().unwrap();
        assert!(formatted.contains("1"));

        // Test 8 decimals (like WBTC)
        tx.decimals = 8;
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
        assert!(result.is_err());
    }

    #[test]
    fn test_from_raw_transaction_rejects_multiple_contracts() {
        let mut tron_tx = Transaction::default();
        let mut raw = transaction::Raw::default();
        raw.contract = vec![
            transaction::Contract::default(),
            transaction::Contract::default(),
        ];
        tron_tx.raw_data = Some(raw);

        let result = WrappedTron::from_raw_transaction(tron_tx, "m/44'/195'/0'/0/0".to_string());
        assert!(result.is_err());
    }

    #[test]
    fn test_from_raw_transaction_decodes_trc20_approve() {
        let owner = [vec![0x41], vec![0x11; 20]].concat();
        let spender = [vec![0x41], vec![0x22; 20]].concat();
        let token = bitcoin::base58::decode_check("TR7NHqjeKQxGTCi8q8ZY4pL8otSzgjLj6t").unwrap();
        let mut data = vec![0x09, 0x5e, 0xa7, 0xb3];
        data.extend_from_slice(&[0u8; 12]);
        data.extend_from_slice(&spender[1..]);
        data.extend_from_slice(&[0u8; 29]);
        data.extend_from_slice(&[0x0f, 0x42, 0x40]);

        let trigger = TriggerSmartContract {
            owner_address: owner,
            contract_address: token,
            call_value: 0,
            data,
            call_token_value: 0,
            token_id: 0,
        };
        let contract = transaction::Contract {
            r#type: 31,
            parameter: Some(Any {
                type_url: "type.googleapis.com/protocol.TriggerSmartContract".to_string(),
                value: trigger.encode_to_vec(),
            }),
            ..Default::default()
        };
        let tx = Transaction {
            raw_data: Some(transaction::Raw {
                ref_block_bytes: vec![0x00, 0x01],
                ref_block_hash: vec![0x22; 8],
                expiration: 2_000,
                contract: vec![contract],
                timestamp: 1_000,
                ..Default::default()
            }),
            ..Default::default()
        };

        let parsed = WrappedTron::from_raw_transaction(tx.clone(), "m/44'/195'/0'/0/0".to_string())
            .unwrap()
            .parse()
            .unwrap();
        assert_eq!("TRC-20 Approve", parsed.detail.method);
        assert_eq!("1 USDT", parsed.detail.value);
        assert_eq!(bitcoin::base58::encode_check(&spender), parsed.detail.to);

        let mut non_canonical = tx;
        non_canonical.raw_data.as_mut().unwrap().contract[0]
            .parameter
            .as_mut()
            .unwrap()
            .value
            .extend_from_slice(&[0x80, 0x06, 0x01]);
        assert!(
            WrappedTron::from_raw_transaction(non_canonical, "m/44'/195'/0'/0/0".to_string(),)
                .is_err()
        );
    }

    #[test]
    fn test_from_payload_ignores_untrusted_token_override() {
        let context = prepare_parse_context(
            "xpub6D1AabNHCupeiLM65ZR9UStMhJ1vCpyV4XbZdyhMZBiJXALQtmn9p42VTQckoHVn8WNqS7dqnJokZHAHcHGoaQgmv8D45oNUKx6DZMNZBCd",
        );
        let payload = Payload {
            r#type: payload::Type::SignTx as i32,
            xfp: "73c5da0a".to_string(),
            content: Some(payload::Content::SignTx(SignTransaction {
                coin_code: "TRON".to_string(),
                sign_id: "override-regression".to_string(),
                hd_path: "m/44'/195'/0'/0/0".to_string(),
                timestamp: 0,
                decimal: 6,
                transaction: Some(
                    ur_registry::pb::protoc::sign_transaction::Transaction::TronTx(
                        ur_registry::pb::protoc::TronTx {
                            token: "FAKE".to_string(),
                            contract_address: "TR7NHqjeKQxGTCi8q8ZY4pL8otSzgjLj6t".to_string(),
                            from: "TTS7Y53sS4rzrDCtZaiuRzqxd16atCe2UR".to_string(),
                            to: "TS5zPoC4XEBmHvDNAnnW2gH3MQhcRN6iRm".to_string(),
                            memo: String::new(),
                            value: "1000000".to_string(),
                            latest_block: Some(LatestBlock {
                                hash: "0000000000000000000000000000000000000000000000000000000000000000".to_string(),
                                number: 1,
                                timestamp: 1_000_000,
                            }),
                            r#override: Some(ur_registry::pb::protoc::Override {
                                token_short_name: "FAKE".to_string(),
                                token_full_name: "Fake Token".to_string(),
                                decimals: 0,
                            }),
                            fee: 0,
                        },
                    ),
                ),
            })),
        };

        let parsed = WrappedTron::from_payload(payload, &context)
            .unwrap()
            .parse()
            .unwrap();
        assert_eq!("1 USDT", parsed.detail.value);
        assert_eq!("USDT", parsed.detail.token);
        assert!(!parsed.detail.value.contains("FAKE"));
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
        assert!(result.is_err());
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
        assert!(result.is_err());
    }

    #[test]
    fn test_from_raw_transaction_no_contract() {
        let mut tron_tx = Transaction::default();
        let mut raw = transaction::Raw::default();
        raw.contract = vec![]; // No contracts
        tron_tx.raw_data = Some(raw);

        let result = WrappedTron::from_raw_transaction(tron_tx, "m/44'/195'/0'/0/0".to_string());
        assert!(result.is_err());
    }

    #[test]
    fn test_format_amount_edge_cases() {
        let hex = "1f8b08000000000000030dcfbd4ac34000c071220ea58bdaa9742a41a84bc87d27270e9ab61890c4268d54bb5dee2e26607b508b4a9fa26fe01bf8b128f812be82b383b8161703ffe9bffd1a5bad9d64d1374a77470bb334d2dc7436567d1b1e96540920ec6fabb99da5e7716b5f4a4e58ae91e36b221d8272ed088ca04399a058f8b2a09075f62297909e0b39edb9a0ce05dde79faf8f0d3868048f56c7ce2e86d3b13abb35833089f4f4be2a97ca04554cd8eaa13c9d5ca9d0b6b3315d8d4c9f5c0e83597837884fe6f309ba0e719494328d5995ce90050fe3e671c17c0ab9d2bc904011a031a502f202e414032e19c60c78be209e409aab1cfa9041e603c204821ad588ddd7f5baddfefd7c7aff03e1cbdbd13f2aab0f710f010000";
        let pubkey_str = "xpub6D1AabNHCupeiLM65ZR9UStMhJ1vCpyV4XbZdyhMZBiJXALQtmn9p42VTQckoHVn8WNqS7dqnJokZHAHcHGoaQgmv8D45oNUKx6DZMNZBCd";
        let payload = prepare_payload(hex);
        let context = prepare_parse_context(pubkey_str);
        let mut tx = WrappedTron::from_payload(payload, &context).unwrap();

        tx.decimals = 3;
        tx.value = "1000".to_string();
        let formatted = tx.format_amount().unwrap();
        assert!(formatted.contains("1"));

        tx.decimals = 0;
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

    #[test]
    fn formats_u256_amount_without_float_rounding() {
        let raw = "115792089237316195423570985008687907853269984665640564039457584007913129639935";
        let formatted = format_decimal_units(raw, 6).unwrap();
        assert_eq!(formatted.replace('.', ""), raw);
        assert!(formatted.ends_with(".639935"));
        assert!(!formatted.contains('e'));
    }

    #[test]
    fn rejects_non_canonical_trc20_calldata() {
        let mut trailing = canonical_transfer_calldata();
        trailing.push(0);
        assert!(WrappedTron::from_raw_transaction(
            valid_trigger_transaction(trailing),
            "m/44'/195'/0'/0/0".to_string(),
        )
        .is_err());

        let mut padding = canonical_transfer_calldata();
        padding[4] = 1;
        assert!(WrappedTron::from_raw_transaction(
            valid_trigger_transaction(padding),
            "m/44'/195'/0'/0/0".to_string(),
        )
        .is_err());
    }

    fn set_trigger_values(
        tx: &mut Transaction,
        owner: Option<Vec<u8>>,
        trx: i64,
        tokens: i64,
        token_id: i64,
    ) {
        let parameter = tx.raw_data.as_mut().unwrap().contract[0]
            .parameter
            .as_mut()
            .unwrap();
        let mut trigger = TriggerSmartContract::decode(parameter.value.as_slice()).unwrap();
        if let Some(owner) = owner {
            trigger.owner_address = owner;
        }
        trigger.call_value = trx;
        trigger.call_token_value = tokens;
        trigger.token_id = token_id;
        parameter.value = trigger.encode_to_vec();
    }

    #[test]
    fn displays_attached_assets_through_parse_check_and_sign() {
        let seed = hex::decode("5eb00bbddcf069084889a8ab9155568165f5c453ccb85e70811aaed6f6da5fc19a5ac40b389cd370d086206dec8aa6c43daea6690f20ad3d8d48b2d2ce9e38e4").unwrap();
        let path = "m/44'/195'/0'/0/0".to_string();
        let xpub = "xpub6D1AabNHCupeiLM65ZR9UStMhJ1vCpyV4XbZdyhMZBiJXALQtmn9p42VTQckoHVn8WNqS7dqnJokZHAHcHGoaQgmv8D45oNUKx6DZMNZBCd";
        let owner = bitcoin::base58::decode_check("TUEZSdKsoDHQMeZwihtdoBiN46zxhGWYdH").unwrap();
        for selector in [
            vec![0xde, 0xad, 0xbe, 0xef],
            vec![0xa9, 0x05, 0x9c, 0xbb],
            vec![0x09, 0x5e, 0xa7, 0xb3],
        ] {
            let mut data = canonical_transfer_calldata();
            data[..4].copy_from_slice(&selector);
            let baseline = valid_trigger_transaction(data.clone());
            let baseline = WrappedTron::from_raw_transaction(baseline, path.clone());
            let expected_token_amount = baseline.unwrap().format_amount().unwrap();
            let mut tx = valid_trigger_transaction(data.clone());
            set_trigger_values(
                &mut tx,
                Some(owner.clone()),
                1_000_000_000_000,
                i64::MAX,
                1_000_001,
            );
            let raw = tx.raw_data.unwrap().encode_to_vec();
            let parsed = crate::parse_tx_request(&raw, &path).unwrap();
            assert_eq!(parsed.detail.call_value, "1000000 TRX");
            assert_eq!(parsed.detail.call_token_value, i64::MAX.to_string());
            assert_eq!(parsed.detail.token_id, "1000001");
            assert_eq!(parsed.detail.call_data, format!("0x{}", hex::encode(data)));
            assert_eq!(
                parsed.detail.raw_data_hash,
                hex::encode(hashing::sha256(&raw))
            );
            if selector[0] == 0xde {
                assert_eq!(parsed.overview.value, "1000000 TRX");
                assert_eq!(parsed.overview.to, parsed.detail.contract_address);
                assert!(!parsed.detail.contract_warning.is_empty());
            } else {
                assert_eq!(parsed.overview.value, expected_token_amount);
                assert!(parsed.detail.contract_warning.is_empty());
            }
            crate::check_tx_request(&raw, &path, xpub).unwrap();
            let signature =
                hex::decode(crate::sign_tx_request(&raw, &path, &seed).unwrap()).unwrap();
            use bitcoin::secp256k1::{
                ecdsa::{RecoverableSignature, RecoveryId},
                Message, Secp256k1,
            };
            let sig = RecoverableSignature::from_compact(
                &signature[..64],
                RecoveryId::from_i32(signature[64] as i32).unwrap(),
            )
            .unwrap();
            let digest = hashing::sha256(&raw);
            let key = Secp256k1::new()
                .recover_ecdsa(&Message::from_digest_slice(&digest).unwrap(), &sig)
                .unwrap();
            assert_eq!(
                crate::address::public_key_to_address(&key),
                "TUEZSdKsoDHQMeZwihtdoBiN46zxhGWYdH"
            );
        }
    }

    #[test]
    fn displays_empty_short_and_long_contract_data_without_truncation() {
        for data in [vec![], vec![0xde], vec![0xde, 0xad, 0xbe], vec![0xab; 4096]] {
            let mut tx = valid_trigger_transaction(data.clone());
            set_trigger_values(&mut tx, None, 1, 0, 0);
            let parsed = WrappedTron::from_raw_transaction(tx, "m/44'/195'/0'/0/0".to_string())
                .unwrap()
                .parse()
                .unwrap();
            assert_eq!(parsed.overview.value, "0.000001 TRX");
            assert_eq!(parsed.detail.call_data, format!("0x{}", hex::encode(data)));
            assert!(parsed.detail.call_token_value.is_empty());
            assert!(parsed.detail.token_id.is_empty());
            assert!(!parsed.detail.contract_warning.is_empty());
        }
    }

    #[test]
    fn rejects_negative_or_unidentified_attached_assets() {
        for (trx, tokens, id) in [(-1, 0, 0), (0, -1, 1), (0, 1, -1), (0, 1, 0)] {
            let mut tx = valid_trigger_transaction(canonical_transfer_calldata());
            set_trigger_values(&mut tx, None, trx, tokens, id);
            assert!(
                WrappedTron::from_raw_transaction(tx, "m/44'/195'/0'/0/0".to_string()).is_err()
            );
        }
    }

    #[test]
    fn rejects_mismatched_or_hidden_contract_metadata() {
        let mut wrong_type = valid_trigger_transaction(canonical_transfer_calldata());
        wrong_type.raw_data.as_mut().unwrap().contract[0]
            .parameter
            .as_mut()
            .unwrap()
            .type_url = "type.googleapis.com/protocol.TransferContract".to_string();
        assert!(
            WrappedTron::from_raw_transaction(wrong_type, "m/44'/195'/0'/0/0".to_string(),)
                .is_err()
        );

        let mut provider = valid_trigger_transaction(canonical_transfer_calldata());
        provider.raw_data.as_mut().unwrap().contract[0].provider = vec![1];
        assert!(
            WrappedTron::from_raw_transaction(provider, "m/44'/195'/0'/0/0".to_string(),).is_err()
        );
    }

    #[test]
    fn rejects_invalid_transaction_header_and_path() {
        let mut tx = valid_trigger_transaction(canonical_transfer_calldata());
        tx.raw_data.as_mut().unwrap().expiration = 1_000;
        assert!(WrappedTron::from_raw_transaction(tx, "m/44'/195'/0'/0/0".to_string(),).is_err());
        assert!(WrappedTron::from_raw_transaction(
            valid_trigger_transaction(canonical_transfer_calldata()),
            "m/44'/60'/0'/0/0".to_string(),
        )
        .is_err());
    }

    #[test]
    fn preserves_non_text_memo_as_hex_and_hash() {
        for memo in [vec![0xff, 0xfe], vec![b'a', 0, b'b']] {
            let mut tx = valid_trigger_transaction(canonical_transfer_calldata());
            tx.raw_data.as_mut().unwrap().data = memo.clone();
            let parsed = WrappedTron::from_raw_transaction(tx, "m/44'/195'/0'/0/0".to_string())
                .unwrap()
                .parse()
                .unwrap();
            assert!(parsed.detail.memo.is_empty());
            assert_eq!(parsed.detail.memo_hex, hex::encode(memo));
            assert_eq!(parsed.detail.memo_hash.len(), 64);
        }
    }
}
