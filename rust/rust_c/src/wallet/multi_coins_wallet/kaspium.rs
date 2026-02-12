use crate::common::ffi::CSliceFFI;
use crate::common::structs::ExtendedPublicKey;
use crate::common::types::{Ptr, PtrBytes};
use crate::common::ur::{UREncodeResult, FRAGMENT_MAX_LENGTH_DEFAULT};
use crate::common::utils::recover_c_array;
use crate::extract_array;
use alloc::{format, string::ToString};
use ur_registry::{
    error::URError, extend::crypto_multi_accounts::CryptoMultiAccounts, traits::RegistryItem,
};

use super::utils::normalize_xpub;

#[no_mangle]
pub unsafe extern "C" fn get_connect_kaspa_ur(
    master_fingerprint: PtrBytes,
    master_fingerprint_length: u32,
    public_keys: Ptr<CSliceFFI<ExtendedPublicKey>>,
) -> Ptr<UREncodeResult> {
    if master_fingerprint_length != 4 {
        return UREncodeResult::from(URError::UrEncodeError(format!(
            "master fingerprint length must be 4, current is {master_fingerprint_length}"
        )))
        .c_ptr();
    }
    let mfp = extract_array!(master_fingerprint, u8, master_fingerprint_length);
    let mfp = match <[u8; 4]>::try_from(mfp) {
        Ok(mfp) => mfp,
        Err(e) => return UREncodeResult::from(URError::UrEncodeError(e.to_string())).c_ptr(),
    };

    let keys = recover_c_array(public_keys);
    match normalize_xpub(keys) {
        Ok(_keys) => match app_wallets::kaspium::generate_crypto_multi_accounts(mfp, _keys) {
            Ok(data) => match data.try_into() {
                Ok(_v) => UREncodeResult::encode(
                    _v,
                    ur_registry::extend::crypto_multi_accounts::CryptoMultiAccounts::get_registry_type().get_type(),
                    FRAGMENT_MAX_LENGTH_DEFAULT,
                )
                .c_ptr(),
                Err(_e) => UREncodeResult::from(_e).c_ptr(),
            },
            Err(_e) => UREncodeResult::from(_e).c_ptr(),
        },
        Err(_e) => UREncodeResult::from(_e).c_ptr(),
    }
}