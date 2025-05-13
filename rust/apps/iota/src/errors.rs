use alloc::format;
use alloc::string::{String, ToString};
use keystore::errors::KeystoreError;
use thiserror;
use thiserror::Error;

#[derive(Error, Debug)]
pub enum IotaError {
    #[error("meet error when encoding address: {0}")]
    AddressError(String),
    #[error("keystore operation failed, reason: {0}")]
    KeystoreError(String),

    #[error("Program `{0}` is not supported yet")]
    UnsupportedProgram(String),

    #[error("Meet invalid data when reading `{0}`")]
    InvalidData(String),

    #[error("Error occurred when parsing program instruction, reason: `{0}`")]
    ProgramError(String),

    #[error("Could not found account for `{0}`")]
    AccountNotFound(String),

    #[error("Could not parse transaction, reason: `{0}`")]
    ParseTxError(String),

    #[error("Invalid field: `{0}`")]
    InvalidField(String),

    #[error("Unexpected EOF")]
    UnexpectedEof,

    #[error("Invalid length")]
    InvalidLength,

    #[error("Invalid command: {0}")]
    InvalidCommand(u8),
}

pub type Result<T> = core::result::Result<T, IotaError>;
impl From<KeystoreError> for IotaError {
    fn from(value: KeystoreError) -> Self {
        Self::KeystoreError(value.to_string())
    }
}

impl From<hex::FromHexError> for IotaError {
    fn from(value: hex::FromHexError) -> Self {
        Self::InvalidData(format!("hex operation failed {}", value))
    }
}
