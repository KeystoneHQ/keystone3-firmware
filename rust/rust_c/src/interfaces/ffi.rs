use alloc::vec::Vec;
use cty::size_t;

use crate::impl_c_ptr;
use crate::interfaces::free::Free;
use crate::interfaces::types::PtrT;

#[repr(C)]
pub struct VecFFI<T: Free> {
    pub(crate) data: PtrT<T>,
    pub(crate) size: size_t,
    pub(crate) cap: size_t,
}

impl_c_ptr!(VecFFI<T>);

impl<T: Free> From<Vec<T>> for VecFFI<T> {
    fn from(value: Vec<T>) -> Self {
        let (ptr, size, cap) = value.into_raw_parts();
        Self {
            data: ptr,
            size,
            cap,
        }
    }
}

#[repr(C)]
pub struct CSliceFFI<T: Free> {
    pub data: PtrT<T>,
    pub size: size_t,
}

impl_c_ptr!(CSliceFFI<T>);
