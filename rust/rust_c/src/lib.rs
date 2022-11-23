#![no_std]
#![feature(alloc_error_handler)]
#![feature(vec_into_raw_parts)]
#![feature(error_in_core)]
#![allow(unused_unsafe)]
extern crate alloc;

use alloc::boxed::Box;
use cty::c_void;

mod bindings;
mod interfaces;
mod my_alloc;

#[cfg(not(test))]
use core::panic::PanicInfo;
#[cfg(not(test))]
use cstr_core::CString;

#[cfg(not(test))]
#[alloc_error_handler]
fn oom(layout: core::alloc::Layout) -> ! {
    unsafe {
        crate::bindings::LogRustPanic(
            CString::new(alloc::format!("Out of memory: {:?}", layout))
                .unwrap()
                .into_raw(),
        )
    };
    loop {}
}

#[cfg(not(test))]
#[panic_handler]
fn panic(e: &PanicInfo) -> ! {
    unsafe {
        crate::bindings::LogRustPanic(
            CString::new(alloc::format!("rust panic: {:?}", e))
                .unwrap()
                .into_raw(),
        )
    }
    loop {}
}

#[no_mangle]
pub extern "C" fn free_rust_value(any_ptr: *mut c_void) {
    if any_ptr.is_null() {
        return;
    }
    unsafe {
        drop(Box::from_raw(any_ptr));
    }
}
