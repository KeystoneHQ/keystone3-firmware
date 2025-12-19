use alloc::string::String;

use alloc::vec::Vec;

use app_utils::impl_public_struct;
use ur_registry::{
    error::URResult,
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

#[cfg(test)]
mod tests {
    use super::*;
    use alloc::vec;
    use alloc::string::ToString;

    #[test]
    fn test_generate_sync_ur() {
        let seed_fingerprint = [1u8; 32];
        let key_infos = vec![
            UFVKInfo {
                key_text: "uview1vmle95235860km865468566554".to_string(),
                key_name: "Account 0".to_string(),
                index: 0,
            },
            UFVKInfo {
                key_text: "uview1vmle95235860km865468566555".to_string(),
                key_name: "Account 1".to_string(),
                index: 1,
            },
        ];

        let result = generate_sync_ur(key_infos, seed_fingerprint);
        assert!(result.is_ok());

        let accounts = result.unwrap();
        let cbor: Vec<u8> = accounts.try_into().unwrap();
        assert!(!cbor.is_empty());
    }
}

