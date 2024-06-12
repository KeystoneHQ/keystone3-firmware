use common_rust_c::free::Free;
use common_rust_c::structs::TransactionParseResult;
use common_rust_c::types::{PtrString, PtrT};
use common_rust_c::{check_and_free_ptr, free_str_ptr, impl_c_ptr, make_free_method};

#[repr(C)]
pub struct DisplayIcpTx {
    pub raw_message: PtrString,
}

impl_c_ptr!(DisplayIcpTx);

impl Free for DisplayIcpTx {
    fn free(&self) {
        free_str_ptr!(self.raw_message);
    }
}

impl From<&DisplayIcpTx> for DisplayIcpTx {
    fn from(value: &DisplayIcpTx) -> Self {
        Self {
            raw_message: value.raw_message,
        }
    }
}

make_free_method!(TransactionParseResult<DisplayIcpTx>);