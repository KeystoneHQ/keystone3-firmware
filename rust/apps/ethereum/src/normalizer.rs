use alloc::format;
use alloc::string::{String, ToString};
use core::ops::Div;
use ethereum_types::U256;

static F_DIVIDER: f64 = 1_000_000_000f64;
static U_DIVIDER: u64 = 1_000_000_000;

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

    let mut res = if len <= 18 {
        let mut val = padded_value;
        while val.ends_with('0') {
            val.pop();
        }
        format!("0.{val}")
    } else {
        let (int_part, decimal_part) = padded_value.split_at(len - 18);
        let mut decimal = decimal_part.to_string();
        while decimal.ends_with('0') {
            decimal.pop();
        }
        format!("{int_part}.{decimal}")
    };
    if res.ends_with('.') {
        res.pop();
    }
    res
}

#[cfg(test)]
mod tests {
    use crate::normalizer::normalize_value;
    use ethereum_types::U256;

    extern crate std;

    use std::println;

    #[test]
    fn test() {
        let x = U256::from(000_000_100_000_000_001u64);
        let y = normalize_value(x);
        println!("{y}");
    }
}
