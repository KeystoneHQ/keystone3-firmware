use super::*;
use bitcoin::secp256k1;
use blake2b_simd::Hash;
use keystore::algorithms::secp256k1::get_private_key_by_seed;
use rand_core::OsRng;
use zcash_vendor::{
    pczt::{roles::low_level_signer, Pczt},
    pczt_ext::{self, PcztSigner},
    transparent::{self, sighash::SignableInput},
};

struct SeedSigner<'a> {
    seed: &'a [u8],
}

impl<'a> PcztSigner for SeedSigner<'a> {
    type Error = ZcashError;
    fn sign_transparent<F>(
        &self,
        index: usize,
        input: &mut transparent::pczt::Input,
        hash: F,
    ) -> Result<(), Self::Error>
    where
        F: FnOnce(SignableInput) -> [u8; 32],
    {
        let fingerprint = calculate_seed_fingerprint(&self.seed)
            .map_err(|e| ZcashError::SigningError(e.to_string()))?;

        let key_path = input.bip32_derivation();

        let path = key_path
            .iter()
            .find_map(|(pubkey, path)| {
                let path_fingerprint = path.seed_fingerprint().clone();
                if fingerprint == path_fingerprint {
                    let path = {
                        let mut ret = "m".to_string();
                        for i in path.derivation_path().iter() {
                            if i.is_hardened() {
                                ret.push_str(&alloc::format!("/{}'", i.index()));
                            } else {
                                ret.push_str(&alloc::format!("/{}", i.index()));
                            }
                        }
                        ret
                    };
                    match get_public_key_by_seed(&self.seed, &path) {
                        Ok(my_pubkey) if my_pubkey.serialize().to_vec().eq(pubkey) => {
                            Some(Ok(path))
                        }
                        Err(e) => Some(Err(e)),
                        _ => None,
                    }
                } else {
                    None
                }
            })
            .transpose()
            .map_err(|e| ZcashError::SigningError(e.to_string()))?;

        if let Some(path) = path {
            let sk = get_private_key_by_seed(self.seed, &path).map_err(|e| {
                ZcashError::SigningError(alloc::format!("failed to get private key: {:?}", e))
            })?;
            let secp = secp256k1::Secp256k1::new();
            input.sign(index, hash, &sk, &secp).map_err(|e| {
                ZcashError::SigningError(alloc::format!("failed to sign input: {:?}", e))
            })?;
        }

        Ok(())
    }

    fn sign_orchard(
        &self,
        action: &mut orchard::pczt::Action,
        hash: Hash,
    ) -> Result<(), Self::Error> {
        let fingerprint = calculate_seed_fingerprint(&self.seed)
            .map_err(|e| ZcashError::SigningError(e.to_string()))?;

        let derivation = action.spend().zip32_derivation().as_ref().ok_or_else(|| {
            ZcashError::SigningError("missing ZIP 32 derivation for Orchard action".into())
        })?;

        if &fingerprint == derivation.seed_fingerprint() {
            sign_message_orchard(
                action,
                &self.seed,
                hash.as_bytes().try_into().expect("correct length"),
                &derivation.derivation_path().clone(),
                OsRng,
            )
            .map_err(|e| ZcashError::SigningError(e.to_string()))
        } else {
            Ok(())
        }
    }
}
pub fn sign_pczt(pczt: Pczt, seed: &[u8]) -> crate::Result<Vec<u8>> {
    let signer = low_level_signer::Signer::new(pczt);

    let signer = pczt_ext::sign(signer, &SeedSigner { seed })
        .map_err(|e| ZcashError::SigningError(e.to_string()))?;

    Ok(signer.finish().serialize())
}
