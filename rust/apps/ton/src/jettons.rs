use alloc::{
    format,
    string::{String, ToString},
    vec,
    vec::Vec,
};

use lazy_static::lazy_static;

use itertools::Itertools;

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
            contract_address: "0".to_string(),
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

pub fn get_jetton_amount_text(coins: String, contract_address: String) -> String {
    let target = JETTONS
        .iter()
        .find_or_first(|v| v.contract_address.eq(&contract_address))
        .unwrap();
    let value = coins.parse::<u128>().unwrap();
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
        format!("{} {}", integer_part, target.symbol)
    } else {
        format!("{}.{} {}", integer_part, fractional_str, target.symbol)
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_get_jetton_amount_text() {
        let coins = "30110292000".to_string();
        let contract_address = "EQA2kCVNwVsil2EM2mB0SkXytxCqQjS4mttjDpnXmwG9T6bO".to_string();
        let result = get_jetton_amount_text(coins, contract_address);
        assert_eq!(result, "30.110292 STON");
    }
}
