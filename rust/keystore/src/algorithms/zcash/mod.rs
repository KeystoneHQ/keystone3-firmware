use core::str::FromStr;

use alloc::{
    string::{String, ToString},
    vec::Vec,
};
use bitcoin::bip32::{ChildNumber, DerivationPath};
use hex;
use rand_chacha::rand_core::SeedableRng;
use rand_chacha::ChaCha8Rng;
use rand_core::{CryptoRng, RngCore};
use zcash_vendor::{
    orchard::keys::{SpendAuthorizingKey, SpendValidatingKey, SpendingKey},
    pasta_curves::{group::ff::PrimeField, Fq},
    zcash_keys::keys::UnifiedSpendingKey,
    zcash_protocol::consensus::MAIN_NETWORK,
    zip32::{fingerprint::SeedFingerprint, AccountId},
};

use crate::errors::{KeystoreError, Result};

use super::utils::normalize_path;

pub fn derive_ufvk(seed: &[u8]) -> Result<String> {
    let usk = UnifiedSpendingKey::from_seed(&MAIN_NETWORK, &seed, AccountId::ZERO)
        .map_err(|e| KeystoreError::DerivationError(e.to_string()))?;
    let ufvk = usk.to_unified_full_viewing_key();
    Ok(ufvk.encode(&MAIN_NETWORK))
}

pub fn calculate_seed_fingerprint(seed: &[u8]) -> Result<[u8; 32]> {
    let sfp = SeedFingerprint::from_seed(seed).ok_or(KeystoreError::SeedError(format!(
        "Invalid seed, cannot calculate ZIP-32 Seed Fingerprint"
    )))?;
    Ok(sfp.to_bytes())
}

pub fn sign_message_orchard<R: RngCore + CryptoRng>(
    seed: &[u8],
    alpha: [u8; 32],
    msg: &[u8],
    path: &str,
    rng: R,
) -> Result<[u8; 64]> {
    let p = normalize_path(path);
    let derivation_path = DerivationPath::from_str(p.as_str())
        .map_err(|e| KeystoreError::InvalidDerivationPath(e.to_string()))?;

    let coin_type = 133;
    let account = derivation_path[2];
    let account_id = match account {
        ChildNumber::Normal { index } => index,
        ChildNumber::Hardened { index } => index,
    };
    let account_id = AccountId::try_from(account_id).unwrap();

    let osk = SpendingKey::from_zip32_seed(seed, coin_type, account_id).unwrap();

    let osak = SpendAuthorizingKey::from(&osk);

    let randm = Fq::from_repr(alpha)
        .into_option()
        .ok_or(KeystoreError::InvalidDataError(format!(
            "invalid orchard alpha"
        )))?;

    let sig = osak.randomize(&randm).sign(rng, &msg);
    let bytes = <[u8; 64]>::from(&sig);
    Ok(bytes)
}

pub fn test_sign_zec(seed: &[u8], alpha: [u8; 32], msg: &[u8]) -> [u8; 64] {
    let mut alpha = alpha;
    alpha.reverse();
    let rng_seed = [0u8; 32];
    let rng = ChaCha8Rng::from_seed(rng_seed);
    let osk = SpendingKey::from_zip32_seed(seed, 133, AccountId::ZERO).unwrap();
    let osak = SpendAuthorizingKey::from(&osk);
    let randm = Fq::from_repr(alpha).unwrap();
    let sig = osak.randomize(&randm).sign(rng, &msg);
    let bytes = <[u8; 64]>::from(&sig);
    rust_tools::debug!(format!("signature: {:?}", hex::encode(&bytes)));
    bytes
}

#[cfg(test)]
mod tests {
    use zcash_vendor::{
        pasta_curves::Fq,
        zcash_keys::keys::{UnifiedAddressRequest, UnifiedSpendingKey},
        zcash_protocol::consensus::{MainNetwork, MAIN_NETWORK},
        zip32::AccountId,
    };

    use zcash_vendor::orchard::{
        keys::{FullViewingKey, SpendAuthorizingKey, SpendingKey},
        redpallas::{Signature, SpendAuth},
    };
    use zcash_vendor::pasta_curves::group::ff::{FromUniformBytes, PrimeField};

    use hex;
    use rand_chacha::rand_core::SeedableRng;
    use rand_chacha::ChaCha8Rng;

    extern crate std;
    use std::println;
    #[test]
    fn spike() {
        let seed = hex::decode("5d741f330207d529ff7af106616bbffa15c1cf3bf9778830e003a17787926c0bd77261f91a4a3e2e52b8f96f35bdadc94187bef0f53c449a3802b4e7332cfedd").unwrap();
        let usk = UnifiedSpendingKey::from_seed(&MAIN_NETWORK, &seed, AccountId::ZERO).unwrap();

        let ufvk = usk.to_unified_full_viewing_key();

        // println!(
        //     "{}",
        //     ufvk.to_unified_incoming_viewing_key().encode(&MAIN_NETWORK)
        // );
        //uivk1xhvuufquepxdr5zyacha4kde0479wr25hk26w07jmtn0ec08t4fh0yqudc7v8ddya5rrx4q34yuuxy524p59radjndx5u3rqgvs6w5q8s9246q4h8ykuqtfmn7tyzdlvnen2x6p0cjlqvr48hqrgf72tr7l9z0vdnh8xwu42ausdrvuvd3w9h50ql64g0plucqfyx9ewjqjr5k7lhv9qrl7whu93jp6t38rpcyl060pz5sqnancrh
        println!("{}", hex::encode(ufvk.transparent().unwrap().serialize()));
        println!("{}", hex::encode(ufvk.orchard().unwrap().to_bytes()));
        // println!("{}", hex::encode(ufvk.orchard().unwrap()));
        println!("{}", ufvk.encode(&MAIN_NETWORK));
        let address = ufvk
            .default_address(UnifiedAddressRequest::all().unwrap())
            .unwrap();
        println!("{:?}", address.0.encode(&MAIN_NETWORK));
    }

    #[test]
    fn spike_address() {
        let seed = hex::decode("a2093a34d4aa4c3ba89aa087a0992cd76e03a303b98f890a7a818d0e1e414db7a3a832791834a6fd9639d9c59430a8855f7f9dd6bed765b6783058ed7bd0ecbd").unwrap();
        let osk = SpendingKey::from_zip32_seed(&seed, 133, AccountId::ZERO).unwrap();
        let ofvk: FullViewingKey = FullViewingKey::from(&osk);
    }

    #[test]
    fn spike_sign_transaction() {
        let rng_seed = [0u8; 32];
        let rng: ChaCha8Rng = ChaCha8Rng::from_seed(rng_seed);
        let seed: std::vec::Vec<u8> = hex::decode("a2093a34d4aa4c3ba89aa087a0992cd76e03a303b98f890a7a818d0e1e414db7a3a832791834a6fd9639d9c59430a8855f7f9dd6bed765b6783058ed7bd0ecbd").unwrap();
        // let usk = UnifiedSpendingKey::from_seed(&MAIN_NETWORK, &seed, AccountId::ZERO).unwrap();
        // let ssk = usk.sapling();
        // let osk = usk.orchard();

        let osk = SpendingKey::from_zip32_seed(&seed, 133, AccountId::ZERO).unwrap();

        let osak = SpendAuthorizingKey::from(&osk);
        //SpendAuthorizingKey(SigningKey(SigningKey { sk: 0x3df9e7346783cfadf079fc8fafbc79ede96f13e2c12a6723e4ea1bc16539949a, pk: VerificationKey { point: Ep { x: 0x1fdab0b7f334c8e163218f17c70b26d33b80143ebc7ab629ff5a28006ce4e219, y: 0x29d265bacc851808114cf6d3585a17ab2a3eeadcd336ee3434da4039366183b3, z: 0x2424bad05cb436309bb30378afd371287c9e96814a9453deaa4d193294013e83 }, bytes: VerificationKeyBytes { bytes: "ac82d5f4bf06f0b51a2fcdcfdb7f0542bf49aa1f6d709ba82dbbdcf5112f0c3f" } } }))
        println!("{:?}", osak);

        // osak.randomize(randomizer)
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
        println!("{:?}", hex::encode(bytes));
        //171d28b28da8e267ba17d0dcd5d61a16b5e39ef13bf57408e1c1cb22ee6d7b866b5de14849ca40a9791631ca042206df1f4e852b6e9e317f815a42b86537d421
    }
}
