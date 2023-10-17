pub mod sync_info;

use crate::keplr::sync_info::SyncInfo;
use crate::DEVICE_TYPE;
use alloc::string::ToString;
use alloc::vec::Vec;
use core::str::FromStr;
use third_party::bitcoin::bip32;
use third_party::bitcoin::bip32::{ChildNumber, DerivationPath};
use third_party::ur_registry::crypto_hd_key::CryptoHDKey;
use third_party::ur_registry::crypto_key_path::{CryptoKeyPath, PathComponent};
use third_party::ur_registry::error::URResult;
use third_party::ur_registry::extend::crypto_multi_accounts::CryptoMultiAccounts;

fn get_origin(
    sync_info: &SyncInfo,
    master_fingerprint: [u8; 4],
    depth: u32,
) -> URResult<CryptoKeyPath> {
    let path = app_utils::normalize_path(&sync_info.hd_path.clone());
    let derivation_path = DerivationPath::from_str(path.as_str()).unwrap();
    let components = derivation_path
        .into_iter()
        .map(|child_number| {
            let index = match child_number {
                ChildNumber::Hardened { index } => index,
                ChildNumber::Normal { index } => index,
            };
            PathComponent::new(Some(index.clone()), child_number.is_hardened()).unwrap()
        })
        .collect::<Vec<PathComponent>>();
    Ok(CryptoKeyPath::new(
        components,
        Some(master_fingerprint.clone()),
        Some(depth),
    ))
}
pub fn generate_sync_ur(
    master_fingerprint: &[u8; 4],
    sync_infos: &Vec<SyncInfo>,
) -> URResult<CryptoMultiAccounts> {
    let mut keys: Vec<CryptoHDKey> = Vec::new();
    sync_infos.into_iter().for_each(|sync_info| {
        if let Ok(xpub) = bip32::ExtendedPubKey::from_str(sync_info.xpub.as_str()) {
            if let Ok(origin) = get_origin(sync_info, master_fingerprint.clone(), xpub.depth as u32)
            {
                let hd_key: CryptoHDKey = CryptoHDKey::new_extended_key(
                    Some(false),
                    xpub.public_key.serialize().to_vec(),
                    None,
                    None,
                    Some(origin),
                    None,
                    None,
                    Some(sync_info.name.clone()),
                    None,
                );
                keys.push(hd_key);
            }
        }
    });
    Ok(CryptoMultiAccounts::new(
        master_fingerprint.clone(),
        keys,
        Some(DEVICE_TYPE.to_string()),
        None,
        None,
    ))
}

#[cfg(test)]
mod tests {
    use super::*;
    use third_party::hex;

    #[test]
    fn test_generate_keplr_sync_ur() {
        let atom_info = SyncInfo {
            name: "ATOM-0".to_string(),
            hd_path: "m/44'/118'/0'/0/0".to_string(),
            xpub: "xpub6GWQSCxh2ug91DcDwj4zYF1JckmwpVx3EAG8SKgfw4wKHSUHqCXUecRcfhWgEwD27Sg7cFFDN45HtVAxxx2XnRdtdVNr8JLFy8YraWDYBb4".to_string(),
        };
        let sync_infos = vec![atom_info];
        let result = generate_sync_ur(&[0x73, 0xC5, 0xDA, 0x0A], &sync_infos).unwrap();
        assert_eq!(
            "024F4E2AD99C34D60B9BA6283C9431A8418AF8673212961F97A77B6377FCD05B62",
            hex::encode(result.get_keys()[0].get_key()).to_uppercase()
        )
    }
}
