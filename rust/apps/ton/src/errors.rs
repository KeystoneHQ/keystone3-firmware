use alloc::string::{String, ToString};
use third_party::{serde_json, thiserror};
use third_party::thiserror::Error;

use crate::vendor::cell::TonCellError;

pub type Result<T: Sized> = core::result::Result<T, TonError>;

#[derive(Error, Debug)]
pub enum TonError {
    #[error("Unknown error")]
    UnknownError,
    #[error("Invalid TON Mnemonic, {0}")]
    MnemonicError(String),
    #[error("Error generate ton address, {0}")]
    AddressError(String),
    #[error("Invalid TON Transaction, {0}")]
    TransactionError(#[from] TonCellError),
    #[error("Invalid TON Transaction, {0}")]
    InvalidTransaction(String),
    #[error("Convert Transaction Json error: {0}")]
    TransactionJsonError(String),
}

#[derive(Debug, Error)]
pub(crate) enum MnemonicError {
    #[error("Invalid mnemonic word count (count: {0})")]
    UnexpectedWordCount(usize),

    #[error("Invalid mnemonic word (word: {0})")]
    InvalidWord(String),

    #[error("Invalid mnemonic with password (first byte: {0:#X})")]
    InvalidFirstByte(u8),

    #[error("Invalid passwordless mnemonic (first byte: {0:#X})")]
    InvalidPasswordlessMenmonicFirstByte(u8),
}

impl From<MnemonicError> for TonError {
    fn from(value: MnemonicError) -> Self {
        TonError::MnemonicError(value.to_string())
    }
}