use alloc::format;
use alloc::string::String;

/// convert wei to eth
pub fn convert_wei_to_eth(wei: &str) -> String {
    let wei = wei.parse::<f64>().unwrap();
    let eth = wei / 1_000_000_000_000_000_000.0;
    format!("{:.6}", eth)
}

/// calculate the max_txn_fee = gas_price * gas_limit
pub fn calculate_max_txn_fee(gase_price: &str, gas_limit: &str) -> String {
    let gas_price = gase_price.parse::<f64>().unwrap();
    let gas_limit = gas_limit.parse::<f64>().unwrap();
    let max_txn_fee = gas_price * gas_limit;
    format!("{:.6}", max_txn_fee)
}
