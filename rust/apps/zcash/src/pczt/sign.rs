use super::*;
use alloc::format;
use blake2b_simd::Hash;

struct SeedSigner<'a> {
    seed: &'a [u8],
}

impl<'a> PcztSigner for SeedSigner<'a> {
    type Error = ZcashError;
    fn sign_transparent(
        &self,
        hash: Option<Hash>,
        key_path: BTreeMap<[u8; 33], Zip32Derivation>,
    ) -> Result<BTreeMap<[u8; 33], ZcashSignature>, Self::Error> {
        let hash = hash.ok_or(ZcashError::InvalidDataError(format!("invalid siging hash")))?;
        let message = Message::from_digest_slice(hash.as_bytes()).unwrap();
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
pub fn sign_pczt(pczt: &Pczt, seed: &[u8]) -> crate::Result<Vec<u8>> {
    pczt.sign(&SeedSigner { seed })
        .map(|pczt| pczt.serialize())
        .map_err(|e| ZcashError::SigningError(e.to_string()))
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
}
