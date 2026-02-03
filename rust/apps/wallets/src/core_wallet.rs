use alloc::{
    string::{String, ToString},
    vec::Vec,
};
use {
    bitcoin::bip32::ChildNumber,
    ur_registry::{
        crypto_hd_key::CryptoHDKey,
        crypto_key_path::{CryptoKeyPath, PathComponent},
        error::{URError, URResult},
        extend::crypto_multi_accounts::CryptoMultiAccounts,
    },
};

use crate::{common::get_path_component, ExtendedPublicKey};

const AVAX_STANDARD_PREFIX: &str = "44'/60'/0'";

fn is_avax_x_p_path(path: &[ChildNumber]) -> bool {
    if path.len() < 3 {
        return false;
    }
    matches!(
        (&path[0], &path[1], &path[2]),
        (
            ChildNumber::Hardened { index: i0 },
            ChildNumber::Hardened { index: i1 },
            ChildNumber::Hardened { index: i2 }
        ) if *i0 == 44 && *i1 == 9000 && *i2 <= 9
    )
}

pub fn generate_crypto_multi_accounts(
    master_fingerprint: [u8; 4],
    extended_public_keys: Vec<ExtendedPublicKey>,
    device_type: &str,
) -> URResult<CryptoMultiAccounts> {
    let mut keys = vec![];
    for ele in extended_public_keys {
        match ele.get_path() {
            _path if _path.to_string().to_lowercase().eq(AVAX_STANDARD_PREFIX) => {
                keys.push(generate_k1_normal_key(
                    master_fingerprint,
                    ele.clone(),
                    Some("account.standard".to_string()),
                )?);
            }
            _path if is_avax_x_p_path(_path.as_ref()) =>
            {
                keys.push(generate_k1_normal_key(
                    master_fingerprint,
                    ele,
                    Some("account.x&p".to_string()),
                )?);
            }
            _ => {
                return Err(URError::UrEncodeError(format!(
                    "Unknown key path: {}",
                    ele.path
                )))
            }
        }
    }

    Ok(CryptoMultiAccounts::new(
        master_fingerprint,
        keys,
        Some(device_type.to_string()),
        None,
        None,
    ))
}

fn generate_k1_normal_key(
    mfp: [u8; 4],
    key: ExtendedPublicKey,
    note: Option<String>,
) -> URResult<CryptoHDKey> {
    let xpub = bitcoin::bip32::Xpub::decode(&key.get_key())
        .map_err(|_e| URError::UrEncodeError(_e.to_string()))?;
    let path = key.get_path();
    let key_path = CryptoKeyPath::new(
        path.into_iter()
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
        xpub.public_key.serialize().to_vec(),
        Some(xpub.chain_code.to_bytes().to_vec()),
        None,
        Some(key_path),
        None,
        Some(xpub.parent_fingerprint.to_bytes()),
        Some("Keystone".to_string()),
        note,
    ))
}
