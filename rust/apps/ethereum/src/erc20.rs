extern crate alloc;
use alloc::format;
use alloc::string::{String, ToString};
use alloc::vec::Vec;
use third_party::hex;
use third_party::rsa::BigUint;

pub struct ParsedErc20Transaction {
    pub to: String,
    pub value: String,
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
}
