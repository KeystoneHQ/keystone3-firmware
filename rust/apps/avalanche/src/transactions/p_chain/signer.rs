use crate::constants::*;
use crate::errors::{AvaxError, Result};
use bytes::{Buf, Bytes};
use core::convert::TryFrom;

#[derive(Debug, Clone)]
pub struct Signer {
    #[allow(
        dead_code,
        reason = "Retained as part of the parsed wire format and debug representation"
    )]
    pubkey: [u8; PROOF_OF_POSESSION_PUBKEY_LEN],
    #[allow(
        dead_code,
        reason = "Retained as part of the parsed wire format and debug representation"
    )]
    signature: [u8; PROOF_OF_POSESSION_SIGNATURE_LEN],
}

impl TryFrom<Bytes> for Signer {
    type Error = AvaxError;

    fn try_from(mut bytes: Bytes) -> Result<Self> {
        let mut pubkey = [0u8; PROOF_OF_POSESSION_PUBKEY_LEN];
        bytes.copy_to_slice(&mut pubkey);

        let mut signature = [0u8; PROOF_OF_POSESSION_SIGNATURE_LEN];
        bytes.copy_to_slice(&mut signature);
        Ok(Signer { pubkey, signature })
    }
}
