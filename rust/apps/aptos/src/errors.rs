use alloc::format;
use alloc::string::{FromUtf8Error, String, ToString};
use keystore::errors::KeystoreError;
use third_party::{hex, thiserror};
use thiserror::Error;

#[derive(Error, Debug)]
pub enum AptosError {
    #[error("keystore operation failed, reason: {0}")]
    KeystoreError(String),
    #[error("Could not parse transaction, reason: `{0}`")]
    ParseTxError(String),
    #[error("sign failed, reason: {0}")]
    SignFailure(String),
    #[error("Meet invalid data when reading `{0}`")]
    InvalidData(String),
}

pub type Result<T> = core::result::Result<T, AptosError>;

impl From<KeystoreError> for AptosError {
    fn from(value: KeystoreError) -> Self {
        Self::KeystoreError(value.to_string())
    }
}

impl From<hex::FromHexError> for AptosError {
    fn from(value: hex::FromHexError) -> Self {
        Self::InvalidData(format!("hex operation failed {}", value.to_string()))
    }
}

impl From<FromUtf8Error> for AptosError {
    fn from(value: FromUtf8Error) -> Self {
        Self::InvalidData(format!("utf8 operation failed {}", value.to_string()))
    }
}

impl From<core::num::ParseIntError> for AptosError {
    fn from(value: core::num::ParseIntError) -> Self {
        Self::InvalidData(format!("parseInt Failed {}", value))
    }
}

impl From<bcs::Error> for AptosError {
    fn from(value: bcs::Error) -> Self {
        Self::InvalidData(format!("bsc operation failed {}", value.to_string()))
    }
}
