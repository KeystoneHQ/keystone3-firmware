



use common_rust_c::free::Free;
use common_rust_c::structs::TransactionParseResult;
use common_rust_c::types::{PtrString, PtrT};

use common_rust_c::{check_and_free_ptr, free_str_ptr, impl_c_ptr, make_free_method};



#[repr(C)]
pub struct DisplayStellarTx {
    pub raw_message: PtrString,
}

impl_c_ptr!(DisplayStellarTx);

impl Free for DisplayStellarTx {
    fn free(&self) {
        free_str_ptr!(self.raw_message);
    }
}

impl From<&DisplayStellarTx> for DisplayStellarTx {
    fn from(value: &DisplayStellarTx) -> Self {
        Self {
            raw_message: value.raw_message,
        }
    }
}

make_free_method!(TransactionParseResult<DisplayStellarTx>);
