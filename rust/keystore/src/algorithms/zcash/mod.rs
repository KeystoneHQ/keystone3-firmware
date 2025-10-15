use core::str::FromStr;

use alloc::string::{String, ToString};
use bitcoin::bip32::{ChildNumber, DerivationPath};
use rand_core::{CryptoRng, RngCore};
use zcash_vendor::{
    orchard::{
        self,
        keys::{SpendAuthorizingKey, SpendingKey},
    },
    zcash_keys::keys::UnifiedSpendingKey,
    zcash_protocol::consensus,
    zip32::{self, fingerprint::SeedFingerprint},
};

use crate::errors::{KeystoreError, Result};

pub fn derive_ufvk<P: consensus::Parameters>(
    params: &P,
    seed: &[u8],
    account_path: &str,
) -> Result<String> {
    let account_path = DerivationPath::from_str(account_path.to_lowercase().as_str())
        .map_err(|e| KeystoreError::DerivationError(e.to_string()))?;
    if account_path.len() != 3 {
        return Err(KeystoreError::DerivationError(format!(
            "invalid account path: {}",
            account_path
        )));
    }
    //should be hardened(32) hardened(133) hardened(account_id)
    let purpose = account_path[0];
    let coin_type = account_path[1];
    let account_id = account_path[2];
    match (purpose, coin_type, account_id) {
        (
            ChildNumber::Hardened { index: 32 },
            ChildNumber::Hardened { index: 133 },
            ChildNumber::Hardened { index: account_id },
        ) => {
            let account_index = zip32::AccountId::try_from(account_id)
                .map_err(|_e| KeystoreError::DerivationError("invalid account index".into()))?;
            let usk = UnifiedSpendingKey::from_seed(params, seed, account_index)
                .map_err(|e| KeystoreError::DerivationError(e.to_string()))?;
            let ufvk = usk.to_unified_full_viewing_key();
            Ok(ufvk.encode(params))
        }
        _ => Err(KeystoreError::DerivationError(format!(
            "invalid account path: {}",
            account_path
        ))),
    }
}

pub fn calculate_seed_fingerprint(seed: &[u8]) -> Result<[u8; 32]> {
    let sfp = SeedFingerprint::from_seed(seed).ok_or(KeystoreError::SeedError(
        "Invalid seed, cannot calculate ZIP-32 Seed Fingerprint".into(),
    ))?;
    Ok(sfp.to_bytes())
}

pub fn sign_message_orchard<R: RngCore + CryptoRng>(
    action: &mut orchard::pczt::Action,
    seed: &[u8],
    sighash: [u8; 32],
    path: &[zip32::ChildIndex],
    rng: R,
) -> Result<()> {
    let coin_type = 133;

    if path.len() == 3
        && path[0] == zip32::ChildIndex::hardened(32)
        && path[1] == zip32::ChildIndex::hardened(coin_type)
    {
        let account_id = zip32::AccountId::try_from(path[2].index() - (1 << 31)).expect("valid");

        let osk = SpendingKey::from_zip32_seed(seed, coin_type, account_id).unwrap();

        let osak = SpendAuthorizingKey::from(&osk);

        action
            .sign(sighash, &osak, rng)
            .map_err(|e| KeystoreError::ZcashOrchardSign(format!("{:?}", e)))
    } else {
        // Keystone only generates UFVKs at the above path; ignore all other signature
        // requests.
        Err(KeystoreError::ZcashOrchardSign(format!(
            "invalid orchard account path: {:?}",
            path
        )))
    }
}

#[cfg(test)]
mod tests {
    use zcash_vendor::{
        pasta_curves::Fq,
        zcash_keys::keys::{UnifiedAddressRequest, UnifiedSpendingKey},
        zcash_protocol::consensus::MAIN_NETWORK,
        zip32::AccountId,
    };

    use zcash_vendor::orchard::keys::{SpendAuthorizingKey, SpendingKey};
    use zcash_vendor::pasta_curves::group::ff::PrimeField;

    use hex;
    use rand_chacha::rand_core::SeedableRng;
    use rand_chacha::ChaCha8Rng;

    extern crate std;
    

    #[test]
    fn test_ufvk_generation_and_encoding() {
        // Test seed from which we'll derive keys
        let seed = hex::decode("000102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f")
            .unwrap();

        // Generate the Unified Spending Key from seed
        let usk = UnifiedSpendingKey::from_seed(&MAIN_NETWORK, &seed, AccountId::ZERO).unwrap();

        // Derive the Unified Full Viewing Key
        let ufvk = usk.to_unified_full_viewing_key();

        // Verify transparent component exists and can be serialized
        let transparent_bytes = ufvk.transparent().unwrap().serialize();
        assert!(!transparent_bytes.is_empty());

        // Verify orchard component exists and can be serialized
        let orchard_bytes = ufvk.orchard().unwrap().to_bytes();
        assert!(!orchard_bytes.is_empty());

        // Verify UFVK can be encoded to string format
        let encoded_ufvk = ufvk.encode(&MAIN_NETWORK);
        assert!(!encoded_ufvk.is_empty());
        assert!(encoded_ufvk.starts_with("uview"));

        // Verify we can generate a valid address from the UFVK
        let address = ufvk
            .default_address(UnifiedAddressRequest::AllAvailableKeys)
            .unwrap();
        let encoded_address = address.0.encode(&MAIN_NETWORK);

        assert!(!encoded_address.is_empty());
        assert!(encoded_address.starts_with("u1"));
        assert_eq!(encoded_address, "u1mnn47asanfdtlsljutd53ha2335q5wvy2r4udavqkxg6e3uzyg2ghrqenzn9my8pr7gfq27yaj80x6ypshmh9tcc3qghzekhzw9tphh2rl3glkdrkg7jl8n3v3mnl7xs7pn7k00zxyx");

        let seed_fingerprint = super::calculate_seed_fingerprint(&seed).unwrap();
        assert_eq!(
            hex::encode(seed_fingerprint),
            "deff604c246710f7176dead02aa746f2fd8d5389f7072556dcb555fdbe5e3ae3"
        );
    }

    #[test]
    fn test_orchard_signing() {
        let rng_seed = [0u8; 32];
        let rng: ChaCha8Rng = ChaCha8Rng::from_seed(rng_seed);
        let seed: std::vec::Vec<u8> =
            hex::decode("000102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f")
                .unwrap();

        let osk = SpendingKey::from_zip32_seed(&seed, 133, AccountId::ZERO).unwrap();

        let osak = SpendAuthorizingKey::from(&osk);

        let mut bytes: [u8; 32] =
            hex::decode("1f98c5acf5b566b8521f7ea2d9f67f1a565f6ab0e57b45aed4a3d69e7c3c8262")
                .unwrap()
                .try_into()
                .unwrap();
        bytes.reverse();
        let randm = Fq::from_repr(bytes).unwrap();
        // randm.to_repr();
        let msg = hex::decode("90b4ba49d75eb5df50b68e927dbd196a6f120ba8544664b116f6d74f0bc3812c")
            .unwrap();
        let sig = osak.randomize(&randm).sign(rng, &msg);
        let bytes = <[u8; 64]>::from(&sig);
        assert_eq!(hex::encode(bytes), "065ef82c33af0ed487e8932112e3359e93c5955d3eac6c3a1f9cb6dd24e19d8a2bba454a4274154dd4ad0c6bdb2022a646950ed521f3de18e99015f4821cbb10");
    }
}
