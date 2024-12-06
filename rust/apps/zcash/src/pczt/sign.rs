use super::*;
use alloc::format;
use blake2b_simd::Hash;

struct SeedSigner {
    seed: [u8; 64],
}

impl PcztSigner for SeedSigner {
    type Error = ZcashError;
    fn sign_transparent(
        &self,
        hash: &[u8],
        key_path: BTreeMap<[u8; 33], Zip32Derivation>,
    ) -> Result<BTreeMap<[u8; 33], ZcashSignature>, Self::Error> {
        let message = Message::from_digest_slice(hash).unwrap();
        let fingerprint = calculate_seed_fingerprint(&self.seed)
            .map_err(|e| ZcashError::SigningError(e.to_string()))?;

        let mut result = BTreeMap::new();
        key_path.iter().try_for_each(|(pubkey, path)| {
            let path_fingerprint = path.seed_fingerprint.clone();
            if fingerprint == path_fingerprint {
                let my_pubkey = get_public_key_by_seed(&self.seed, &path.to_string())
                    .map_err(|e| ZcashError::SigningError(e.to_string()))?;
                if my_pubkey.serialize().to_vec().eq(pubkey) {
                    let signature = sign_message_by_seed(&self.seed, &path.to_string(), &message)
                        .map(|(_rec_id, signature)| signature)
                        .map_err(|e| ZcashError::SigningError(e.to_string()))?;
                    result.insert(pubkey.clone(), signature);
                }
            }
            Ok(())
        })?;

        Ok(result)
    }

    fn sign_sapling(
        &self,
        _hash: Option<Hash>,
        _alpha: [u8; 32],
        _path: Zip32Derivation,
    ) -> Result<Option<ZcashSignature>, Self::Error> {
        // we don't support sapling yet
        Err(ZcashError::SigningError(
            "sapling not supported".to_string(),
        ))
    }

    fn sign_orchard(
        &self,
        hash: Option<Hash>,
        alpha: [u8; 32],
        path: Zip32Derivation,
    ) -> Result<Option<ZcashSignature>, Self::Error> {
        let fingerprint = calculate_seed_fingerprint(&self.seed)
            .map_err(|e| ZcashError::SigningError(e.to_string()))?;

        let hash = hash.ok_or(ZcashError::InvalidDataError(format!("invalid siging hash")))?;

        let path_fingerprint = path.seed_fingerprint.clone();
        if fingerprint == path_fingerprint {
            sign_message_orchard(&self.seed, alpha, hash.as_bytes(), &path.to_string())
                .map(|signature| Some(signature))
                .map_err(|e| ZcashError::SigningError(e.to_string()))
        } else {
            Ok(None)
        }
    }
}

#[cfg(test)]
mod tests {
    use alloc::{collections::btree_map::BTreeMap, vec};
    use zcash_vendor::pczt::{
        common::{Global, Zip32Derivation},
        orchard::{self, Action},
        sapling, transparent, Pczt, V5_TX_VERSION, V5_VERSION_GROUP_ID,
    };

    use super::*;

    extern crate std;

    const HARDENED_MASK: u32 = 0x8000_0000;

    use std::println;

    #[test]
    fn test_pczt_sign() {
        let seed = hex::decode("00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000").unwrap();

        let fingerprint =
            hex::decode("a833c2361e2d72d8fef1ec19071a6433b5f3c0b8aafb82ce2930b2349ad985c5")
                .unwrap();

        let signer = SeedSigner {
            seed: seed.try_into().unwrap(),
        };


        // let signed = pczt.sign(&signer).unwrap();

        // assert_eq!("274d411da4e2cdeab282ac5b61b6b2acb0d6edfe9b9fc6282c200ed621a581a1234b44710fedee313667cc315d896ec69bb0e233b9897bf3fea2820f84757419", hex::encode(signed.orchard.actions[0].spend.spend_auth_sig.unwrap()));
        // assert_eq!("cc49f7b09c5d2bb2ed55390da728e7a37639d461b040183a8ac87020d8236f1db920b3b162c41bfeba278c170702716e81db09320e4ce69fda4095c53091052f", hex::encode(signed.orchard.actions[1].spend.spend_auth_sig.unwrap()));
    }
}
