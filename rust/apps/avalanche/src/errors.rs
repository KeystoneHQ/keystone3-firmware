use alloc::string::{String, ToString};
use core2::io;
use keystore::errors::KeystoreError;
use thiserror;
use thiserror::Error;

pub type Result<T> = core::result::Result<T, AvaxError>;

#[derive(Error, Debug, PartialEq)]
pub enum AvaxError {
    #[error("invalid input")]
    InvalidInput,
    
    #[error("invalid output")]
    InvalidOutput,
    
    #[error("invalid transaction, reason: {0}")]
    InvalidTransaction(String),
    
    #[error("sign failed, reason: {0}")]
    SignFailure(String),
    
    #[error("get addresses failed, reason: {0}")]
    AddressError(String),
    
    #[error("get key error: {0}")]
    GetKeyError(String),
    
    #[error("unsupported: {0}")]
    UnsupportedTransaction(String),
    
    #[error("unsupported network")]
    UnsupportedNetwork(String),
    
    #[error("consensus encode error, reason: {0}")]
    TransactionConsensusEncodeError(String),
    
    #[error("invalid hex: {0}")]
    InvalidHex(String),
    
    #[error("bech32 decode failed, reason: {0}")]
    Bech32DecodeError(String),
    
    #[error("keystore operation failed, reason: {0}")]
    KeystoreError(String),
    
    #[error("derive public key error: {0}")]
    DerivePublicKeyError(String),

    #[error("unknown type id: {0}")]
    UnknownTypeId(u32),

    #[error("Invalid hd_Path: {0}")]
    InvalidHDPath(String),
}

impl From<io::Error> for AvaxError {
    fn from(value: io::Error) -> Self {
        Self::TransactionConsensusEncodeError(format!("{}", value))
    }
}

impl From<bech32::segwit::DecodeError> for AvaxError {
    fn from(value: bech32::segwit::DecodeError) -> Self {
        Self::Bech32DecodeError(format!("{}", value))
    }
}


impl From<KeystoreError> for AvaxError {
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
