use alloc::boxed::Box;
use alloc::string::ToString;
use alloc::vec::Vec;

use app_wallets::keplr::sync_info::SyncInfo;
use ur_registry::extend::key_derivation_schema::{Curve, DerivationAlgo};
use ur_registry::extend::qr_hardware_call::{CallParams, CallType, QRHardwareCall};

use crate::common::errors::RustCError;
use crate::common::ffi::VecFFI;
use crate::common::free::Free;
use crate::common::structs::Response;
use crate::common::types::{Ptr, PtrString, PtrT};
use crate::common::utils::{convert_c_char, recover_c_char};
use crate::{check_and_free_ptr, free_str_ptr, impl_c_ptr, make_free_method};

#[repr(C)]
pub struct KeplrAccount {
    pub(crate) name: PtrString,
    pub(crate) path: PtrString,
    pub(crate) xpub: PtrString,
}

impl Free for KeplrAccount {
    fn free(&self) {
        free_str_ptr!(self.name);
        free_str_ptr!(self.path);
        free_str_ptr!(self.xpub);
    }
}

impl From<&KeplrAccount> for SyncInfo {
    fn from(value: &KeplrAccount) -> Self {
        SyncInfo {
            name: recover_c_char(value.name),
            hd_path: recover_c_char(value.path),
            xpub: recover_c_char(value.xpub),
        }
    }
}
