use core::{ptr::null_mut, slice};

use crate::common::{
    errors::RustCError,
    ffi::VecFFI,
    free::Free,
    types::{Ptr, PtrString},
    utils::convert_c_char,
};
use crate::{free_str_ptr, free_vec, impl_c_ptr, impl_c_ptrs};
use alloc::{string::ToString, vec::Vec};
use app_zcash::pczt::structs::{
    ParsedFrom, ParsedOrchard, ParsedPczt, ParsedTo, ParsedTransparent,
};
use cstr_core;

#[repr(C)]
pub struct DisplayPczt {
    pub transparent: Ptr<DisplayTransparent>,
    pub orchard: Ptr<DisplayOrchard>,
    pub ironwood: Ptr<DisplayOrchard>,
    pub total_transfer_value: PtrString,
    pub fee_value: PtrString,
    pub has_sapling: bool,
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
            ironwood: pczt
                .get_ironwood()
                .map(|o| DisplayOrchard::from(&o).c_ptr())
                .unwrap_or(null_mut()),
            total_transfer_value: convert_c_char(pczt.get_total_transfer_value()),
            fee_value: convert_c_char(pczt.get_fee_value()),
            has_sapling: pczt.get_has_sapling(),
        }
    }
}

impl Free for DisplayPczt {
    unsafe fn free(&self) {
        free_str_ptr!(self.total_transfer_value);
        free_str_ptr!(self.fee_value);
        free_display_ptr(self.transparent);
        free_display_ptr(self.orchard);
        free_display_ptr(self.ironwood);
    }
}

#[repr(C)]
pub struct DisplayZcashBatch {
    pub txs: Ptr<VecFFI<DisplayPczt>>,
}

impl From<Vec<DisplayPczt>> for DisplayZcashBatch {
    fn from(txs: Vec<DisplayPczt>) -> Self {
        Self {
            txs: VecFFI::from(txs).c_ptr(),
        }
    }
}

impl Free for DisplayZcashBatch {
    unsafe fn free(&self) {
        free_vec!(self.txs);
    }
}

unsafe fn free_display_ptr<T: Free>(ptr: Ptr<T>) {
    if ptr.is_null() {
        return;
    }

    let boxed = alloc::boxed::Box::from_raw(ptr);
    boxed.free();
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
    unsafe fn free(&self) {
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
            address: convert_c_char(from.get_address().unwrap_or("<shielded>".into())),
            value: convert_c_char(from.get_value()),
            is_mine: from.get_is_mine(),
        }
    }
}

impl Free for DisplayFrom {
    unsafe fn free(&self) {
        free_str_ptr!(self.address);
        free_str_ptr!(self.value);
    }
}

#[repr(C)]
pub struct DisplayTo {
    pub address: PtrString,
    pub value: PtrString,
    pub is_change: bool,
    pub memo: PtrString,
}

impl From<&ParsedTo> for DisplayTo {
    fn from(to: &ParsedTo) -> Self {
        Self {
            address: convert_c_char(to.get_address()),
            value: convert_c_char(to.get_value()),
            is_change: to.get_is_change(),
            memo: to.get_memo().map(convert_c_char).unwrap_or(null_mut()),
        }
    }
}

impl Free for DisplayTo {
    unsafe fn free(&self) {
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
    unsafe fn free(&self) {
        free_vec!(self.from);
        free_vec!(self.to);
    }
}

impl_c_ptrs!(
    DisplayPczt,
    DisplayZcashBatch,
    DisplayTransparent,
    DisplayFrom,
    DisplayTo,
    DisplayOrchard
);

/// The batch display rows produced by the check pass and converted to FFI
/// structs by the batch parse FFI, so parse no longer re-decrypts every output.
///
/// Opaque to C: this is a plain Rust struct (deliberately not `#[repr(C)]`), and C
/// only ever holds it behind the [`ZcashCheckedPczt::display`] pointer without
/// dereferencing it. The batch check FFI builds it; the batch parse FFI reads it
/// Rust-side and turns each [`ParsedPczt`] into a [`DisplayPczt`].
#[cfg(feature = "cypherpunk")]
pub struct BatchDisplayCache {
    rows: Vec<ParsedPczt>,
}

#[cfg(feature = "cypherpunk")]
impl BatchDisplayCache {
    /// Stores the final review rows after any migration compaction.
    pub fn new(rows: Vec<ParsedPczt>) -> Self {
        Self { rows }
    }

    /// Returns the final review rows that batch parse converts for the C UI.
    pub fn rows(&self) -> &[ParsedPczt] {
        &self.rows
    }
}

/// Normalized transaction bytes verified during check and retained by C between
/// the check, display, and sign stages (`checked_PCZT` on the C side).
///
/// `data` is opaque to C: the normalized PCZT encoding in the single-transaction
/// flow, or the normalized `ZcashSignBatch` CBOR in the batch flow. `display`
/// (cypherpunk only) is the opaque [`BatchDisplayCache`] the batch check builds
/// so parse converts stored rows instead of re-decrypting; it is null for the
/// single-transaction and multi-coins flows. Construct exclusively from check
/// results.
#[repr(C)]
pub struct ZcashCheckedPczt {
    pub data: Ptr<VecFFI<u8>>,
    /// Opaque batch display cache (null for the single-tx / multi-coins flows).
    /// C never dereferences this.
    #[cfg(feature = "cypherpunk")]
    pub display: Ptr<BatchDisplayCache>,
}

impl ZcashCheckedPczt {
    /// Wraps bytes verified during check. The display cache is null; the batch
    /// flow uses [`Self::new_with_display`] instead.
    pub fn new(data: Vec<u8>) -> Self {
        Self {
            data: VecFFI::from(data).c_ptr(),
            #[cfg(feature = "cypherpunk")]
            display: null_mut(),
        }
    }

    /// Wraps checked batch bytes together with the display cache the check
    /// produced, so the batch parse FFI converts the stored rows instead of
    /// re-decrypting every output. The cache is freed with the container in
    /// [`Free::free`].
    #[cfg(feature = "cypherpunk")]
    pub fn new_with_display(data: Vec<u8>, display: BatchDisplayCache) -> Self {
        let mut checked = Self::new(data);
        checked.display = alloc::boxed::Box::into_raw(alloc::boxed::Box::new(display));
        checked
    }

    /// Borrows the bytes produced by a successful check.
    pub unsafe fn checked_bytes(&self) -> Result<&[u8], RustCError> {
        if self.data.is_null() {
            return Err(RustCError::InvalidData(
                "checked PCZT has no data".to_string(),
            ));
        }
        let vec = &*self.data;
        Ok(slice::from_raw_parts(vec.data, vec.size))
    }
}

impl_c_ptr!(ZcashCheckedPczt);

impl Free for ZcashCheckedPczt {
    unsafe fn free(&self) {
        if !self.data.is_null() {
            let vec_ffi = alloc::boxed::Box::from_raw(self.data);
            drop(Vec::from_raw_parts(vec_ffi.data, vec_ffi.size, vec_ffi.cap));
        }
        // Free the batch display cache exactly once when present (the single-tx
        // and multi-coins flows leave it null).
        #[cfg(feature = "cypherpunk")]
        if !self.display.is_null() {
            drop(alloc::boxed::Box::from_raw(self.display));
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use alloc::vec::Vec;

    #[test]
    fn test_checked_pczt_bytes_round_trip() {
        let checked = ZcashCheckedPczt::new(b"normalized-bytes".to_vec());
        let bytes = unsafe { checked.checked_bytes() }.unwrap();
        assert_eq!(bytes, b"normalized-bytes");
        unsafe { checked.free() };
    }
}
