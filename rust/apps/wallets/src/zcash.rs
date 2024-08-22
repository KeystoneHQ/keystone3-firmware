use alloc::string::{String, ToString};
use alloc::vec;
use alloc::vec::Vec;

use app_utils::impl_public_struct;
use keystore::algorithms::zcash::vendor::{
    zcash_keys::keys::UnifiedFullViewingKey, zcash_protocol::consensus::MainNetwork,
};
use third_party::ur_registry::{
    crypto_hd_key::CryptoHDKey,
    crypto_key_path::CryptoKeyPath,
    error::{URError, URResult},
    zcash::{
        zcash_accounts::ZcashAccounts, zcash_full_viewing_key::ZcashFullViewingKey,
        zcash_unified_full_viewing_key::ZcashUnifiedFullViewingKey,
    },
};

impl_public_struct!(UFVKInfo {
    key_text: String,
    key_name: String,
    transparent_key_path: String,
    orchard_key_path: String
});

pub fn generate_sync_ur(key_infos: Vec<UFVKInfo>, seed_fingerprint: [u8; 32]) -> URResult<ZcashAccounts> {
    let keys = key_infos
        .iter()
        .map(|info| {
            let ufvk = UnifiedFullViewingKey::decode(&MainNetwork, &info.key_text)
                .map_err(|e| URError::UrEncodeError(e.to_string()))?;

            let transprant = ufvk.transparent().and_then(|v| Some(v.serialize()));
            let orchard = ufvk
                .orchard()
                .and_then(|v| Some(v.to_bytes()))
                .ok_or(URError::UrEncodeError(format!("Zcash missing orchard fvk")))?;

            let transparent_key = transprant
                .map(|v| {
                    let (chaincode, pubkey) = v.split_at(32);
                    let keypath = CryptoKeyPath::from_path(info.transparent_key_path.clone(), None)
                        .map_err(|e| URError::UrEncodeError(e))?;
                    Ok(CryptoHDKey::new_extended_key(
                        None,
                        pubkey.to_vec(),
                        Some(chaincode.to_vec()),
                        None,
                        Some(keypath),
                        None,
                        None,
                        None,
                        None,
                    ))
                })
                .transpose()?;

            let keypath = CryptoKeyPath::from_path(info.orchard_key_path.clone(), None)
                .map_err(|e| URError::UrEncodeError(e))?;

            let orchard_key = ZcashFullViewingKey::new(keypath, orchard.to_vec());

            Ok(ZcashUnifiedFullViewingKey::new(
                transparent_key,
                orchard_key,
                Some(info.key_name.clone()),
            ))
        })
        .collect::<URResult<Vec<ZcashUnifiedFullViewingKey>>>()?;
    let accounts = ZcashAccounts::new(seed_fingerprint.to_vec(), keys);
    Ok(accounts)
}
