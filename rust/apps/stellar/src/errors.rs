use core::array::TryFromSliceError;

use alloc::string::{String, ToString};
use third_party::{base58, bcs, hex, serde_json, thiserror};
use thiserror::Error;

#[derive(Error, Debug)]
pub enum StellarError {
    #[error("sui transaction parse failed, reason: `{0}`")]
    ParseTxError(String),
    #[error("sign failed, reason: {0}")]
    SignFailure(String),
    #[error("Meet invalid data when reading `{0}`")]
    InvalidData(String),
    #[error("Invalid xpub: `{0}`")]
    InvalidXPub(String),
}

pub type Result<T> = core::result::Result<T, StellarError>;

impl From<hex::FromHexError> for StellarError {
    fn from(value: hex::FromHexError) -> Self {
        Self::InvalidData(format!("hex operation failed {}", value))
    }
}

impl From<serde_json::Error> for StellarError {
    fn from(value: serde_json::Error) -> Self {
        Self::InvalidData(format!("serde_json operation failed {}", value))
    }
}

impl From<base58::Error> for StellarError {
    fn from(value: base58::Error) -> Self {
        Self::InvalidXPub(value.to_string())
    }
}

impl From<bcs::Error> for StellarError {
    fn from(value: bcs::Error) -> Self {
        Self::InvalidData(value.to_string())
    }
}

impl From<TryFromSliceError> for StellarError {
    fn from(value: TryFromSliceError) -> Self {
        Self::InvalidXPub(value.to_string())
    }
}
