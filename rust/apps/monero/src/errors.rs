use alloc::string::{String, ToString};
use keystore::errors::KeystoreError;
use thiserror::Error;
use thiserror;

#[derive(Error, Debug, PartialEq)]
pub enum MoneroError {
    #[error("keystore operation failed, reason: {0}")]
    KeystoreError(String),
    #[error("invalid prefix: {0}")]
    InvalidPrefix(String),
    #[error("Base58 decode error")]
    Base58DecodeError,
    #[error("format error")]
    FormatError,
    #[error("unknown network")]
    UnknownNetwork,
    #[error("pub_keyring length error")]
    PubKeyringLengthError,
    #[error("public key from_bytes error")]
    PublicKeyFromBytesError,
    #[error("generate_keypair error")]
    GenerateKeypairError,
    #[error("generate key image error")]
    GenerateKeyImageError,
    #[error("decrypt error: Invalid signature")]
    DecryptInvalidSignature,
    #[error("unsupported input type")]
    UnsupportedInputType,
}

pub type Result<T> = core::result::Result<T, MoneroError>;

impl From<KeystoreError> for MoneroError {
    fn from(value: KeystoreError) -> Self {
        match value {
            _ => Self::KeystoreError(value.to_string()),
        }
    }
}
