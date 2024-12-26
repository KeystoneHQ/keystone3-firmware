use alloc::format;
use alloc::string::ToString;
use core::slice;

use app_wallets::thor_wallet::generate_crypto_multi_accounts;
use ur_registry::error::URError;
use ur_registry::extend::crypto_multi_accounts::CryptoMultiAccounts;
use ur_registry::traits::RegistryItem;

use crate::common::ffi::CSliceFFI;
use crate::common::structs::ExtendedPublicKey;
use crate::common::types::{PtrString, PtrT};
use crate::common::ur::{UREncodeResult, FRAGMENT_MAX_LENGTH_DEFAULT};
use crate::common::utils::{recover_c_array, recover_c_char};

use super::utils::normalize_xpub;

#[no_mangle]
pub extern "C" fn get_connect_thor_wallet_ur(
    master_fingerprint: *mut u8,
    length: u32,
    serial_number: PtrString,
    public_keys: PtrT<CSliceFFI<ExtendedPublicKey>>,
    device_type: PtrString,
    device_version: PtrString,
) -> *mut UREncodeResult {
    if length != 4 {
        return UREncodeResult::from(URError::UrEncodeError(format!(
            "master fingerprint length must be 4, current is {}",
            length
        )))
        .c_ptr();
    }
    unsafe {
        let mfp = slice::from_raw_parts(master_fingerprint, length as usize);
        let keys = recover_c_array(public_keys);
        let serial_number = recover_c_char(serial_number);
        let device_type = recover_c_char(device_type);
        let device_version = recover_c_char(device_version);
        let key1 = keys.first();
        let key2 = keys.get(1);
        let key3 = keys.get(2);
        let key4 = keys.get(3);
        let key5 = keys.get(4);
        if let (Some(_k1), Some(_k2), Some(_k3), Some(_k4), Some(_k5)) =
            (key1, key2, key3, key4, key5)
        {
            let mfp = match <&[u8; 4]>::try_from(mfp) {
                Ok(mfp) => mfp,
                Err(e) => {
                    return UREncodeResult::from(URError::UrEncodeError(e.to_string())).c_ptr();
                }
            };

            let keys = normalize_xpub(keys).unwrap();
            let result = generate_crypto_multi_accounts(
                *mfp,
                &serial_number,
                keys,
                &device_type,
                &device_version,
            );
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
        } else {
            UREncodeResult::from(URError::UrEncodeError("getting key error".to_string())).c_ptr()
        }
    }
}
