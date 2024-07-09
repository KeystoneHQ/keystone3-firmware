use alloc::boxed::Box;
use alloc::string::ToString;
use alloc::vec::Vec;
use core::ptr::null_mut;

use app_solana::parser::overview::{ProgramOverviewGeneral, SolanaOverview};
use app_solana::parser::structs::{ParsedSolanaTx, SolanaTxDisplayType};
use app_solana::structs::SolanaMessage;
use third_party::itertools::Itertools;

use common_rust_c::{check_and_free_ptr, free_str_ptr, impl_c_ptr, impl_c_ptrs, make_free_method};
use common_rust_c::ffi::VecFFI;
use common_rust_c::free::Free;
use common_rust_c::structs::TransactionParseResult;
use common_rust_c::types::{PtrString, PtrT};
use common_rust_c::utils::convert_c_char;

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
