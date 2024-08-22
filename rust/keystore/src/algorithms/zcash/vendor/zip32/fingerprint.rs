//! Seed Fingerprints according to ZIP 32
//!
//! Implements section [Seed Fingerprints] of Shielded Hierarchical Deterministic Wallets (ZIP 32).
//!
//! [Seed Fingerprints]: https://zips.z.cash/zip-0032#seed-fingerprints
use blake2b_simd::Params as Blake2bParams;

const ZIP32_SEED_FP_PERSONALIZATION: &[u8; 16] = b"Zcash_HD_Seed_FP";

/// The fingerprint for a wallet's seed bytes, as defined in [ZIP 32].
///
/// [ZIP 32]: https://zips.z.cash/zip-0032#seed-fingerprints
#[derive(Clone, Copy, PartialEq, Eq, PartialOrd, Ord, Hash)]
pub struct SeedFingerprint([u8; 32]);

impl ::core::fmt::Debug for SeedFingerprint {
    fn fmt(&self, f: &mut ::core::fmt::Formatter) -> ::core::fmt::Result {
        write!(f, "SeedFingerprint(")?;
        for i in self.0 {
            write!(f, "{:02x}", i)?;
        }
        write!(f, ")")?;

        Ok(())
    }
}

impl SeedFingerprint {
    /// Derives the fingerprint of the given seed bytes.
    ///
    /// Returns `None` if the length of `seed_bytes` is less than 32 or greater than 252.
    pub fn from_seed(seed_bytes: &[u8]) -> Option<SeedFingerprint> {
        let seed_len = seed_bytes.len();

        if (32..=252).contains(&seed_len) {
            let seed_len: u8 = seed_len.try_into().unwrap();
            Some(SeedFingerprint(
                Blake2bParams::new()
                    .hash_length(32)
                    .personal(ZIP32_SEED_FP_PERSONALIZATION)
                    .to_state()
                    .update(&[seed_len])
                    .update(seed_bytes)
                    .finalize()
                    .as_bytes()
                    .try_into()
                    .expect("hash length should be 32 bytes"),
            ))
        } else {
            None
        }
    }

    /// Reconstructs the fingerprint from a buffer containing a previously computed fingerprint.
    pub fn from_bytes(hash: [u8; 32]) -> Self {
        Self(hash)
    }

    /// Returns the fingerprint as a byte array.
    pub fn to_bytes(&self) -> [u8; 32] {
        self.0
    }
}
