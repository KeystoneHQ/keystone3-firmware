use alloc::string::{String, ToString};
use third_party::{hex, thiserror, base58, serde_json, bcs};
use thiserror::Error;

#[derive(Error, Debug)]
pub enum SuiError {
    #[error("sui transaction parse failed, reason: `{0}`")]
    ParseTxError(String),
    #[error("sign failed, reason: {0}")]
    SignFailure(String),
    #[error("Meet invalid data when reading `{0}`")]
    InvalidData(String),
    #[error("Invalid xpub: `{0}`")]
    InvalidXPub(String),
}

pub type Result<T> = core::result::Result<T, SuiError>;

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

impl From<bcs::Error> for SuiError {
    fn from(value: bcs::Error) -> Self {
        Self::InvalidData(value.to_string())
    }
}
