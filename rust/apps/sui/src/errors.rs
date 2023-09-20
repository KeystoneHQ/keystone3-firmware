use alloc::string::{String, ToString};
use keystore::errors::KeystoreError;
use third_party::{hex, thiserror, base58};
use thiserror::Error;

#[derive(Error, Debug)]
pub enum SuiError {
    #[error("sui transaction parse failed, reason: `{0}`")]
    ParseTxError(String),
    #[error("sign failed, reason: {0}")]
    SignFailure(String),
    #[error("Invalid hd_Path: {0}")]
    InvalidHDPath(String),
    #[error("KeystoreError: {0}")]
    KeystoreError(String),
    #[error("Invalid Address: {0}")]
    InvalidAddressError(String),
    #[error("Meet invalid data when reading `{0}`")]
    InvalidData(String),
    #[error("Invalid xpub: `{0}`")]
    InvalidXPub(String),
}

pub type Result<T> = core::result::Result<T, SuiError>;

impl From<KeystoreError> for SuiError {
    fn from(value: KeystoreError) -> Self {
        Self::KeystoreError(value.to_string())
    }
}

impl From<hex::FromHexError> for SuiError {
    fn from(value: hex::FromHexError) -> Self {
        Self::InvalidData(format!("hex operation failed {}", value))
    }
}

impl From<serde_json::Error> for SuiError {
    fn from(value: serde_json::Error) -> Self {
        Self::InvalidData(format!("serde_json operation failed {}", value))
    }
}

impl From<base58::Error> for SuiError {
    fn from(value: base58::Error) -> Self {
        Self::InvalidXPub(value.to_string())
    }
}
