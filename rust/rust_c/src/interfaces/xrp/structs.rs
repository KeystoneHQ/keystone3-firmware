use crate::check_and_free_ptr;
use crate::interfaces::free::{free_ptr_string, Free};
use crate::interfaces::types::{PtrString, PtrT};
use crate::interfaces::utils::convert_c_char;
use crate::{free_str_ptr, impl_c_ptr};

use alloc::string::ToString;
use app_xrp::parser::overview::XrpTxOverview;
use app_xrp::parser::structs::{ParsedXrpTx, XrpTxDisplayType};
use core::ptr::null_mut;

#[repr(C)]
pub struct XRPHDPath {
    pub(crate) path: PtrString,
}

impl Free for XRPHDPath {
    fn free(&self) {
        free_str_ptr!(self.path);
    }
}

#[repr(C)]
pub struct DisplayXrpTxOverview {
    pub display_type: PtrString,
    // common field
    pub transaction_type: PtrString,
    pub from: PtrString,
    pub fee: PtrString,
    pub sequence: PtrString,
    // payment
    pub value: PtrString,
    pub to: PtrString,
}

#[repr(C)]
pub struct DisplayXrpTx {
    pub network: PtrString,
    pub overview: PtrT<DisplayXrpTxOverview>,
    pub detail: PtrString,
    pub signing_pubkey: PtrString,
}

impl Default for DisplayXrpTxOverview {
    fn default() -> Self {
        Self {
            display_type: null_mut(),
            transaction_type: null_mut(),
            from: null_mut(),
            fee: null_mut(),
            sequence: null_mut(),
            value: null_mut(),
            to: null_mut(),
        }
    }
}

impl_c_ptr!(DisplayXrpTx);
impl_c_ptr!(DisplayXrpTxOverview);

impl Free for DisplayXrpTx {
    fn free(&self) {
        unsafe {
            check_and_free_ptr!(self.overview);
            free_ptr_string(self.network);
            free_ptr_string(self.detail);
            free_ptr_string(self.signing_pubkey);
        }
    }
}

impl Free for DisplayXrpTxOverview {
    fn free(&self) {
        free_str_ptr!(self.display_type);
        free_str_ptr!(self.transaction_type);
        free_str_ptr!(self.from);
        free_str_ptr!(self.fee);
        free_str_ptr!(self.sequence);
        free_str_ptr!(self.value);
        free_str_ptr!(self.to);
    }
}

impl From<&ParsedXrpTx> for DisplayXrpTxOverview {
    fn from(value: &ParsedXrpTx) -> Self {
        let display_type = convert_c_char(value.display_type.to_string());
        match value.display_type {
            XrpTxDisplayType::Payment => {
                if let XrpTxOverview::Payment(overview) = &value.overview {
                    return Self {
                        display_type,
                        value: convert_c_char(overview.value.to_string()),
                        transaction_type: convert_c_char(overview.transaction_type.to_string()),
                        from: convert_c_char(overview.from.to_string()),
                        to: convert_c_char(overview.to.to_string()),
                        ..DisplayXrpTxOverview::default()
                    };
                }
            }
            XrpTxDisplayType::General => {
                if let XrpTxOverview::General(overview) = &value.overview {
                    return Self {
                        display_type,
                        transaction_type: convert_c_char(overview.transaction_type.to_string()),
                        from: convert_c_char(overview.from.to_string()),
                        fee: convert_c_char(overview.fee.to_string()),
                        sequence: convert_c_char(overview.sequence.to_string()),
                        ..DisplayXrpTxOverview::default()
                    };
                }
            }
        }
        DisplayXrpTxOverview::default()
    }
}

impl From<ParsedXrpTx> for DisplayXrpTx {
    fn from(value: ParsedXrpTx) -> Self {
        DisplayXrpTx {
            network: convert_c_char(value.network.to_string()),
            overview: DisplayXrpTxOverview::from(&value).c_ptr(),
            detail: convert_c_char(value.detail),
            signing_pubkey: convert_c_char(value.signing_pubkey),
        }
    }
}
