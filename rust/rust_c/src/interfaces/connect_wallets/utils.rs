use core::str::FromStr;

use alloc::vec;
use alloc::vec::Vec;
use app_utils::normalize_path;
use third_party::{
    bitcoin::bip32::{DerivationPath, ExtendedPubKey},
    hex,
};

use crate::interfaces::{errors::RustCError, structs::ExtendedPublicKey, utils::recover_c_char};

pub fn normalize_xpub(
    keys: &[ExtendedPublicKey],
) -> Result<Vec<app_wallets::ExtendedPublicKey>, RustCError> {
    unsafe {
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
                _ => ExtendedPubKey::from_str(&xpub)
                    .map_err(|_e| RustCError::InvalidXPub)?
                    .encode()
                    .to_vec(),
            };
            result.push(app_wallets::ExtendedPublicKey::new(derivation_path, key));
        }
        Ok(result)
    }
}
