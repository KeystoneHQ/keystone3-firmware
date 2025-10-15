#![allow(dead_code)]

use super::errors::ErrorCodes;
use super::errors::RustCError;
use alloc::boxed::Box;
use alloc::string::{String, ToString};
use core::ptr::null_mut;
use cstr_core::CString;
use keystore::errors::KeystoreError;
use ur_registry::error::URError;

use super::free::Free;
use super::types::{PtrString, PtrT};
use crate::{
    free_str_ptr, impl_c_ptr, impl_new_error, impl_response, impl_simple_c_ptr,
    impl_simple_new_error, make_free_method,
};

#[repr(C)]
pub struct TransactionParseResult<T> {
    data: *mut T,
    error_code: u32,
    error_message: PtrString,
}

impl<T> TransactionParseResult<T> {
    pub fn new() -> Self {
        Self {
            data: null_mut(),
            error_code: 0,
            error_message: null_mut(),
        }
    }
    pub fn success(data: *mut T) -> Self {
        let _self = Self::new();
        Self { data, .._self }
    }
}

impl<T: Free> Free for TransactionParseResult<T> {
    unsafe fn free(&self) {
        free_str_ptr!(self.error_message);
        if self.data.is_null() {
            return;
        }
        let x = Box::from_raw(self.data);
        x.free()
    }
}

impl_response!(TransactionParseResult<T>);

#[repr(C)]
pub struct TransactionCheckResult {
    error_code: u32,
    error_message: PtrString,
}

impl TransactionCheckResult {
    pub fn new() -> Self {
        Self {
            error_code: 0,
            error_message: null_mut(),
        }
    }
}

impl Free for TransactionCheckResult {
    unsafe fn free(&self) {
        free_str_ptr!(self.error_message);
    }
}

impl_response!(TransactionCheckResult);

make_free_method!(TransactionCheckResult);

#[repr(C)]
pub struct SimpleResponse<T> {
    pub data: *mut T,
    error_code: u32,
    error_message: PtrString,
}

impl<T> SimpleResponse<T> {
    pub fn new() -> Self {
        Self {
            data: null_mut(),
            error_code: 0,
            error_message: null_mut(),
        }
    }
    pub fn success(data: *mut T) -> Self {
        let _self = Self::new();
        Self { data, .._self }
    }
}

impl Free for SimpleResponse<u8> {
    unsafe fn free(&self) {
        if !self.data.is_null() {
            let _x = Box::from_raw(self.data);
        }
        free_str_ptr!(self.error_message);
    }
}

impl Free for SimpleResponse<i8> {
    unsafe fn free(&self) {
        if !self.data.is_null() {
            let _x = Box::from_raw(self.data);
        }
        free_str_ptr!(self.error_message);
    }
}

impl_simple_c_ptr!(SimpleResponse<T>);
impl_simple_new_error!(SimpleResponse<T>);

#[repr(C)]
pub struct Response<T> {
    data: *mut T,
    error_code: u32,
    error_message: PtrString,
}

impl<T> Response<T> {
    pub fn new() -> Self {
        Self {
            data: null_mut(),
            error_code: 0,
            error_message: null_mut(),
        }
    }
    pub fn success_ptr(data: *mut T) -> Self {
        let _self = Self::new();
        Self { data, .._self }
    }
    pub fn success(data: T) -> Self {
        let data = Box::into_raw(Box::new(data));
        let _self = Self::new();
        Self { data, .._self }
    }
}

impl<T: Free> Free for Response<T> {
    unsafe fn free(&self) {
        free_str_ptr!(self.error_message);
        if self.data.is_null() {
            return;
        }
        let x = Box::from_raw(self.data);
        x.free()
    }
}

impl_response!(Response<T>);

#[repr(C)]
pub struct ExtendedPublicKey {
    pub path: PtrString,
    pub xpub: PtrString,
}

impl_c_ptr!(ExtendedPublicKey);

impl Free for ExtendedPublicKey {
    unsafe fn free(&self) {
        free_str_ptr!(self.path);
        free_str_ptr!(self.xpub);
    }
}

impl Free for PtrT<ExtendedPublicKey> {
    unsafe fn free(&self) {
        let x = Box::from_raw(*self);
        x.free()
    }
}

#[repr(C)]
pub struct ZcashKey {
    pub key_text: PtrString,
    pub key_name: PtrString,
    pub index: u32,
}

impl_c_ptr!(ZcashKey);

impl Free for ZcashKey {
    unsafe fn free(&self) {
        free_str_ptr!(self.key_text);
        free_str_ptr!(self.key_name);
    }
}

impl Free for PtrT<ZcashKey> {
    unsafe fn free(&self) {
        let x = Box::from_raw(*self);
        x.free()
    }
}
