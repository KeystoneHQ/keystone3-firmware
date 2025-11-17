use alloc::string::String;
use thiserror;
use thiserror::Error;
use zcash_vendor::{orchard, transparent};

pub type Result<T> = core::result::Result<T, ZcashError>;

#[derive(Error, Debug, PartialEq)]
pub enum ZcashError {
    #[error("failed to generate zcash address, {0}")]
    GenerateAddressError(String),
    #[error("invalid zcash data: {0}")]
    InvalidDataError(String),
    #[error("failed to sign zcash data, {0}")]
    SigningError(String),
    #[error("invalid pczt, {0}")]
    InvalidPczt(String),
}

impl From<orchard::pczt::ParseError> for ZcashError {
    fn from(e: orchard::pczt::ParseError) -> Self {
        Self::InvalidPczt(alloc::format!("Invalid Orchard bundle: {e:?}"))
    }
}

impl From<transparent::pczt::ParseError> for ZcashError {
    fn from(e: transparent::pczt::ParseError) -> Self {
        Self::InvalidPczt(alloc::format!("Invalid transparent bundle: {e:?}"))
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    extern crate std;
    use alloc::string::ToString;

    #[test]
    fn test_error_display() {
        let error = ZcashError::GenerateAddressError("test error".to_string());
        assert_eq!(
            error.to_string(),
            "failed to generate zcash address, test error"
        );

        let error = ZcashError::InvalidDataError("invalid data".to_string());
        assert_eq!(error.to_string(), "invalid zcash data: invalid data");

        let error = ZcashError::SigningError("signing failed".to_string());
        assert_eq!(
            error.to_string(),
            "failed to sign zcash data, signing failed"
        );

        let error = ZcashError::InvalidPczt("invalid pczt".to_string());
        assert_eq!(error.to_string(), "invalid pczt, invalid pczt");
    }

    #[test]
    fn test_error_equality() {
        let error1 = ZcashError::GenerateAddressError("test".to_string());
        let error2 = ZcashError::GenerateAddressError("test".to_string());
        let error3 = ZcashError::GenerateAddressError("different".to_string());

        assert_eq!(error1, error2);
        assert_ne!(error1, error3);
    }

    #[test]
    fn test_error_debug() {
        let error = ZcashError::InvalidDataError("debug test".to_string());
        let debug_str = std::format!("{error:?}");
        assert!(debug_str.contains("InvalidDataError"));
        assert!(debug_str.contains("debug test"));
    }
}
