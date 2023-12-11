#[cfg(not(test))]
pub struct KTAllocator;

#[allow(dead_code)]
extern "C" {
    pub fn RustMalloc(size: i32) -> *mut cty::c_void;
    pub fn RustFree(p: *mut cty::c_void);
}

#[cfg(not(test))]
unsafe impl core::alloc::GlobalAlloc for KTAllocator {
    unsafe fn alloc(&self, layout: core::alloc::Layout) -> *mut u8 {
        let ptr = RustMalloc(layout.size() as i32);
        crate::bindings::LogRustMalloc(ptr, layout.size() as u32);
        ptr as *mut u8
    }
    unsafe fn dealloc(&self, ptr: *mut u8, _layout: core::alloc::Layout) {
        crate::bindings::LogRustFree(ptr as *mut cty::c_void);
        RustFree(ptr as _)
    }
}
