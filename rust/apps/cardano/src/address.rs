use crate::errors::{CardanoError, Result};
use alloc::string::{String, ToString};

use cardano_serialization_lib::protocol_types::credential::*;
use cardano_serialization_lib::protocol_types::{
    BaseAddress, Ed25519KeyHash, EnterpriseAddress, RewardAddress,
};
use cryptoxide::hashing::blake2b_224;
use ed25519_bip32_core::{DerivationScheme, XPub};
use hex;
use ur_registry::crypto_key_path::CryptoKeyPath;

pub enum AddressType {
    Base,
    Stake,
    Enterprise,
}

pub fn calc_stake_address_from_xpub(stake_key: [u8; 32]) -> Result<String> {
    let stake_key_hash = blake2b_224(&stake_key);
    let address = RewardAddress::new(
        1,
        &Credential::from_keyhash(&Ed25519KeyHash::from(stake_key_hash)),
    );
    address
        .to_address()
        .to_bech32(None)
        .map_err(|e| CardanoError::AddressEncodingError(e.to_string()))
}

pub fn derive_xpub_from_xpub(xpub: String, path: CryptoKeyPath) -> Result<String> {
    let xpub_bytes = hex::decode(xpub).map_err(|e| CardanoError::DerivationError(e.to_string()))?;
    let xpub =
        XPub::from_slice(&xpub_bytes).map_err(|e| CardanoError::DerivationError(e.to_string()))?;
    let mut xpub = xpub;
    for component in path.get_components() {
        xpub = xpub.derive(
            DerivationScheme::V2,
            component
                .get_index()
                .ok_or(CardanoError::DerivationError("Index is None".to_string()))
                .map_err(|e| CardanoError::DerivationError(e.to_string()))?,
        )?;
    }
    Ok(hex::encode(xpub.public_key()))
}

pub fn derive_address(
    xpub: String,
    change: u32,
    index: u32,
    stake_key_index: u32,
    address_type: AddressType,
    network: u8,
) -> Result<String> {
    let xpub_bytes = hex::decode(xpub).map_err(|e| CardanoError::DerivationError(e.to_string()))?;
    let xpub =
        XPub::from_slice(&xpub_bytes).map_err(|e| CardanoError::DerivationError(e.to_string()))?;
    match address_type {
        AddressType::Base => {
            let payment_key = xpub
                .derive(DerivationScheme::V2, change)?
                .derive(DerivationScheme::V2, index)?
                .public_key();
            let payment_key_hash = blake2b_224(&payment_key);
            let stake_key = xpub
                .derive(DerivationScheme::V2, 2)?
                .derive(DerivationScheme::V2, stake_key_index)?
                .public_key();
            let stake_key_hash = blake2b_224(&stake_key);
            let address = BaseAddress::new(
                network,
                &Credential::from_keyhash(&Ed25519KeyHash::from(payment_key_hash)),
                &Credential::from_keyhash(&Ed25519KeyHash::from(stake_key_hash)),
            );
            address
                .to_address()
                .to_bech32(None)
                .map_err(|e| CardanoError::AddressEncodingError(e.to_string()))
        }
        AddressType::Stake => {
            let stake_key = xpub
                .derive(DerivationScheme::V2, 2)?
                .derive(DerivationScheme::V2, stake_key_index)?
                .public_key();
            let stake_key_hash = blake2b_224(&stake_key);
            let address = RewardAddress::new(
                network,
                &Credential::from_keyhash(&Ed25519KeyHash::from(stake_key_hash)),
            );
            address
                .to_address()
                .to_bech32(None)
                .map_err(|e| CardanoError::AddressEncodingError(e.to_string()))
        }
        AddressType::Enterprise => {
            let payment_key = xpub
                .derive(DerivationScheme::V2, 0)?
                .derive(DerivationScheme::V2, index)?
                .public_key();
            let payment_key_hash = blake2b_224(&payment_key);
            let address = EnterpriseAddress::new(
                network,
                &Credential::from_keyhash(&Ed25519KeyHash::from(payment_key_hash)),
            );
            address
                .to_address()
                .to_bech32(None)
                .map_err(|e| CardanoError::AddressEncodingError(e.to_string()))
        }
    }
}

pub fn derive_pubkey_hash(xpub: String, change: u32, index: u32) -> Result<[u8; 28]> {
    let xpub_bytes = hex::decode(xpub).map_err(|e| CardanoError::DerivationError(e.to_string()))?;
    let xpub =
        XPub::from_slice(&xpub_bytes).map_err(|e| CardanoError::DerivationError(e.to_string()))?;

    let payment_key = xpub
        .derive(DerivationScheme::V2, change)?
        .derive(DerivationScheme::V2, index)?
        .public_key();
    Ok(blake2b_224(&payment_key))
}

#[cfg(test)]
mod tests {
    use super::*;
    use alloc::string::ToString;
    use alloc::vec;
    use bech32;

    use cryptoxide::hashing::blake2b_224;
    use keystore;

    extern crate std;

    use crate::address::{derive_address, AddressType};
    use bech32::Bech32;
    use hex;
    use std::println;
    use ur_registry::crypto_key_path::PathComponent;

    #[test]
    fn spike() {
        let path = "m/1852'/1815'/0'/0/0";
        let path2 = "m/1852'/1815'/0'/2/0";
        let entropy = hex::decode("00000000000000000000000000000000").unwrap();
        let spend_key =
            keystore::algorithms::ed25519::bip32_ed25519::get_extended_public_key_by_entropy(
                entropy.as_slice(),
                b"",
                &path.to_string(),
            )
            .unwrap();
        let stake_key =
            keystore::algorithms::ed25519::bip32_ed25519::get_extended_public_key_by_entropy(
                entropy.as_slice(),
                b"",
                &path2.to_string(),
            )
            .unwrap();
        let spend = blake2b_224(spend_key.public_key_bytes());
        let stake = blake2b_224(stake_key.public_key_bytes());

        let prefix = "addr";
        let header: u8 = 0b00000001;
        let mut buf = vec![];
        buf.push(header);
        buf.extend(spend);
        buf.extend(stake);
        let spend_address =
            bech32::encode::<Bech32>(bech32::Hrp::parse_unchecked(prefix), &buf).unwrap();
        println!("{spend_address}");

        let mut buf2 = vec![];
        buf2.push(0b1110_0001);
        buf2.extend(stake);
        let reward_address =
            bech32::encode::<Bech32>(bech32::Hrp::parse_unchecked("stake"), &buf2).unwrap();
        println!("{reward_address}");
    }

    #[test]
    fn test_address() {
        let path = "m/1852'/1815'/0'";
        let entropy = hex::decode("00000000000000000000000000000000").unwrap();
        let xpub =
            keystore::algorithms::ed25519::bip32_ed25519::get_extended_public_key_by_entropy(
                entropy.as_slice(),
                b"",
                &path.to_string(),
            )
            .unwrap();
        {
            println!("xpub = {xpub}");
            let spend_address =
                derive_address(xpub.to_string(), 0, 0, 0, AddressType::Base, 1).unwrap();
            assert_eq!("addr1qy8ac7qqy0vtulyl7wntmsxc6wex80gvcyjy33qffrhm7sh927ysx5sftuw0dlft05dz3c7revpf7jx0xnlcjz3g69mq4afdhv", spend_address)
        }
        {
            let reward_address =
                derive_address(xpub.to_string(), 0, 0, 0, AddressType::Stake, 1).unwrap();
            assert_eq!(
                "stake1u8j40zgr2gy4788kl54h6x3gu0pukq5lfr8nflufpg5dzaskqlx2l",
                reward_address
            )
        }
        {
            let enterprise_address =
                derive_address(xpub.to_string(), 0, 0, 0, AddressType::Enterprise, 1).unwrap();
            assert_eq!(
                "addr1vy8ac7qqy0vtulyl7wntmsxc6wex80gvcyjy33qffrhm7ss7lxrqp",
                enterprise_address
            )
        }
    }

    #[test]
    fn test_calc_stake_address_from_xpub() {
        let stake_pub =
            hex::decode("ca0e65d9bb8d0dca5e88adc5e1c644cc7d62e5a139350330281ed7e3a6938d2c")
                .unwrap();
        let address = calc_stake_address_from_xpub(stake_pub.try_into().unwrap()).unwrap();
        assert_eq!(
            "stake1uye6fu05dpz5w39jlumyfv4t082gua4rrpleqtlg5x704tg82ull2".to_string(),
            address
        );
    }

    #[test]
    fn test_calc_stake_address_from_xpub_different_keys() {
        let key1 = [0u8; 32];
        let key2 = [1u8; 32];
        let addr1 = calc_stake_address_from_xpub(key1).unwrap();
        let addr2 = calc_stake_address_from_xpub(key2).unwrap();
        assert_ne!(addr1, addr2);
        assert!(addr1.starts_with("stake"));
        assert!(addr2.starts_with("stake"));
    }

    #[test]
    fn test_derive_pubkey_hash() {
        let path = "m/1852'/1815'/0'";
        let entropy = hex::decode("00000000000000000000000000000000").unwrap();
        let xpub =
            keystore::algorithms::ed25519::bip32_ed25519::get_extended_public_key_by_entropy(
                entropy.as_slice(),
                b"",
                &path.to_string(),
            )
            .unwrap();
        let pubkey_hash = derive_pubkey_hash(xpub.to_string(), 0, 0).unwrap();
        assert_eq!(
            hex::encode(pubkey_hash),
            "0fdc780023d8be7c9ff3a6bdc0d8d3b263bd0cc12448c40948efbf42",
        );
    }

    #[test]
    fn test_derive_xpub_from_xpub() {
        let path1 = PathComponent::new(Some(2), false).unwrap();
        let path2 = PathComponent::new(Some(0), false).unwrap();

        let source_fingerprint: [u8; 4] = [18, 52, 86, 120];
        let components = vec![path1, path2];
        let crypto_key_path = CryptoKeyPath::new(components, Some(source_fingerprint), None);

        let xpub = "cc077f786b2f9d5e8fcdef0c7aad56efc4a70abb7bf5947148d5921d23bfe22abe95c9196a0ece66f56065665aeb8d081ba1e19bbf4fe5d27f07d4c362bb39a5".to_string();
        let derived_xpub = derive_xpub_from_xpub(xpub, crypto_key_path).unwrap();

        assert_eq!(
            derived_xpub,
            "ca0e65d9bb8d0dca5e88adc5e1c644cc7d62e5a139350330281ed7e3a6938d2c"
        );
    }

    #[test]
    fn test_address_from_slip39_ms() {
        let path = "m/1852'/1815'/0'";
        let seed = hex::decode("c080e9d40873204bb1bb5837dc88886b").unwrap();
        let xpub = crate::slip23::from_seed_slip23_path(&seed, path)
            .unwrap()
            .public()
            .to_string();
        let spend_address =
            derive_address(xpub.to_string(), 0, 0, 0, AddressType::Base, 1).unwrap();
        assert_eq!("addr1q9jlm0nq3csn7e6hs9ndt8yhwy4pzxtaq5vvs7zqdzyqv0e9wqpqu38y55a5xjx36lvu49apd4ke34q3ajus2ayneqcqqqnxcc", spend_address)
    }

    #[test]
    fn test_base_address_change_chain_internal_vs_external() {
        let account_path = "m/1852'/1815'/0'";
        let entropy = hex::decode("00000000000000000000000000000000").unwrap();
        let xpub =
            keystore::algorithms::ed25519::bip32_ed25519::get_extended_public_key_by_entropy(
                entropy.as_slice(),
                b"",
                &account_path.to_string(),
            )
            .unwrap();

        let external = derive_address(xpub.to_string(), 0, 0, 0, AddressType::Base, 1).unwrap();
        let internal = derive_address(xpub.to_string(), 1, 0, 0, AddressType::Base, 1).unwrap();

        assert_ne!(external, internal);
        assert!(external.starts_with("addr"));
        assert!(internal.starts_with("addr"));
    }

    #[test]
    fn test_base_address_changes_with_stake_index() {
        let account_path = "m/1852'/1815'/0'";
        let entropy = hex::decode("00000000000000000000000000000000").unwrap();
        let xpub =
            keystore::algorithms::ed25519::bip32_ed25519::get_extended_public_key_by_entropy(
                entropy.as_slice(),
                b"",
                &account_path.to_string(),
            )
            .unwrap();

        let base_idx0 = derive_address(xpub.to_string(), 0, 0, 0, AddressType::Base, 1).unwrap();
        let base_idx1 = derive_address(xpub.to_string(), 0, 0, 1, AddressType::Base, 1).unwrap();
        assert_ne!(base_idx0, base_idx1);
    }

    #[test]
    fn test_derive_address_testnet() {
        let account_path = "m/1852'/1815'/0'";
        let entropy = hex::decode("00000000000000000000000000000000").unwrap();
        let xpub =
            keystore::algorithms::ed25519::bip32_ed25519::get_extended_public_key_by_entropy(
                entropy.as_slice(),
                b"",
                &account_path.to_string(),
            )
            .unwrap();

        let testnet_address =
            derive_address(xpub.to_string(), 0, 0, 0, AddressType::Base, 0).unwrap();
        assert!(testnet_address.starts_with("addr_test"));
    }

    #[test]
    fn test_derive_address_different_indexes() {
        let account_path = "m/1852'/1815'/0'";
        let entropy = hex::decode("00000000000000000000000000000000").unwrap();
        let xpub =
            keystore::algorithms::ed25519::bip32_ed25519::get_extended_public_key_by_entropy(
                entropy.as_slice(),
                b"",
                &account_path.to_string(),
            )
            .unwrap();

        let addr0 = derive_address(xpub.to_string(), 0, 0, 0, AddressType::Base, 1).unwrap();
        let addr1 = derive_address(xpub.to_string(), 0, 1, 0, AddressType::Base, 1).unwrap();
        assert_ne!(addr0, addr1);
    }

    #[test]
    fn test_derive_pubkey_hash_different_paths() {
        let path = "m/1852'/1815'/0'";
        let entropy = hex::decode("00000000000000000000000000000000").unwrap();
        let xpub =
            keystore::algorithms::ed25519::bip32_ed25519::get_extended_public_key_by_entropy(
                entropy.as_slice(),
                b"",
                &path.to_string(),
            )
            .unwrap();

        let hash0 = derive_pubkey_hash(xpub.to_string(), 0, 0).unwrap();
        let hash1 = derive_pubkey_hash(xpub.to_string(), 0, 1).unwrap();
        assert_ne!(hash0, hash1);
    }

    #[test]
    fn test_derive_xpub_from_xpub_invalid_hex() {
        let invalid_xpub = "invalid_hex".to_string();
        let path1 = PathComponent::new(Some(2), false).unwrap();
        let source_fingerprint: [u8; 4] = [18, 52, 86, 120];
        let components = vec![path1];
        let crypto_key_path = CryptoKeyPath::new(components, Some(source_fingerprint), None);
        let result = derive_xpub_from_xpub(invalid_xpub, crypto_key_path);
        assert!(result.is_err());
    }

    #[test]
    fn test_derive_xpub_from_xpub_empty_path() {
        let xpub = "cc077f786b2f9d5e8fcdef0c7aad56efc4a70abb7bf5947148d5921d23bfe22abe95c9196a0ece66f56065665aeb8d081ba1e19bbf4fe5d27f07d4c362bb39a5".to_string();
        let source_fingerprint: [u8; 4] = [18, 52, 86, 120];
        let crypto_key_path = CryptoKeyPath::new(vec![], Some(source_fingerprint), None);
        let derived_xpub = derive_xpub_from_xpub(xpub, crypto_key_path).unwrap();
        // Empty path should return the same xpub (just the public key part)
        assert_eq!(derived_xpub.len(), 64); // 32 bytes hex encoded
    }
}
