use alloc::string::{String, ToString};
use app_aptos::parser::AptosTx;
use common_rust_c::free::Free;
use common_rust_c::structs::TransactionParseResult;
use common_rust_c::types::{PtrString, PtrT};
use common_rust_c::utils::convert_c_char;
use common_rust_c::{check_and_free_ptr, free_str_ptr, impl_c_ptr, make_free_method};
use third_party::serde_json::Value;

#[repr(C)]
pub struct DisplayAptosTx {
    pub detail: PtrString,
    pub is_msg: bool,
}

impl From<AptosTx> for DisplayAptosTx {
    fn from(tx: AptosTx) -> Self {
        let tx_json = match tx
            .get_formatted_json()
            .unwrap_or(Value::String("".to_string()))
        {
            Value::String(s) => s,
            _ => "".to_string(),
        };
        Self {
            detail: convert_c_char(tx_json),
            is_msg: false,
        }
    }
}

impl From<String> for DisplayAptosTx {
    fn from(s: String) -> Self {
        Self {
            detail: convert_c_char(s),
            is_msg: true,
        }
    }
}

impl_c_ptr!(DisplayAptosTx);

impl Free for DisplayAptosTx {
    fn free(&self) {
        free_str_ptr!(self.detail);
    }
}

make_free_method!(TransactionParseResult<DisplayAptosTx>);
