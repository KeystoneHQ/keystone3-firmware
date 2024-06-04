use alloc::format;
use alloc::string::{String, ToString};
use keystore::errors::KeystoreError;
use third_party::thiserror::Error;
use third_party::{serde_json, thiserror};

#[derive(Error, Debug, PartialEq)]
pub enum ICPError {
    #[error("invalid hd_path: {0}")]
    InvalidHDPath(String),
    #[error("keystore operation failed, reason: {0}")]
    KeystoreError(String),
    #[error("sign failed, reason: {0}")]
    SignFailure(String),
    #[error("Could not parse transaction, reason: `{0}`")]
    ParseTxError(String), 
}

pub type Result<T> = core::result::Result<T, ICPError>;

impl From<KeystoreError> for ICPError {
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

impl From<serde_json::Error> for ICPError {
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
        let error = ICPError::InvalidHDPath("Invalid path".to_string());
        assert_eq!(error.to_string(), "invalid hd_path: Invalid path");
    }

    #[test]
    fn test_keystore_error() {
        let error = ICPError::KeystoreError("Keystore failure".to_string());
        assert_eq!(
            error.to_string(),
            "keystore operation failed, reason: Keystore failure"
        );
    }

    #[test]
    fn test_sign_failure_error() {
        let error = ICPError::SignFailure("Sign failed".to_string());
        assert_eq!(error.to_string(), "sign failed, reason: Sign failed");
    }

    #[test]
    fn test_from_keystore_error() {
        let keystore_error = KeystoreError::DerivePubKey("Derive pub key error".to_string());
        let error: ICPError = keystore_error.into();
        assert_eq!(
            error.to_string(),
            "keystore operation failed, reason: Derive pub key error"
        );
    }
}
