use thiserror::Error;

#[derive(Debug, Error)]
pub enum OTAError {

    #[error("file not exist")]
    NotExist,

    #[error("chunck data error")]
    ChunckError,

    #[error("header generation error")]
    HeaderError,
    
}