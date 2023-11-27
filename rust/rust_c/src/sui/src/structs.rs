use alloc::string::String;
use app_sui::Intent;
use third_party::serde_json;

use common_rust_c::free::Free;
use common_rust_c::structs::TransactionParseResult;
use common_rust_c::types::{PtrString, PtrT};
use common_rust_c::utils::convert_c_char;
use common_rust_c::{check_and_free_ptr, free_str_ptr, impl_c_ptr, make_free_method};

#[repr(C)]
pub struct DisplaySuiIntentMessage {
    pub detail: PtrString,
}

impl From<Intent> for DisplaySuiIntentMessage {
    fn from(message: Intent) -> Self {
        Self {
            detail: match message {
                Intent::TransactionData(tx) => {
                    convert_c_char(serde_json::to_string_pretty(&tx).unwrap_or(String::from("")))
                }
                Intent::PersonalMessage(m) => {
                    convert_c_char(serde_json::to_string_pretty(&m).unwrap_or(String::from("")))
                }
            },
        }
    }
}

impl_c_ptr!(DisplaySuiIntentMessage);

impl Free for DisplaySuiIntentMessage {
    fn free(&self) {
        free_str_ptr!(self.detail);
    }
}

make_free_method!(TransactionParseResult<DisplaySuiIntentMessage>);
