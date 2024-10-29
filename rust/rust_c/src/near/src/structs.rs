use alloc::boxed::Box;
use alloc::string::ToString;
use alloc::vec::Vec;
use app_near::parser::overview::NearTxOverview;
use app_near::parser::structs::{NearTxDisplayType, ParsedNearTx};
use common_rust_c::ffi::VecFFI;
use common_rust_c::free::{free_ptr_string, Free};
use common_rust_c::structs::TransactionParseResult;
use common_rust_c::types::{PtrString, PtrT};
use common_rust_c::utils::convert_c_char;
use common_rust_c::{check_and_free_ptr, free_str_ptr, impl_c_ptr, impl_c_ptrs, make_free_method};
use core::ptr::null_mut;
use itertools::Itertools;

#[repr(C)]
pub struct DisplayNearTx {
    pub network: PtrString,
    pub overview: PtrT<DisplayNearTxOverview>,
    pub detail: PtrString,
}

#[repr(C)]
pub struct DisplayNearTxOverviewGeneralAction {
    pub action: PtrString,
}

impl Free for DisplayNearTxOverviewGeneralAction {
    fn free(&self) {
        free_str_ptr!(self.action)
    }
}

#[repr(C)]
pub struct DisplayNearTxOverview {
    // `Transfer`, `General`
    pub display_type: PtrString,
    // 'Transfer',
    pub main_action: PtrString,
    // transfer
    pub transfer_value: PtrString,
    pub transfer_from: PtrString,
    pub transfer_to: PtrString,
    // general
    pub action_list: PtrT<VecFFI<DisplayNearTxOverviewGeneralAction>>,
}

impl_c_ptrs!(DisplayNearTx, DisplayNearTxOverview);

impl Default for DisplayNearTxOverview {
    fn default() -> Self {
        Self {
            display_type: null_mut(),
            transfer_value: null_mut(),
            main_action: null_mut(),
            transfer_from: null_mut(),
            transfer_to: null_mut(),
            action_list: null_mut(),
        }
    }
}

impl Free for DisplayNearTx {
    fn free(&self) {
        check_and_free_ptr!(self.overview);
        free_ptr_string(self.network);
        free_ptr_string(self.detail);
    }
}

impl Free for DisplayNearTxOverview {
    fn free(&self) {
        free_str_ptr!(self.display_type);
        free_str_ptr!(self.main_action);
        free_str_ptr!(self.transfer_value);
        free_str_ptr!(self.transfer_from);
        free_str_ptr!(self.transfer_to);
        unsafe {
            if !self.action_list.is_null() {
                let x = Box::from_raw(self.action_list);
                let ve = Vec::from_raw_parts(x.data, x.size, x.cap);
                ve.iter().for_each(|v| {
                    v.free();
                });
            }
        }
    }
}

impl From<ParsedNearTx> for DisplayNearTx {
    fn from(value: ParsedNearTx) -> Self {
        DisplayNearTx {
            network: convert_c_char(value.network.to_string()),
            overview: DisplayNearTxOverview::from(&value).c_ptr(),
            detail: convert_c_char(value.detail),
        }
    }
}

impl From<&ParsedNearTx> for DisplayNearTxOverview {
    fn from(value: &ParsedNearTx) -> Self {
        let display_type = convert_c_char(value.display_type.to_string());
        match value.display_type {
            NearTxDisplayType::Transfer => {
                if let NearTxOverview::Transfer(overview) = &value.overview {
                    return Self {
                        display_type,
                        //transfer
                        transfer_value: convert_c_char(overview.value.to_string()),
                        main_action: convert_c_char(overview.main_action.to_string()),
                        transfer_from: convert_c_char(overview.from.to_string()),
                        transfer_to: convert_c_char(overview.to.to_string()),
                        ..DisplayNearTxOverview::default()
                    };
                }
            }
            NearTxDisplayType::General => {
                if let NearTxOverview::General(overview) = &value.overview {
                    return Self {
                        display_type,
                        //vote
                        action_list: VecFFI::from(
                            overview
                                .action_list
                                .iter()
                                .map(|v| DisplayNearTxOverviewGeneralAction {
                                    action: convert_c_char(v.to_string()),
                                })
                                .collect_vec(),
                        )
                        .c_ptr(),
                        ..DisplayNearTxOverview::default()
                    };
                }
            }
        }
        DisplayNearTxOverview::default()
    }
}

make_free_method!(TransactionParseResult<DisplayNearTx>);
