use alloc::string::{String, ToString};
use thiserror;
use thiserror::Error;

use crate::vendor::cell::TonCellError;

pub type Result<T> = core::result::Result<T, TonError>;

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
    #[error("Invalid TON Proof, {0}")]
    InvalidProof(String),
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

#[cfg(test)]
mod tests {
    extern crate std;

    use alloc::string::ToString;
    use super::{MnemonicError, TonError};
    use crate::vendor::cell::TonCellError;

    #[test]
    fn test_mnemonic_error_conversion_preserves_context() {
        let cases = [
            (
                MnemonicError::UnexpectedWordCount(11),
                "Invalid mnemonic word count (count: 11)",
            ),
            (
                MnemonicError::InvalidWord("foobar".to_string()),
                "Invalid mnemonic word (word: foobar)",
            ),
            (
                MnemonicError::InvalidFirstByte(0xAB),
                "Invalid mnemonic with password (first byte: 0xAB)",
            ),
            (
                MnemonicError::InvalidPasswordlessMenmonicFirstByte(0xCD),
                "Invalid passwordless mnemonic (first byte: 0xCD)",
            ),
        ];

        for (mnemonic_err, expected) in cases {
            let ton_err: TonError = mnemonic_err.into();
            match ton_err {
                TonError::MnemonicError(message) => assert_eq!(message, expected),
                _ => panic!("expected mnemonic error"),
            }
        }
    }

    #[test]
    fn test_transaction_error_conversion_from_ton_cell_error() {
        let ton_err: TonError = TonCellError::CellParserError("bad payload".to_string()).into();
        match ton_err {
            TonError::TransactionError(TonCellError::CellParserError(message)) => {
                assert_eq!(message, "bad payload")
            }
            _ => panic!("expected transaction error"),
        }
    }
}

