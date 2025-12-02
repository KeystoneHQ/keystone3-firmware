use crate::parser::overview::XrpTxOverview;
use alloc::string::{String, ToString};

#[derive(Clone, Debug)]
pub struct ParsedXrpTx {
    pub display_type: XrpTxDisplayType,
    pub overview: XrpTxOverview,
    pub detail: String,
    pub network: String,
    pub signing_pubkey: String,
    pub service_fee_detail: Option<String>,
}

// method label on ui
#[derive(Clone, Debug, PartialEq)]
pub enum XrpTxDisplayType {
    Payment,
    General,
}

impl ToString for XrpTxDisplayType {
    fn to_string(&self) -> String {
        match &self {
            XrpTxDisplayType::Payment => "Payment".to_string(),
            XrpTxDisplayType::General => "General".to_string(),
        }
    }
}
