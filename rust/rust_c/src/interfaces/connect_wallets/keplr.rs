use crate::extract_array;
use crate::interfaces::connect_wallets::structs::KeplrAccount;
use crate::interfaces::ffi::CSliceFFI;
use crate::interfaces::types::{PtrBytes, PtrT};
use crate::interfaces::ur::{UREncodeResult, FRAGMENT_MAX_LENGTH_DEFAULT};
use crate::interfaces::utils::recover_c_array;
use alloc::format;
use alloc::string::ToString;
use alloc::vec::Vec;
use app_wallets::keplr::{generate_sync_ur, sync_info::SyncInfo};
use cty::uint32_t;
use third_party::ur_registry::error::URError;
use third_party::ur_registry::extend::crypto_multi_accounts::CryptoMultiAccounts;
use third_party::ur_registry::traits::RegistryItem;

#[no_mangle]
pub extern "C" fn get_connect_keplr_wallet_ur(
    master_fingerprint: PtrBytes,
    master_fingerprint_length: uint32_t,
    keplr_accounts: PtrT<CSliceFFI<KeplrAccount>>,
) -> *mut UREncodeResult {
    if master_fingerprint_length != 4 {
        return UREncodeResult::from(URError::UrEncodeError(format!(
            "master fingerprint length must be 4, current is {}",
            master_fingerprint_length
        )))
        .c_ptr();
    }
    let mfp = extract_array!(master_fingerprint, u8, master_fingerprint_length);
    let mfp = match <&[u8; 4]>::try_from(mfp) {
        Ok(mfp) => mfp,
        Err(e) => return UREncodeResult::from(URError::UrEncodeError(e.to_string())).c_ptr(),
    };
    unsafe {
        let accounts: &[KeplrAccount] = recover_c_array(keplr_accounts);
        let sync_infos: Vec<SyncInfo> =
            accounts.into_iter().map(|account| account.into()).collect();
        let result = generate_sync_ur(mfp, &sync_infos);
        match result.map(|v| v.try_into()) {
            Ok(v) => match v {
                Ok(data) => UREncodeResult::encode(
                    data,
                    CryptoMultiAccounts::get_registry_type().get_type(),
                    FRAGMENT_MAX_LENGTH_DEFAULT,
                )
                .c_ptr(),
                Err(e) => UREncodeResult::from(e).c_ptr(),
            },
            Err(e) => UREncodeResult::from(e).c_ptr(),
        }
    }
}
