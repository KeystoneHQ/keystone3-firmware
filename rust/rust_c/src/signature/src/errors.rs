use alloc::string::String;
use thiserror::Error;

pub type Result<T> = core::result::Result<T, RustCError>;

#[derive(Error, Debug, PartialEq)]
pub enum RustCError {
    #[error("FormatTypeError, type is {0}")]
    FormatTypeError(String),
}
