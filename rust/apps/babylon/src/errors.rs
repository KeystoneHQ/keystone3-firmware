use alloc::string::String;
use thiserror::Error;

pub type Result<T> = core::result::Result<T, BabylonError>;

#[derive(Error, Debug, Clone, PartialEq, Eq)]
pub enum BabylonError {
    #[error("app not allowed: {0}")]
    AppNotAllowed(String),
    #[error("invalid app name: {0}")]
    InvalidAppName(String),
    #[error("invalid context: {0}")]
    InvalidContext(String),
    #[error("unsupported network: {0}")]
    UnsupportedNetwork(String),
    #[error("derivation failed: {0}")]
    DeriveError(String),
}
