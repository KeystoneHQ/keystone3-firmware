use alloc::string::{String, ToString};
use keystore::errors::KeystoreError;
use thiserror::Error;
use {hex, thiserror};

#[derive(Error, Debug)]
pub enum CosmosError {
    #[error("cosmos transaction parse failed, reason: `{0}`")]
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
}

pub type Result<T> = core::result::Result<T, CosmosError>;

impl From<core::fmt::Error> for CosmosError {
    fn from(value: core::fmt::Error) -> Self {
        Self::InvalidAddressError(format!("bech32 encode error {:?}", value.to_string()))
    }
}

impl From<bech32::EncodeError> for CosmosError {
    fn from(value: bech32::EncodeError) -> Self {
        Self::InvalidAddressError(format!("bech32 encode error {:?}", value.to_string()))
    }
}

impl From<KeystoreError> for CosmosError {
    fn from(value: KeystoreError) -> Self {
        Self::KeystoreError(value.to_string())
    }
}

impl From<hex::FromHexError> for CosmosError {
    fn from(value: hex::FromHexError) -> Self {
        Self::InvalidData(format!("hex operation failed {}", value))
    }
}

impl From<serde_json::Error> for CosmosError {
    fn from(value: serde_json::Error) -> Self {
        Self::InvalidData(format!("serde_json operation failed {}", value))
    }
}

impl From<core::num::ParseFloatError> for CosmosError {
    fn from(value: core::num::ParseFloatError) -> Self {
        CosmosError::InvalidData(format!("parse float failed {}", value))
    }
}
