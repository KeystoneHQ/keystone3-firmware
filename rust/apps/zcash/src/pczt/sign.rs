use super::*;
use bitcoin::secp256k1;
use blake2b_simd::Hash;
use keystore::algorithms::secp256k1::get_private_key_by_seed;
use rand_core::OsRng;
use zcash_vendor::{
    pczt::{
        roles::{low_level_signer, redactor::Redactor},
        Pczt,
    },
    pczt_ext::{self, PcztSigner},
    transparent::{self, sighash::SignableInput},
};

struct SeedSigner<'a> {
    seed: &'a [u8],
}

impl PcztSigner for SeedSigner<'_> {
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
        let fingerprint = calculate_seed_fingerprint(self.seed)
            .map_err(|e| ZcashError::SigningError(e.to_string()))?;

        let key_path = input.bip32_derivation();

        let path = key_path
            .iter()
            .find_map(|(pubkey, path)| {
                let path_fingerprint = *path.seed_fingerprint();
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
                    match get_public_key_by_seed(self.seed, &path) {
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
                ZcashError::SigningError(alloc::format!("failed to get private key: {e:?}"))
            })?;
            let secp = secp256k1::Secp256k1::new();
            input.sign(index, hash, &sk, &secp).map_err(|e| {
                ZcashError::SigningError(alloc::format!("failed to sign input: {e:?}"))
            })?;
        }

        Ok(())
    }

    #[cfg(feature = "orchard")]
    fn sign_orchard(
        &self,
        action: &mut orchard::pczt::Action,
        hash: Hash,
    ) -> Result<(), Self::Error> {
        let fingerprint = calculate_seed_fingerprint(self.seed)
            .map_err(|e| ZcashError::SigningError(e.to_string()))?;

        let derivation = action.spend().zip32_derivation().as_ref().ok_or_else(|| {
            ZcashError::SigningError("missing ZIP 32 derivation for Orchard action".into())
        })?;

        if &fingerprint == derivation.seed_fingerprint() {
            sign_message_orchard(
                action,
                self.seed,
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

    // Now that we've created the signature, remove the other optional fields from the
    // PCZT, to reduce its size for the return trip and make the QR code scanning more
    // reliable. The wallet that provided the unsigned PCZT can retain it for combining if
    // these fields are needed.
    let signed_pczt = Redactor::new(signer.finish())
        .redact_orchard_with(|mut r| {
            r.redact_actions(|mut ar| {
                ar.clear_spend_recipient();
                ar.clear_spend_value();
                ar.clear_spend_rho();
                ar.clear_spend_rseed();
                ar.clear_spend_fvk();
                ar.clear_spend_witness();
                ar.clear_spend_alpha();
                ar.clear_spend_zip32_derivation();
                ar.clear_spend_dummy_sk();
                ar.clear_output_recipient();
                ar.clear_output_value();
                ar.clear_output_rseed();
                ar.clear_output_ock();
                ar.clear_output_zip32_derivation();
                ar.clear_output_user_address();
                ar.clear_rcv();
            });
            r.clear_zkproof();
            r.clear_bsk();
        })
        .redact_sapling_with(|mut r| {
            r.redact_spends(|mut sr| {
                sr.clear_zkproof();
                sr.clear_recipient();
                sr.clear_value();
                sr.clear_rcm();
                sr.clear_rseed();
                sr.clear_rcv();
                sr.clear_proof_generation_key();
                sr.clear_witness();
                sr.clear_alpha();
                sr.clear_zip32_derivation();
                sr.clear_dummy_ask();
            });
            r.redact_outputs(|mut or| {
                or.clear_zkproof();
                or.clear_recipient();
                or.clear_value();
                or.clear_rseed();
                or.clear_rcv();
                or.clear_ock();
                or.clear_zip32_derivation();
                or.clear_user_address();
            });
            r.clear_bsk();
        })
        .redact_transparent_with(|mut r| {
            r.redact_inputs(|mut ir| {
                ir.clear_redeem_script();
                ir.clear_bip32_derivation();
                ir.clear_ripemd160_preimages();
                ir.clear_sha256_preimages();
                ir.clear_hash160_preimages();
                ir.clear_hash256_preimages();
            });
            r.redact_outputs(|mut or| {
                or.clear_redeem_script();
                or.clear_bip32_derivation();
                or.clear_user_address();
            });
        })
        .finish();

    Ok(signed_pczt.serialize())
}
