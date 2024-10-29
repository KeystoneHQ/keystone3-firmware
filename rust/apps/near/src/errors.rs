use alloc::format;
use alloc::string::{String, ToString};
use keystore::errors::KeystoreError;
use thiserror::Error;
use {serde_json, thiserror};

#[derive(Error, Debug, PartialEq)]
pub enum NearError {
    #[error("invalid hd_path: {0}")]
    InvalidHDPath(String),
    #[error("keystore operation failed, reason: {0}")]
    KeystoreError(String),
    #[error("sign failed, reason: {0}")]
    SignFailure(String),
    #[error("Could not parse transaction, reason: `{0}`")]
    ParseTxError(String),
}

pub type Result<T> = core::result::Result<T, NearError>;

impl From<KeystoreError> for NearError {
    fn from(value: KeystoreError) -> Self {
        match value {
            KeystoreError::DerivePubKey(data) => Self::KeystoreError(format!("{}", data)),
            KeystoreError::InvalidDerivationPath(data) => Self::KeystoreError(format!("{}", data)),
            KeystoreError::XPubError(data) => Self::KeystoreError(data),
            KeystoreError::SeedError(data) => Self::KeystoreError(data),
            KeystoreError::DerivationError(data) => Self::KeystoreError(data),
            KeystoreError::GenerateSigningKeyError(data) => Self::KeystoreError(data),
            KeystoreError::RSASignError => Self::KeystoreError("rsa sign error".to_string()),
            KeystoreError::RSAVerifyError => Self::KeystoreError("rsa verify error".to_string()),
            _ => Self::KeystoreError(value.to_string()),
        }
    }
}

impl From<serde_json::Error> for NearError {
    fn from(value: serde_json::Error) -> Self {
        Self::ParseTxError(format!(
            "serde json operation failed {:?}",
            value.to_string()
        ))
    }
}
