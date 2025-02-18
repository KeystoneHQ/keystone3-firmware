use thiserror::Error;

#[derive(Debug, Error)]
pub enum OTAError {
    #[error("file not exist")]
    NotExist,
    #[error("invalid format")]
    InvalidFormat,
    #[error("decompression error")]
    DecompressionError,
}