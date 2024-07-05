use alloc::string::{String, ToString};

use crate::parser::overview::SolanaOverview;

#[derive(Clone, Debug)]
pub struct ParsedSolanaTx {
    pub display_type: SolanaTxDisplayType,
    pub overview: SolanaOverview,
    pub detail: String,
    pub network: String,
}

// method label on ui
#[derive(Clone, Debug)]
pub enum SolanaTxDisplayType {
    Transfer,
    Vote,
    General,
    Unknown,
}

impl ToString for SolanaTxDisplayType {
    fn to_string(&self) -> String {
        match &self {
            SolanaTxDisplayType::Transfer => "Transfer".to_string(),
            SolanaTxDisplayType::Vote => "Vote".to_string(),
            SolanaTxDisplayType::General => "General".to_string(),
            SolanaTxDisplayType::Unknown => "Unknown".to_string(),
        }
    }
}
