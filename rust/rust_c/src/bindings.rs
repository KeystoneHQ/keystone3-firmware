#[cfg(not(test))]
extern "C" {
    pub fn LogRustMalloc(p: *mut cty::c_void, size: u32);
    pub fn LogRustFree(p: *mut cty::c_void);
    pub fn LogRustPanic(p: *mut cty::c_char);
}
//
// #[cfg(test)]
// #[allow(non_snake_case)]
// pub fn LogRustMalloc(p: *mut c_void, size: u32) {}
//
// #[cfg(test)]
// #[allow(non_snake_case)]
// pub fn LogRustPanic(p: *mut c_void, size: u32) {}
//
// #[cfg(test)]
// #[allow(non_snake_case)]
// pub fn LogRustFree(p: *mut c_void) {}
