#![no_std]
extern crate alloc;
use alloc::format;
use alloc::string::ToString;
use common_rust_c::ffi::CSliceFFI;
use common_rust_c::structs::ExtendedPublicKey;
use common_rust_c::types::PtrT;
use common_rust_c::ur::{UREncodeResult, FRAGMENT_MAX_LENGTH_DEFAULT};
use common_rust_c::utils::{recover_c_array, recover_c_char};
use core::slice;
use third_party::ur_registry::crypto_account::CryptoAccount;
use third_party::ur_registry::error::URError;
use third_party::ur_registry::traits::RegistryItem;

#[no_mangle]
pub extern "C" fn get_connect_thor_wallet_ur(
    master_fingerprint: *mut u8,
    length: u32,
    public_keys: PtrT<CSliceFFI<ExtendedPublicKey>>,
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
        let key1 = keys.get(0);
        let key2 = keys.get(1);
        let key3 = keys.get(2);
        let key4 = keys.get(3);
        let key5 = keys.get(4);
        return if let (Some(k1), Some(k2), Some(k3), Some(k4), Some(k5)) =
            (key1, key2, key3, key4, key5)
        {
            let k1_x_pub = recover_c_char(k1.xpub);
            let k1_x_pub_path = recover_c_char(k1.path);
            let k2_x_pub = recover_c_char(k2.xpub);
            let k2_x_pub_path = recover_c_char(k2.path);
            let k3_x_pub = recover_c_char(k3.xpub);
            let k3_x_pub_path = recover_c_char(k3.path);
            let k4_x_pub = recover_c_char(k4.xpub);
            let k4_x_pub_path = recover_c_char(k4.path);
            let k5_x_pub = recover_c_char(k5.xpub);
            let k5_x_pub_path = recover_c_char(k5.path);
            let extended_public_keys = [
                k1_x_pub.trim(),
                k2_x_pub.trim(),
                k3_x_pub.trim(),
                k4_x_pub.trim(),
                k5_x_pub.trim(),
            ];
            let extend_public_key_paths = [
                k1_x_pub_path.trim(),
                k2_x_pub_path.trim(),
                k3_x_pub_path.trim(),
                k4_x_pub_path.trim(),
                k5_x_pub_path.trim(),
            ];
            let mfp = match <&[u8; 4]>::try_from(mfp) {
                Ok(mfp) => mfp,
                Err(e) => {
                    return UREncodeResult::from(URError::UrEncodeError(e.to_string())).c_ptr();
                }
            };

            let result = app_wallets::blue_wallet::generate_crypto_account(
                mfp,
                &extended_public_keys,
                &extend_public_key_paths,
            );
            match result.map(|v| v.try_into()) {
                Ok(v) => match v {
                    Ok(data) => UREncodeResult::encode(
                        data,
                        CryptoAccount::get_registry_type().get_type(),
                        FRAGMENT_MAX_LENGTH_DEFAULT.clone(),
                    )
                    .c_ptr(),
                    Err(e) => UREncodeResult::from(e).c_ptr(),
                },
                Err(e) => UREncodeResult::from(e).c_ptr(),
            }
        } else {
            UREncodeResult::from(URError::UrEncodeError(format!("getting key error"))).c_ptr()
        };
    }
}
