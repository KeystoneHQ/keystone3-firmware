use alloc::string::{String, ToString};
use alloc::vec::Vec;

#[derive(Debug, Clone)]
pub struct ProgramOverviewTransfer {
    pub value: String,
    pub main_action: String,
    pub from: String,
    pub to: String,
}

#[derive(Debug, Clone)]
pub struct ProgramOverviewVote {
    pub votes_on: Vec<String>,
    pub main_action: String,
    pub vote_account: String,
}

#[derive(Debug, Clone)]
pub struct ProgramOverviewGeneral {
    pub program: String,
    pub method: String,
}

#[derive(Debug, Clone)]
pub struct ProgramOverviewInstructions {
    pub overview_accounts: Vec<String>,
    pub overview_instructions: Vec<ProgramOverviewInstruction>,
}

#[derive(Debug, Clone)]
pub struct ProgramOverviewInstruction {
    pub accounts: Vec<String>,
    pub data: String,
    pub program_address: String,
}

#[derive(Debug, Clone)]
pub struct ProgramOverviewUnknown {
    pub description: String,
}

impl Default for ProgramOverviewUnknown {
    fn default() -> Self {
        Self {
            description: "This transaction can not be decoded".to_string(),
        }
    }
}

#[derive(Debug, Clone)]
pub enum SolanaOverview {
    Transfer(ProgramOverviewTransfer),
    Vote(ProgramOverviewVote),
    General(Vec<ProgramOverviewGeneral>),
    Unknown(ProgramOverviewUnknown),
    Instructions(ProgramOverviewInstructions),
}
