use crate::parser::overview::NearTxOverview;

use alloc::string::{String, ToString};

#[derive(Clone, Debug)]
pub struct ParsedNearTx {
    pub display_type: NearTxDisplayType,
    pub overview: NearTxOverview,
    pub detail: String,
    pub network: String,
}

// method label on ui
#[derive(Clone, Debug)]
pub enum NearTxDisplayType {
    Transfer,
    General,
}

impl ToString for NearTxDisplayType {
    fn to_string(&self) -> String {
        match &self {
            NearTxDisplayType::Transfer => "Transfer".to_string(),
            NearTxDisplayType::General => "General".to_string(),
        }
    }
}
