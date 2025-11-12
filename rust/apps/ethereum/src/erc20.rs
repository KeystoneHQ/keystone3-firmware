extern crate alloc;
use alloc::format;
use alloc::string::{String, ToString};
use alloc::vec::Vec;
use ethereum_types::{H160, U256};
use hex;
use rsa::BigUint;

pub struct ParsedErc20Transaction {
    pub to: String,
    pub value: String,
}

pub struct ParsedErc20Approval {
    pub spender: String,
    pub value: String,
}

// ERC20 transfer function selector: keccak256("transfer(address,uint256)")[0:4] = 0xa9059cbb
// Reference: https://eips.ethereum.org/EIPS/eip-20
const TRANSFER_SELECTOR: &str = "a9059cbb";
// ERC20 approve function selector: keccak256("approve(address,uint256)")[0:4] = 0x095ea7b3
const APPROVE_SELECTOR: &str = "095ea7b3";
// ABI encoding: selector(8) + address(64) + amount(64) = 136 hex chars (transfer/approve)
const CALLDATA_LEN: usize = 136;
const SELECTOR_LEN: usize = 8; // 4 bytes -> 8 hex chars
const ADDRESS_START: usize = 32; // 32..72: address (right-aligned in 32-byte slot)
const ADDRESS_END: usize = 72;
const AMOUNT_START: usize = 72; // 72..136: amount

/// Encode ERC20 transfer function call data
/// Returns: function_selector(8) + encoded_address(64) + encoded_amount(64) = 136 hex chars
pub fn encode_erc20_transfer_calldata(to: H160, amount: U256) -> String {
    let mut calldata = String::with_capacity(CALLDATA_LEN);
    calldata.push_str(TRANSFER_SELECTOR);
    // Address: 20 bytes padded to 32 bytes (64 hex chars)
    calldata.push_str(&format!("{:0>64}", hex::encode(to.as_bytes())));
    // Amount: 32 bytes (64 hex chars)
    calldata.push_str(&format!("{amount:0>64x}"));
    calldata
}

// parse erc20 transfer calldata
pub fn parse_erc20_transfer(
    input: &str,
    decimal: u32,
) -> Result<ParsedErc20Transaction, &'static str> {
    validate_calldata_length(input)?;
    if &input[0..SELECTOR_LEN] != TRANSFER_SELECTOR {
        return Err("Invalid transfer function selector");
    }

    let to = decode_address_from_calldata(input)?;
    let value = parse_amount_from_calldata(input, decimal)?;

    Ok(ParsedErc20Transaction { to, value })
}

pub fn parse_erc20_approval(
    input: &str,
    decimal: u32,
) -> Result<ParsedErc20Approval, &'static str> {
    validate_calldata_length(input)?;
    if &input[0..SELECTOR_LEN] != APPROVE_SELECTOR {
        return Err("Invalid approve function selector");
    }

    let spender = decode_address_from_calldata(input)?;
    let value = parse_amount_from_calldata(input, decimal)?;

    Ok(ParsedErc20Approval { spender, value })
}

fn validate_calldata_length(input: &str) -> Result<(), &'static str> {
    if input.len() != CALLDATA_LEN {
        Err("Input must be 136 characters long")
    } else {
        Ok(())
    }
}

fn decode_address_from_calldata(input: &str) -> Result<String, &'static str> {
    let address_hex = &input[ADDRESS_START..ADDRESS_END];
    let address_bytes = hex::decode(address_hex).map_err(|_| "Failed to decode address hex")?;
    let address = &address_bytes[address_bytes.len().saturating_sub(20)..];
    Ok(format!("0x{}", hex::encode(address)))
}

fn parse_amount_from_calldata(input: &str, decimal: u32) -> Result<String, &'static str> {
    let value_hex = &input[AMOUNT_START..];
    let value_biguint = BigUint::parse_bytes(value_hex.as_bytes(), 16)
        .ok_or("Failed to parse 'value' as a big uint")?;

    let decimal_biguint = BigUint::from(10u64.pow(decimal));
    let value_decimal = &value_biguint / &decimal_biguint;
    let remainder = &value_biguint % &decimal_biguint;

    let value = if remainder != BigUint::new(Vec::new()) {
        let remainder_decimal = remainder.to_string();
        let padded_remainder = format!("{:0>width$}", remainder_decimal, width = decimal as usize);
        format!("{value_decimal}.{padded_remainder}")
            .trim_end_matches('0')
            .to_string()
    } else {
        value_decimal.to_string()
    };

    Ok(value)
}
#[cfg(test)]
mod tests {
    use super::*;
    use crate::crypto::keccak256;
    use core::str::FromStr;

    #[test]
    fn test_parse_erc20() {
        // Prepare an example input
        let input = "a9059cbb0000000000000000000000005df9b87991262f6ba471f09758cde1c0fc1de7340000000000000000000000000000000000000000000000000000000000000064";
        let decimal = 18;

        // Call the function
        let result = parse_erc20_transfer(input, decimal);

        // Check the result
        match result {
            Ok(transaction) => {
                assert_eq!(transaction.to, "0x5df9b87991262f6ba471f09758cde1c0fc1de734");
                assert_eq!(transaction.value, "0.0000000000000001"); // modify this as per your expected output
            }
            Err(err) => panic!("Test failed due to error: {}", err),
        }
    }

    #[test]
    fn test_encode_erco20_transfer_calldata() {
        let to = H160::from_str("0x49aB56B91fc982Fd6Ec1EC7Bb87d74EFA6dA30ab").unwrap();
        let calldata = encode_erc20_transfer_calldata(to, U256::from(92341344232709677u64));
        assert_eq!(calldata, "a9059cbb00000000000000000000000049ab56b91fc982fd6ec1ec7bb87d74efa6da30ab00000000000000000000000000000000000000000000000001480ff69d129e2d".to_string());
    }
    #[test]
    fn test_parse_erc20_boundary() {
        let input1 = "a9059cbb0000000000000000000000005df9b87991262f6ba471f09758cde1c0fc1de7340000000000000000000000000000000000000000000000008ac7230489e80000";
        let decimal = 18;

        let result1 = parse_erc20_transfer(input1, decimal);

        match result1 {
            Ok(transaction) => {
                assert_eq!(transaction.to, "0x5df9b87991262f6ba471f09758cde1c0fc1de734");
                assert_eq!(transaction.value, "10");
            }
            Err(err) => panic!("Test failed due to error: {}", err),
        }

        let input2 = "a9059cbb0000000000000000000000005df9b87991262f6ba471f09758cde1c0fc1de7340000000000000000000000000000000000000000000000000000000000000000";
        let result2 = parse_erc20_transfer(input2, decimal);

        match result2 {
            Ok(transaction) => {
                assert_eq!(transaction.to, "0x5df9b87991262f6ba471f09758cde1c0fc1de734");
                assert_eq!(transaction.value, "0");
            }
            Err(err) => panic!("Test failed due to error: {}", err),
        }

        let input3 = "a9059cbb0000000000000000000000005df9b87991262f6ba471f09758cde1c0fc1de734ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff";

        let result3 = parse_erc20_transfer(input3, decimal);

        match result3 {
            Ok(transaction) => {
                assert_eq!(transaction.to, "0x5df9b87991262f6ba471f09758cde1c0fc1de734");
                assert_eq!(transaction.value, "115792089237316195423570985008687907853269984665640564039457.584007913129639935");
            }
            Err(err) => panic!("Test failed due to error: {}", err),
        }
    }

    #[test]
    fn test_transfer_function_selector_deterministic() {
        let function_signature = "transfer(address,uint256)";

        let hash = keccak256(function_signature.as_bytes());

        let selector = &hash[0..4];
        let selector_hex = hex::encode(selector);

        assert_eq!(
            selector_hex, "a9059cbb",
            "keccak256('transfer(address,uint256)')[0:4] = a9059cbb"
        );
    }

    #[test]
    fn test_parse_erc20_approval() {
        // Build an example ERC20 approve calldata:
        // selector (095ea7b3) + spender (padded to 64) + amount (padded to 64)
        let spender = "5df9b87991262f6ba471f09758cde1c0fc1de734";
        let amount_hex = "6acfc0"; // 7,000,000 (decimal)
        let calldata = format!(
            "095ea7b3{spender_padded}{amount_padded}",
            spender_padded = format!("{:0>64}", spender),
            amount_padded = format!("{:0>64}", amount_hex),
        );

        let decimal = 18;
        let result = parse_erc20_approval(&calldata, decimal);

        match result {
            Ok(approval) => {
                assert_eq!(approval.spender, format!("0x{spender}"));
                // 7,000,000 / 10^18 = 0.000000000007
                assert_eq!(approval.value, "0.000000000007");
            }
            Err(err) => panic!("Approval parse failed: {}", err),
        }
    }
}
