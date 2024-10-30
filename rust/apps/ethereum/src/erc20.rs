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

pub fn encode_erc20_transfer_calldata(to: H160, amount: U256) -> String {
    // transfer(address recipient, uint256 amount) function signature is 0xa9059cbb
    let mut calldata = "a9059cbb".to_string();
    calldata.push_str(&format!("{:0>64}", hex::encode(to)));
    // convert value to hex and pad it to 64 bytes
    let amount_hex = format!("{:x}", amount);
    let amount_padding = format!("{:0>64}", amount_hex);
    calldata.push_str(&amount_padding);
    calldata
}

pub fn parse_erc20(input: &str, decimal: u32) -> Result<ParsedErc20Transaction, &'static str> {
    if input.len() != 136 {
        return Err("Input must be 136 characters long");
    }

    let to = match hex::decode(&input[32..72]) {
        Ok(bytes) => format!("0x{}", hex::encode(bytes)),
        Err(_) => return Err("Failed to decode 'to' address"),
    };

    let value_hex = &input[72..];
    let value_biguint = BigUint::parse_bytes(value_hex.as_bytes(), 16)
        .ok_or("Failed to parse 'value' as a big uint")?;

    let decimal_biguint = BigUint::from(10u64.pow(decimal));

    let value_decimal = value_biguint.clone() / &decimal_biguint;
    let remainder = &value_biguint % &decimal_biguint;

    let value = if remainder != BigUint::new(Vec::new()) {
        // If there is a remainder, convert it to a decimal
        let remainder_decimal = remainder.to_string();
        let padded_remainder = format!("{:0>width$}", remainder_decimal, width = decimal as usize);
        format!("{}.{}", value_decimal.to_string(), padded_remainder)
            .trim_end_matches('0')
            .to_string()
    } else {
        value_decimal.to_string()
    };

    Ok(ParsedErc20Transaction { to, value })
}

#[cfg(test)]
mod tests {
    use super::*;
    use core::str::FromStr;

    #[test]
    fn test_parse_erc20() {
        // Prepare an example input
        let input = "a9059cbb0000000000000000000000005df9b87991262f6ba471f09758cde1c0fc1de7340000000000000000000000000000000000000000000000000000000000000064";
        let decimal = 18;

        // Call the function
        let result = parse_erc20(input, decimal);

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

        let result1 = parse_erc20(input1, decimal);

        match result1 {
            Ok(transaction) => {
                assert_eq!(transaction.to, "0x5df9b87991262f6ba471f09758cde1c0fc1de734");
                assert_eq!(transaction.value, "10");
            }
            Err(err) => panic!("Test failed due to error: {}", err),
        }

        let input2 = "a9059cbb0000000000000000000000005df9b87991262f6ba471f09758cde1c0fc1de7340000000000000000000000000000000000000000000000000000000000000000";
        let result2 = parse_erc20(input2, decimal);

        match result2 {
            Ok(transaction) => {
                assert_eq!(transaction.to, "0x5df9b87991262f6ba471f09758cde1c0fc1de734");
                assert_eq!(transaction.value, "0");
            }
            Err(err) => panic!("Test failed due to error: {}", err),
        }

        let input3 = "a9059cbb0000000000000000000000005df9b87991262f6ba471f09758cde1c0fc1de734ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff";

        let result3 = parse_erc20(input3, decimal);

        match result3 {
            Ok(transaction) => {
                assert_eq!(transaction.to, "0x5df9b87991262f6ba471f09758cde1c0fc1de734");
                assert_eq!(transaction.value, "115792089237316195423570985008687907853269984665640564039457.584007913129639935");
            }
            Err(err) => panic!("Test failed due to error: {}", err),
        }
    }
}
