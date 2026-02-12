// Shared structures for Kaspa FFI

use crate::common::ffi::VecFFI;
use crate::common::free::Free;
use crate::common::types::{PtrString, PtrT};
use crate::make_free_method;
use alloc::boxed::Box;
use alloc::string::ToString;
use crate::common::utils::recover_c_char;

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
        // Free PtrString fields (heap-allocated via convert_c_char)
        let _ = Box::from_raw(self.address);
        let _ = Box::from_raw(self.amount);
        // if let Some(path) = self.path {
        //     let _ = Box::from_raw(path);
        // }

        if !self.path.is_null() {
            // 如果是 Free trait 释放内存：
            unsafe { recover_c_char(self.path) };
        }
    }
}

impl Free for DisplayKaspaOutput {
    unsafe fn free(&self) {
        // Free PtrString fields (heap-allocated via convert_c_char)
        let _ = Box::from_raw(self.address);
        let _ = Box::from_raw(self.amount);
        if !self.path.is_null() {
            // 如果是 Free trait 释放内存：
            unsafe { recover_c_char(self.path) };
        }
    }
}

impl Free for DisplayKaspaTx {
    unsafe fn free(&self) {
        self.inputs.free();
        self.outputs.free();
    }
}

use crate::common::structs::TransactionParseResult;
make_free_method!(TransactionParseResult<DisplayKaspaTx>);
