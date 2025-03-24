use crate::common::ffi::VecFFI;
use crate::common::free::Free;
use crate::common::structs::TransactionParseResult;
use crate::common::types::{PtrString, PtrT};
use crate::common::utils::convert_c_char;
use crate::{free_str_ptr, impl_c_ptr, impl_c_ptrs, make_free_method};
use alloc::boxed::Box;
use alloc::string::ToString;
use alloc::vec::Vec;
use app_ergo::structs::{ParsedErgoInput, ParsedErgoOutput, ParsedErgoTx};
use itertools::Itertools;

#[repr(C)]
pub struct DisplayErgoTx {
    pub fee: PtrString,
    pub total_input: PtrString,
    pub total_output: PtrString,
    pub from: PtrT<VecFFI<DisplayErgoFrom>>,
    pub to: PtrT<VecFFI<DisplayErgoTo>>,
}

impl From<ParsedErgoTx> for DisplayErgoTx {
    fn from(value: ParsedErgoTx) -> Self {
        Self {
            fee: convert_c_char(value.get_fee()),
            total_input: convert_c_char(value.get_total_input()),
            total_output: convert_c_char(value.get_total_output()),
            from: VecFFI::from(
                value
                    .get_from()
                    .iter()
                    .map(DisplayErgoFrom::from)
                    .collect_vec(),
            )
            .c_ptr(),
            to: VecFFI::from(value.get_to().iter().map(DisplayErgoTo::from).collect_vec()).c_ptr(),
        }
    }
}

impl Free for DisplayErgoTx {
    fn free(&self) {
        unsafe {
            let x = Box::from_raw(self.from);
            let ve = Vec::from_raw_parts(x.data, x.size, x.cap);
            ve.iter().for_each(|v| {
                v.free();
            });
            let x = Box::from_raw(self.to);
            let ve = Vec::from_raw_parts(x.data, x.size, x.cap);
            ve.iter().for_each(|v| {
                v.free();
            });
            free_str_ptr!(self.fee);
            free_str_ptr!(self.total_input);
            free_str_ptr!(self.total_output);
        }
    }
}

impl_c_ptrs!(DisplayErgoTx);
make_free_method!(TransactionParseResult<DisplayErgoTx>);

#[repr(C)]
pub struct DisplayErgoFrom {
    address: PtrString,
    amount: PtrString,
    is_mine: bool,
    has_assets: bool,
    assets: PtrT<VecFFI<PtrString>>,
}

impl From<&ParsedErgoInput> for DisplayErgoFrom {
    fn from(value: &ParsedErgoInput) -> Self {
        Self {
            address: convert_c_char(value.get_address()),
            amount: convert_c_char(value.get_amount()),
            is_mine: value.get_is_mine(),
            has_assets: value.get_assets().is_some(),
            assets: VecFFI::from(
                value
                    .get_assets()
                    .unwrap_or_default()
                    .iter()
                    .map(|v| convert_c_char(v.to_string()))
                    .collect_vec(),
            )
            .c_ptr(),
        }
    }
}

impl Free for DisplayErgoFrom {
    fn free(&self) {
        free_str_ptr!(self.address);
        free_str_ptr!(self.amount);
        unsafe {
            if !self.assets.is_null() {
                let x = Box::from_raw(self.assets);
                let ve = Vec::from_raw_parts(x.data, x.size, x.cap);
                ve.iter().for_each(|v| {
                    free_str_ptr!(*v);
                });
            }
        }
    }
}

#[repr(C)]
pub struct DisplayErgoTo {
    address: PtrString,
    amount: PtrString,
    has_assets: bool,
    assets: PtrT<VecFFI<PtrString>>,
    is_change: bool,
    is_fee: bool,
}

impl From<&ParsedErgoOutput> for DisplayErgoTo {
    fn from(value: &ParsedErgoOutput) -> Self {
        Self {
            address: convert_c_char(value.get_address()),
            amount: convert_c_char(value.get_amount()),
            has_assets: value.get_assets().is_some(),
            assets: VecFFI::from(
                value
                    .get_assets()
                    .unwrap_or_default()
                    .iter()
                    .map(|v| convert_c_char(v.to_string()))
                    .collect_vec(),
            )
            .c_ptr(),
            is_fee: value.get_is_fee(),
            is_change: value.get_is_change(),
        }
    }
}

impl Free for DisplayErgoTo {
    fn free(&self) {
        free_str_ptr!(self.address);
        free_str_ptr!(self.amount);
        unsafe {
            if !self.assets.is_null() {
                let x = Box::from_raw(self.assets);
                let ve = Vec::from_raw_parts(x.data, x.size, x.cap);
                ve.iter().for_each(|v| {
                    free_str_ptr!(*v);
                });
            }
        }
    }
}
