use core::ptr::null_mut;

use crate::common::{
    ffi::VecFFI,
    free::Free,
    structs::TransactionParseResult,
    types::{PtrString, PtrT},
    utils::convert_c_char,
};
use crate::{check_and_free_ptr, free_str_ptr, impl_c_ptr, make_free_method};
use alloc::vec::Vec;
use app_ton::structs::{TonMessage, TonProof, TonTransaction};

#[repr(C)]
pub struct DisplayTonMessage {
    amount: PtrString,
    action: PtrString,
    to: PtrString,
    comment: PtrString,
    data_view: PtrString,
    contract_data: PtrString,
}

#[repr(C)]
pub struct DisplayTonTransaction {
    raw_data: PtrString,
    messages: PtrT<VecFFI<DisplayTonMessage>>,
}

impl_c_ptr!(DisplayTonMessage);
impl_c_ptr!(DisplayTonTransaction);

impl From<&TonMessage> for DisplayTonMessage {
    fn from(message: &TonMessage) -> Self {
        DisplayTonMessage {
            amount: convert_c_char(message.amount.clone()),
            action: convert_c_char(message.action.clone()),
            to: convert_c_char(message.to.clone()),
            comment: message
                .comment
                .clone()
                .map(convert_c_char)
                .unwrap_or(null_mut()),
            data_view: message
                .data_view
                .clone()
                .map(convert_c_char)
                .unwrap_or(null_mut()),
            contract_data: message
                .contract_data
                .clone()
                .map(convert_c_char)
                .unwrap_or(null_mut()),
        }
    }
}

impl From<&TonTransaction> for DisplayTonTransaction {
    fn from(tx: &TonTransaction) -> Self {
        DisplayTonTransaction {
            raw_data: convert_c_char(tx.raw_data.clone()),
            messages: VecFFI::from(
                tx.messages
                    .iter()
                    .map(DisplayTonMessage::from)
                    .collect::<Vec<_>>(),
            )
            .c_ptr(),
        }
    }
}

impl Free for DisplayTonMessage {
    unsafe fn free(&self) {
        free_str_ptr!(self.amount);
        free_str_ptr!(self.action);
        free_str_ptr!(self.to);
        free_str_ptr!(self.comment);
        free_str_ptr!(self.data_view);
        free_str_ptr!(self.contract_data);
    }
}

impl Free for DisplayTonTransaction {
    unsafe fn free(&self) {
        free_str_ptr!(self.raw_data);
        check_and_free_ptr!(self.messages);
    }
}

make_free_method!(TransactionParseResult<DisplayTonTransaction>);

#[repr(C)]
pub struct DisplayTonProof {
    pub domain: PtrString,
    pub payload: PtrString,
    pub address: PtrString,
    pub raw_message: PtrString,
}

impl_c_ptr!(DisplayTonProof);

impl From<&TonProof> for DisplayTonProof {
    fn from(tx: &TonProof) -> Self {
        DisplayTonProof {
            domain: convert_c_char(tx.domain.clone()),
            payload: convert_c_char(tx.payload.clone()),
            address: convert_c_char(tx.address.clone()),
            raw_message: convert_c_char(tx.raw_message.clone()),
        }
    }
}

impl Free for DisplayTonProof {
    unsafe fn free(&self) {
        free_str_ptr!(self.domain);
        free_str_ptr!(self.payload);
        free_str_ptr!(self.address);
        free_str_ptr!(self.raw_message);
    }
}

make_free_method!(TransactionParseResult<DisplayTonProof>);
