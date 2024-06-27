use alloc::boxed::Box;
use alloc::string::ToString;
use alloc::vec::Vec;
use common_rust_c::ffi::VecFFI;
use common_rust_c::free::Free;
use common_rust_c::structs::TransactionParseResult;
use common_rust_c::types::{PtrString, PtrT};
use common_rust_c::utils::convert_c_char;
use common_rust_c::{check_and_free_ptr, free_str_ptr, impl_c_ptr, impl_c_ptrs, make_free_method};
use core::ptr::null_mut;
use third_party::itertools::Itertools;

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
