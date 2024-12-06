use alloc::string::{String, ToString};
use alloc::vec;
use alloc::vec::Vec;

use app_utils::impl_public_struct;
use ur_registry::{
    crypto_hd_key::CryptoHDKey,
    crypto_key_path::CryptoKeyPath,
    error::{URError, URResult},
    zcash::{
        zcash_accounts::ZcashAccounts, zcash_unified_full_viewing_key::ZcashUnifiedFullViewingKey,
    },
};

impl_public_struct!(UFVKInfo {
    key_text: String,
    key_name: String,
    index: u32
});

pub fn generate_sync_ur(
    key_infos: Vec<UFVKInfo>,
    seed_fingerprint: [u8; 32],
) -> URResult<ZcashAccounts> {
    let keys = key_infos
        .iter()
        .map(|info| {
            Ok(ZcashUnifiedFullViewingKey::new(
                info.key_text.clone(),
                info.index,
                Some(info.key_name.clone()),
            ))
        })
        .collect::<URResult<Vec<ZcashUnifiedFullViewingKey>>>()?;
    let accounts = ZcashAccounts::new(seed_fingerprint.to_vec(), keys);
    Ok(accounts)
}
