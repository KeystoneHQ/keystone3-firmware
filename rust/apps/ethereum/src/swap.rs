use alloc::format;
use alloc::string::String;
use alloc::string::ToString;
use alloc::vec::Vec;

use crate::abi::parse_contract_data;
use crate::abi::ContractData;
use crate::errors::EthereumError;
use crate::errors::Result;

// =:e:0x742636d8FBD2C1dD721Db619b49eaD254385D77d:256699:-_/kns:20/0
// =:ETH.USDT-0XDAC17F958D2EE523A2206206994597C13D831EC7:0x742636d8FBD2C1dD721Db619b49eaD254385D77d:662901600:-_/kns:20/0

// =:{asset}:{receiveAddress}:{ignore others}

pub fn swapkit_asset_name_convert(asset: &str) -> Result<&str> {
    match asset {
        "e" => Ok("ETH"),
        "bitcoin" => Ok("BTC"),
        "b" => Ok("BTC"),
        x => Ok(x),
    }
}

fn parse_swapkit_memo(memo: &str) -> Result<SwapkitMemo> {
    let memo = memo.trim();
    if memo.is_empty() {
        return Err(EthereumError::InvalidSwapkitMemo);
    }
    let parts = memo.split(':').collect::<Vec<&str>>();
    if parts.len() < 3 {
        return Err(EthereumError::InvalidSwapkitMemo);
    }

    let asset = swapkit_asset_name_convert(parts[1])?;
    let receive_address = parts[2];
    Ok(SwapkitMemo::new(
        asset.to_string(),
        receive_address.to_string(),
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
        return Err(EthereumError::InvalidSwapTransaction(format!(
            "Invalid swapkit contract data"
        )));
    }

    let swapkit_memo = parse_swapkit_memo(&memo.unwrap())?;

    return Ok(SwapkitContractData::new(
        vault.unwrap(),
        swap_in_asset.unwrap(),
        swap_in_amount.unwrap(),
        swapkit_memo.asset,
        swapkit_memo.receive_address,
        expiration,
        contract_data,
    ));
}

pub struct SwapkitMemo {
    pub asset: String,
    pub receive_address: String,
}

impl SwapkitMemo {
    pub fn new(asset: String, receive_address: String) -> Self {
        Self {
            asset,
            receive_address,
        }
    }
}

pub struct SwapkitContractData {
    pub vault: String,
    pub swap_in_asset: String,
    pub swap_in_amount: String,
    pub swap_out_asset: String,
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
        receive_address: String,
        expiration: Option<String>,
        contract_data: ContractData,
    ) -> Self {
        Self {
            vault,
            swap_in_asset,
            swap_in_amount,
            swap_out_asset,
            receive_address,
            expiration,
            contract_data,
        }
    }
}
