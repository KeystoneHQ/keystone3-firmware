use core::str::FromStr;

use alloc::{
    string::{String, ToString},
    vec::Vec,
};
use third_party::{
    bitcoin::bip32::{ChildNumber, DerivationPath},
    secp256k1::Secp256k1,
    ur_registry::{
        crypto_hd_key::CryptoHDKey,
        crypto_key_path::{CryptoKeyPath, PathComponent},
        error::{URError, URResult},
        extend::crypto_multi_accounts::CryptoMultiAccounts,
    },
};

use crate::{common::get_path_component, ExtendedPublicKey};

const ETH_STANDARD_PREFIX: &str = "m/44'/60'/0'";
const ETH_LEDGER_LIVE_PREFIX: &str = "m/44'/60'"; //overlap with ETH_STANDARD at 0
const SOL_PREFIX: &str = "m/44'/501'";

pub fn generate_crypto_multi_accounts(
    master_fingerprint: [u8; 4],
    extended_public_keys: Vec<ExtendedPublicKey>,
) -> URResult<CryptoMultiAccounts> {
    let mut keys = vec![];

    for ele in extended_public_keys {
        match ele.get_path() {
            _path if _path.to_string().to_lowercase().starts_with(SOL_PREFIX) => {
                keys.push(generate_ed25519_key(master_fingerprint, ele.clone(), None)?);
            }
            _path if _path.to_string().to_lowercase().eq(ETH_STANDARD_PREFIX) => {
                keys.push(generate_k1_normal_key(
                    master_fingerprint,
                    ele.clone(),
                    Some("account.standard".to_string()),
                )?);
                keys.push(generate_eth_ledger_live_key(
                    master_fingerprint,
                    ele,
                    Some("account.ledger_live".to_string()),
                )?);
            }
            _path
                if _path
                    .to_string()
                    .to_lowercase()
                    .starts_with(ETH_LEDGER_LIVE_PREFIX) =>
            {
                keys.push(generate_eth_ledger_live_key(
                    master_fingerprint,
                    ele,
                    Some("account.ledger_live".to_string()),
                )?);
            }
            _ => {
                return Err(URError::UrEncodeError(format!(
                    "Unknown key path: {}",
                    ele.path.to_string()
                )))
            }
        }
    }

    Ok(CryptoMultiAccounts::new(
        master_fingerprint,
        keys,
        None,
        None,
        None,
    ))
}

fn generate_ed25519_key(
    mfp: [u8; 4],
    key: ExtendedPublicKey,
    note: Option<String>,
) -> URResult<CryptoHDKey> {
    let path = key.get_path();
    let key_path = CryptoKeyPath::new(
        path.into_iter()
            .map(|v| match v {
                ChildNumber::Normal { index } => get_path_component(Some(index.clone()), false),
                ChildNumber::Hardened { index } => get_path_component(Some(index.clone()), true),
            })
            .collect::<URResult<Vec<PathComponent>>>()?,
        Some(mfp),
        None,
    );
    Ok(CryptoHDKey::new_extended_key(
        Some(false),
        key.get_key(),
        None,
        None,
        Some(key_path),
        None,
        None,
        Some("Keystone".to_string()),
        note,
    ))
}

fn generate_k1_normal_key(
    mfp: [u8; 4],
    key: ExtendedPublicKey,
    note: Option<String>,
) -> URResult<CryptoHDKey> {
    let xpub = third_party::bitcoin::bip32::Xpub::decode(&key.get_key())
        .map_err(|_e| URError::UrEncodeError(_e.to_string()))?;
    let path = key.get_path();
    let key_path = CryptoKeyPath::new(
        path.into_iter()
            .map(|v| match v {
                ChildNumber::Normal { index } => get_path_component(Some(index.clone()), false),
                ChildNumber::Hardened { index } => get_path_component(Some(index.clone()), true),
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

fn generate_eth_ledger_live_key(
    mfp: [u8; 4],
    key: ExtendedPublicKey,
    note: Option<String>,
) -> URResult<CryptoHDKey> {
    let xpub = third_party::bitcoin::bip32::Xpub::decode(&key.get_key())
        .map_err(|_e| URError::UrEncodeError(_e.to_string()))?;
    let path = key.get_path();
    let sub_path =
        DerivationPath::from_str("m/0/0").map_err(|_e| URError::UrEncodeError(_e.to_string()))?;
    let _target_key = xpub
        .derive_pub(&Secp256k1::new(), &sub_path)
        .map_err(|_e| URError::UrEncodeError(_e.to_string()))?;
    let target_path = path
        .child(ChildNumber::Normal { index: 0 })
        .child(ChildNumber::Normal { index: 0 });
    let key_path = CryptoKeyPath::new(
        target_path
            .into_iter()
            .map(|v| match v {
                ChildNumber::Normal { index } => get_path_component(Some(index.clone()), false),
                ChildNumber::Hardened { index } => get_path_component(Some(index.clone()), true),
            })
            .collect::<URResult<Vec<PathComponent>>>()?,
        Some(mfp),
        Some(xpub.depth as u32),
    );
    Ok(CryptoHDKey::new_extended_key(
        Some(false),
        _target_key.public_key.serialize().to_vec(),
        None,
        None,
        Some(key_path),
        None,
        Some(_target_key.parent_fingerprint.to_bytes()),
        Some("Keystone".to_string()),
        note,
    ))
}

#[cfg(test)]
mod tests {
    extern crate std;

    use crate::backpack::generate_crypto_multi_accounts;
    use alloc::vec::Vec;
    use core::str::FromStr;

    use crate::ExtendedPublicKey;
    use third_party::bitcoin::bip32::{DerivationPath, Xpub};
    use third_party::hex;
    use third_party::hex::FromHex;

    #[test]
    fn test_generate_crypto_multi_accounts() {
        let mfp = "73C5DA0A";
        let mfp = Vec::from_hex(mfp).unwrap();
        let mfp: [u8; 4] = mfp.try_into().unwrap();

        let sol_pub_1 = "e79ecfa6a398b265da28a2fdad84cf1064a8e2c29300eeb8d7fb9f6977029742";
        let sol_pub_2 = "add0e011624cc6b3fded0c0bc6591b8338ff723a8829e353469df4f27342a831";
        let eth_bip44_standard_xpub = "xpub6DCoCpSuQZB2jawqnGMEPS63ePKWkwWPH4TU45Q7LPXWuNd8TMtVxRrgjtEshuqpK3mdhaWHPFsBngh5GFZaM6si3yZdUsT8ddYM3PwnATt";
        let eth_ledger_live_xpub_1 = "xpub6DCoCpSuQZB2k9PnGSMK9tinTK8kx3hcv7F4BWwhs5N2wnwGiLg17r9J7j2JcYP9gkip3sC87J1F99YxeBHGuFMg6ejA8qQEKSuzzaKvqBR";

        let sol_pub_1_path = "m/44'/501'/0'";
        let sol_pub_2_path = "m/44'/501'/1'";
        let eth_bip44_standard_xpub_path = "m/44'/60'/0'";
        let eth_ledger_live_xpub_1_path = "m/44'/60'/1'";

        let account = generate_crypto_multi_accounts(
            mfp,
            vec![
                ExtendedPublicKey::new(
                    DerivationPath::from_str(&sol_pub_1_path).unwrap(),
                    hex::decode(&sol_pub_1).unwrap(),
                ),
                ExtendedPublicKey::new(
                    DerivationPath::from_str(&sol_pub_2_path).unwrap(),
                    hex::decode(&sol_pub_2).unwrap(),
                ),
                ExtendedPublicKey::new(
                    DerivationPath::from_str(&eth_bip44_standard_xpub_path).unwrap(),
                    Xpub::from_str(&eth_bip44_standard_xpub)
                        .unwrap()
                        .encode()
                        .to_vec(),
                ),
                ExtendedPublicKey::new(
                    DerivationPath::from_str(&eth_ledger_live_xpub_1_path).unwrap(),
                    Xpub::from_str(&eth_ledger_live_xpub_1)
                        .unwrap()
                        .encode()
                        .to_vec(),
                ),
            ],
        )
        .unwrap();
        let cbor: Vec<u8> = account.try_into().unwrap();

        assert_eq!("a2011a73c5da0a0285d9012fa402f4035820e79ecfa6a398b265da28a2fdad84cf1064a8e2c29300eeb8d7fb9f697702974206d90130a20186182cf51901f5f500f5021a73c5da0a09684b657973746f6e65d9012fa402f4035820add0e011624cc6b3fded0c0bc6591b8338ff723a8829e353469df4f27342a83106d90130a20186182cf51901f5f501f5021a73c5da0a09684b657973746f6e65d9012fa702f403582102eae4b876a8696134b868f88cc2f51f715f2dbedb7446b8e6edf3d4541c4eb67b045820d882718b7a42806803eeb17f7483f20620611adb88fc943c898dc5aba94c281906d90130a30186182cf5183cf500f5021a73c5da0a0303081ad32e450809684b657973746f6e650a706163636f756e742e7374616e64617264d9012fa602f40358210237b0bb7a8288d38ed49a524b5dc98cff3eb5ca824c9f9dc0dfdb3d9cd600f29906d90130a3018a182cf5183cf500f500f400f4021a73c5da0a0303081ae438961409684b657973746f6e650a736163636f756e742e6c65646765725f6c697665d9012fa602f4035821038ccc8186e5933e845afd096cc6d3f2fdb25fbe4db4864b944619afa8e4e8bd5e06d90130a3018a182cf5183cf501f500f400f4021a73c5da0a0303081a7697b33909684b657973746f6e650a736163636f756e742e6c65646765725f6c697665",
                   hex::encode(cbor).to_lowercase()
        );
    }
}
