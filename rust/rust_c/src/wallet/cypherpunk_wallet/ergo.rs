use crate::common::ffi::CSliceFFI;
use crate::common::structs::ExtendedPublicKey;
use crate::common::types::{Ptr, PtrBytes, PtrString};
use crate::common::ur::{UREncodeResult, FRAGMENT_MAX_LENGTH_DEFAULT};
use crate::common::utils::{recover_c_array, recover_c_char};
use crate::extract_array;
use alloc::{format, string::ToString};
use ur_registry::{
    error::URError, extend::crypto_multi_accounts::CryptoMultiAccounts, traits::RegistryItem,
};

use crate::wallet::normalize_xpub;

#[no_mangle]
pub extern "C" fn generate_ergo_wallet_ur(
    master_fingerprint: PtrBytes,
    master_fingerprint_length: u32,
    serial_number: PtrString,
    public_keys: Ptr<CSliceFFI<ExtendedPublicKey>>,
    device_type: PtrString,
    device_version: PtrString,
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
        let serial_number = recover_c_char(serial_number);
        let device_version = recover_c_char(device_version);
        let device_type = recover_c_char(device_type);
        match normalize_xpub(keys) {
            Ok(_keys) => {
                match app_wallets::ergo::generate_general_multi_accounts_ur(
                    mfp,
                    &serial_number,
                    _keys,
                    &device_type,
                    &device_version,
                ) {
                    Ok(data) => match data.try_into() {
                        Ok(_v) => UREncodeResult::encode(
                            _v,
                            CryptoMultiAccounts::get_registry_type().get_type(),
                            FRAGMENT_MAX_LENGTH_DEFAULT,
                        )
                        .c_ptr(),
                        Err(_e) => UREncodeResult::from(_e).c_ptr(),
                    },
                    Err(_e) => UREncodeResult::from(_e).c_ptr(),
                }
            }
            Err(_e) => UREncodeResult::from(_e).c_ptr(),
        }
    }
}
