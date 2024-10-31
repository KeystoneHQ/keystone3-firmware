use crate::utils::generate_crypto_multi_accounts_sync_ur;
use alloc::collections::BTreeMap;
use alloc::string::String;
use bitcoin::bip32::DerivationPath;
use ur_registry::error::URResult;
use ur_registry::extend::crypto_multi_accounts::CryptoMultiAccounts;

pub const DEVICE_TYPE: &str = "keystone 3";

pub fn generate_sync_ur(
    master_fingerprint: &[u8; 4],
    public_keys: BTreeMap<DerivationPath, String>,
) -> URResult<CryptoMultiAccounts> {
    generate_crypto_multi_accounts_sync_ur(master_fingerprint, public_keys, &"APT")
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_generate_sync_ur() {
        let master_fingerprint = [0u8; 4];
        let public_keys = BTreeMap::new();
        let result = generate_sync_ur(&master_fingerprint, public_keys);
        assert!(result.is_ok());
        assert_eq!(result.unwrap().get_master_fingerprint(), master_fingerprint);
    }
}
