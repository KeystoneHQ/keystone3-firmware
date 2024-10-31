use alloc::string::{String, ToString};
use bitcoin::secp256k1::Message;
use keystore::algorithms::secp256k1::sign_message_by_seed;
use keystore::algorithms::zcash::sign_message_orchard;
use zcash_vendor::pczt::pczt_ext::{PcztSigner, ZcashSignature};

use crate::errors::ZcashError;

struct SeedSigner {
    seed: [u8; 32],
}

impl PcztSigner for SeedSigner {
    type Error = ZcashError;
    fn sign_transparent(&self, hash: &[u8], path: String) -> Result<ZcashSignature, Self::Error> {
        let message = Message::from_digest_slice(hash).unwrap();
        sign_message_by_seed(&self.seed, &path, &message)
            .map(|(_rec_id, signature)| ZcashSignature::from(signature))
            .map_err(|e| ZcashError::SigningError(e.to_string()))
    }

    fn sign_sapling(
        &self,
        hash: &[u8],
        alpha: [u8; 32],
        path: String,
    ) -> Result<ZcashSignature, Self::Error> {
        // we don't support sapling yet
        Err(ZcashError::SigningError(
            "sapling not supported".to_string(),
        ))
    }

    fn sign_orchard(
        &self,
        hash: &[u8],
        alpha: [u8; 32],
        path: String,
    ) -> Result<ZcashSignature, Self::Error> {
        sign_message_orchard(&self.seed, alpha, hash, &path)
            .map(|signature| ZcashSignature::from(signature))
            .map_err(|e| ZcashError::SigningError(e.to_string()))
    }
}
