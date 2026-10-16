use alloc::string::{String, ToString};

use alloc::vec::Vec;

use crate::parser::overview::{ProgramOverviewGeneral, SolanaOverview};

#[derive(Clone, Debug)]
pub struct ParsedSolanaTx {
    pub display_type: SolanaTxDisplayType,
    pub overview: SolanaOverview,
    pub additional_overviews: Vec<ProgramOverviewGeneral>,
    pub unknown_programs: Vec<String>,
    pub overview_order: Vec<(usize, OverviewAction)>,
    pub detail: String,
    pub advanced: Vec<crate::advanced::AdvancedItem>,
    pub network: String,
}

#[derive(Clone, Debug, PartialEq)]
pub enum OverviewAction {
    Primary,
    General(usize),
    Unknown(usize),
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

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn display_types_have_stable_labels() {
        let cases = [
            (SolanaTxDisplayType::Transfer, "Transfer"),
            (SolanaTxDisplayType::TokenTransfer, "TokenTransfer"),
            (SolanaTxDisplayType::Vote, "Vote"),
            (SolanaTxDisplayType::General, "General"),
            (SolanaTxDisplayType::Unknown, "Unknown"),
            (SolanaTxDisplayType::SquadsV4, "SquadsV4"),
            (SolanaTxDisplayType::JupiterV6, "JupiterV6"),
        ];

        for (display_type, expected) in cases {
            assert_eq!(display_type.to_string(), expected);
        }
    }
}
