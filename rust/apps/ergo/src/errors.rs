use alloc::string::{String, ToString};
use thiserror;
use thiserror::Error;

#[derive(Error, Debug)]
pub enum ErgoError {
    #[error("Unknown error")]
    UnknownError,
    #[error("meet error when derive ergo key, {0}")]
    DerivationError(String),
    #[error("Invalid Ergo Mnemonic, {0}")]
    MnemonicError(String),
}

#[derive(Debug, Error)]
pub enum MnemonicError {
    #[error("Invalid seed")]
    InvalidSeed(String),
}

pub type Result<T> = core::result::Result<T, ErgoError>;

impl From<MnemonicError> for ErgoError {
    fn from(value: MnemonicError) -> Self {
        ErgoError::MnemonicError(value.to_string())
    }
}
