use alloc::string::String;
use app_sui::Intent;
use serde_json;

use common_rust_c::free::Free;
use common_rust_c::structs::TransactionParseResult;
use common_rust_c::types::{PtrString, PtrT};
use common_rust_c::utils::convert_c_char;
use common_rust_c::{check_and_free_ptr, free_str_ptr, impl_c_ptr, make_free_method};

#[repr(C)]
pub struct DisplaySuiIntentMessage {
    pub detail: PtrString,
}

#[repr(C)]
pub struct DisplaySuiSignMessageHash {
    pub network: PtrString,
    pub path: PtrString,
    pub from_address: PtrString,
    pub message: PtrString,
}

impl DisplaySuiSignMessageHash {
    pub fn new(network: String, path: String, message: String, from_address: String) -> Self {
        Self {
            network: convert_c_char(network),
            path: convert_c_char(path),
            message: convert_c_char(message),
            from_address: convert_c_char(from_address),
        }
    }
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

impl_c_ptr!(DisplaySuiSignMessageHash);

impl Free for DisplaySuiIntentMessage {
    fn free(&self) {
        free_str_ptr!(self.detail);
    }
}

impl Free for DisplaySuiSignMessageHash {
    fn free(&self) {
        free_str_ptr!(self.network);
        free_str_ptr!(self.path);
        free_str_ptr!(self.message);
        free_str_ptr!(self.from_address);
    }
}

make_free_method!(TransactionParseResult<DisplaySuiIntentMessage>);
make_free_method!(TransactionParseResult<DisplaySuiSignMessageHash>);
