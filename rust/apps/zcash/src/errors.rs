use alloc::string::{String, ToString};
use third_party::thiserror;
use thiserror::Error;

pub type Result<T> = core::result::Result<T, ZcashError>;

#[derive(Error, Debug, PartialEq)]
pub enum ZcashError {
    #[error("failed to generate zcash address, {0}")]
    GenerateAddressError(String),
    #[error("invalid zcash data: {0}")]
    InvalidDataError(String),
    #[error("invalid pczt transaction data: {0}")]
    InvalidPcztError(String),
}

impl From<pczt::protos::prost::DecodeError> for ZcashError {
    fn from(value: pczt::protos::prost::DecodeError) -> Self {
        ZcashError::InvalidPcztError(value.to_string())
    }
}
