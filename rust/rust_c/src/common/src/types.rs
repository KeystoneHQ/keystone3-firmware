use alloc::boxed::Box;
use core::ffi::c_void;

use crate::free::Free;
use cty::c_char;

pub type PtrString = *mut c_char;
pub type PtrVoid = *mut c_void;
pub type ConstPtrVoid = *const c_void; //for compiler warning
pub type PtrBytes = *mut u8;
pub type PtrUR = PtrVoid;
pub type ConstPtrUR = ConstPtrVoid;
pub type PtrDecoder = PtrVoid;
pub type PtrEncoder = PtrVoid;
pub type PtrT<T> = *mut T;
pub type Ptr<T> = *mut T;

impl Free for PtrString {
    fn free(&self) {
        unsafe {
            let _ = Box::from_raw(*self);
        }
    }
}
