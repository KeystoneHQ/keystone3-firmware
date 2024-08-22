use alloc::string::ToString;
use alloc::vec::Vec;
use alloc::{format, slice};
use app_wallets::zcash::{generate_sync_ur, UFVKInfo};
use common_rust_c::extract_array;
use common_rust_c::ffi::CSliceFFI;
use common_rust_c::structs::ZcashKey;
use common_rust_c::types::{Ptr, PtrBytes, PtrString};
use common_rust_c::ur::{UREncodeResult, FRAGMENT_MAX_LENGTH_DEFAULT};
use common_rust_c::utils::{recover_c_array, recover_c_char};
use third_party::ur_registry::bytes::Bytes;
use third_party::ur_registry::error::URError;
use third_party::ur_registry::traits::RegistryItem;
use third_party::ur_registry::zcash::zcash_accounts::ZcashAccounts;

#[no_mangle]
pub extern "C" fn get_connect_zcash_wallet_ur(
    seed_fingerprint: PtrBytes,
    seed_fingerprint_len: u32,
    zcash_keys: Ptr<CSliceFFI<ZcashKey>>,
) -> *mut UREncodeResult {
    if seed_fingerprint_len != 32 {
        return UREncodeResult::from(URError::UrEncodeError(format!(
            "zip-32 seed fingerprint length must be 32, current is {}",
            seed_fingerprint_len
        )))
        .c_ptr();
    }
    let seed_fingerprint = extract_array!(seed_fingerprint, u8, seed_fingerprint_len);
    let seed_fingerprint = match <[u8; 32]>::try_from(seed_fingerprint) {
        Ok(seed_fingerprint) => seed_fingerprint,
        Err(e) => return UREncodeResult::from(URError::UrEncodeError(e.to_string())).c_ptr(),
    };
    unsafe {
        let keys = recover_c_array(zcash_keys);
        let ufvks: Vec<UFVKInfo> = keys
            .iter()
            .map(|v| {
                UFVKInfo::new(
                    recover_c_char(v.key_text),
                    recover_c_char(v.key_name),
                    recover_c_char(v.transparent_key_path),
                    recover_c_char(v.orchard_key_path),
                )
            })
            .collect();
        let result = generate_sync_ur(ufvks, seed_fingerprint);
        match result.map(|v| v.try_into()) {
            Ok(v) => match v {
                Ok(data) => UREncodeResult::encode(
                    data,
                    ZcashAccounts::get_registry_type().get_type(),
                    FRAGMENT_MAX_LENGTH_DEFAULT,
                )
                .c_ptr(),
                Err(e) => UREncodeResult::from(e).c_ptr(),
            },
            Err(e) => UREncodeResult::from(e).c_ptr(),
        }
    }
}
