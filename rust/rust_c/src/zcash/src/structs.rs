use core::ptr::null_mut;

use alloc::vec::Vec;
use app_zcash::pczt::structs::{
    ParsedFrom, ParsedOrchard, ParsedPczt, ParsedTo, ParsedTransparent,
};
use common_rust_c::{
    extract_ptr_with_type,
    ffi::VecFFI,
    free::Free,
    free_ptr_with_type,
    free_str_ptr,
    free_vec,
    impl_c_ptr,
    impl_c_ptrs,
    types::{Ptr, PtrString},
    utils::convert_c_char,
};

use cstr_core;

#[repr(C)]
pub struct DisplayPczt {
    pub transparent: Ptr<DisplayTransparent>,
    pub orchard: Ptr<DisplayOrchard>,
    pub total_transfer_value: PtrString,
}

impl From<&ParsedPczt> for DisplayPczt {
    fn from(pczt: &ParsedPczt) -> Self {
        Self {
            transparent: pczt
                .get_transparent()
                .map(|t| DisplayTransparent::from(&t).c_ptr())
                .unwrap_or(null_mut()),
            orchard: pczt
                .get_orchard()
                .map(|o| DisplayOrchard::from(&o).c_ptr())
                .unwrap_or(null_mut()),
            total_transfer_value: convert_c_char(pczt.get_total_transfer_value()),
        }
    }
}

impl Free for DisplayPczt {
    fn free(&self) {
        free_str_ptr!(self.total_transfer_value);
        free_ptr_with_type!(self.transparent, DisplayTransparent);
        free_ptr_with_type!(self.orchard, DisplayOrchard);
    }
}

#[repr(C)]
pub struct DisplayTransparent {
    pub from: Ptr<VecFFI<DisplayFrom>>,
    pub to: Ptr<VecFFI<DisplayTo>>,
}

impl From<&ParsedTransparent> for DisplayTransparent {
    fn from(transparent: &ParsedTransparent) -> Self {
        Self {
            from: VecFFI::from(
                transparent
                    .get_from()
                    .iter()
                    .map(|f| f.into())
                    .collect::<Vec<DisplayFrom>>(),
            )
            .c_ptr(),
            to: VecFFI::from(
                transparent
                    .get_to()
                    .iter()
                    .map(|t| t.into())
                    .collect::<Vec<DisplayTo>>(),
            )
            .c_ptr(),
        }
    }
}

impl Free for DisplayTransparent {
    fn free(&self) {
        free_vec!(self.from);
        free_vec!(self.to);
    }
}

#[repr(C)]
pub struct DisplayFrom {
    pub address: PtrString,
    pub value: PtrString,
    pub is_mine: bool,
}

impl From<&ParsedFrom> for DisplayFrom {
    fn from(from: &ParsedFrom) -> Self {
        Self {
            address: convert_c_char(from.get_address()),
            value: convert_c_char(from.get_value()),
            is_mine: from.get_is_mine(),
        }
    }
}

impl Free for DisplayFrom {
    fn free(&self) {
        free_str_ptr!(self.address);
        free_str_ptr!(self.value);
    }
}

#[repr(C)]
pub struct DisplayTo {
    pub address: PtrString,
    pub value: PtrString,
    pub is_change: bool,
    pub visible: bool,
    pub memo: PtrString,
}

impl From<&ParsedTo> for DisplayTo {
    fn from(to: &ParsedTo) -> Self {
        Self {
            address: convert_c_char(to.get_address()),
            value: convert_c_char(to.get_value()),
            is_change: to.get_is_change(),
            visible: to.get_visible(),
            memo: to.get_memo().map(convert_c_char).unwrap_or(null_mut()),
        }
    }
}

impl Free for DisplayTo {
    fn free(&self) {
        free_str_ptr!(self.address);
        free_str_ptr!(self.value);
        free_str_ptr!(self.memo);
    }
}

#[repr(C)]
pub struct DisplayOrchard {
    pub from: Ptr<VecFFI<DisplayFrom>>,
    pub to: Ptr<VecFFI<DisplayTo>>,
}

impl From<&ParsedOrchard> for DisplayOrchard {
    fn from(orchard: &ParsedOrchard) -> Self {
        Self {
            from: VecFFI::from(
                orchard
                    .get_from()
                    .iter()
                    .map(|f| f.into())
                    .collect::<Vec<DisplayFrom>>(),
            )
            .c_ptr(),
            to: VecFFI::from(
                orchard
                    .get_to()
                    .iter()
                    .map(|t| t.into())
                    .collect::<Vec<DisplayTo>>(),
            )
            .c_ptr(),
        }
    }
}

impl Free for DisplayOrchard {
    fn free(&self) {
        free_vec!(self.from);
        free_vec!(self.to);
    }
}

impl_c_ptrs!(
    DisplayPczt,
    DisplayTransparent,
    DisplayFrom,
    DisplayTo,
    DisplayOrchard
);
