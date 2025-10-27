use alloc::format;
use alloc::string::String;
use core::str::Utf8Error;
use keystore::errors::KeystoreError;
use serde_json::Error;
use thiserror::Error;
use {hex, serde_json, thiserror};

#[derive(Error, Debug, PartialEq)]
pub enum XRPError {
    #[error("invalid hd_path: {0}")]
    InvalidHDPath(String),
    #[error("keystore operation failed, reason: {0}")]
    KeystoreError(String),
    #[error("sign failed, reason: {0}")]
    SignFailure(String),
    #[error("Meet invalid data when reading `{0}`")]
    InvalidData(String),
    #[error("Could not parse transaction, reason: `{0}`")]
    ParseTxError(String),
}

pub type R<T> = Result<T, XRPError>;

impl From<KeystoreError> for XRPError {
    fn from(value: KeystoreError) -> Self {
        Self::KeystoreError(format!("{value}"))
    }
}

impl From<Utf8Error> for XRPError {
    fn from(value: Utf8Error) -> Self {
        Self::InvalidData(format!("utf8 operation failed {value}"))
    }
}

impl From<hex::FromHexError> for XRPError {
    fn from(value: hex::FromHexError) -> Self {
        Self::InvalidData(format!("hex operation failed {value}"))
    }
}

impl From<serde_json::Error> for XRPError {
    fn from(value: Error) -> Self {
        Self::InvalidData(format!("serde_json operation failed {value}"))
    }
}

impl From<bitcoin::bip32::Error> for XRPError {
    fn from(value: bitcoin::bip32::Error) -> Self {
        Self::InvalidData(format!("bip32 operation failed {value}"))
    }
}
