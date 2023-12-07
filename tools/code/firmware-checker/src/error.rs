use thiserror::Error;

#[derive(Debug, Error)]
pub enum OTAError {
    #[error("file not exist")]
    NotExist,
}