use crate::structs::KeplrAccount;
use alloc::format;
use alloc::string::ToString;
use alloc::vec::Vec;
use app_wallets::keplr::{generate_sync_ur, sync_info::SyncInfo};
use common_rust_c::extract_array;
use common_rust_c::ffi::CSliceFFI;
use common_rust_c::types::{PtrBytes, PtrT};
use common_rust_c::ur::{UREncodeResult, FRAGMENT_MAX_LENGTH_DEFAULT};
use common_rust_c::utils::{recover_c_array, recover_c_char};
use cty::uint32_t;
use keystore::algorithms::secp256k1::derive_extend_public_key;
use ur_registry::error::URError;
use ur_registry::extend::crypto_multi_accounts::CryptoMultiAccounts;
use ur_registry::traits::RegistryItem;

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
        let sync_infos: Vec<SyncInfo> = accounts
            .into_iter()
            .map(|account| {
                let hd_path = recover_c_char(account.path);
                let path_parts: Vec<&str> = hd_path.split("/").collect();
                let path_len = path_parts.len();
                SyncInfo {
                    name: recover_c_char(account.name),
                    hd_path: hd_path.clone(),
                    xpub: derive_extend_public_key(
                        &recover_c_char(account.xpub),
                        &format!(
                            "m/{}/{}",
                            path_parts[path_len - 2],
                            path_parts[path_len - 1]
                        ),
                    )
                    .map(|e| e.to_string())
                    .unwrap_or("".to_string()),
                }
            })
            .collect();
        let result = generate_sync_ur(mfp, &sync_infos);
        match result.map(|v| v.try_into()) {
            Ok(v) => match v {
                Ok(data) => UREncodeResult::encode(
                    data,
                    CryptoMultiAccounts::get_registry_type().get_type(),
                    FRAGMENT_MAX_LENGTH_DEFAULT.clone(),
                )
                .c_ptr(),
                Err(e) => UREncodeResult::from(e).c_ptr(),
            },
            Err(e) => UREncodeResult::from(e).c_ptr(),
        }
    }
}
