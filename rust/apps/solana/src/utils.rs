use alloc::string::{String, ToString};

use crate::errors::{Result, SolanaError};

pub fn format_token_amount(amount: &str, decimals: u8) -> Result<String> {
    if amount.is_empty() || !amount.bytes().all(|value| value.is_ascii_digit()) {
        return Err(SolanaError::InvalidData("invalid token amount".to_string()));
    }

    let normalized = amount.trim_start_matches('0');
    let digits = if normalized.is_empty() {
        "0"
    } else {
        normalized
    };
    let decimals = decimals as usize;
    if decimals == 0 {
        return Ok(digits.to_string());
    }

    let mut result = String::new();
    if digits.len() <= decimals {
        result.push_str("0.");
        for _ in 0..decimals - digits.len() {
            result.push('0');
        }
        result.push_str(digits);
    } else {
        let split = digits.len() - decimals;
        result.push_str(&digits[..split]);
        result.push('.');
        result.push_str(&digits[split..]);
    }

    while result.ends_with('0') {
        result.pop();
    }
    if result.ends_with('.') {
        result.pop();
    }
    Ok(result)
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn formats_large_token_amounts_without_floating_point() {
        assert_eq!(
            format_token_amount("18446744073709551615", 6).unwrap(),
            "18446744073709.551615"
        );
        assert_eq!(format_token_amount("1000000", 6).unwrap(), "1");
        assert_eq!(format_token_amount("1", 10).unwrap(), "0.0000000001");
        assert_eq!(format_token_amount("0", u8::MAX).unwrap(), "0");
    }
}
