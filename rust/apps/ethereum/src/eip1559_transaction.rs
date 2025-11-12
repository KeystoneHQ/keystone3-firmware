use crate::normalizer::{normalize_price, normalize_value};
use crate::structs::TransactionAction;
use crate::traits::BaseTransaction;
use crate::{impl_base_transaction, Bytes};
use alloc::format;
use alloc::string::{String, ToString};

use core::ops::Mul;
use ethereum_types::U256;

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

#[cfg(test)]
mod tests {
    use super::*;
    use crate::H160;
    use alloc::vec;
    use core::str::FromStr;

    extern crate std;

    #[test]
    fn test_parsed_eip1559_transaction() {
        let tx = EIP1559Transaction {
            chain_id: 1,
            nonce: U256::from(42),
            max_priority_fee_per_gas: U256::from(2000000000u64), // 2 Gwei
            max_fee_per_gas: U256::from(100000000000u64),        // 100 Gwei
            gas_limit: U256::from(21000),
            action: crate::structs::TransactionAction::Call(
                H160::from_str("0x49aB56B91fc982Fd6Ec1EC7Bb87d74EFA6dA30ab").unwrap(),
            ),
            value: U256::from_dec_str("1000000000000000000").unwrap(), // 1 ETH
            input: vec![],
        };

        let parsed = ParsedEIP1559Transaction::from(tx);
        assert_eq!(parsed.chain_id, 1);
        assert_eq!(parsed.nonce, 42);
        assert_eq!(parsed.max_priority_fee_per_gas, "2 Gwei");
        assert_eq!(parsed.max_fee_per_gas, "100 Gwei");
        assert_eq!(parsed.gas_limit, "21000");
        assert_eq!(parsed.value, "1");
    }

    #[test]
    fn test_parsed_eip1559_transaction_fee_calculation() {
        let tx = EIP1559Transaction {
            chain_id: 1,
            nonce: U256::from(0),
            max_priority_fee_per_gas: U256::from(1000000000u64), // 1 Gwei
            max_fee_per_gas: U256::from(50000000000u64),         // 50 Gwei
            gas_limit: U256::from(21000),
            action: crate::structs::TransactionAction::Call(
                H160::from_str("0x0000000000000000000000000000000000000000").unwrap(),
            ),
            value: U256::from(0),
            input: vec![],
        };

        let parsed = ParsedEIP1559Transaction::from(tx);
        // max_fee = max_fee_per_gas * gas_limit = 50 Gwei * 21000 = 1050000000000000 wei = 0.00105 ETH
        assert_eq!(parsed.max_fee, "0.00105");
        // max_priority = max_priority_fee_per_gas * gas_limit = 1 Gwei * 21000 = 21000000000000 wei = 0.000021 ETH
        assert_eq!(parsed.max_priority, "0.000021");
        assert_eq!(parsed.max_txn_fee, "0.00105");
    }

    #[test]
    fn test_parsed_eip1559_transaction_create() {
        // Test contract creation (TransactionAction::Create)
        let tx = EIP1559Transaction {
            chain_id: 1,
            nonce: U256::from(5),
            max_priority_fee_per_gas: U256::from(3000000000u64), // 3 Gwei
            max_fee_per_gas: U256::from(150000000000u64),        // 150 Gwei
            gas_limit: U256::from(500000),
            action: crate::structs::TransactionAction::Create,
            value: U256::from_dec_str("5000000000000000000").unwrap(), // 5 ETH
            input: vec![0x60, 0x80, 0x60, 0x40, 0x52],                 // Some contract bytecode
        };

        let parsed = ParsedEIP1559Transaction::from(tx);
        assert_eq!(parsed.chain_id, 1);
        assert_eq!(parsed.nonce, 5);
        assert_eq!(parsed.max_priority_fee_per_gas, "3 Gwei");
        assert_eq!(parsed.max_fee_per_gas, "150 Gwei");
        assert_eq!(parsed.gas_limit, "500000");
        assert_eq!(parsed.value, "5");
        assert_eq!(parsed.input, "6080604052");
        // For Create action, to should be empty or zero address
        assert_eq!(parsed.to, "0x0000000000000000000000000000000000000000");
    }

    #[test]
    fn test_parsed_eip1559_transaction_different_chain_id() {
        // Test with different chain_id
        let tx = EIP1559Transaction {
            chain_id: 137, // Polygon
            nonce: U256::from(10),
            max_priority_fee_per_gas: U256::from(1000000000u64), // 1 Gwei
            max_fee_per_gas: U256::from(50000000000u64),         // 50 Gwei
            gas_limit: U256::from(100000),
            action: crate::structs::TransactionAction::Call(
                H160::from_str("0x49aB56B91fc982Fd6Ec1EC7Bb87d74EFA6dA30ab").unwrap(),
            ),
            value: U256::from_dec_str("2000000000000000000").unwrap(), // 2 ETH
            input: vec![0xa9, 0x05, 0x9c, 0xbb],                       // transfer function selector
        };

        let parsed = ParsedEIP1559Transaction::from(tx);
        assert_eq!(parsed.chain_id, 137);
        assert_eq!(parsed.nonce, 10);
        assert_eq!(parsed.value, "2");
        assert_eq!(parsed.input, "a9059cbb");
    }

    #[test]
    fn test_parsed_eip1559_transaction_zero_value() {
        // Test with zero value
        let tx = EIP1559Transaction {
            chain_id: 1,
            nonce: U256::from(0),
            max_priority_fee_per_gas: U256::from(1000000000u64), // 1 Gwei
            max_fee_per_gas: U256::from(20000000000u64),         // 20 Gwei
            gas_limit: U256::from(21000),
            action: crate::structs::TransactionAction::Call(
                H160::from_str("0x49aB56B91fc982Fd6Ec1EC7Bb87d74EFA6dA30ab").unwrap(),
            ),
            value: U256::from(0),
            input: vec![],
        };

        let parsed = ParsedEIP1559Transaction::from(tx);
        assert_eq!(parsed.value, "0");
        assert_eq!(parsed.max_fee, "0.00042"); // 20 Gwei * 21000 = 420000000000000 wei
    }

    #[test]
    fn test_parsed_eip1559_transaction_large_gas_limit() {
        // Test with large gas limit
        let tx = EIP1559Transaction {
            chain_id: 1,
            nonce: U256::from(100),
            max_priority_fee_per_gas: U256::from(2000000000u64), // 2 Gwei
            max_fee_per_gas: U256::from(100000000000u64),        // 100 Gwei
            gas_limit: U256::from(1000000),                      // 1M gas
            action: crate::structs::TransactionAction::Call(
                H160::from_str("0x49aB56B91fc982Fd6Ec1EC7Bb87d74EFA6dA30ab").unwrap(),
            ),
            value: U256::from(0),
            input: vec![0x12, 0x34, 0x56],
        };

        let parsed = ParsedEIP1559Transaction::from(tx);
        assert_eq!(parsed.gas_limit, "1000000");
        assert_eq!(parsed.input, "123456");
        // max_fee = 100 Gwei * 1000000 = 100000000000000000 wei = 0.1 ETH
        assert_eq!(parsed.max_fee, "0.1");
        // max_priority = 2 Gwei * 1000000 = 2000000000000000 wei = 0.002 ETH
        assert_eq!(parsed.max_priority, "0.002");
    }

    #[test]
    fn test_parsed_eip1559_transaction_small_fees() {
        // Test with very small fees
        let tx = EIP1559Transaction {
            chain_id: 1,
            nonce: U256::from(1),
            max_priority_fee_per_gas: U256::from(1000000u64), // 0.001 Gwei
            max_fee_per_gas: U256::from(100000000u64),        // 0.1 Gwei
            gas_limit: U256::from(21000),
            action: crate::structs::TransactionAction::Call(
                H160::from_str("0x49aB56B91fc982Fd6Ec1EC7Bb87d74EFA6dA30ab").unwrap(),
            ),
            value: U256::from(1000000000000000u64), // 0.001 ETH
            input: vec![],
        };

        let parsed = ParsedEIP1559Transaction::from(tx);
        assert_eq!(parsed.max_priority_fee_per_gas, "0.001 Gwei");
        assert_eq!(parsed.max_fee_per_gas, "0.1 Gwei");
        assert_eq!(parsed.value, "0.001");
    }

    #[test]
    fn test_eip1559_transaction_decode_raw() {
        let invalid_data = vec![];
        let result = EIP1559Transaction::decode_raw(&invalid_data);
        assert!(result.is_err());
    }

    #[test]
    fn test_parsed_eip1559_transaction_large_nonce() {
        let tx = EIP1559Transaction {
            chain_id: 1,
            nonce: U256::from(999999u64),
            max_priority_fee_per_gas: U256::from(1000000000u64),
            max_fee_per_gas: U256::from(20000000000u64),
            gas_limit: U256::from(21000),
            action: crate::structs::TransactionAction::Call(
                H160::from_str("0x49aB56B91fc982Fd6Ec1EC7Bb87d74EFA6dA30ab").unwrap(),
            ),
            value: U256::from(0),
            input: vec![],
        };

        let parsed = ParsedEIP1559Transaction::from(tx);
        assert_eq!(parsed.nonce, 999999);
    }
}
