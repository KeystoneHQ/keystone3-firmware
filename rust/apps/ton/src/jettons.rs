use alloc::{
    format,
    string::{String, ToString},
    vec,
    vec::Vec,
};

use lazy_static::lazy_static;
use num_bigint::BigUint;
use third_party::itertools::Itertools;

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
    let value = u64::from_str_radix(&coins, 10).unwrap();
    let divisor = 10u64.pow(target.decimal as u32) as f64;
    return format!("{} {}", (value as f64) / divisor, target.symbol);
}
