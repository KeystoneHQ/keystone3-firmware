use alloc::boxed::Box;
use app_tron::{DetailTx, OverviewTx, ParsedTx};

use crate::common::free::Free;
use crate::common::structs::TransactionParseResult;
use crate::common::types::{PtrString, PtrT};
use crate::common::utils::convert_c_char;
use crate::{check_and_free_ptr, free_str_ptr, impl_c_ptr, make_free_method};
use app_tron::structs::PersonalMessage;
use core::ptr::null_mut;
use ur_registry::tron::tron_sign_request::DataType;

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
    pub memo: PtrString,
    pub expiration: PtrString,
    pub raw_value: PtrString,
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
            memo: convert_c_char(value.memo),
            expiration: convert_c_char(value.expiration),
            raw_value: convert_c_char(value.raw_value),
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
        free_str_ptr!(self.memo);
        free_str_ptr!(self.expiration);
        free_str_ptr!(self.raw_value);
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

#[derive(Debug)]
pub enum TransactionType {
    Transaction,
    PersonalMessage,
}

impl From<DataType> for TransactionType {
    fn from(value: DataType) -> Self {
        match value {
            DataType::Transaction => TransactionType::Transaction,
            DataType::PersonalMessage => TransactionType::PersonalMessage,
        }
    }
}

#[repr(C)]
pub struct DisplayTRONPersonalMessage {
    raw_message: PtrString,
    utf8_message: PtrString,
    from: PtrString,
}

impl From<PersonalMessage> for DisplayTRONPersonalMessage {
    fn from(message: PersonalMessage) -> Self {
        Self {
            raw_message: convert_c_char(message.raw_message),
            utf8_message: if message.utf8_message.is_empty() {
                null_mut()
            } else {
                convert_c_char(message.utf8_message)
            },
            from: message.from.map(convert_c_char).unwrap_or(null_mut()),
        }
    }
}

impl_c_ptr!(DisplayTRONPersonalMessage);

impl Free for DisplayTRONPersonalMessage {
    unsafe fn free(&self) {
        free_str_ptr!(self.raw_message);
        free_str_ptr!(self.utf8_message);
        free_str_ptr!(self.from);
    }
}

make_free_method!(TransactionParseResult<DisplayTron>);
make_free_method!(TransactionParseResult<DisplayTRONPersonalMessage>);
