use alloc::string::String;
use alloc::string::ToString;
use alloc::vec::Vec;

use crate::abi::parse_contract_data;
use crate::abi::ContractData;
use crate::errors::EthereumError;
use crate::errors::Result;

pub fn swapkit_asset_name_convert(asset: &str) -> Result<(String, Option<String>)> {
    match asset.to_lowercase().as_str() {
        "e" | "eth.eth" => Ok(("ETH".to_string(), None)),
        "bitcoin" | "b" | "btc.btc" => Ok(("BTC".to_string(), None)),
        "x" => Ok(("XRP".to_string(), None)),
        "d" | "doge.doge" => Ok(("DOGE".to_string(), None)),
        "s" | "bnb.bnb" => Ok(("BNB".to_string(), None)),
        x => {
            //ETH.USDT-0XDAC17F958D2EE523A2206206994597C13D831EC7
            let parts = x.split('-').collect::<Vec<&str>>();
            let asset_name = parts[0];
            let contract_address = parts.get(1).map(|s| s.to_string());
            Ok((asset_name.to_string(), contract_address))
        }
    }
}

// =:e:0x742636d8FBD2C1dD721Db619b49eaD254385D77d:256699:-_/kns:20/0
// =:ETH.USDT-0XDAC17F958D2EE523A2206206994597C13D831EC7:0x742636d8FBD2C1dD721Db619b49eaD254385D77d:662901600:-_/kns:20/0

// =:{asset}:{receiveAddress}:{ignore others}
fn parse_swapkit_memo(memo: &str) -> Result<SwapkitMemo> {
    let memo = memo.trim();
    if memo.is_empty() {
        return Err(EthereumError::InvalidSwapkitMemo);
    }
    let parts = memo.split(':').collect::<Vec<&str>>();
    if parts.len() < 3 {
        return Err(EthereumError::InvalidSwapkitMemo);
    }

    let (asset, swap_out_asset_contract_address) = swapkit_asset_name_convert(parts[1])?;
    let receive_address = parts[2];
    Ok(SwapkitMemo::new(
        asset,
        receive_address.to_string(),
        swap_out_asset_contract_address,
    ))
}

pub fn parse_swapkit_contract(
    input_data: Vec<u8>,
    contract: String,
) -> Result<SwapkitContractData> {
    let contract_data = parse_contract_data(input_data, contract)?;
    let params = contract_data.get_params();
    let mut vault = None;
    let mut swap_in_asset = None;
    let mut swap_in_amount = None;
    let mut memo = None;
    let mut expiration = None;

    for param in params {
        match param.get_name().as_str() {
            "vault" => {
                vault = Some(param.get_value().to_string());
            }
            "asset" => {
                swap_in_asset = Some(param.get_value().to_string());
            }
            "amount" => {
                swap_in_amount = Some(param.get_value().to_string());
            }
            "memo" => {
                memo = Some(param.get_value().to_string());
            }
            "expiration" => {
                expiration = Some(param.get_value().to_string());
            }
            _ => {}
        }
    }

    if vault.is_none() || swap_in_asset.is_none() || swap_in_amount.is_none() || memo.is_none() {
        return Err(EthereumError::InvalidSwapTransaction(
            "Invalid swapkit contract data".to_string(),
        ));
    }

    let swapkit_memo = parse_swapkit_memo(&memo.unwrap())?;

    Ok(SwapkitContractData::new(
        vault.unwrap(),
        swap_in_asset.unwrap(),
        swap_in_amount.unwrap(),
        swapkit_memo.asset,
        swapkit_memo.swap_out_asset_contract_address,
        swapkit_memo.receive_address,
        expiration,
        contract_data,
    ))
}

pub struct SwapkitMemo {
    pub asset: String,
    pub receive_address: String,
    pub swap_out_asset_contract_address: Option<String>,
}

impl SwapkitMemo {
    pub fn new(
        asset: String,
        receive_address: String,
        swap_out_asset_contract_address: Option<String>,
    ) -> Self {
        Self {
            asset,
            receive_address,
            swap_out_asset_contract_address,
        }
    }
}

pub struct SwapkitContractData {
    pub vault: String,
    pub swap_in_asset: String,
    pub swap_in_amount: String,
    pub swap_out_asset: String,
    pub swap_out_asset_contract_address: Option<String>,
    pub receive_address: String,
    pub expiration: Option<String>,

    pub contract_data: ContractData,
}

impl SwapkitContractData {
    pub fn new(
        vault: String,
        swap_in_asset: String,
        swap_in_amount: String,
        swap_out_asset: String,
        swap_out_asset_contract_address: Option<String>,
        receive_address: String,
        expiration: Option<String>,
        contract_data: ContractData,
    ) -> Self {
        Self {
            vault,
            swap_in_asset,
            swap_in_amount,
            swap_out_asset,
            swap_out_asset_contract_address,
            receive_address,
            expiration,
            contract_data,
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    extern crate std;

    #[test]
    fn test_swapkit_asset_name_convert_simple() {
        assert_eq!(
            swapkit_asset_name_convert("e").unwrap(),
            ("ETH".to_string(), None)
        );
        assert_eq!(
            swapkit_asset_name_convert("ETH.ETH").unwrap(),
            ("ETH".to_string(), None)
        );
        assert_eq!(
            swapkit_asset_name_convert("bitcoin").unwrap(),
            ("BTC".to_string(), None)
        );
        assert_eq!(
            swapkit_asset_name_convert("b").unwrap(),
            ("BTC".to_string(), None)
        );
        assert_eq!(
            swapkit_asset_name_convert("BTC.BTC").unwrap(),
            ("BTC".to_string(), None)
        );
        assert_eq!(
            swapkit_asset_name_convert("x").unwrap(),
            ("XRP".to_string(), None)
        );
        assert_eq!(
            swapkit_asset_name_convert("d").unwrap(),
            ("DOGE".to_string(), None)
        );
        assert_eq!(
            swapkit_asset_name_convert("DOGE.DOGE").unwrap(),
            ("DOGE".to_string(), None)
        );
        assert_eq!(
            swapkit_asset_name_convert("s").unwrap(),
            ("BNB".to_string(), None)
        );
        assert_eq!(
            swapkit_asset_name_convert("BNB.BNB").unwrap(),
            ("BNB".to_string(), None)
        );
    }

    #[test]
    fn test_swapkit_asset_name_convert_with_contract() {
        // Test asset with contract address
        let result =
            swapkit_asset_name_convert("ETH.USDT-0XDAC17F958D2EE523A2206206994597C13D831EC7")
                .unwrap();
        assert_eq!(result.0, "eth.usdt");
        assert_eq!(
            result.1,
            Some("0xdac17f958d2ee523a2206206994597c13d831ec7".to_string())
        );

        let result =
            swapkit_asset_name_convert("ETH.USDT-0xdac17f958d2ee523a2206206994597c13d831ec7")
                .unwrap();
        assert_eq!(result.0, "eth.usdt");
        assert_eq!(
            result.1,
            Some("0xdac17f958d2ee523a2206206994597c13d831ec7".to_string())
        );
    }

    #[test]
    fn test_swapkit_asset_name_convert_case_insensitive() {
        // Test case insensitivity
        assert_eq!(
            swapkit_asset_name_convert("E").unwrap(),
            ("ETH".to_string(), None)
        );
        assert_eq!(
            swapkit_asset_name_convert("B").unwrap(),
            ("BTC".to_string(), None)
        );
        assert_eq!(
            swapkit_asset_name_convert("X").unwrap(),
            ("XRP".to_string(), None)
        );
    }

    #[test]
    fn test_parse_swapkit_memo_valid() {
        // Test valid memo format: =:e:0x742636d8FBD2C1dD721Db619b49eaD254385D77d:256699:-_/kns:20/0
        let memo = "=:e:0x742636d8FBD2C1dD721Db619b49eaD254385D77d:256699:-_/kns:20/0";
        let result = parse_swapkit_memo(memo);
        assert!(result.is_ok());
        let swapkit_memo = result.unwrap();
        assert_eq!(swapkit_memo.asset, "ETH");
        assert_eq!(
            swapkit_memo.receive_address,
            "0x742636d8FBD2C1dD721Db619b49eaD254385D77d"
        );
        assert_eq!(swapkit_memo.swap_out_asset_contract_address, None);
    }

    #[test]
    fn test_parse_swapkit_memo_with_contract() {
        // Test memo with contract address
        let memo = "=:ETH.USDT-0XDAC17F958D2EE523A2206206994597C13D831EC7:0x742636d8FBD2C1dD721Db619b49eaD254385D77d:662901600:-_/kns:20/0";
        let result = parse_swapkit_memo(memo);
        assert!(result.is_ok());
        let swapkit_memo = result.unwrap();
        assert_eq!(swapkit_memo.asset, "eth.usdt");
        assert_eq!(
            swapkit_memo.receive_address,
            "0x742636d8FBD2C1dD721Db619b49eaD254385D77d"
        );
        assert_eq!(
            swapkit_memo.swap_out_asset_contract_address,
            Some("0xdac17f958d2ee523a2206206994597c13d831ec7".to_string())
        );
    }

    #[test]
    fn test_parse_swapkit_memo_invalid_empty() {
        let memo = "";
        let result = parse_swapkit_memo(memo);
        assert!(result.is_err());
    }

    #[test]
    fn test_parse_swapkit_memo_invalid_too_short() {
        let memo = "=:e";
        let result = parse_swapkit_memo(memo);
        assert!(result.is_err());
    }

    #[test]
    fn test_swapkit_memo_new() {
        let memo = SwapkitMemo::new(
            "ETH".to_string(),
            "0x742636d8FBD2C1dD721Db619b49eaD254385D77d".to_string(),
            Some("0xDAC17F958D2EE523A2206206994597C13D831EC7".to_string()),
        );
        assert_eq!(memo.asset, "ETH");
        assert_eq!(
            memo.receive_address,
            "0x742636d8FBD2C1dD721Db619b49eaD254385D77d"
        );
        assert_eq!(
            memo.swap_out_asset_contract_address,
            Some("0xDAC17F958D2EE523A2206206994597C13D831EC7".to_string())
        );
    }
}
