use alloc::format;
use alloc::string::{String, ToString};
use alloc::vec;
use alloc::vec::Vec;
use app_utils::normalize_path;

use crate::common::errors::RustCError;
use crate::common::ffi::CSliceFFI;
use crate::common::structs::ExtendedPublicKey;
use crate::common::types::{Ptr, PtrBytes, PtrString, PtrT};
use crate::common::ur::{UREncodeResult, FRAGMENT_MAX_LENGTH_DEFAULT};
use crate::common::utils::{recover_c_array, recover_c_char};
use crate::extract_array;
use bitcoin::bip32::{DerivationPath, Xpub};
use core::slice;
use core::str::FromStr;
use cty::{int32_t, uint32_t};
use hex;
use ur_registry::bytes::Bytes;
use ur_registry::crypto_account::CryptoAccount;
use ur_registry::error::URError;
use ur_registry::extend::crypto_multi_accounts::CryptoMultiAccounts;
use ur_registry::traits::RegistryItem;

#[no_mangle]
pub unsafe extern "C" fn generate_btc_crypto_account_ur(
    master_fingerprint: *mut u8,
    length: u32,
    public_keys: PtrT<CSliceFFI<ExtendedPublicKey>>,
) -> *mut UREncodeResult {
    if length != 4 {
        return UREncodeResult::from(URError::UrEncodeError(format!(
            "master fingerprint length must be 4, current is {length}"
        )))
        .c_ptr();
    }
    let mfp = extract_array!(master_fingerprint, u8, length as usize);
    let keys = recover_c_array(public_keys);

    let extended_public_keys_str: Vec<String> = keys
        .iter()
        .map(|k| recover_c_char(k.xpub).trim().to_string())
        .collect();
    let extend_public_key_paths_str: Vec<String> = keys
        .iter()
        .map(|k| recover_c_char(k.path).trim().to_string())
        .collect();

    let mfp = match <&[u8; 4]>::try_from(mfp) {
        Ok(mfp) => mfp,
        Err(e) => {
            return UREncodeResult::from(URError::UrEncodeError(e.to_string())).c_ptr();
        }
    };

    let extended_public_keys: Vec<&str> = extended_public_keys_str
        .iter()
        .map(|s| s.as_str())
        .collect();
    let extend_public_key_paths: Vec<&str> = extend_public_key_paths_str
        .iter()
        .map(|s| s.as_str())
        .collect();

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
                FRAGMENT_MAX_LENGTH_DEFAULT,
            )
            .c_ptr(),
            Err(e) => UREncodeResult::from(e).c_ptr(),
        },
        Err(e) => UREncodeResult::from(e).c_ptr(),
    }
}

unsafe fn normalize_xpub(
    keys: &[ExtendedPublicKey],
) -> Result<Vec<app_wallets::ExtendedPublicKey>, RustCError> {
    let mut result = vec![];
    for ele in keys {
        let xpub = recover_c_char(ele.xpub);
        let path = recover_c_char(ele.path);
        let path = normalize_path(&path);
        let derivation_path =
            DerivationPath::from_str(&path).map_err(|_e| RustCError::InvalidHDPath)?;
        let key = match xpub.len() {
            //32 bytes ed25519 public key or 64 bytes bip32-ed25519 xpub;
            64 | 128 => hex::decode(&xpub).map_err(|_e| RustCError::InvalidXPub)?,
            _ => Xpub::from_str(&xpub)
                .map_err(|_e| RustCError::InvalidXPub)?
                .encode()
                .to_vec(),
        };
        result.push(app_wallets::ExtendedPublicKey::new(derivation_path, key));
    }
    Ok(result)
}
