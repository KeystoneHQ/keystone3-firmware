use alloc::collections::BTreeMap;
use alloc::string::{String, ToString};
use alloc::vec::Vec;
use core::str::FromStr;
use third_party::bitcoin::bip32::{self, DerivationPath};
use third_party::ur_registry::crypto_hd_key::CryptoHDKey;
use third_party::ur_registry::error::{URError, URResult};
use third_party::ur_registry::extend::crypto_multi_accounts::CryptoMultiAccounts;

use crate::utils::get_origin;
use crate::DEVICE_TYPE;

pub fn generate_sync_ur(
    master_fingerprint: &[u8; 4],
    public_keys: BTreeMap<DerivationPath, String>,
) -> URResult<CryptoMultiAccounts> {
    let mut keys: Vec<CryptoHDKey> = Vec::new();
    for (index, (path, pubkey)) in public_keys.into_iter().enumerate() {
        let depth = path.len();
        let bip32_extended_pub_key = match bip32::Xpub::from_str(&pubkey) {
            Ok(k) => k,
            Err(e) => return Err(URError::UrEncodeError(e.to_string())),
        };
        if let Ok(origin) = get_origin(master_fingerprint, depth, path) {
            let hd_key: CryptoHDKey = CryptoHDKey::new_extended_key(
                Some(false),
                bip32_extended_pub_key.public_key.serialize().to_vec(),
                Some(bip32_extended_pub_key.chain_code[..].to_vec()),
                None,
                Some(origin),
                None,
                Some(bip32_extended_pub_key.parent_fingerprint.to_bytes()),
                Some(format!("BTC-{}", index)),
                None,
            );
            keys.push(hd_key);
        }
    }
    Ok(CryptoMultiAccounts::new(
        master_fingerprint.clone(),
        keys,
        Some(DEVICE_TYPE.to_string()),
        None,
        None,
    ))
}
