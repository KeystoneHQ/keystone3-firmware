use crate::erc20::parse_erc20_approval;
use alloc::format;
use alloc::string::ToString;
use alloc::vec::Vec;

use crate::{errors::EthereumError, structs::ParsedEthereumTransaction};

pub fn rule_swap(txs: Vec<ParsedEthereumTransaction>) -> Result<(), EthereumError> {
    if txs.is_empty() || txs.len() > 3 {
        return Err(EthereumError::InvalidSwapTransaction(format!(
            "invalid transaction count: {}",
            txs.len()
        )));
    }

    // If there is only one transaction, it is a swap
    if txs.len() == 1 {
        return Ok(());
    }

    // If there are two transactions, the first is an approval and the second is swap
    if txs.len() == 2 {
        let _ = parse_erc20_approval(&txs[0].input, 0)
            .map_err(|e| EthereumError::InvalidSwapTransaction(e.to_string()))?;
    }

    // If there are three transactions, the first is a revoke approval, the second is an approval and the third is a swap
    if txs.len() == 3 {
        let approval_0 = parse_erc20_approval(&txs[0].input, 0)
            .map_err(|e| EthereumError::InvalidSwapTransaction(e.to_string()))?;
        let amount = approval_0.value;
        if amount != "0" {
            return Err(EthereumError::InvalidSwapTransaction(format!(
                "invalid revoke amount: {amount}"
            )));
        }
        let _ = parse_erc20_approval(&txs[1].input, 0)
            .map_err(|e| EthereumError::InvalidSwapTransaction(e.to_string()))?;
    }

    Ok(())
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::structs::ParsedEthereumTransaction;
    use alloc::{string::String, vec};

    extern crate std;

    fn create_test_transaction(input: String) -> ParsedEthereumTransaction {
        ParsedEthereumTransaction {
            nonce: 0,
            chain_id: 1,
            from: None,
            to: "0x0000000000000000000000000000000000000000".to_string(),
            value: "0".to_string(),
            input,
            gas_price: None,
            max_fee_per_gas: None,
            max_priority_fee_per_gas: None,
            max_fee: None,
            max_priority: None,
            gas_limit: "21000".to_string(),
            max_txn_fee: "0".to_string(),
        }
    }

    #[test]
    fn test_rule_swap_empty() {
        let txs = vec![];
        let result = rule_swap(txs);
        assert!(result.is_err());
        assert!(matches!(
            result.unwrap_err(),
            EthereumError::InvalidSwapTransaction(_)
        ));
    }

    #[test]
    fn test_rule_swap_too_many() {
        let txs = vec![
            create_test_transaction("".to_string()),
            create_test_transaction("".to_string()),
            create_test_transaction("".to_string()),
            create_test_transaction("".to_string()),
        ];
        let result = rule_swap(txs);
        assert!(result.is_err());
        assert!(matches!(
            result.unwrap_err(),
            EthereumError::InvalidSwapTransaction(_)
        ));
    }

    #[test]
    fn test_rule_swap_single() {
        let txs = vec![create_test_transaction("".to_string())];
        let result = rule_swap(txs);
        assert!(result.is_ok());
    }

    #[test]
    fn test_rule_swap_two_invalid_first() {
        // First transaction is not an approval
        let tx0 = create_test_transaction("a9059cbb00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000001".to_string()); // transfer, not approve

        let tx1 = create_test_transaction("a9059cbb00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000001".to_string());

        let txs = vec![tx0, tx1];
        let result = rule_swap(txs);
        assert!(result.is_err());
    }

    #[test]
    fn test_rule_swap_three_valid() {
        let tx0_input = "095ea7b300000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000";
        assert_eq!(tx0_input.len(), 136, "tx0 input must be 136 characters");
        let tx0 = create_test_transaction(tx0_input.to_string());

        let tx1_input = "095ea7b300000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000001";
        assert_eq!(tx1_input.len(), 136, "tx1 input must be 136 characters");
        let tx1 = create_test_transaction(tx1_input.to_string());

        let tx2_input = "a9059cbb00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000001";
        assert_eq!(tx2_input.len(), 136, "tx2 input must be 136 characters");
        let tx2 = create_test_transaction(tx2_input.to_string());

        let txs = vec![tx0, tx1, tx2];
        let result = rule_swap(txs);
        match &result {
            Ok(_) => {}
            Err(e) => panic!("rule_swap failed: {:?}", e),
        }
        assert!(result.is_ok());
    }

    #[test]
    fn test_rule_swap_three_invalid_revoke() {
        // First: invalid revoke (amount != 0)
        let tx0 = create_test_transaction("095ea7b30000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000001".to_string()); // amount = 1, not 0

        let tx1 = create_test_transaction("095ea7b30000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000001".to_string());

        let tx2 = create_test_transaction("a9059cbb00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000001".to_string());

        let txs = vec![tx0, tx1, tx2];
        let result = rule_swap(txs);
        assert!(result.is_err());
        assert!(matches!(
            result.unwrap_err(),
            EthereumError::InvalidSwapTransaction(_)
        ));
    }
}
