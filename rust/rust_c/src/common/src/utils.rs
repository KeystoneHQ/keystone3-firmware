use alloc::string::{String, ToString};
use core::slice;

use crate::extract_ptr_with_type;
use crate::ffi::CSliceFFI;
use crate::free::Free;
use cstr_core::{CStr, CString};
use cty::c_char;

use crate::types::{PtrString, PtrT};

pub fn convert_c_char(s: String) -> PtrString {
    CString::new(s.clone()).unwrap().into_raw()
}

pub fn recover_c_char(s: *mut c_char) -> String {
    unsafe { CStr::from_ptr(s).to_str().unwrap().to_string() }
}

pub unsafe fn recover_c_array<'a, T: Free>(s: PtrT<CSliceFFI<T>>) -> &'a [T] {
    let boxed_keys = extract_ptr_with_type!(s, CSliceFFI<T>);
    slice::from_raw_parts(boxed_keys.data, boxed_keys.size)
}
