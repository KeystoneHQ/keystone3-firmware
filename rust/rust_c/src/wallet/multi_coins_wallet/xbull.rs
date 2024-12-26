use alloc::collections::BTreeMap;
use alloc::format;
use bitcoin::bip32::DerivationPath;
use core::slice;
use core::str::FromStr;
use hex;
use ur_registry::error::URError;
use ur_registry::traits::RegistryItem;

use crate::common::errors::RustCError;
use crate::common::ffi::CSliceFFI;
use crate::common::structs::ExtendedPublicKey;
use crate::common::types::PtrT;
use crate::common::ur::{UREncodeResult, FRAGMENT_MAX_LENGTH_DEFAULT};
use crate::common::utils::{recover_c_array, recover_c_char};
use app_wallets::xbull::generate_sync_ur;
use ur_registry::extend::crypto_multi_accounts::CryptoMultiAccounts;

#[no_mangle]
pub extern "C" fn get_connect_xbull_wallet_ur(
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
        let public_keys = recover_c_array(public_keys);
        let master_fingerprint =
            bitcoin::bip32::Fingerprint::from_str(hex::encode(mfp).as_str())
                .map_err(|_e| RustCError::InvalidMasterFingerprint);
        match master_fingerprint {
            Ok(fp) => {
                let mut keys = BTreeMap::new();
                for x in public_keys {
                    let pubkey = recover_c_char(x.xpub);
                    let path = recover_c_char(x.path);
                    match DerivationPath::from_str(path.as_str()) {
                        Ok(path) => {
                            keys.insert(path, pubkey);
                        }
                        Err(_e) => {
                            return UREncodeResult::from(RustCError::InvalidHDPath).c_ptr();
                        }
                    }
                }
                let result = generate_sync_ur(fp.as_ref(), keys);
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
            Err(e) => UREncodeResult::from(e).c_ptr(),
        }
    }
}
