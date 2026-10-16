use crate::my_alloc::KTAllocator;

#[global_allocator]
static KT_ALLOCATOR: KTAllocator = KTAllocator;

use core::panic::PanicInfo;

static OOM_MESSAGE: &[u8] = b"rust out of memory\0";

#[alloc_error_handler]
fn oom(_layout: core::alloc::Layout) -> ! {
    unsafe { crate::bindings::LogRustPanic(OOM_MESSAGE.as_ptr() as *mut cty::c_char) };
    loop {}
}

#[panic_handler]
fn panic(e: &PanicInfo) -> ! {
    #[cfg(any(
        feature = "production-multi-coins",
        feature = "production-btc-only",
        feature = "production-cypherpunk"
    ))]
    {
        let _ = e;
        static PANIC_MESSAGE: &[u8] = b"rust panic\0";
        unsafe { crate::bindings::LogRustPanic(PANIC_MESSAGE.as_ptr() as *mut cty::c_char) };
    }
    #[cfg(not(any(
        feature = "production-multi-coins",
        feature = "production-btc-only",
        feature = "production-cypherpunk"
    )))]
    {
        let message = match e.location() {
            Some(location) => {
                alloc::format!("rust panic at {}:{}", location.file(), location.line())
            }
            None => alloc::string::String::from("rust panic"),
        };
        unsafe { crate::bindings::LogRustPanic(crate::common::utils::convert_c_char(message)) }
    }
    loop {}
}
