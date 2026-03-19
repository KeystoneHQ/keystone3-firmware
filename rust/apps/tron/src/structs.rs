use crate::address::public_key_to_address;
use crate::errors::Result;
use alloc::string::{String};

use bitcoin::secp256k1::PublicKey;

#[derive(Clone, Debug)]
pub struct PersonalMessage {
    pub raw_message: String,
    pub utf8_message: String,
    pub from: Option<String>,
}

impl PersonalMessage {
    pub fn from(
        raw_message: String,
        utf8_message: String,
        from: Option<PublicKey>,
    ) -> Result<Self> {
        Ok(Self {
            raw_message,
            utf8_message,
            from: from.map(|key| public_key_to_address(&key)),
        })
    }
}

