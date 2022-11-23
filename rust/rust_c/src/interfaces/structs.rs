#![allow(dead_code)]

use alloc::boxed::Box;
use alloc::string::{String, ToString};
use core::ptr::null_mut;

use app_bitcoin::errors::BitcoinError;
use app_cardano::errors::CardanoError;
use app_ethereum::errors::EthereumError;

use cstr_core::CString;
use keystore::errors::KeystoreError;
use third_party::ur_registry::error::URError;

use crate::interfaces::errors::{ErrorCodes, RustCError};
use crate::interfaces::free::Free;
use crate::interfaces::types::PtrString;
use crate::{
    free_str_ptr, impl_c_ptr, impl_new_error, impl_response, impl_simple_c_ptr,
    impl_simple_new_error,
};

use super::types::PtrT;

#[repr(C)]
pub struct TransactionParseResult<T: Free> {
    data: *mut T,
    error_code: u32,
    error_message: PtrString,
}

impl<T: Free> TransactionParseResult<T> {
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
    fn free(&self) {
        free_str_ptr!(self.error_message);
        if self.data.is_null() {
            return;
        }
        unsafe {
            let x = Box::from_raw(self.data);
            x.free()
        }
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
    fn free(&self) {
        free_str_ptr!(self.error_message);
    }
}

impl_response!(TransactionCheckResult);

#[repr(C)]
pub struct SimpleResponse<T> {
    data: *mut T,
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
    fn free(&self) {
        unsafe {
            if !self.data.is_null() {
                let _x = Box::from_raw(self.data);
            }
        }
        free_str_ptr!(self.error_message);
    }
}

impl Free for SimpleResponse<i8> {
    fn free(&self) {
        unsafe {
            if !self.data.is_null() {
                let _x = Box::from_raw(self.data);
            }
        }
        free_str_ptr!(self.error_message);
    }
}

impl_simple_c_ptr!(SimpleResponse<T>);
impl_simple_new_error!(SimpleResponse<T>);

#[repr(C)]
pub struct Response<T: Free> {
    data: *mut T,
    error_code: u32,
    error_message: PtrString,
}

impl<T: Free> Response<T> {
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
    fn free(&self) {
        free_str_ptr!(self.error_message);
        if self.data.is_null() {
            return;
        }
        unsafe {
            let x = Box::from_raw(self.data);
            x.free()
        }
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
    fn free(&self) {
        free_str_ptr!(self.path);
        free_str_ptr!(self.xpub);
    }
}

impl Free for PtrT<ExtendedPublicKey> {
    fn free(&self) {
        unsafe {
            let x = Box::from_raw(*self);
            x.free()
        }
    }
}
