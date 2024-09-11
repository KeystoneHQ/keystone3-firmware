use ff::PrimeField;
use pasta_curves::Fq;
use rand_chacha::rand_core::SeedableRng;
use rand_chacha::ChaCha8Rng;
use third_party::hex;
use vendor::{
    orchard::keys::{SpendAuthorizingKey, SpendingKey},
    zip32::AccountId,
};
mod vendor;

pub fn test_sign_zec(seed: &[u8], alpha: [u8; 32], msg: &[u8]) -> [u8; 64] {
    let mut alpha = alpha;
    alpha.reverse();
    let rng_seed = [0u8; 32];
    let rng = ChaCha8Rng::from_seed(rng_seed);
    let osk = SpendingKey::from_zip32_seed(seed, 133, AccountId::ZERO);
    let osak = SpendAuthorizingKey::from(&osk);
    let randm = Fq::from_repr(alpha).unwrap();
    let sig = osak.randomize(&randm).sign(rng, &msg);
    let bytes = <[u8; 64]>::from(&sig);
    rust_tools::debug!(format!("signature: {:?}", hex::encode(&bytes)));
    bytes
}

#[cfg(test)]
mod tests {
    use crate::algorithms::zcash::vendor::zip32::AccountId;

    use super::vendor::orchard::{
        keys::{FullViewingKey, SpendAuthorizingKey, SpendingKey},
        redpallas::{Signature, SpendAuth},
    };
    use pasta_curves::group::ff::{FromUniformBytes, PrimeField};
    use zcash_keys::address::Address;
    use zcash_keys::keys::{UnifiedAddressRequest, UnifiedSpendingKey};
    use zcash_keys;
    use zcash_protocol::consensus::{MainNetwork, MAIN_NETWORK};
    use zip32::AccountId;

    use pasta_curves::{self, Fq};
    use rand_chacha::rand_core::SeedableRng;
    use rand_chacha::ChaCha8Rng;
    use third_party::hex;

    extern crate std;
    use std::println;
    #[test]
    fn spike() {
        let seed = hex::decode("a2093a34d4aa4c3ba89aa087a0992cd76e03a303b98f890a7a818d0e1e414db7a3a832791834a6fd9639d9c59430a8855f7f9dd6bed765b6783058ed7bd0ecbd").unwrap();
        let usk = UnifiedSpendingKey::from_seed(&MAIN_NETWORK, &seed, AccountId::ZERO).unwrap();

        let ufvk = usk.to_unified_full_viewing_key();
        //uview1nkce3lf47a8r0yu8zkzh4ggccsey769fwj794jdwql2ga8v09f8xasfxhp9kme6uxungsz2y0m5rh4xntxsd2xy93exf88denusfc7qaevrecslahahkwdlwlkzygunut3cnen4dsjqv9lrh6dr0gg4succ07xcwaeylpy2vd7vlawugwxf3w4ssjfwnl8thmdetxfemuprfmqx2565m58tynphhu8ndwskkhgwd72kt0chzl3azhtqkhxysqvdtkf5ft5g95kysxcspafhex4chlmck9wpspcavmhzccxslmexw67ur7p6swq5kv7t652qpcd6x769cy6cr43chngcugf928nk8jn3warfkye4zwelzpaac3hgupsqyevz8a8khpf0xs32z2m7sjml728rucv7ztf7mm3jx8vlhgv97xt47k4hsjvvclyzfswwy3ks5khmadkgh8h0ep3aafzld9m4sjelezpzsyvmq6mfff0u0lceht242

        println!(
            "{}",
            ufvk.to_unified_incoming_viewing_key().encode(&MAIN_NETWORK)
        );
        println!("{}", hex::encode(ufvk.transparent().unwrap().serialize()));
        println!("{}", ufvk.encode(&MAIN_NETWORK));
        let address = ufvk
            .default_address(UnifiedAddressRequest::all().unwrap())
            .unwrap();
        println!("{:?}", address.0.encode(&MAIN_NETWORK));
    }

    #[test]
    fn spike_address() {
        let seed = hex::decode("a2093a34d4aa4c3ba89aa087a0992cd76e03a303b98f890a7a818d0e1e414db7a3a832791834a6fd9639d9c59430a8855f7f9dd6bed765b6783058ed7bd0ecbd").unwrap();
        let osk = SpendingKey::from_zip32_seed(&seed, 133, AccountId::ZERO);
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

        let osk = SpendingKey::from_zip32_seed(&seed, 133, AccountId::ZERO);

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
