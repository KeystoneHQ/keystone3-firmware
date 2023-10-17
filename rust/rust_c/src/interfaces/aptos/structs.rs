use alloc::string::ToString;
use app_aptos::parser::AptosTx;
use third_party::serde_json::json;

use crate::{
    free_str_ptr, impl_c_ptr,
    interfaces::{free::Free, types::PtrString, utils::convert_c_char},
};

#[repr(C)]
pub struct DisplayAptosTx {
    pub detail: PtrString,
}

impl From<AptosTx> for DisplayAptosTx {
    fn from(tx: AptosTx) -> Self {
        Self {
            detail: convert_c_char(tx.get_formatted_json().unwrap_or(json!("")).to_string()),
        }
    }
}

impl_c_ptr!(DisplayAptosTx);

impl Free for DisplayAptosTx {
    fn free(&self) {
        unsafe {
            free_str_ptr!(self.detail);
        }
    }
}
