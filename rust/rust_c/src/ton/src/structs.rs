use core::ptr::null_mut;

use app_ton::structs::TonTransaction;
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
