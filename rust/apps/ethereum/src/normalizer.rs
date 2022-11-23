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
    let mut value = format!("{}", value.div(U_DIVIDER));
    let len = value.len();
    let mut res = match len {
        0 => "0".to_string(),
        1..10 => {
            //less than 1 ETH
            value = format!("{:0>9}", value);
            while value.ends_with('0') {
                value.pop();
            }
            format!("0.{}", value)
        }
        _l => {
            let (int_part, decimal_part) = value.split_at(len - 9);
            let mut decimal_part = decimal_part.to_string();
            while decimal_part.ends_with('0') {
                decimal_part.pop();
            }
            format!("{}.{}", int_part, decimal_part)
        }
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
        println!("{}", y);
    }
}
