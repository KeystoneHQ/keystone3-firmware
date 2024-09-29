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
    TokenTransfer,
    Vote,
    General,
    Unknown,
    SquadsV4,
    JupiterV6,
}

impl ToString for SolanaTxDisplayType {
    fn to_string(&self) -> String {
        match &self {
            SolanaTxDisplayType::Transfer => "Transfer".to_string(),
            SolanaTxDisplayType::Vote => "Vote".to_string(),
            SolanaTxDisplayType::General => "General".to_string(),
            SolanaTxDisplayType::Unknown => "Unknown".to_string(),
            SolanaTxDisplayType::SquadsV4 => "SquadsV4".to_string(),
            SolanaTxDisplayType::TokenTransfer => "TokenTransfer".to_string(),
            SolanaTxDisplayType::JupiterV6 => "JupiterV6".to_string(),
        }
    }
}
