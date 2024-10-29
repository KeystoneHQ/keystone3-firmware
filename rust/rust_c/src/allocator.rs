use crate::my_alloc::KTAllocator;

#[global_allocator]
static KT_ALLOCATOR: KTAllocator = KTAllocator;

use core::panic::PanicInfo;
use cstr_core::CString;

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
