use crate::errors::{EthereumError, Result};
use alloc::string::{String, ToString};
use alloc::vec::Vec;
use alloc::{format, vec};
use app_utils::impl_public_struct;
use ethabi::{Contract, Function, Token};
use serde_json::Value;
use {hex, serde_json};

impl_public_struct!(ContractData {
    contract_name: String,
    method_name: String,
    params: Vec<ContractMethodParam>
});

impl_public_struct!(ContractMethodParam {
    name: String,
    value: String
});

pub fn parse_contract_data(input: Vec<u8>, contract_str: String) -> Result<ContractData> {
    let contract_json: Value = serde_json::from_str(contract_str.as_str())
        .map_err(|_e| EthereumError::InvalidContractABI)?;
    #[allow(unused_assignments)] //stupid compiler
    let mut contract_abi: Value = Default::default();
    let mut contract_name: String = Default::default();
    if let Some(_name) = contract_json.get("name") {
        contract_name = serde_json::from_value(_name.clone())
            .map_err(|_e| EthereumError::InvalidContractABI)?;
    }
    if let Some(abi) = contract_json.pointer("/metadata/output/abi") {
        contract_abi = abi.clone();
    } else {
        return Err(EthereumError::InvalidContractABI);
    }
    let contract: Contract =
        serde_json::from_value(contract_abi).map_err(|_e| EthereumError::InvalidContractABI)?;
    let (_signature, _data) = input.split_at(4);
    for functions in contract.functions.values() {
        let (signature, data) = input.split_at(4);
        for function in functions {
            let method_name = function.name.clone();
            let params = _parse_by_function(signature, data, function.clone());
            if let Some(Ok(_params)) = params {
                return Ok(ContractData::new(contract_name, method_name, _params));
            } else if let Some(Err(_e)) = params {
                return Err(_e);
            }
        }
    }
    Err(EthereumError::DecodeContractDataError(String::from(
        "Unable to parse this contract data, no matching function",
    )))
}

pub fn parse_method_data(
    input: Vec<u8>,
    contract_name: String,
    method_str: String,
) -> Result<ContractData> {
    let function: Function = serde_json::from_str(method_str.as_str())
        .map_err(|_e| EthereumError::InvalidContractABI)?;
    let method_name = function.name.clone();
    if input.len() <= 4 {
        return Err(EthereumError::DecodeContractDataError(format!(
            "invalid input data: {}",
            hex::encode(input)
        )));
    }
    let (signature, data) = input.split_at(4);
    let params = _parse_by_function(signature, data, function.clone());
    match params {
        Some(Ok(_params)) => Ok(ContractData::new(contract_name, method_name, _params)),
        Some(Err(e)) => Err(e),
        None => Err(EthereumError::DecodeContractDataError(format!(
            "input method selector [{}] not match function ({}) signature [{}]",
            hex::encode(signature),
            method_name,
            hex::encode(function.short_signature())
        ))),
    }
}

fn _parse_by_function(
    signature: &[u8],
    data: &[u8],
    function: Function,
) -> Option<Result<Vec<ContractMethodParam>>> {
    if signature.eq(&function.short_signature()) {
        let tokens = function.decode_input(data).map_err(|_e| {
            EthereumError::DecodeContractDataError(String::from("invalid input data"))
        });
        let tokens = match tokens {
            Ok(_tokens) => _tokens,
            Err(_err) => return Some(Err(_err)),
        };
        let inputs = function.inputs;
        if tokens.len() != inputs.len() {
            return Some(Err(EthereumError::DecodeContractDataError(String::from(
                "decoded tokens length not match function inputs length",
            ))));
        }
        let _method_name = function.name;
        let mut params = vec![];
        for i in 0..tokens.len() {
            let token = tokens.get(i);
            let input = inputs.get(i);
            if let (Some(_token), Some(_input)) = (token, input) {
                params.push(ContractMethodParam::new(
                    _input.name.clone(),
                    match _token {
                        Token::Address(_) => {
                            format!("0x{}", _token.to_string())
                        }
                        _ => _token.to_string(),
                    },
                ))
            } else {
                return Some(Err(EthereumError::DecodeContractDataError(String::from(
                    "[IMPOSSIBLE]cannot get token or input. ",
                ))));
            }
        }
        return Some(Ok(params));
    }
    None
}

#[cfg(test)]
mod tests {
    use alloc::string::ToString;
    use hex;

    extern crate std;

    use crate::abi::parse_contract_data;
    use crate::abi::parse_method_data;

    #[test]
    fn test_parse_contract_data() {
        let json = r#"{
          "name": "UniversalRouter",
          "address": "0x3fC91A3afd70395Cd496C647d5a6CC9D4B2b7FAD",
          "metadata": {
            "output": {
              "abi": [{"inputs":[{"components":[{"internalType":"address","name":"permit2","type":"address"},{"internalType":"address","name":"weth9","type":"address"},{"internalType":"address","name":"seaportV1_5","type":"address"},{"internalType":"address","name":"seaportV1_4","type":"address"},{"internalType":"address","name":"openseaConduit","type":"address"},{"internalType":"address","name":"nftxZap","type":"address"},{"internalType":"address","name":"x2y2","type":"address"},{"internalType":"address","name":"foundation","type":"address"},{"internalType":"address","name":"sudoswap","type":"address"},{"internalType":"address","name":"elementMarket","type":"address"},{"internalType":"address","name":"nft20Zap","type":"address"},{"internalType":"address","name":"cryptopunks","type":"address"},{"internalType":"address","name":"looksRareV2","type":"address"},{"internalType":"address","name":"routerRewardsDistributor","type":"address"},{"internalType":"address","name":"looksRareRewardsDistributor","type":"address"},{"internalType":"address","name":"looksRareToken","type":"address"},{"internalType":"address","name":"v2Factory","type":"address"},{"internalType":"address","name":"v3Factory","type":"address"},{"internalType":"bytes32","name":"pairInitCodeHash","type":"bytes32"},{"internalType":"bytes32","name":"poolInitCodeHash","type":"bytes32"}],"internalType":"struct RouterParameters","name":"params","type":"tuple"}],"stateMutability":"nonpayable","type":"constructor"},{"inputs":[],"name":"BalanceTooLow","type":"error"},{"inputs":[],"name":"BuyPunkFailed","type":"error"},{"inputs":[],"name":"ContractLocked","type":"error"},{"inputs":[],"name":"ETHNotAccepted","type":"error"},{"inputs":[{"internalType":"uint256","name":"commandIndex","type":"uint256"},{"internalType":"bytes","name":"message","type":"bytes"}],"name":"ExecutionFailed","type":"error"},{"inputs":[],"name":"FromAddressIsNotOwner","type":"error"},{"inputs":[],"name":"InsufficientETH","type":"error"},{"inputs":[],"name":"InsufficientToken","type":"error"},{"inputs":[],"name":"InvalidBips","type":"error"},{"inputs":[{"internalType":"uint256","name":"commandType","type":"uint256"}],"name":"InvalidCommandType","type":"error"},{"inputs":[],"name":"InvalidOwnerERC1155","type":"error"},{"inputs":[],"name":"InvalidOwnerERC721","type":"error"},{"inputs":[],"name":"InvalidPath","type":"error"},{"inputs":[],"name":"InvalidReserves","type":"error"},{"inputs":[],"name":"InvalidSpender","type":"error"},{"inputs":[],"name":"LengthMismatch","type":"error"},{"inputs":[],"name":"SliceOutOfBounds","type":"error"},{"inputs":[],"name":"TransactionDeadlinePassed","type":"error"},{"inputs":[],"name":"UnableToClaim","type":"error"},{"inputs":[],"name":"UnsafeCast","type":"error"},{"inputs":[],"name":"V2InvalidPath","type":"error"},{"inputs":[],"name":"V2TooLittleReceived","type":"error"},{"inputs":[],"name":"V2TooMuchRequested","type":"error"},{"inputs":[],"name":"V3InvalidAmountOut","type":"error"},{"inputs":[],"name":"V3InvalidCaller","type":"error"},{"inputs":[],"name":"V3InvalidSwap","type":"error"},{"inputs":[],"name":"V3TooLittleReceived","type":"error"},{"inputs":[],"name":"V3TooMuchRequested","type":"error"},{"anonymous":false,"inputs":[{"indexed":false,"internalType":"uint256","name":"amount","type":"uint256"}],"name":"RewardsSent","type":"event"},{"inputs":[{"internalType":"bytes","name":"looksRareClaim","type":"bytes"}],"name":"collectRewards","outputs":[],"stateMutability":"nonpayable","type":"function"},{"inputs":[{"internalType":"bytes","name":"commands","type":"bytes"},{"internalType":"bytes[]","name":"inputs","type":"bytes[]"}],"name":"execute","outputs":[],"stateMutability":"payable","type":"function"},{"inputs":[{"internalType":"bytes","name":"commands","type":"bytes"},{"internalType":"bytes[]","name":"inputs","type":"bytes[]"},{"internalType":"uint256","name":"deadline","type":"uint256"}],"name":"execute","outputs":[],"stateMutability":"payable","type":"function"},{"inputs":[{"internalType":"address","name":"","type":"address"},{"internalType":"address","name":"","type":"address"},{"internalType":"uint256[]","name":"","type":"uint256[]"},{"internalType":"uint256[]","name":"","type":"uint256[]"},{"internalType":"bytes","name":"","type":"bytes"}],"name":"onERC1155BatchReceived","outputs":[{"internalType":"bytes4","name":"","type":"bytes4"}],"stateMutability":"pure","type":"function"},{"inputs":[{"internalType":"address","name":"","type":"address"},{"internalType":"address","name":"","type":"address"},{"internalType":"uint256","name":"","type":"uint256"},{"internalType":"uint256","name":"","type":"uint256"},{"internalType":"bytes","name":"","type":"bytes"}],"name":"onERC1155Received","outputs":[{"internalType":"bytes4","name":"","type":"bytes4"}],"stateMutability":"pure","type":"function"},{"inputs":[{"internalType":"address","name":"","type":"address"},{"internalType":"address","name":"","type":"address"},{"internalType":"uint256","name":"","type":"uint256"},{"internalType":"bytes","name":"","type":"bytes"}],"name":"onERC721Received","outputs":[{"internalType":"bytes4","name":"","type":"bytes4"}],"stateMutability":"pure","type":"function"},{"inputs":[{"internalType":"bytes4","name":"interfaceId","type":"bytes4"}],"name":"supportsInterface","outputs":[{"internalType":"bool","name":"","type":"bool"}],"stateMutability":"pure","type":"function"},{"inputs":[{"internalType":"int256","name":"amount0Delta","type":"int256"},{"internalType":"int256","name":"amount1Delta","type":"int256"},{"internalType":"bytes","name":"data","type":"bytes"}],"name":"uniswapV3SwapCallback","outputs":[],"stateMutability":"nonpayable","type":"function"},{"stateMutability":"payable","type":"receive"}]    }
          },
          "version": 1,
          "checkPoints": []
        }"#;
        let contract_data = hex::decode("3593564c000000000000000000000000000000000000000000000000000000000000006000000000000000000000000000000000000000000000000000000000000000a00000000000000000000000000000000000000000000000000000000064996e5f00000000000000000000000000000000000000000000000000000000000000020b000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000002000000000000000000000000000000000000000000000000000000000000004000000000000000000000000000000000000000000000000000000000000000a000000000000000000000000000000000000000000000000000000000000000400000000000000000000000000000000000000000000000000000000000000002000000000000000000000000000000000000000000000000002386f26fc1000000000000000000000000000000000000000000000000000000000000000001000000000000000000000000000000000000000000000000000000000000000001000000000000000000000000000000000000000000000000002386f26fc10000000000000000000000000000000000000000000000000000f84605ccc515414000000000000000000000000000000000000000000000000000000000000000a00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000002bc02aaa39b223fe8d0a0e5c4f27ead9083c756cc20001f46b175474e89094c44da98b954eedeac495271d0f000000000000000000000000000000000000000000").unwrap();
        let result = parse_contract_data(contract_data, json.to_string());
        assert_eq!(true, result.is_ok());
        let result = result.unwrap();
        assert_eq!("UniversalRouter", result.contract_name);
        assert_eq!("execute", result.method_name);
        assert_eq!(3, result.params.len());
        assert_eq!(result.params.get(0).unwrap().name, "commands");
        assert_eq!(result.params.get(0).unwrap().value, "0b00");
        assert_eq!(result.params.get(1).unwrap().name, "inputs");
        assert_eq!(result.params.get(1).unwrap().value, "[0000000000000000000000000000000000000000000000000000000000000002000000000000000000000000000000000000000000000000002386f26fc10000,0000000000000000000000000000000000000000000000000000000000000001000000000000000000000000000000000000000000000000002386f26fc10000000000000000000000000000000000000000000000000000f84605ccc515414000000000000000000000000000000000000000000000000000000000000000a00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000002bc02aaa39b223fe8d0a0e5c4f27ead9083c756cc20001f46b175474e89094c44da98b954eedeac495271d0f000000000000000000000000000000000000000000]");
        assert_eq!(result.params.get(2).unwrap().name, "deadline");
        assert_eq!(result.params.get(2).unwrap().value, "64996e5f");
    }
    #[test]
    fn test_parse_method_data() {
        let json = r#"{
                    "inputs": [
                      { "internalType": "bytes", "name": "commands", "type": "bytes" },
                      { "internalType": "bytes[]", "name": "inputs", "type": "bytes[]" },
                      { "internalType": "uint256", "name": "deadline", "type": "uint256" }
                    ],
                    "name": "execute",
                    "outputs": [],
                    "stateMutability": "payable",
                    "type": "function"
                  }"#;
        let data = hex::decode("3593564c000000000000000000000000000000000000000000000000000000000000006000000000000000000000000000000000000000000000000000000000000000a00000000000000000000000000000000000000000000000000000000064996e5f00000000000000000000000000000000000000000000000000000000000000020b000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000002000000000000000000000000000000000000000000000000000000000000004000000000000000000000000000000000000000000000000000000000000000a000000000000000000000000000000000000000000000000000000000000000400000000000000000000000000000000000000000000000000000000000000002000000000000000000000000000000000000000000000000002386f26fc1000000000000000000000000000000000000000000000000000000000000000001000000000000000000000000000000000000000000000000000000000000000001000000000000000000000000000000000000000000000000002386f26fc10000000000000000000000000000000000000000000000000000f84605ccc515414000000000000000000000000000000000000000000000000000000000000000a00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000002bc02aaa39b223fe8d0a0e5c4f27ead9083c756cc20001f46b175474e89094c44da98b954eedeac495271d0f000000000000000000000000000000000000000000").unwrap();
        let result = parse_method_data(data, "UniversalRouter".to_string(), json.to_string());
        assert_eq!(true, result.is_ok());
        let result = result.unwrap();
        assert_eq!("UniversalRouter", result.contract_name);
        assert_eq!("execute", result.method_name);
        assert_eq!(3, result.params.len());
        assert_eq!(result.params.get(0).unwrap().name, "commands");
        assert_eq!(result.params.get(0).unwrap().value, "0b00");
        assert_eq!(result.params.get(1).unwrap().name, "inputs");
        assert_eq!(result.params.get(1).unwrap().value, "[0000000000000000000000000000000000000000000000000000000000000002000000000000000000000000000000000000000000000000002386f26fc10000,0000000000000000000000000000000000000000000000000000000000000001000000000000000000000000000000000000000000000000002386f26fc10000000000000000000000000000000000000000000000000000f84605ccc515414000000000000000000000000000000000000000000000000000000000000000a00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000002bc02aaa39b223fe8d0a0e5c4f27ead9083c756cc20001f46b175474e89094c44da98b954eedeac495271d0f000000000000000000000000000000000000000000]");
        assert_eq!(result.params.get(2).unwrap().name, "deadline");
        assert_eq!(result.params.get(2).unwrap().value, "64996e5f");
    }
}
