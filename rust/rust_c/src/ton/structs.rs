use core::ptr::null_mut;

use app_ton::structs::{TonProof, TonTransaction};
use common_rust_c::{
    check_and_free_ptr,
    free::Free,
    free_str_ptr, impl_c_ptr, make_free_method,
    structs::TransactionParseResult,
    types::{PtrString, PtrT},
    utils::convert_c_char,
};

#[repr(C)]
pub struct DisplayTonTransaction {
    amount: PtrString,
    action: PtrString,
    to: PtrString,
    comment: PtrString,
    data_view: PtrString,
    raw_data: PtrString,
    contract_data: PtrString,
}

impl_c_ptr!(DisplayTonTransaction);

impl From<&TonTransaction> for DisplayTonTransaction {
    fn from(tx: &TonTransaction) -> Self {
        DisplayTonTransaction {
            amount: convert_c_char(tx.amount.clone()),
            action: convert_c_char(tx.action.clone()),
            to: convert_c_char(tx.to.clone()),
            comment: tx
                .comment
                .clone()
                .map(|v| convert_c_char(v))
                .unwrap_or(null_mut()),
            data_view: tx
                .data_view
                .clone()
                .map(|v| convert_c_char(v))
                .unwrap_or(null_mut()),
            raw_data: convert_c_char(tx.raw_data.clone()),
            contract_data: tx
                .contract_data
                .clone()
                .map(|e| convert_c_char(e))
                .unwrap_or(null_mut()),
        }
    }
}

impl Free for DisplayTonTransaction {
    fn free(&self) {
        free_str_ptr!(self.amount);
        free_str_ptr!(self.action);
        free_str_ptr!(self.to);
        free_str_ptr!(self.comment);
        free_str_ptr!(self.data_view);
        free_str_ptr!(self.raw_data);
        free_str_ptr!(self.contract_data);
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
    fn free(&self) {
        free_str_ptr!(self.domain);
        free_str_ptr!(self.payload);
        free_str_ptr!(self.address);
        free_str_ptr!(self.raw_message);
    }
}

make_free_method!(TransactionParseResult<DisplayTonProof>);
