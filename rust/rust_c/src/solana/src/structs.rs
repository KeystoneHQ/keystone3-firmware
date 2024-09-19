use alloc::boxed::Box;
use alloc::string::ToString;
use alloc::vec::Vec;
use core::ptr::null_mut;

use app_solana::parser::overview::{ProgramOverviewGeneral, SolanaOverview};
use app_solana::parser::structs::{ParsedSolanaTx, SolanaTxDisplayType};
use app_solana::structs::SolanaMessage;
use third_party::itertools::Itertools;

use common_rust_c::ffi::VecFFI;
use common_rust_c::free::Free;
use common_rust_c::structs::TransactionParseResult;
use common_rust_c::types::{PtrString, PtrT};
use common_rust_c::utils::convert_c_char;
use common_rust_c::{check_and_free_ptr, free_str_ptr, impl_c_ptr, impl_c_ptrs, make_free_method};

#[repr(C)]
pub struct DisplaySolanaTx {
    pub network: PtrString,
    pub overview: PtrT<DisplaySolanaTxOverview>,
    pub detail: PtrString,
}

#[repr(C)]
pub struct DisplaySolanaTxOverviewGeneral {
    pub program: PtrString,
    pub method: PtrString,
}

impl Free for DisplaySolanaTxOverviewGeneral {
    fn free(&self) {
        free_str_ptr!(self.program);
        free_str_ptr!(self.method);
    }
}

impl From<&ProgramOverviewGeneral> for DisplaySolanaTxOverviewGeneral {
    fn from(value: &ProgramOverviewGeneral) -> Self {
        Self {
            program: convert_c_char(value.program.to_string()),
            method: convert_c_char(value.method.to_string()),
        }
    }
}
#[repr(C)]
pub struct DisplaySolanaTxOverviewUnknownInstructions {
    pub overview_accounts: PtrT<VecFFI<PtrString>>,
    pub overview_instructions: PtrT<VecFFI<Instruction>>,
}

impl_c_ptrs!(DisplaySolanaTxOverviewUnknownInstructions);

#[repr(C)]
pub struct Instruction {
    pub accounts: PtrT<VecFFI<PtrString>>,
    pub data: PtrString,
    pub program_address: PtrString,
}

impl Free for DisplaySolanaTxOverviewUnknownInstructions {
    fn free(&self) {
        unsafe {
            if !self.overview_accounts.is_null() {
                let x = Box::from_raw(self.overview_accounts);
                let ve = Vec::from_raw_parts(x.data, x.size, x.cap);
                ve.iter().for_each(|v| {
                    v.free();
                });
            }
            if !self.overview_instructions.is_null() {
                let x = Box::from_raw(self.overview_instructions);
                let ve = Vec::from_raw_parts(x.data, x.size, x.cap);
                ve.iter().for_each(|v| {
                    let accounts = Box::from_raw(v.accounts);
                    let accounts = Vec::from_raw_parts(accounts.data, accounts.size, accounts.cap);
                    accounts.iter().for_each(|a| {
                        free_str_ptr!(*a);
                    });
                    free_str_ptr!(v.data);
                    free_str_ptr!(v.program_address);
                });
            }
        }
    }
}

#[repr(C)]
pub struct DisplaySolanaTxProposalOverview {
    pub program: PtrString,
    pub method: PtrString,
    pub memo: PtrString,
    pub data: PtrString,
}
impl_c_ptrs!(DisplaySolanaTxProposalOverview);
impl Free for DisplaySolanaTxProposalOverview {
    fn free(&self) {
        free_str_ptr!(self.program);
        free_str_ptr!(self.method);
        free_str_ptr!(self.memo);
        free_str_ptr!(self.data);
    }
}
#[repr(C)]
pub struct DisplaySolanaTxSplTokenTransferOverview {
    pub source: PtrString,
    pub destination: PtrString,
    pub authority: PtrString,
    pub decimals: u8,
    pub amount: PtrString,
    pub token_mint_account: PtrString,
    pub token_symbol: PtrString,
    pub token_name: PtrString,
}
impl_c_ptrs!(DisplaySolanaTxSplTokenTransferOverview);
impl Free for DisplaySolanaTxSplTokenTransferOverview {
    fn free(&self) {
        free_str_ptr!(self.source);
        free_str_ptr!(self.destination);
        free_str_ptr!(self.authority);
        free_str_ptr!(self.amount);
        free_str_ptr!(self.token_mint_account);
        free_str_ptr!(self.token_symbol);
        free_str_ptr!(self.token_name);
    }
}

#[repr(C)]
pub struct JupiterV6SwapTokenInfoOverview {
    pub token_name: PtrString,
    pub token_symbol: PtrString,
    pub token_address: PtrString,
    pub token_amount: PtrString,
    pub exist_in_address_lookup_table: bool,
}

#[repr(C)]
pub struct DisplaySolanaTxOverviewJupiterV6Swap {
    pub program_name: PtrString,
    pub program_address: PtrString,
    pub instruction_name: PtrString,
    pub token_a_overview: PtrT<JupiterV6SwapTokenInfoOverview>,
    pub token_b_overview: PtrT<JupiterV6SwapTokenInfoOverview>,
    pub slippage_bps: PtrString,
    pub platform_fee_bps: PtrString,
}
impl_c_ptrs!(DisplaySolanaTxOverviewJupiterV6Swap, JupiterV6SwapTokenInfoOverview);

impl Free for JupiterV6SwapTokenInfoOverview {
    fn free(&self) {
        free_str_ptr!(self.token_name);
        free_str_ptr!(self.token_symbol);
        free_str_ptr!(self.token_address);
        free_str_ptr!(self.token_amount);
    }
}
impl Free for DisplaySolanaTxOverviewJupiterV6Swap {
    fn free(&self) {
        free_str_ptr!(self.program_name);
        free_str_ptr!(self.program_address);
        free_str_ptr!(self.instruction_name);
        free_str_ptr!(self.slippage_bps);
        free_str_ptr!(self.platform_fee_bps);
        if !self.token_a_overview.is_null() {
            let x = unsafe { Box::from_raw(self.token_a_overview) };
            x.free();
        }
        if !self.token_b_overview.is_null() {
            let x = unsafe { Box::from_raw(self.token_b_overview) };
            x.free();
        }
    }
}



#[repr(C)]
pub struct DisplaySolanaTxOverview {
    // `Transfer`, `Vote`, `General`, `Unknown`
    pub display_type: PtrString,
    // 'Transfer', 'Vote'
    pub main_action: PtrString,
    // transfer
    pub transfer_value: PtrString,
    pub transfer_from: PtrString,
    pub transfer_to: PtrString,
    // vote
    pub votes_on: PtrT<VecFFI<DisplaySolanaTxOverviewVotesOn>>,
    pub vote_account: PtrString,
    // general
    pub general: PtrT<VecFFI<DisplaySolanaTxOverviewGeneral>>,
    // instructions
    pub unknown_instructions: PtrT<DisplaySolanaTxOverviewUnknownInstructions>,

    // squads_v4
    pub squads_multisig_create: PtrT<DisplaySolanaTxOverviewSquadsV4MultisigCreate>,
    pub squads_proposal: PtrT<VecFFI<DisplaySolanaTxProposalOverview>>,
    // spl token transfer
    pub spl_token_transfer: PtrT<DisplaySolanaTxSplTokenTransferOverview>,
    
    // jupiter_v6 swap 
    pub jupiter_v6_swap: PtrT<DisplaySolanaTxOverviewJupiterV6Swap>,
}

#[repr(C)]
pub struct DisplaySolanaTxOverviewSquadsV4MultisigCreate {
    pub wallet_name: PtrString,
    pub wallet_desc: PtrString,
    pub threshold: u16,
    pub member_count: usize,
    pub members: PtrT<VecFFI<PtrString>>,
    pub total_value: PtrString,
    // transfer vec
    pub transfers: PtrT<VecFFI<ProgramOverviewTransfer>>,
}

#[repr(C)]
pub struct ProgramOverviewTransfer {
    pub value: PtrString,
    pub main_action: PtrString,
    pub from: PtrString,
    pub to: PtrString,
}

impl_c_ptrs!(
    DisplaySolanaTxOverviewSquadsV4MultisigCreate,
    ProgramOverviewTransfer
);

impl Free for DisplaySolanaTxOverviewSquadsV4MultisigCreate {
    fn free(&self) {
        free_str_ptr!(self.wallet_name);
        free_str_ptr!(self.wallet_desc);
        free_str_ptr!(self.total_value);
        unsafe {
            if !self.members.is_null() {
                let x = Box::from_raw(self.members);
                let ve = Vec::from_raw_parts(x.data, x.size, x.cap);
                ve.iter().for_each(|v| {
                    free_str_ptr!(*v);
                });
            }
            if !self.transfers.is_null() {
                let x = Box::from_raw(self.transfers);
                let ve = Vec::from_raw_parts(x.data, x.size, x.cap);
                ve.iter().for_each(|v| {
                    v.free();
                });
            }
        }
    }
}

impl Free for ProgramOverviewTransfer {
    fn free(&self) {
        free_str_ptr!(self.value);
        free_str_ptr!(self.main_action);
        free_str_ptr!(self.from);
        free_str_ptr!(self.to);
    }
}

#[repr(C)]
pub struct DisplaySolanaTxOverviewVotesOn {
    slot: PtrString,
}

impl Free for DisplaySolanaTxOverviewVotesOn {
    fn free(&self) {
        free_str_ptr!(self.slot);
    }
}

impl_c_ptrs!(DisplaySolanaTx, DisplaySolanaTxOverview);

impl Default for DisplaySolanaTxOverview {
    fn default() -> Self {
        Self {
            display_type: null_mut(),
            transfer_value: null_mut(),
            main_action: null_mut(),
            transfer_from: null_mut(),
            transfer_to: null_mut(),
            votes_on: null_mut(),
            vote_account: null_mut(),
            general: null_mut(),
            unknown_instructions: null_mut(),
            squads_multisig_create: null_mut(),
            squads_proposal: null_mut(),
            spl_token_transfer: null_mut(),
            jupiter_v6_swap: null_mut(),
        }
    }
}

impl Free for DisplaySolanaTx {
    fn free(&self) {
        check_and_free_ptr!(self.overview);
        free_str_ptr!(self.network);
        free_str_ptr!(self.detail);
    }
}

impl Free for DisplaySolanaTxOverview {
    fn free(&self) {
        free_str_ptr!(self.display_type);
        free_str_ptr!(self.main_action);
        free_str_ptr!(self.transfer_value);
        free_str_ptr!(self.transfer_from);
        free_str_ptr!(self.transfer_to);
        free_str_ptr!(self.vote_account);
        unsafe {
            if !self.general.is_null() {
                let x = Box::from_raw(self.general);
                let ve = Vec::from_raw_parts(x.data, x.size, x.cap);
                ve.iter().for_each(|v| {
                    v.free();
                });
            }
            if !self.votes_on.is_null() {
                let x = Box::from_raw(self.votes_on);
                let ve = Vec::from_raw_parts(x.data, x.size, x.cap);
                ve.iter().for_each(|v| {
                    v.free();
                });
            }
            // free unknown_instructions
            if !self.unknown_instructions.is_null() {
                let x = Box::from_raw(self.unknown_instructions);
                x.free();
            }
            if !self.squads_multisig_create.is_null() {
                let x = Box::from_raw(self.squads_multisig_create);
                x.free();
            }

            if !self.squads_proposal.is_null() {
                let x = Box::from_raw(self.squads_proposal);
                let ve = Vec::from_raw_parts(x.data, x.size, x.cap);
                ve.iter().for_each(|v| {
                    v.free();
                });
            }
            if !self.spl_token_transfer.is_null() {
                let x = Box::from_raw(self.spl_token_transfer);
                x.free();
            }
        }
    }
}

impl From<ParsedSolanaTx> for DisplaySolanaTx {
    fn from(value: ParsedSolanaTx) -> Self {
        DisplaySolanaTx {
            network: convert_c_char(value.network.to_string()),
            overview: DisplaySolanaTxOverview::from(&value).c_ptr(),
            detail: convert_c_char(value.detail),
        }
    }
}

impl From<&ParsedSolanaTx> for DisplaySolanaTxOverview {
    fn from(value: &ParsedSolanaTx) -> Self {
        let display_type = convert_c_char(value.display_type.to_string());
        match value.display_type {
            SolanaTxDisplayType::Transfer => {
                if let SolanaOverview::Transfer(overview) = &value.overview {
                    return Self {
                        display_type,
                        //transfer
                        transfer_value: convert_c_char(overview.value.to_string()),
                        main_action: convert_c_char(overview.main_action.to_string()),
                        transfer_from: convert_c_char(overview.from.to_string()),
                        transfer_to: convert_c_char(overview.to.to_string()),
                        ..DisplaySolanaTxOverview::default()
                    };
                }
            }
            SolanaTxDisplayType::TokenTransfer => {
                if let SolanaOverview::SplTokenTransfer(overview) = &value.overview {
                    return Self {
                        display_type,
                        spl_token_transfer: DisplaySolanaTxSplTokenTransferOverview {
                            source: convert_c_char(overview.source.to_string()),
                            destination: convert_c_char(overview.destination.to_string()),
                            authority: convert_c_char(overview.authority.to_string()),
                            decimals: overview.decimals,
                            amount: convert_c_char(overview.amount.to_string()),
                            token_mint_account: convert_c_char(
                                overview.token_mint_account.to_string(),
                            ),
                            token_symbol: convert_c_char(overview.token_symbol.to_string()),
                            token_name: convert_c_char(overview.token_name.to_string()),
                        }
                        .c_ptr(),
                        ..DisplaySolanaTxOverview::default()
                    };
                }
            }
            SolanaTxDisplayType::Vote => {
                if let SolanaOverview::Vote(overview) = &value.overview {
                    return Self {
                        display_type,
                        //vote
                        votes_on: VecFFI::from(
                            overview
                                .votes_on
                                .iter()
                                .map(|v| DisplaySolanaTxOverviewVotesOn {
                                    slot: convert_c_char(v.to_string()),
                                })
                                .collect_vec(),
                        )
                        .c_ptr(),
                        main_action: convert_c_char(overview.main_action.to_string()),
                        vote_account: convert_c_char(overview.vote_account.to_string()),
                        ..DisplaySolanaTxOverview::default()
                    };
                }
            }
            SolanaTxDisplayType::General => {
                if let SolanaOverview::General(overview) = &value.overview {
                    return Self {
                        display_type,
                        //general
                        general: VecFFI::from(
                            overview
                                .iter()
                                .map(DisplaySolanaTxOverviewGeneral::from)
                                .collect_vec(),
                        )
                        .c_ptr(),
                        ..DisplaySolanaTxOverview::default()
                    };
                }
            }
            SolanaTxDisplayType::SquadsV4 => {
                if let SolanaOverview::SquadsV4Proposal(overview) = &value.overview {
                    let display_type = convert_c_char("squads_proposal".to_string());
                    let mut squads_proposal = VecFFI::from(
                        overview
                            .iter()
                            .map(|v| DisplaySolanaTxProposalOverview {
                                program: convert_c_char(v.program.to_string()),
                                method: convert_c_char(v.method.to_string()),
                                memo: convert_c_char(v.memo.clone().unwrap_or_default()),
                                data: convert_c_char(v.data.clone().unwrap_or_default()),
                            })
                            .collect_vec(),
                    );
                    return Self {
                        display_type,
                        squads_proposal: squads_proposal.c_ptr(),
                        ..DisplaySolanaTxOverview::default()
                    };
                }

                if let SolanaOverview::SquadsV4MultisigCreate(overview) = &value.overview {
                    let squads_overview = DisplaySolanaTxOverviewSquadsV4MultisigCreate {
                        wallet_name: convert_c_char(overview.wallet_name.to_string()),
                        wallet_desc: convert_c_char(overview.wallet_desc.to_string()),
                        threshold: overview.threshold,
                        member_count: overview.member_count,
                        members: VecFFI::from(
                            overview
                                .members
                                .iter()
                                .map(|v| convert_c_char(v.to_string()))
                                .collect_vec(),
                        )
                        .c_ptr(),
                        total_value: convert_c_char(overview.total_value.to_string()),
                        transfers: VecFFI::from(
                            overview
                                .transfers
                                .iter()
                                .map(|v| ProgramOverviewTransfer {
                                    value: convert_c_char(v.value.to_string()),
                                    main_action: convert_c_char(v.main_action.to_string()),
                                    from: convert_c_char(v.from.to_string()),
                                    to: convert_c_char(v.to.to_string()),
                                })
                                .collect_vec(),
                        )
                        .c_ptr(),
                    };
                    let display_type = convert_c_char("squads_multisig_create".to_string());
                    return Self {
                        display_type,
                        squads_multisig_create: squads_overview.c_ptr(),
                        ..DisplaySolanaTxOverview::default()
                    };
                }
            }

            SolanaTxDisplayType::JupiterV6 => {
                if let SolanaOverview::JupiterV6SwapOverview(overview) = &value.overview {
                    let display_type = convert_c_char("jupiterv6_swap".to_string());
                    return Self {
                        display_type,
                        jupiter_v6_swap: DisplaySolanaTxOverviewJupiterV6Swap {
                            program_name: convert_c_char(overview.program_name.to_string()),
                            program_address: convert_c_char(overview.program_address.to_string()),
                            instruction_name: convert_c_char(overview.instruction_name.to_string()),
                            token_a_overview: JupiterV6SwapTokenInfoOverview {
                                token_name: convert_c_char(overview.token_a_overview.token_name.to_string()),
                                token_symbol: convert_c_char(overview.token_a_overview.token_symbol.to_string()),
                                token_address: convert_c_char(overview.token_a_overview.token_address.to_string()),
                                token_amount: convert_c_char(overview.token_a_overview.token_amount.to_string()),
                                exist_in_address_lookup_table: overview.token_a_overview.exist_in_address_lookup_table,
                            }
                            .c_ptr(),
                            token_b_overview: JupiterV6SwapTokenInfoOverview {
                                token_name: convert_c_char(overview.token_b_overview.token_name.to_string()),
                                token_symbol: convert_c_char(overview.token_b_overview.token_symbol.to_string()),
                                token_address: convert_c_char(overview.token_b_overview.token_address.to_string()),
                                token_amount: convert_c_char(overview.token_b_overview.token_amount.to_string()),
                                exist_in_address_lookup_table: overview.token_b_overview.exist_in_address_lookup_table,
                            }
                            .c_ptr(),
                            slippage_bps: convert_c_char(overview.slippage_bps.to_string()),
                            platform_fee_bps: convert_c_char(overview.platform_fee_bps.to_string()),
                        }
                        .c_ptr(),
                        ..DisplaySolanaTxOverview::default()
                    };
                }
            }
            

            SolanaTxDisplayType::Unknown => {
                if let SolanaOverview::Instructions(overview) = &value.overview {
                    let display_overview_instructions =
                        DisplaySolanaTxOverviewUnknownInstructions {
                            overview_accounts: VecFFI::from(
                                overview
                                    .overview_accounts
                                    .iter()
                                    .map(|v| convert_c_char(v.to_string()))
                                    .collect_vec(),
                            )
                            .c_ptr(),
                            overview_instructions: VecFFI::from(
                                overview
                                    .overview_instructions
                                    .iter()
                                    .map(|v| {
                                        let accounts = VecFFI::from(
                                            v.accounts
                                                .iter()
                                                .map(|v| convert_c_char(v.to_string()))
                                                .collect_vec(),
                                        );
                                        Instruction {
                                            accounts: accounts.c_ptr(),
                                            data: convert_c_char(v.data.to_string()),
                                            program_address: convert_c_char(
                                                v.program_address.to_string(),
                                            ),
                                        }
                                    })
                                    .collect_vec(),
                            )
                            .c_ptr(),
                        };
                    return Self {
                        display_type,
                        unknown_instructions: display_overview_instructions.c_ptr(),
                        ..DisplaySolanaTxOverview::default()
                    };
                }
            }
        }
        DisplaySolanaTxOverview::default()
    }
}

#[repr(C)]
pub struct DisplaySolanaMessage {
    raw_message: PtrString,
    utf8_message: PtrString,
    from: PtrString,
}

impl From<SolanaMessage> for DisplaySolanaMessage {
    fn from(message: SolanaMessage) -> Self {
        Self {
            raw_message: convert_c_char(message.raw_message),
            utf8_message: if message.utf8_message.is_empty() {
                null_mut()
            } else {
                convert_c_char(message.utf8_message)
            },
            from: convert_c_char(message.from),
        }
    }
}

impl_c_ptr!(DisplaySolanaMessage);

impl Free for DisplaySolanaMessage {
    fn free(&self) {
        free_str_ptr!(self.raw_message);
        free_str_ptr!(self.utf8_message);
        free_str_ptr!(self.from);
    }
}

make_free_method!(TransactionParseResult<DisplaySolanaTx>);
make_free_method!(TransactionParseResult<DisplaySolanaMessage>);
