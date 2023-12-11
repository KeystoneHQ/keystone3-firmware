#![no_std]
#![feature(alloc_error_handler)]
#![feature(vec_into_raw_parts)]
#![feature(error_in_core)]
#![allow(unused_unsafe)]
extern crate alloc;

use cstr_core::CString;

pub mod bindings;
pub mod my_alloc;

#[cfg(not(test))]
use core::panic::PanicInfo;

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
