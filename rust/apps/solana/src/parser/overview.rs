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
pub struct ProgramOverviewSplTokenTransfer {
    pub source: String,
    pub destination: String,
    pub authority: String,
    pub decimals: u8,
    pub amount: String,
    pub token_mint_account: String,
    pub token_symbol: String,
    pub token_name: String,
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
pub struct ProgramOverviewMultisigCreate {
    pub wallet_name: String,
    pub wallet_desc: String,
    pub threshold: u16,
    pub member_count: usize,
    pub members: Vec<String>,
    pub total_value: String,
    // transfer vec
    pub transfers: Vec<ProgramOverviewTransfer>,
}
#[derive(Debug, Clone)]
pub struct JupiterV6SwapTokenInfoOverview {
    pub token_name: String,
    pub token_symbol: String,
    pub token_address: String,
    pub token_amount: String,
    pub exist_in_address_lookup_table: bool,
}

#[derive(Debug, Clone)]
pub struct JupiterV6SwapOverview {
    pub program_name: String,
    pub program_address: String,
    pub instruction_name: String,
    pub token_a_overview: JupiterV6SwapTokenInfoOverview,
    pub token_b_overview: JupiterV6SwapTokenInfoOverview,
    pub slippage_bps: String,
    pub platform_fee_bps: String,
}

#[derive(Debug, Clone)]
pub struct ProgramOverviewProposal {
    pub program: String,
    pub method: String,
    pub memo: Option<String>,
    pub data: Option<String>,
}

#[derive(Debug, Clone)]
pub enum SolanaOverview {
    Transfer(ProgramOverviewTransfer),
    Vote(ProgramOverviewVote),
    General(Vec<ProgramOverviewGeneral>),
    Unknown(ProgramOverviewUnknown),
    Instructions(ProgramOverviewInstructions),
    SquadsV4MultisigCreate(ProgramOverviewMultisigCreate),
    SquadsV4MultisigCreateV2(ProgramOverviewMultisigCreate),
    SquadsV4Proposal(Vec<ProgramOverviewProposal>),
    SplTokenTransfer(ProgramOverviewSplTokenTransfer),
    JupiterV6SwapOverview(JupiterV6SwapOverview),
}
