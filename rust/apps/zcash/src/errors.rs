use alloc::string::{String, ToString};
use thiserror;
use thiserror::Error;

pub type Result<T> = core::result::Result<T, ZcashError>;

#[derive(Error, Debug, PartialEq)]
pub enum ZcashError {
    #[error("failed to generate zcash address, {0}")]
    GenerateAddressError(String),
    #[error("invalid zcash data: {0}")]
    InvalidDataError(String),
}