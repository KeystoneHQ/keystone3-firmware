use alloc::format;
use alloc::string::{String, ToString};
use core::ops::Div;
use ethereum_types::U256;

const F_DIVIDER: f64 = 1_000_000_000f64;

pub fn normalize_price(gas: u64) -> String {
    format!("{} Gwei", (gas as f64).div(F_DIVIDER))
}

pub fn normalize_value(value: U256) -> String {
    let value_str = value.to_string();
    if value_str == "0" {
        return "0".to_string();
    }

    let padded_value = format!("{value_str:0>18}");
    let len = padded_value.len();

    let res = if len <= 18 {
        let val = padded_value.trim_end_matches('0');
        if val.is_empty() {
            "0".to_string()
        } else {
            format!("0.{val}")
        }
    } else {
        let (int_part, decimal_part) = padded_value.split_at(len - 18);
        let decimal = decimal_part.trim_end_matches('0');
        if decimal.is_empty() {
            int_part.to_string()
        } else {
            format!("{int_part}.{decimal}")
        }
    };
    res
}

#[cfg(test)]
mod tests {
    use crate::normalizer::{normalize_price, normalize_value};
    use ethereum_types::U256;

    extern crate std;

    #[test]
    fn test_normalize_value_zero() {
        let value = U256::from(0u64);
        let result = normalize_value(value);
        assert_eq!("0", result);
    }

    #[test]
    fn test_normalize_value_small() {
        let value = U256::from(1u64);
        let result = normalize_value(value);
        assert_eq!("0.000000000000000001", result);

        let value = U256::from(1000000000000000u64);
        let result = normalize_value(value);
        assert_eq!("0.001", result);
    }

    #[test]
    fn test_normalize_value_medium() {
        let value = U256::from(1000000000000000000u64); // 1 ETH
        let result = normalize_value(value);
        assert_eq!("1", result);

        let value = U256::from(1500000000000000000u64); // 1.5 ETH
        let result = normalize_value(value);
        assert_eq!("1.5", result);
    }

    #[test]
    fn test_normalize_value_large() {
        let value = U256::from_dec_str("1000000000000000000000").unwrap(); // 1000 ETH
        let result = normalize_value(value);
        assert_eq!("1000", result);

        let value = U256::from_dec_str("1234567890000000000000").unwrap(); // 1234.56789 ETH
        let result = normalize_value(value);
        assert_eq!("1234.56789", result);
    }

    #[test]
    fn test_normalize_value_trailing_zeros() {
        let value = U256::from_dec_str("100000000000000000000").unwrap(); // 100 ETH
        let result = normalize_value(value);
        assert_eq!("100", result);

        let value = U256::from_dec_str("10000000000000000000").unwrap(); // 10 ETH
        let result = normalize_value(value);
        assert_eq!("10", result);
    }

    #[test]
    fn test_normalize_price() {
        let gas = 1000000000u64; // 1 Gwei
        let result = normalize_price(gas);
        assert_eq!("1 Gwei", result);

        let gas = 20000000000u64; // 20 Gwei
        let result = normalize_price(gas);
        assert_eq!("20 Gwei", result);

        let gas = 500000000u64; // 0.5 Gwei
        let result = normalize_price(gas);
        assert_eq!("0.5 Gwei", result);
    }
}
