use super::free::{Free, SimpleFree};
use alloc::vec::Vec;
use cty::size_t;

use super::types::PtrT;
use crate::{impl_c_ptr, impl_simple_free};

#[repr(C)]
pub struct VecFFI<T> {
    pub data: PtrT<T>,
    pub size: size_t,
    pub cap: size_t,
}

impl_c_ptr!(VecFFI<T>);

impl<T> From<Vec<T>> for VecFFI<T> {
    fn from(value: Vec<T>) -> Self {
        let (ptr, size, cap) = value.into_raw_parts();
        Self {
            data: ptr,
            size,
            cap,
        }
    }
}

impl_simple_free!(u8);

impl<T: SimpleFree> SimpleFree for VecFFI<T> {
    unsafe fn free(&self) {
        if self.data.is_null() {
            return;
        }
        let _x = Vec::from_raw_parts(self.data, self.size, self.cap);
    }
}

impl<T: Free> Free for VecFFI<T> {
    unsafe fn free(&self) {
        if self.data.is_null() {
            return;
        }
        let x = Vec::from_raw_parts(self.data, self.size, self.cap);
        x.iter().for_each(|v| {
            v.free();
        });
    }
}

#[repr(C)]
pub struct CSliceFFI<T> {
    pub data: PtrT<T>,
    pub size: size_t,
}

impl_c_ptr!(CSliceFFI<T>);
