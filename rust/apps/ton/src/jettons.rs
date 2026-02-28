use alloc::{
    format,
    string::{String, ToString},
    vec,
    vec::Vec,
};

use lazy_static::lazy_static;

use itertools::Itertools;

use crate::errors::Result;
use crate::errors::TonError;

pub struct JettonData {
    pub contract_address: String,
    pub decimal: u8,
    pub symbol: String,
}

lazy_static! {
    static ref JETTONS: Vec<JettonData> = vec![
        JettonData {
            contract_address: "NULL".to_string(),
            decimal: 0,
            symbol: "Unit".to_string(),
        },
        JettonData {
            contract_address: "EQAvlWFDxGF2lXm67y4yzC17wYKD9A0guwPkMs1gOsM__NOT".to_string(),
            decimal: 8,
            symbol: "NOT".to_string(),
        },
        JettonData {
            contract_address: "EQCxE6mUtQJKFnGfaROTKOt1lZbDiiX1kCixRv7Nw2Id_sDs".to_string(),
            decimal: 6,
            symbol: "USDT".to_string(),
        },
        JettonData {
            contract_address: "EQA2kCVNwVsil2EM2mB0SkXytxCqQjS4mttjDpnXmwG9T6bO".to_string(),
            decimal: 9,
            symbol: "STON".to_string(),
        }
    ];
}

pub fn get_jetton_amount_text(coins: String, contract_address: String) -> Result<String> {
    let target = JETTONS
        .iter()
        .find_or_first(|v| v.contract_address.eq(&contract_address))
        .ok_or_else(|| {
            TonError::InvalidTransaction(format!(
                "Invalid jetton contract address: {contract_address}"
            ))
        })?;
    let value = coins
        .parse::<u128>()
        .map_err(|_e| TonError::InvalidTransaction(format!("Invalid jetton amount: {coins}")))?;
    let divisor = 10u128.pow(target.decimal as u32);

    let integer_part = value / divisor;
    let fractional_part = value % divisor;

    let fractional_str = format!(
        "{:0width$}",
        fractional_part,
        width = target.decimal as usize
    );
    let fractional_str = fractional_str.trim_end_matches('0');
    if fractional_str.is_empty() {
        Ok(format!("{} {}", integer_part, target.symbol))
    } else {
        Ok(format!(
            "{}.{} {}",
            integer_part, fractional_str, target.symbol
        ))
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    extern crate std;
    use alloc::string::ToString;

    #[test]
    fn test_get_jetton_amount_text() {
        let coins = "30110292000".to_string();
        let contract_address = "EQA2kCVNwVsil2EM2mB0SkXytxCqQjS4mttjDpnXmwG9T6bO".to_string();
        let result = get_jetton_amount_text(coins, contract_address).unwrap();
        assert_eq!(result, "30.110292 STON");
    }

    #[test]
    fn test_get_jetton_amount_null_unit() {
        let result = get_jetton_amount_text("100".to_string(), "NULL".to_string()).unwrap();
        assert_eq!(result, "100 Unit");
    }

    #[test]
    fn test_get_jetton_amount_not_token() {
        // NOT token has 8 decimals
        let result = get_jetton_amount_text(
            "100000000".to_string(),
            "EQAvlWFDxGF2lXm67y4yzC17wYKD9A0guwPkMs1gOsM__NOT".to_string(),
        )
        .unwrap();
        assert_eq!(result, "1 NOT");
    }

    #[test]
    fn test_get_jetton_amount_not_token_partial() {
        // NOT token has 8 decimals - testing fractional amount
        let result = get_jetton_amount_text(
            "12345678".to_string(),
            "EQAvlWFDxGF2lXm67y4yzC17wYKD9A0guwPkMs1gOsM__NOT".to_string(),
        )
        .unwrap();
        assert_eq!(result, "0.12345678 NOT");
    }

    #[test]
    fn test_get_jetton_amount_usdt_token() {
        // USDT has 6 decimals
        let result = get_jetton_amount_text("1000000".to_string(), "0".to_string()).unwrap();
        assert_eq!(result, "1 USDT");
    }

    #[test]
    fn test_get_jetton_amount_usdt_token_large() {
        // USDT has 6 decimals - testing large amount
        let result = get_jetton_amount_text("123456789000".to_string(), "0".to_string()).unwrap();
        assert_eq!(result, "123456.789 USDT");
    }

    #[test]
    fn test_get_jetton_amount_ston_token() {
        // STON token has 9 decimals
        let result = get_jetton_amount_text(
            "1000000000".to_string(),
            "EQA2kCVNwVsil2EM2mB0SkXytxCqQjS4mttjDpnXmwG9T6bO".to_string(),
        )
        .unwrap();
        assert_eq!(result, "1 STON");
    }

    #[test]
    fn test_get_jetton_amount_ston_token_partial() {
        // STON token has 9 decimals - testing fractional amount
        let result = get_jetton_amount_text(
            "987654321".to_string(),
            "EQA2kCVNwVsil2EM2mB0SkXytxCqQjS4mttjDpnXmwG9T6bO".to_string(),
        )
        .unwrap();
        assert_eq!(result, "0.987654321 STON");
    }

    #[test]
    fn test_get_jetton_amount_zero() {
        // Test zero amount
        let result = get_jetton_amount_text("0".to_string(), "NULL".to_string()).unwrap();
        assert_eq!(result, "0 Unit");
    }

    #[test]
    fn test_get_jetton_amount_unknown_address_fallback() {
        // Unknown contract address should fall back to first jetton (NULL/Unit)
        let result =
            get_jetton_amount_text("123".to_string(), "UNKNOWN_ADDRESS_12345".to_string()).unwrap();
        assert_eq!(result, "123 Unit");
    }

    #[test]
    fn test_get_jetton_amount_invalid_amount_string() {
        // Invalid amount string should return error
        let result = get_jetton_amount_text("not_a_number".to_string(), "NULL".to_string());
        assert!(result.is_err());

        if let Err(e) = result {
            assert!(e.to_string().contains("Invalid jetton amount"));
        }
    }

    #[test]
    fn test_get_jetton_amount_empty_amount() {
        // Empty amount string should return error
        let result = get_jetton_amount_text("".to_string(), "NULL".to_string());
        assert!(result.is_err());
    }

    #[test]
    fn test_get_jetton_amount_negative_amount() {
        // Negative amount should return error (u64 can't be negative)
        let result = get_jetton_amount_text("-100".to_string(), "NULL".to_string());
        assert!(result.is_err());
    }

    #[test]
    fn test_get_jetton_amount_large_number() {
        // Test with large number (limited by f64 precision)
        let result = get_jetton_amount_text(
            "1000000000000".to_string(), // 1 trillion - large but within f64 safe integer range
            "NULL".to_string(),
        )
        .unwrap();
        assert_eq!(result, "1000000000000 Unit");
    }

    #[test]
    fn test_get_jetton_amount_overflow() {
        // Test with number that exceeds u128::MAX
        let result = get_jetton_amount_text(
            "340282366920938463463374607431768211456".to_string(), // u128::MAX + 1
            "NULL".to_string(),
        );
        assert!(result.is_err());
    }

    #[test]
    fn test_get_jetton_amount_decimal_precision() {
        // Test decimal precision for each token type
        let result = get_jetton_amount_text("1".to_string(), "NULL".to_string()).unwrap();
        assert_eq!(result, "1 Unit"); // 0 decimals

        let result = get_jetton_amount_text("1".to_string(), "0".to_string()).unwrap();
        assert_eq!(result, "0.000001 USDT"); // 6 decimals

        let result = get_jetton_amount_text(
            "1".to_string(),
            "EQAvlWFDxGF2lXm67y4yzC17wYKD9A0guwPkMs1gOsM__NOT".to_string(),
        )
        .unwrap();
        assert_eq!(result, "0.00000001 NOT"); // 8 decimals

        let result = get_jetton_amount_text(
            "1".to_string(),
            "EQA2kCVNwVsil2EM2mB0SkXytxCqQjS4mttjDpnXmwG9T6bO".to_string(),
        )
        .unwrap();
        assert_eq!(result, "0.000000001 STON"); // 9 decimals
    }

    #[test]
    fn test_jetton_data_structure() {
        // Verify the JETTONS static data structure is correctly initialized
        assert_eq!(JETTONS.len(), 4);

        assert_eq!(JETTONS[0].contract_address, "NULL");
        assert_eq!(JETTONS[0].decimal, 0);
        assert_eq!(JETTONS[0].symbol, "Unit");

        assert_eq!(
            JETTONS[1].contract_address,
            "EQAvlWFDxGF2lXm67y4yzC17wYKD9A0guwPkMs1gOsM__NOT"
        );
        assert_eq!(JETTONS[1].decimal, 8);
        assert_eq!(JETTONS[1].symbol, "NOT");

        assert_eq!(JETTONS[2].contract_address, "0");
        assert_eq!(JETTONS[2].decimal, 6);
        assert_eq!(JETTONS[2].symbol, "USDT");

        assert_eq!(
            JETTONS[3].contract_address,
            "EQA2kCVNwVsil2EM2mB0SkXytxCqQjS4mttjDpnXmwG9T6bO"
        );
        assert_eq!(JETTONS[3].decimal, 9);
        assert_eq!(JETTONS[3].symbol, "STON");
    }

    #[test]
    fn test_get_jetton_amount_with_whitespace() {
        // Test that strings with whitespace fail (as they should)
        let result = get_jetton_amount_text("100 ".to_string(), "NULL".to_string());
        assert!(result.is_err());

        let result = get_jetton_amount_text(" 100".to_string(), "NULL".to_string());
        assert!(result.is_err());
    }
}
