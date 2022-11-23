use crate::transaction::overview::CosmosTxOverview;
use alloc::string::{String, ToString};
use serde::Serialize;

#[derive(Debug)]
pub enum SignMode {
    COSMOS,
    EVM,
}

#[derive(Debug)]
pub enum DataType {
    Amino,
    Direct,
}

#[derive(Clone, Debug, PartialEq)]
pub enum CosmosTxDisplayType {
    Send,
    Vote,
    Delegate,
    Undelegate,
    Redelegate,
    Transfer,
    WithdrawReward,
    Unknown,
    Multiple,
}

impl ToString for CosmosTxDisplayType {
    fn to_string(&self) -> String {
        match &self {
            CosmosTxDisplayType::Send => "Send".to_string(),
            CosmosTxDisplayType::Delegate => "Delegate".to_string(),
            CosmosTxDisplayType::Undelegate => "Undelegate".to_string(),
            CosmosTxDisplayType::Redelegate => "Redelegate".to_string(),
            CosmosTxDisplayType::WithdrawReward => "Withdraw Reward".to_string(),
            CosmosTxDisplayType::Transfer => "IBC Transfer".to_string(),
            CosmosTxDisplayType::Unknown => "Unknown".to_string(),
            CosmosTxDisplayType::Multiple => "Multiple".to_string(),
            CosmosTxDisplayType::Vote => "Vote".to_string(),
        }
    }
}

#[derive(Clone, Debug)]
pub struct ParsedCosmosTx {
    pub overview: CosmosTxOverview,
    pub detail: String,
}

#[derive(Clone, Debug, Serialize)]
pub struct FeeDetail {
    #[serde(
        skip_serializing_if = "String::is_empty",
        rename(serialize = "Max Fee")
    )]
    pub max_fee: String,
    #[serde(skip_serializing_if = "String::is_empty", rename(serialize = "Fee"))]
    pub fee: String,
    #[serde(
        skip_serializing_if = "String::is_empty",
        rename(serialize = "Gas Limit")
    )]
    pub gas_limit: String,
}
