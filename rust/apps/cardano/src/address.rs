use crate::errors::{CardanoError, R};
use alloc::string::{String, ToString};

use cardano_serialization_lib::address::{BaseAddress, RewardAddress, StakeCredential};
use cardano_serialization_lib::crypto::Ed25519KeyHash;

use third_party::cryptoxide::hashing::blake2b_224;
use third_party::ed25519_bip32_core::{DerivationScheme, XPub};
use third_party::hex;

pub enum AddressType {
    Base,
    Stake,
}

pub fn derive_address(
    xpub: String,
    index: u32,
    address_type: AddressType,
    network: u8,
) -> R<String> {
    let xpub_bytes = hex::decode(xpub).map_err(|e| CardanoError::DerivationError(e.to_string()))?;
    let xpub =
        XPub::from_slice(&xpub_bytes).map_err(|e| CardanoError::DerivationError(e.to_string()))?;
    match address_type {
        AddressType::Base => {
            let payment_key = xpub
                .derive(DerivationScheme::V2, 0)?
                .derive(DerivationScheme::V2, index.clone())?
                .public_key();
            let payment_key_hash = blake2b_224(&payment_key);
            let stake_key = xpub
                .derive(DerivationScheme::V2, 2)?
                .derive(DerivationScheme::V2, index.clone())?
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
                .derive(DerivationScheme::V2, index.clone())?
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
    }
}

#[cfg(test)]
mod tests {
    use alloc::string::ToString;
    use alloc::vec;
    use keystore;
    use third_party::bitcoin::bech32;
    use third_party::bitcoin::bech32::{ToBase32, Variant};
    use third_party::cryptoxide::hashing::blake2b_224;

    extern crate std;

    use crate::address::{derive_address, AddressType};
    use std::println;
    use third_party::hex;

    #[test]
    fn spike() {
        let path = "m/1852'/1815'/0'/0/0";
        let path2 = "m/1852'/1815'/0'/2/0";
        let entropy = hex::decode("00000000000000000000000000000000").unwrap();
        let spend_key =
            keystore::algorithms::ed25519::bip32_ed25519::get_extended_public_key_by_entropy(
                &path.to_string(),
                entropy.as_slice(),
            )
            .unwrap();
        let stake_key =
            keystore::algorithms::ed25519::bip32_ed25519::get_extended_public_key_by_entropy(
                &path2.to_string(),
                entropy.as_slice(),
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
        let spend_address = bech32::encode(prefix, buf.to_base32(), Variant::Bech32).unwrap();
        println!("{}", spend_address);

        let mut buf2 = vec![];
        buf2.push(0b1110_0001);
        buf2.extend(stake);
        let reward_address = bech32::encode("stake", buf2.to_base32(), Variant::Bech32).unwrap();
        println!("{}", reward_address);
    }

    #[test]
    fn test_address() {
        let path = "m/1852'/1815'/0'";
        let entropy = hex::decode("00000000000000000000000000000000").unwrap();
        let xpub =
            keystore::algorithms::ed25519::bip32_ed25519::get_extended_public_key_by_entropy(
                &path.to_string(),
                entropy.as_slice(),
            )
            .unwrap();
        {
            let spend_address = derive_address(xpub.to_string(), 0, AddressType::Base, 1).unwrap();
            assert_eq!("addr1qy8ac7qqy0vtulyl7wntmsxc6wex80gvcyjy33qffrhm7sh927ysx5sftuw0dlft05dz3c7revpf7jx0xnlcjz3g69mq4afdhv", spend_address)
        }
        {
            let reward_address =
                derive_address(xpub.to_string(), 0, AddressType::Stake, 1).unwrap();
            assert_eq!(
                "stake1u8j40zgr2gy4788kl54h6x3gu0pukq5lfr8nflufpg5dzaskqlx2l",
                reward_address
            )
        }
    }
}
