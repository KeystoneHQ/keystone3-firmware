use crate::errors::{CardanoError, R};
use alloc::string::{String, ToString};

use cardano_serialization_lib::address::{
    BaseAddress, EnterpriseAddress, RewardAddress, StakeCredential,
};
use cardano_serialization_lib::crypto::Ed25519KeyHash;

use third_party::cryptoxide::hashing::blake2b_224;
use third_party::ed25519_bip32_core::{DerivationScheme, XPub};
use third_party::hex;
use third_party::ur_registry::crypto_key_path::CryptoKeyPath;

pub enum AddressType {
    Base,
    Stake,
    Enterprise,
}

pub fn calc_stake_address_from_xpub(stake_key: [u8; 32]) -> R<String> {
    let stake_key_hash = blake2b_224(&stake_key);
    let address = RewardAddress::new(
        1,
        &StakeCredential::from_keyhash(&Ed25519KeyHash::from(stake_key_hash)),
    );
    address
        .to_address()
        .to_bech32(None)
        .map_err(|e| CardanoError::AddressEncodingError(e.to_string()))
}

pub fn derive_xpub_from_xpub(xpub: String, path: CryptoKeyPath) -> R<String> {
    let xpub_bytes = hex::decode(xpub).map_err(|e| CardanoError::DerivationError(e.to_string()))?;
    let xpub =
        XPub::from_slice(&xpub_bytes).map_err(|e| CardanoError::DerivationError(e.to_string()))?;
    let mut xpub = xpub;
    for component in path.get_components() {
        xpub = xpub.derive(DerivationScheme::V2, component.get_index().unwrap())?;
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
) -> R<String> {
    let xpub_bytes = hex::decode(xpub).map_err(|e| CardanoError::DerivationError(e.to_string()))?;
    let xpub =
        XPub::from_slice(&xpub_bytes).map_err(|e| CardanoError::DerivationError(e.to_string()))?;
    match address_type {
        AddressType::Base => {
            let payment_key = xpub
                .derive(DerivationScheme::V2, change)?
                .derive(DerivationScheme::V2, index.clone())?
                .public_key();
            let payment_key_hash = blake2b_224(&payment_key);
            let stake_key = xpub
                .derive(DerivationScheme::V2, 2)?
                .derive(DerivationScheme::V2, stake_key_index.clone())?
                .public_key();
            let stake_key_hash = blake2b_224(&stake_key);
            let address = BaseAddress::new(
                network,
                &StakeCredential::from_keyhash(&Ed25519KeyHash::from(payment_key_hash)),
                &StakeCredential::from_keyhash(&Ed25519KeyHash::from(stake_key_hash)),
            );
            address
                .to_address()
                .to_bech32(None)
                .map_err(|e| CardanoError::AddressEncodingError(e.to_string()))
        }
        AddressType::Stake => {
            let stake_key = xpub
                .derive(DerivationScheme::V2, 2)?
                .derive(DerivationScheme::V2, stake_key_index.clone())?
                .public_key();
            let stake_key_hash = blake2b_224(&stake_key);
            let address = RewardAddress::new(
                network,
                &StakeCredential::from_keyhash(&Ed25519KeyHash::from(stake_key_hash)),
            );
            address
                .to_address()
                .to_bech32(None)
                .map_err(|e| CardanoError::AddressEncodingError(e.to_string()))
        }
        AddressType::Enterprise => {
            let payment_key = xpub
                .derive(DerivationScheme::V2, 0)?
                .derive(DerivationScheme::V2, index.clone())?
                .public_key();
            let payment_key_hash = blake2b_224(&payment_key);
            let address = EnterpriseAddress::new(
                network,
                &StakeCredential::from_keyhash(&Ed25519KeyHash::from(payment_key_hash)),
            );
            address
                .to_address()
                .to_bech32(None)
                .map_err(|e| CardanoError::AddressEncodingError(e.to_string()))
        }
    }
}

pub fn derive_pubkey_hash(xpub: String, change: u32, index: u32) -> R<[u8; 28]> {
    let xpub_bytes = hex::decode(xpub).map_err(|e| CardanoError::DerivationError(e.to_string()))?;
    let xpub =
        XPub::from_slice(&xpub_bytes).map_err(|e| CardanoError::DerivationError(e.to_string()))?;

    let payment_key = xpub
        .derive(DerivationScheme::V2, change)?
        .derive(DerivationScheme::V2, index.clone())?
        .public_key();
    Ok(blake2b_224(&payment_key))
}

#[cfg(test)]
mod tests {
    use super::*;
    use alloc::string::ToString;
    use alloc::vec;
    use cardano_serialization_lib::address::{Address, BaseAddress};
    use keystore;
    use third_party::bech32;
    use third_party::cryptoxide::hashing::blake2b_224;

    extern crate std;

    use crate::address::{derive_address, AddressType};
    use std::println;
    use third_party::bech32::Bech32;
    use third_party::hex;
    use third_party::ur_registry::crypto_key_path::PathComponent;

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
            bech32::encode::<Bech32>(third_party::bech32::Hrp::parse_unchecked(prefix), &buf)
                .unwrap();
        println!("{}", spend_address);

        let mut buf2 = vec![];
        buf2.push(0b1110_0001);
        buf2.extend(stake);
        let reward_address =
            bech32::encode::<Bech32>(third_party::bech32::Hrp::parse_unchecked("stake"), &buf2)
                .unwrap();
        println!("{}", reward_address);
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
}
