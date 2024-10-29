use alloc::string::{String, ToString};
use keystore::errors::KeystoreError;
use rlp::DecoderError;

use thiserror;
use thiserror::Error;

pub type Result<T> = core::result::Result<T, EthereumError>;

#[derive(Error, Debug)]
pub enum EthereumError {
    #[error("Rlp Decoding error: {0}")]
    RlpDecodingError(String),
    #[error("Invalid Transaction")]
    InvalidTransaction,
    #[error("sign failed, reason: {0}")]
    SignFailure(String),
    #[error("Invalid hd_Path: {0}")]
    InvalidHDPath(String),
    #[error("KeystoreError: {0}")]
    KeystoreError(String),
    #[error("Invalid Address: {0}")]
    InvalidAddressError(String),
    #[error("Invalid utf-8 sequence: {0}")]
    InvalidUtf8Error(String),
    #[error("Invalid contract abi")]
    InvalidContractABI,
    #[error("Decode Contract data error: {0}")]
    DecodeContractDataError(String),
    #[error("Invalid TypedData message error: {0} message: {1}")]
    InvalidTypedData(String, String),
    #[error("Hash TypedData error: {0}")]
    HashTypedDataError(String),
}

impl From<DecoderError> for EthereumError {
    fn from(value: DecoderError) -> Self {
        EthereumError::RlpDecodingError(value.to_string())
    }
}

impl From<KeystoreError> for EthereumError {
    fn from(value: KeystoreError) -> Self {
        EthereumError::KeystoreError(value.to_string())
    }
}
