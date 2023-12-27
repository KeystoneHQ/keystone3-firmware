use crate::utils::generate_crypto_multi_accounts_sync_ur;
use alloc::collections::BTreeMap;
use alloc::string::String;
use third_party::bitcoin::bip32::DerivationPath;
use third_party::ur_registry::error::URResult;
use third_party::ur_registry::extend::crypto_multi_accounts::CryptoMultiAccounts;

pub const DEVICE_TYPE: &str = "keystone 3";

pub fn generate_sync_ur(
    master_fingerprint: &[u8; 4],
    public_keys: BTreeMap<DerivationPath, String>,
) -> URResult<CryptoMultiAccounts> {
    generate_crypto_multi_accounts_sync_ur(master_fingerprint, public_keys, &"NEAR")
}
