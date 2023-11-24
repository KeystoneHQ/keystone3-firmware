#[cfg(not(test))]
pub struct KTAllocator;

#[allow(dead_code)]
extern "C" {
    pub fn RustMalloc(size: i32) -> *mut cty::c_void;
    pub fn RustFree(p: *mut cty::c_void);
}

#[cfg(feature = "memory_profile")]
pub mod alloc_tracker {
    use alloc::{collections::BTreeMap, string::String};
    pub type AllocTracker = BTreeMap<u32, TrackerNode>;

    const TRACKER: AllocTracker = BTreeMap::new();
    pub struct TrackerNode {
        rust_type: String,
        file: String,
        line: u32,
        col: u32,
    }
    pub fn get_alloc_tracker() -> AllocTracker {
        return TRACKER;
    }
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
