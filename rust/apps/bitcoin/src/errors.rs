use alloc::string::{String, ToString};
use keystore::errors::KeystoreError;
use third_party::bitcoin::base58::Error as Base58Error;
use third_party::bitcoin::hashes::hex::Error as HashHexError;
use third_party::bitcoin::script::PushBytesError;
use third_party::core2::io;
use third_party::thiserror;
use third_party::thiserror::Error;

pub type Result<T> = core::result::Result<T, BitcoinError>;

#[derive(Error, Debug, PartialEq)]
pub enum BitcoinError {
    #[error("invalid input")]
    InvalidInput,
    #[error("invalid output")]
    InvalidOutput,
    #[error("cannot deserialize psbt, {0}")]
    InvalidPsbt(String),
    #[error("invalid transaction, reason: {0}")]
    InvalidTransaction(String),
    #[error("PSBT has no input")]
    NoInputs,
    #[error("PSBT has no output")]
    NoOutputs,
    #[error("PSBT does not have any input associated with current wallet")]
    NoMyInputs,
    #[error("input value may be tampered, {0}")]
    InputValueTampered(String),
    #[error("sign failed, reason: {0}")]
    SignFailure(String),
    #[error("get addresses failed, reason: {0}")]
    AddressError(String),
    #[error("get key error: {0}")]
    GetKeyError(String),
    #[error("sign legacy transaction failed, reason: {0}")]
    SignLegacyTxError(String),
    #[error("unsupported: {0}")]
    UnsupportedTransaction(String),
    #[error("unsupported")]
    UnsupportedNetwork(String),
    #[error("consensus encode error, reason: {0}")]
    TransactionConsensusEncodeError(String),
    #[error("push bytes failed, reason: {0}")]
    PushBytesFailed(String),
    #[error("hash hex failed, reason: {0}")]
    InvalidHashLength(String),
    #[error("base58 operation failed, reason: {0}")]
    Base58Error(String),
    #[error("keystore operation failed, reason: {0}")]
    KeystoreError(String),
    #[error("Raw Transaction crypto bytes has invalid data, field: {0}")]
    InvalidRawTxCryptoBytes(String),
    #[error("Raw Transaction Input has invalid field: {0}")]
    InvalidParseContext(String),
    #[error("Invalid TxData, field: {0}")]
    InvalidTxData(String),
    #[error("Invalid script type, field: {0}")]
    UnsupportedScriptType(String),
}

impl From<io::Error> for BitcoinError {
    fn from(value: io::Error) -> Self {
        Self::TransactionConsensusEncodeError(format!("{}", value))
    }
}

impl From<PushBytesError> for BitcoinError {
    fn from(value: PushBytesError) -> Self {
        Self::PushBytesFailed(format!("{}", value))
    }
}

impl From<HashHexError> for BitcoinError {
    fn from(value: HashHexError) -> Self {
        Self::InvalidHashLength(format!("{}", value))
    }
}

impl From<Base58Error> for BitcoinError {
    fn from(value: Base58Error) -> Self {
        match value {
            Base58Error::BadByte(byte) => Self::Base58Error(format!("bad bytes: {}", byte)),
            Base58Error::TooShort(size) => Self::Base58Error(format!("too short: {}", size)),
            Base58Error::InvalidLength(size) => {
                Self::Base58Error(format!("invalid length: {}", size))
            }
            Base58Error::BadChecksum(expected, actual) => Self::Base58Error(format!(
                "bad checksum, expected {}, actual {}",
                expected, actual
            )),
            _ => Self::Base58Error(format!(": {}", value)),
        }
    }
}

impl From<KeystoreError> for BitcoinError {
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
