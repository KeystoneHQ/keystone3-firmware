use alloc::format;
use alloc::string::{String, ToString};
use bitcoin::base58::Error as Base58Error;
use keystore::errors::KeystoreError;
use thiserror;
use thiserror::Error;

#[derive(Error, Debug, PartialEq)]
pub enum TronError {
    #[error("invalid hd_path: {0}")]
    InvalidHDPath(String),
    #[error("keystore operation failed, reason: {0}")]
    KeystoreError(String),
    #[error("raw Transaction crypto bytes has invalid data, field: {0}")]
    InvalidRawTxCryptoBytes(String),
    #[error("raw Transaction Input has invalid field: {0}")]
    InvalidParseContext(String),
    #[error("base58 operation failed, reason: {0}")]
    Base58Error(String),
    #[error("sign failed, reason: {0}")]
    SignFailure(String),
    #[error("protobuf operation failed, reason: {0}")]
    ProtobufError(String),
    #[error("parse number failed, reason: {0}")]
    ParseNumberError(String),
    #[error("from address does not belongs to current wallet")]
    NoMyInputs,
}

pub type Result<T> = core::result::Result<T, TronError>;

impl From<KeystoreError> for TronError {
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

impl From<Base58Error> for TronError {
    fn from(value: Base58Error) -> Self {
        Self::Base58Error(value.to_string())
    }
}

impl From<core::num::ParseFloatError> for TronError {
    fn from(value: core::num::ParseFloatError) -> Self {
        TronError::ParseNumberError(value.to_string())
    }
}
