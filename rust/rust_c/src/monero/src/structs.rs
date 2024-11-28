use common_rust_c::free::Free;
use common_rust_c::structs::TransactionParseResult;
use common_rust_c::types::{PtrString, PtrT};

use common_rust_c::{check_and_free_ptr, free_str_ptr, impl_c_ptr, make_free_method};

#[repr(C)]
pub struct DisplayMoneroOutput {
    pub raw: PtrString,
}

#[repr(C)]
pub struct DisplayMoneroUnsignedTx {
    pub raw: PtrString,
}

impl_c_ptr!(DisplayMoneroOutput);
impl_c_ptr!(DisplayMoneroUnsignedTx);

impl Free for DisplayMoneroOutput {
    fn free(&self) {
        free_str_ptr!(self.raw);
    }
}

impl Free for DisplayMoneroUnsignedTx {
    fn free(&self) {
        free_str_ptr!(self.raw);
    }
}

impl From<&DisplayMoneroOutput> for DisplayMoneroOutput {
    fn from(value: &DisplayMoneroOutput) -> Self {
        Self {
            raw: value.raw,
        }
    }
}

impl From<&DisplayMoneroUnsignedTx> for DisplayMoneroUnsignedTx {
    fn from(value: &DisplayMoneroUnsignedTx) -> Self {
        Self {
            raw: value.raw,
        }
    }
}

make_free_method!(TransactionParseResult<DisplayMoneroOutput>);
make_free_method!(TransactionParseResult<DisplayMoneroUnsignedTx>);
