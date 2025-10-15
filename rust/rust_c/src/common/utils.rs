use alloc::string::{String, ToString};
use core::slice;

use super::ffi::CSliceFFI;
use super::free::Free;
use crate::{extract_array, extract_ptr_with_type};
use cstr_core::{CStr, CString};
use cty::c_char;

use crate::common::types::{PtrString, PtrT};

pub fn convert_c_char(s: String) -> PtrString {
    CString::new(s).unwrap().into_raw()
}

pub unsafe fn recover_c_char(s: *mut c_char) -> String {
    CStr::from_ptr(s).to_str().unwrap().to_string()
}

pub unsafe fn recover_c_array<'a, T: Free>(s: PtrT<CSliceFFI<T>>) -> &'a [T] {
    let boxed_keys = extract_ptr_with_type!(s, CSliceFFI<T>);
    extract_array!(boxed_keys.data, T, boxed_keys.size)
}
