use alloc::{format, string::ToString};
use common_rust_c::extract_array;
use common_rust_c::ffi::CSliceFFI;
use common_rust_c::structs::ExtendedPublicKey;
use common_rust_c::types::{Ptr, PtrBytes, PtrString};
use common_rust_c::ur::{UREncodeResult, FRAGMENT_MAX_LENGTH_DEFAULT};
use common_rust_c::utils::{recover_c_array, recover_c_char};
use third_party::ur_registry::{
    error::URError, extend::crypto_multi_accounts::CryptoMultiAccounts, traits::RegistryItem,
};

use super::utils::normalize_xpub;

#[no_mangle]
pub extern "C" fn get_backpack_wallet_ur(
    master_fingerprint: PtrBytes,
    master_fingerprint_length: u32,
    public_keys: Ptr<CSliceFFI<ExtendedPublicKey>>,
) -> Ptr<UREncodeResult> {
    if master_fingerprint_length != 4 {
        return UREncodeResult::from(URError::UrEncodeError(format!(
            "master fingerprint length must be 4, current is {}",
            master_fingerprint_length
        )))
        .c_ptr();
    }
    let mfp = extract_array!(master_fingerprint, u8, master_fingerprint_length);
    let mfp = match <[u8; 4]>::try_from(mfp) {
        Ok(mfp) => mfp,
        Err(e) => return UREncodeResult::from(URError::UrEncodeError(e.to_string())).c_ptr(),
    };
    unsafe {
        let keys = recover_c_array(public_keys);
        match normalize_xpub(keys) {
            Ok(_keys) => match app_wallets::backpack::generate_crypto_multi_accounts(mfp, _keys) {
                Ok(data) => match data.try_into() {
                    Ok(_v) => UREncodeResult::encode(
                        _v,
                        CryptoMultiAccounts::get_registry_type().get_type(),
                        FRAGMENT_MAX_LENGTH_DEFAULT.clone(),
                    )
                    .c_ptr(),
                    Err(_e) => UREncodeResult::from(_e).c_ptr(),
                },
                Err(_e) => UREncodeResult::from(_e).c_ptr(),
            },
            Err(_e) => UREncodeResult::from(_e).c_ptr(),
        }
    }
}
