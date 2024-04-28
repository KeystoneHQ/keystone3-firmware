use common_rust_c::free::Free;
use common_rust_c::structs::TransactionParseResult;
use common_rust_c::types::{Ptr, PtrString, PtrT};
use common_rust_c::{check_and_free_ptr, free_str_ptr, impl_c_ptr, make_free_method};

#[repr(C)]
pub struct DisplayArweaveTx {
    pub from: PtrString,
    pub to: PtrString,
}

impl Free for DisplayArweaveTx {
    fn free(&self) {
        free_str_ptr!(self.from);
        free_str_ptr!(self.to);
    }
}

make_free_method!(TransactionParseResult<DisplayArweaveTx>);
