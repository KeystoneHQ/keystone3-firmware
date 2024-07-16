use alloc::string::{String, ToString};
use cardano_serialization_lib::error::DeserializeError;
use third_party::ed25519_bip32_core::DerivationError;
use third_party::thiserror;
use third_party::thiserror::Error;

#[derive(Error, Debug)]
pub enum CardanoError {
    #[error("meet error when encoding address: {0}")]
    AddressEncodingError(String),
    #[error("meet error when derive cardano key, {0}")]
    DerivationError(String),
    #[error("invalid transaction: {0}")]
    InvalidTransaction(String),
    #[error("unsupported transaction type: {0}")]
    UnsupportedTransaction(String),
    #[error("error occurs when signing cardano transaction: {0}")]
    SigningFailed(String),
}

pub type R<T> = Result<T, CardanoError>;

impl From<DeserializeError> for CardanoError {
    fn from(value: DeserializeError) -> Self {
        Self::InvalidTransaction(value.to_string())
    }
}

impl From<DerivationError> for CardanoError {
    fn from(value: DerivationError) -> Self {
        Self::DerivationError(value.to_string())
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_cardano_error() {
        let error = CardanoError::AddressEncodingError("test".to_string());
        assert_eq!(error.to_string(), "meet error when encoding address: test");
    }

    #[test]
    fn test_cardano_error2() {
        let error = CardanoError::InvalidTransaction("test".to_string());
        assert_eq!(error.to_string(), "invalid transaction: test");
    }
}
