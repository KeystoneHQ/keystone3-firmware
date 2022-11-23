use crate::free_str_ptr;
use crate::interfaces::free::Free;
use crate::interfaces::types::PtrString;
use crate::interfaces::utils::recover_c_char;
use app_wallets::keplr::sync_info::SyncInfo;

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
