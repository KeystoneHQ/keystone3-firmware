use thiserror::Error;

#[derive(Debug, Error)]
pub enum OTAError {
    #[error("file not exist")]
    NotExist,
    #[error("invalid header")]
    InvalidHeader,
    #[error("io error")]
    IoError(std::io::Error),
}
