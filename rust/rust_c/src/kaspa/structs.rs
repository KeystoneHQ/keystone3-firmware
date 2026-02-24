// Shared structures for Kaspa FFI

use crate::common::ffi::VecFFI;
use crate::common::free::Free;
use crate::common::types::{PtrString, PtrT};
use crate::make_free_method;
use alloc::boxed::Box;
use alloc::string::ToString;
use crate::common::utils::recover_c_char;
use crate::{free_str_ptr};

/// Display transaction for Kaspa
/// 
#[repr(C)]
pub struct DisplayKaspaTx {
    pub network: PtrString,
    pub total_spend: PtrString,
    pub fee: u64,
    pub fee_per_sompi: f64,
    pub inputs: VecFFI<DisplayKaspaInput>,
    pub outputs: VecFFI<DisplayKaspaOutput>,
    pub total_input: u64,
    pub total_output: u64,
}

#[repr(C)]
pub struct DisplayKaspaInput {
    pub address: PtrString,
    pub amount: PtrString,
    pub value: u64,
    pub path: PtrString,
    pub is_mine: bool,
}

#[repr(C)]
pub struct DisplayKaspaOutput {
    pub address: PtrString,
    pub amount: PtrString,
    pub value: u64,
    pub path: PtrString,
    pub is_mine: bool,
    pub is_external: bool,
}

// Implement Free trait for memory management
impl Free for DisplayKaspaInput {
    unsafe fn free(&self) {
        free_str_ptr!(self.address);
        free_str_ptr!(self.amount);
        free_str_ptr!(self.path);
    }
}

impl Free for DisplayKaspaOutput {
    unsafe fn free(&self) {
        free_str_ptr!(self.address);
        free_str_ptr!(self.amount);
        free_str_ptr!(self.path);
    }
}

impl Free for DisplayKaspaTx {
    unsafe fn free(&self) {
        free_str_ptr!(self.network);
        free_str_ptr!(self.total_spend);

        self.inputs.free();
        self.outputs.free();
    }
}

use crate::common::structs::TransactionParseResult;
make_free_method!(TransactionParseResult<DisplayKaspaTx>);
