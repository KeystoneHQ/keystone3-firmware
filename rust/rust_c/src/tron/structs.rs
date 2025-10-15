use alloc::boxed::Box;
use app_tron::{DetailTx, OverviewTx, ParsedTx};

use crate::common::free::Free;
use crate::common::structs::TransactionParseResult;
use crate::common::types::{PtrString, PtrT};
use crate::common::utils::convert_c_char;
use crate::{check_and_free_ptr, free_str_ptr, impl_c_ptr, make_free_method};

#[repr(C)]
pub struct DisplayTron {
    overview: *mut DisplayTronOverview,
    detail: *mut DisplayTronDetail,
}

#[repr(C)]
pub struct DisplayTronOverview {
    value: PtrString,
    method: PtrString,
    from: PtrString,
    to: PtrString,
    network: PtrString,
}

impl_c_ptr!(DisplayTronOverview);

#[repr(C)]
pub struct DisplayTronDetail {
    value: PtrString,
    method: PtrString,
    from: PtrString,
    to: PtrString,
    network: PtrString,
    token: PtrString,
    contract_address: PtrString,
}

impl_c_ptr!(DisplayTronDetail);

impl From<OverviewTx> for DisplayTronOverview {
    fn from(value: OverviewTx) -> Self {
        Self {
            value: convert_c_char(value.value),
            method: convert_c_char(value.method),
            from: convert_c_char(value.from),
            to: convert_c_char(value.to),
            network: convert_c_char(value.network),
        }
    }
}

impl From<DetailTx> for DisplayTronDetail {
    fn from(value: DetailTx) -> Self {
        Self {
            value: convert_c_char(value.value),
            method: convert_c_char(value.method),
            from: convert_c_char(value.from),
            to: convert_c_char(value.to),
            network: convert_c_char(value.network),
            token: convert_c_char(value.token),
            contract_address: convert_c_char(value.contract_address),
        }
    }
}
impl From<ParsedTx> for DisplayTron {
    fn from(value: ParsedTx) -> Self {
        DisplayTron {
            overview: DisplayTronOverview::from(value.overview).c_ptr(),
            detail: DisplayTronDetail::from(value.detail).c_ptr(),
        }
    }
}

impl Free for DisplayTronOverview {
    unsafe fn free(&self) {
        free_str_ptr!(self.from);
        free_str_ptr!(self.to);
        free_str_ptr!(self.value);
        free_str_ptr!(self.method);
        free_str_ptr!(self.network);
    }
}

impl Free for DisplayTronDetail {
    unsafe fn free(&self) {
        free_str_ptr!(self.from);
        free_str_ptr!(self.to);
        free_str_ptr!(self.value);
        free_str_ptr!(self.method);
        free_str_ptr!(self.network);
        free_str_ptr!(self.token);
        free_str_ptr!(self.contract_address);
    }
}

impl Free for DisplayTron {
    unsafe fn free(&self) {
        let x = Box::from_raw(self.overview);
        let y = Box::from_raw(self.detail);
        x.free();
        y.free();
    }
}

make_free_method!(TransactionParseResult<DisplayTron>);
