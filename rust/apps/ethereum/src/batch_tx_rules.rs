use crate::erc20::parse_erc20_approval;
use alloc::format;
use alloc::string::ToString;
use alloc::vec::Vec;

use crate::{errors::EthereumError, structs::ParsedEthereumTransaction};

pub fn rule_swap(txs: Vec<ParsedEthereumTransaction>) -> Result<(), EthereumError> {
    //
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
