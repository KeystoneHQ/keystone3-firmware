use alloc::string::ToString;
use core::ptr::null_mut;

use app_xrp::parser::overview::XrpTxOverview;
use app_xrp::parser::structs::{ParsedXrpTx, XrpTxDisplayType};
use serde_json;
use ur_registry::pb::protoc::XrpTx;

use crate::common::free::{free_ptr_string, Free};
use crate::common::structs::TransactionParseResult;
use crate::common::types::{PtrString, PtrT};
use crate::common::utils::convert_c_char;
use crate::{check_and_free_ptr, free_str_ptr, impl_c_ptr, make_free_method};

#[repr(C)]
pub struct XRPHDPath {
    pub(crate) path: PtrString,
}

impl Free for XRPHDPath {
    unsafe fn free(&self) {
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
    pub service_fee_detail: PtrString,
}

impl TryFrom<XrpTx> for DisplayXrpTx {
    type Error = ();
    fn try_from(xrp_tx: XrpTx) -> Result<Self, Self::Error> {
        let display_overview = DisplayXrpTxOverview {
            display_type: convert_c_char("XRP mainnet".to_string()),
            transaction_type: convert_c_char("Payment".to_string()),
            from: convert_c_char(xrp_tx.change_address.to_string()),
            fee: convert_c_char(xrp_tx.fee.to_string()),
            sequence: convert_c_char(xrp_tx.sequence.to_string()),
            value: convert_c_char(xrp_tx.amount.to_string()),
            to: convert_c_char(xrp_tx.to.to_string()),
        };
        let detail = serde_json::to_string_pretty(&xrp_tx).unwrap();
        let display_xrp_tx = DisplayXrpTx {
            network: convert_c_char("XRP Mainnet".to_string()),
            overview: display_overview.c_ptr(),
            detail: convert_c_char(detail),
            signing_pubkey: convert_c_char("signing_pubkey".to_string()),
            service_fee_detail: null_mut(),
        };
        Ok(display_xrp_tx)
    }
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
    unsafe fn free(&self) {
        check_and_free_ptr!(self.overview);
        free_ptr_string(self.network);
        free_ptr_string(self.detail);
        free_ptr_string(self.signing_pubkey);
    }
}

impl Free for DisplayXrpTxOverview {
    unsafe fn free(&self) {
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
            service_fee_detail: if value.service_fee_detail.is_some() {
                convert_c_char(value.service_fee_detail.unwrap())
            } else {
                null_mut()
            },
        }
    }
}

make_free_method!(TransactionParseResult<DisplayXrpTx>);
