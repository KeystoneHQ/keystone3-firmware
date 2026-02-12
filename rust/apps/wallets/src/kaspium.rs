use bitcoin::bip32::{ChildNumber, Xpub};
use ur_registry::{
    crypto_coin_info::{CoinType, CryptoCoinInfo, Network},
    crypto_hd_key::CryptoHDKey,
    crypto_key_path::{CryptoKeyPath, PathComponent},
    error::{URError, URResult}, extend::crypto_multi_accounts::CryptoMultiAccounts,
};
use crate::{common::get_path_component, ExtendedPublicKey};
use alloc::{
    string::{String, ToString},
    vec::Vec,
};

pub fn generate_crypto_multi_accounts(
    mfp: [u8; 4],
    keys: Vec<ExtendedPublicKey>,
) -> URResult<CryptoMultiAccounts> {
    let mut crypto_keys = vec![];
    for key in keys {
        crypto_keys.push(generate_kaspa_key(mfp, key, None)?);
    }
    Ok(CryptoMultiAccounts::new(mfp, crypto_keys, None, None, None))
}

pub fn generate_kaspa_key(
    mfp: [u8; 4],
    key: ExtendedPublicKey,
    note: Option<String>,
) -> URResult<CryptoHDKey> {
    let xpub = Xpub::decode(&key.key)
        .map_err(|_e| URError::UrEncodeError(_e.to_string()))?;
    
    let key_path = CryptoKeyPath::new(
        key.path.into_iter()
            .map(|v| match v {
                ChildNumber::Normal { index } => get_path_component(Some(*index), false),
                ChildNumber::Hardened { index } => get_path_component(Some(*index), true),
            })
            .collect::<URResult<Vec<PathComponent>>>()?,
        Some(mfp),
        Some(xpub.depth as u32),
    );

    Ok(CryptoHDKey::new_extended_key(
        Some(false),
        xpub.public_key.serialize().to_vec(), // pub key (33 bytes)
        Some(xpub.chain_code.to_bytes().to_vec()),
        None, 
        Some(key_path),
        None,
        Some(xpub.parent_fingerprint.to_bytes()),
        Some("Keystone".to_string()),
        note,
    ))
}