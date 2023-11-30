use alloc::string::{String, ToString};
use core::array::TryFromSliceError;
use third_party::hex;
use third_party::thiserror;
use third_party::thiserror::Error;

#[derive(Error, Debug)]
pub enum StellarError {
    #[error("sui transaction parse failed, reason: `{0}`")]
    ParseTxError(String),
    #[error("sign failed, reason: {0}")]
    SignFailure(String),
    #[error("Meet invalid data when reading `{0}`")]
    InvalidData(String),
    #[error("Invalid xpub: `{0}`")]
    InvalidXPub(String),
}

pub type Result<T> = core::result::Result<T, StellarError>;

impl From<hex::FromHexError> for StellarError {
    fn from(value: hex::FromHexError) -> Self {
        Self::InvalidData(format!("hex operation failed {}", value))
    }
}

impl From<TryFromSliceError> for StellarError {
    fn from(value: TryFromSliceError) -> Self {
        Self::InvalidXPub(value.to_string())
    }
}

impl From<stellar_xdr::curr::Error> for StellarError {
    fn from(value: stellar_xdr::curr::Error) -> Self {
        Self::ParseTxError(value.to_string())
    }
}
