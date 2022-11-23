use crate::interfaces::free::Free;
use crate::interfaces::types::PtrString;
use crate::interfaces::utils::convert_c_char;
use crate::{free_str_ptr, impl_c_ptr};
use alloc::boxed::Box;
use app_tron::{DetailTx, OverviewTx, ParsedTx};

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
    fn free(&self) {
        free_str_ptr!(self.from);
        free_str_ptr!(self.to);
        free_str_ptr!(self.value);
        free_str_ptr!(self.method);
        free_str_ptr!(self.network);
    }
}

impl Free for DisplayTronDetail {
    fn free(&self) {
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
    fn free(&self) {
        unsafe {
            let x = Box::from_raw(self.overview);
            let y = Box::from_raw(self.detail);
            x.free();
            y.free();
        }
    }
}
