use alloc::string::String;
use thiserror;
use thiserror::Error;

pub type Result<T> = core::result::Result<T, KeystoreError>;

#[derive(Error, Debug, PartialEq)]
pub enum KeystoreError {
    #[error("failed to derive pubkey, {0}")]
    DerivePubKey(String),
    #[error("invalid derivation path, {0}")]
    InvalidDerivationPath(String),
    #[error("invalid seed, {0}")]
    SeedError(String),
    #[error("invalid xpub, {0}")]
    XPubError(String),
    #[error("derivation error: {0}")]
    DerivationError(String),
    #[error("GenerateSigningKeyError: {0}")]
    GenerateSigningKeyError(String),
    #[error("RSASignError")]
    RSASignError,
    #[error("RSAVerifyError")]
    RSAVerifyError,
    #[error("InvalidDataError: {0}")]
    InvalidDataError(String),
}
