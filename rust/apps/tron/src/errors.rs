use alloc::string::{String, ToString};
use bitcoin::base58::Error as Base58Error;
use keystore::errors::KeystoreError;
use thiserror;
use thiserror::Error;

#[derive(Error, Debug, PartialEq)]
pub enum TronError {
    #[error("invalid hd_path: {0}")]
    InvalidHDPath(String),
    #[error("keystore operation failed, reason: {0}")]
    KeystoreError(String),
    #[error("raw Transaction crypto bytes has invalid data, field: {0}")]
    InvalidRawTxCryptoBytes(String),
    #[error("raw Transaction Input has invalid field: {0}")]
    InvalidParseContext(String),
    #[error("base58 operation failed, reason: {0}")]
    Base58Error(String),
    #[error("sign failed, reason: {0}")]
    SignFailure(String),
    #[error("protobuf operation failed, reason: {0}")]
    ProtobufError(String),
    #[error("parse number failed, reason: {0}")]
    ParseNumberError(String),
    #[error("from address does not belongs to current wallet")]
    NoMyInputs,
}

pub type Result<T> = core::result::Result<T, TronError>;

impl From<KeystoreError> for TronError {
    fn from(value: KeystoreError) -> Self {
        match value {
            KeystoreError::DerivePubKey(data) => Self::KeystoreError(data.to_string()),
            KeystoreError::InvalidDerivationPath(data) => Self::KeystoreError(data.to_string()),
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

impl From<Base58Error> for TronError {
    fn from(value: Base58Error) -> Self {
        Self::Base58Error(value.to_string())
    }
}

impl From<core::num::ParseFloatError> for TronError {
    fn from(value: core::num::ParseFloatError) -> Self {
        TronError::ParseNumberError(value.to_string())
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use bitcoin::base58;
    use keystore::errors::KeystoreError;

    #[test]
    fn test_comprehensive_error_conversions() {
        let ks_cases = vec![
            KeystoreError::RSASignError,
            KeystoreError::XPubError("xpub".to_string()),
        ];
        for ks_err in ks_cases {
            let tr_err: TronError = ks_err.into();
            assert!(format!("{}", tr_err).contains("keystore"));
        }

        let base58_err = base58::decode_check("123").unwrap_err();
        let tr_base58_err = TronError::from(base58_err);
        assert!(format!("{}", tr_base58_err).contains("base58"));

        let float_err = "not_a_number".parse::<f64>().unwrap_err();
        let tr_float_err: TronError = float_err.into();
        assert!(matches!(tr_float_err, TronError::ParseNumberError(_)));

        let all_variants = vec![
            TronError::InvalidHDPath("path".to_string()),
            TronError::NoMyInputs,
            TronError::Base58Error("b58".to_string()),
        ];

        for err in all_variants {
            let _ = format!("{}", err);
            assert_eq!(err, err);
        }
    }
}
