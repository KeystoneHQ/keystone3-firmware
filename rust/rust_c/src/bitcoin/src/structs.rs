use alloc::boxed::Box;
use alloc::vec::Vec;
use core::ptr::null_mut;

use app_bitcoin;
use app_bitcoin::parsed_tx::{DetailTx, OverviewTx, ParsedInput, ParsedOutput, ParsedTx};
use common_rust_c::ffi::VecFFI;
use common_rust_c::free::Free;
use common_rust_c::structs::TransactionParseResult;
use common_rust_c::types::{PtrString, PtrT};
use common_rust_c::utils::convert_c_char;
use common_rust_c::{check_and_free_ptr, impl_c_ptr, make_free_method};

#[repr(C)]
pub struct DisplayTx {
    overview: *mut DisplayTxOverview,
    detail: *mut DisplayTxDetail,
}

#[repr(C)]
pub struct DisplayTxOverview {
    total_output_amount: PtrString,
    fee_amount: PtrString,
    total_output_sat: PtrString,
    fee_sat: PtrString,
    from: PtrT<VecFFI<DisplayTxOverviewInput>>,
    to: PtrT<VecFFI<DisplayTxOverviewOutput>>,
    network: PtrString,
    fee_larger_than_amount: bool,
}

impl_c_ptr!(DisplayTxOverview);

#[repr(C)]
pub struct DisplayTxDetail {
    total_input_amount: PtrString,
    total_output_amount: PtrString,
    fee_amount: PtrString,
    from: PtrT<VecFFI<DisplayTxDetailInput>>,
    to: PtrT<VecFFI<DisplayTxDetailOutput>>,
    network: PtrString,
    total_input_sat: PtrString,
    total_output_sat: PtrString,
    fee_sat: PtrString,
}

impl_c_ptr!(DisplayTxDetail);

#[repr(C)]
pub struct DisplayTxOverviewInput {
    address: PtrString,
}

#[repr(C)]
pub struct DisplayTxDetailInput {
    has_address: bool,
    address: PtrString,
    amount: PtrString,
    is_mine: bool,
    path: PtrString,
}

#[repr(C)]
pub struct DisplayTxOverviewOutput {
    address: PtrString,
}

#[repr(C)]
pub struct DisplayTxDetailOutput {
    address: PtrString,
    amount: PtrString,
    is_mine: bool,
    path: PtrString,
}

impl From<ParsedTx> for DisplayTx {
    fn from(value: ParsedTx) -> Self {
        DisplayTx {
            overview: DisplayTxOverview::from(value.overview).c_ptr(),
            detail: DisplayTxDetail::from(value.detail).c_ptr(),
        }
    }
}

impl From<OverviewTx> for DisplayTxOverview {
    fn from(value: OverviewTx) -> Self {
        DisplayTxOverview {
            total_output_amount: convert_c_char(value.total_output_amount),
            fee_amount: convert_c_char(value.fee_amount),
            fee_larger_than_amount: value.fee_larger_than_amount,
            total_output_sat: convert_c_char(value.total_output_sat),
            fee_sat: convert_c_char(value.fee_sat),
            from: VecFFI::from(
                value
                    .from
                    .iter()
                    .map(|v| DisplayTxOverviewInput {
                        address: convert_c_char(v.clone()),
                    })
                    .collect::<Vec<DisplayTxOverviewInput>>(),
            )
            .c_ptr(),
            to: VecFFI::from(
                value
                    .to
                    .iter()
                    .map(|v| DisplayTxOverviewOutput {
                        address: convert_c_char(v.clone()),
                    })
                    .collect::<Vec<DisplayTxOverviewOutput>>(),
            )
            .c_ptr(),
            network: convert_c_char(value.network),
        }
    }
}

impl From<DetailTx> for DisplayTxDetail {
    fn from(value: DetailTx) -> Self {
        DisplayTxDetail {
            total_input_amount: convert_c_char(value.total_input_amount),
            total_output_amount: convert_c_char(value.total_output_amount),
            fee_amount: convert_c_char(value.fee_amount),
            from: VecFFI::from(
                value
                    .from
                    .iter()
                    .map(|v| DisplayTxDetailInput::from(v.clone()))
                    .collect::<Vec<DisplayTxDetailInput>>(),
            )
            .c_ptr(),
            to: VecFFI::from(
                value
                    .to
                    .iter()
                    .map(|v| DisplayTxDetailOutput::from(v.clone()))
                    .collect::<Vec<DisplayTxDetailOutput>>(),
            )
            .c_ptr(),
            network: convert_c_char(value.network),
            total_output_sat: convert_c_char(value.total_output_sat),
            total_input_sat: convert_c_char(value.total_input_sat),
            fee_sat: convert_c_char(value.fee_sat),
        }
    }
}

impl From<ParsedInput> for DisplayTxDetailInput {
    fn from(value: ParsedInput) -> Self {
        DisplayTxDetailInput {
            has_address: value.address.is_some(),
            address: value
                .address
                .map(|v| convert_c_char(v))
                .unwrap_or(null_mut()),
            amount: convert_c_char(value.amount),
            is_mine: value.path.is_some(),
            path: value.path.map(|v| convert_c_char(v)).unwrap_or(null_mut()),
        }
    }
}

impl From<ParsedOutput> for DisplayTxDetailOutput {
    fn from(value: ParsedOutput) -> Self {
        DisplayTxDetailOutput {
            address: convert_c_char(value.address),
            amount: convert_c_char(value.amount),
            is_mine: value.path.is_some(),
            path: value.path.map(|v| convert_c_char(v)).unwrap_or(null_mut()),
        }
    }
}

impl Free for DisplayTx {
    fn free(&self) {
        unsafe {
            let x = Box::from_raw(self.overview);
            let y = Box::from_raw(self.detail);
            x.free();
            y.free();
        }
    }
}

make_free_method!(TransactionParseResult<DisplayTx>);

impl Free for DisplayTxOverview {
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

            let _ = Box::from_raw(self.total_output_amount);
            let _ = Box::from_raw(self.fee_amount);
            let _ = Box::from_raw(self.total_output_sat);
            let _ = Box::from_raw(self.fee_sat);
            let _ = Box::from_raw(self.network);
        }
    }
}

impl Free for DisplayTxDetail {
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

            let _ = Box::from_raw(self.total_input_amount);
            let _ = Box::from_raw(self.total_output_amount);
            let _ = Box::from_raw(self.fee_amount);
            let _ = Box::from_raw(self.network);
            let _ = Box::from_raw(self.total_input_sat);
            let _ = Box::from_raw(self.total_output_sat);
            let _ = Box::from_raw(self.fee_sat);
        }
    }
}

impl Free for DisplayTxOverviewInput {
    fn free(&self) {
        unsafe {
            let _ = Box::from_raw(self.address);
        }
    }
}

impl Free for DisplayTxDetailInput {
    fn free(&self) {
        unsafe {
            let _ = Box::from_raw(self.address);
            let _ = Box::from_raw(self.amount);
            let _ = Box::from_raw(self.path);
        }
    }
}

impl Free for DisplayTxOverviewOutput {
    fn free(&self) {
        unsafe {
            let _ = Box::from_raw(self.address);
        }
    }
}

impl Free for DisplayTxDetailOutput {
    fn free(&self) {
        unsafe {
            let _ = Box::from_raw(self.address);
            let _ = Box::from_raw(self.amount);
            let _ = Box::from_raw(self.path);
        }
    }
}
