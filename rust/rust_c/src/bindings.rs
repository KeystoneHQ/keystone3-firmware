extern "C" {
    pub fn LogRustMalloc(p: *mut cty::c_void, size: u32);
    pub fn LogRustFree(p: *mut cty::c_void);
    pub fn LogRustPanic(p: *mut cty::c_char);
}
