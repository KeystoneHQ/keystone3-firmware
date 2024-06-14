use alloc::format;
use alloc::string::{String, ToString};
use keystore::errors::KeystoreError;
use third_party::thiserror::Error;
use third_party::{serde_json, thiserror};

#[derive(Error, Debug, PartialEq)]
pub enum ArweaveError {
    #[error("invalid hd_path: {0}")]
    InvalidHDPath(String),
    #[error("keystore operation failed, reason: {0}")]
    KeystoreError(String),
    #[error("sign failed, reason: {0}")]
    SignFailure(String),
    #[error("Could not parse transaction, reason: `{0}`")]
    ParseTxError(String),
    #[error("This content is not supported yet")]
    NotSupportedError,
    #[error("Decode Avro value failed: {0}")]
    AvroError(String),
    #[error("Transaction is not a AO transaction")]
    NotAOTransaction,
}

pub type Result<T> = core::result::Result<T, ArweaveError>;

impl From<KeystoreError> for ArweaveError {
    fn from(value: KeystoreError) -> Self {
        match value {
            KeystoreError::DerivePubKey(data) => Self::KeystoreError(format!("{}", data)),
            KeystoreError::InvalidDerivationPath(data) => Self::KeystoreError(format!("{}", data)),
            KeystoreError::XPubError(data) => Self::KeystoreError(data),
            KeystoreError::SeedError(data) => Self::KeystoreError(data),
            KeystoreError::DerivationError(data) => Self::KeystoreError(data),
            KeystoreError::GenerateSigningKeyError(data) => Self::KeystoreError(data),
            KeystoreError::RSASignError => Self::KeystoreError("rsa sign error".to_string()),
            KeystoreError::RSAVerifyError => Self::KeystoreError("rsa verify error".to_string()),
            _ => Self::KeystoreError(value.to_string()),
        }
    }
}

impl From<serde_json::Error> for ArweaveError {
    fn from(value: serde_json::Error) -> Self {
        Self::ParseTxError(format!(
            "serde json operation failed {:?}",
            value.to_string()
        ))
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use keystore::errors::KeystoreError;

    #[test]
    fn test_invalid_hd_path_error() {
        let error = ArweaveError::InvalidHDPath("Invalid path".to_string());
        assert_eq!(error.to_string(), "invalid hd_path: Invalid path");
    }

    #[test]
    fn test_keystore_error() {
        let error = ArweaveError::KeystoreError("Keystore failure".to_string());
        assert_eq!(
            error.to_string(),
            "keystore operation failed, reason: Keystore failure"
        );
    }

    #[test]
    fn test_sign_failure_error() {
        let error = ArweaveError::SignFailure("Sign failed".to_string());
        assert_eq!(error.to_string(), "sign failed, reason: Sign failed");
    }

    #[test]
    fn test_parse_tx_error() {
        let error = ArweaveError::ParseTxError("Parse failed".to_string());
        assert_eq!(
            error.to_string(),
            "Could not parse transaction, reason: `Parse failed`"
        );
    }

    #[test]
    fn test_from_keystore_error() {
        let keystore_error = KeystoreError::DerivePubKey("Derive pub key error".to_string());
        let error: ArweaveError = keystore_error.into();
        assert_eq!(
            error.to_string(),
            "keystore operation failed, reason: Derive pub key error"
        );
    }

    #[test]
    fn test_from_serde_json_error() {
        let json_str = "invalid json";
        let json_error = serde_json::from_str::<serde_json::Value>(json_str).unwrap_err();
        let error: ArweaveError = json_error.into();
        assert_eq!(
            error.to_string(),
            "Could not parse transaction, reason: `serde json operation failed \"expected value at line 1 column 1\"`"
        );
    }
}
