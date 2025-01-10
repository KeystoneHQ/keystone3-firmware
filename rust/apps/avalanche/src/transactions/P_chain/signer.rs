use crate::constants::*;
use crate::errors::{AvaxError, Result};
use alloc::string::ToString;
use bytes::{Buf, Bytes};
use core::convert::TryFrom;

#[derive(Debug, Clone)]
pub struct Signer {
    pubkey: [u8; PROOF_OF_POSESSION_PUBKEY_LEN],
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
