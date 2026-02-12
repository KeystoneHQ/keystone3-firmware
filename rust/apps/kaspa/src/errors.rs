use alloc::string::String;
use thiserror::Error;

pub type Result<T> = core::result::Result<T, KaspaError>;

#[derive(Error, Debug, Clone, PartialEq, Eq)]
pub enum KaspaError {
    #[error("Invalid PSBT: {0}")]
    InvalidPsbt(String),

    #[error("Invalid PSKT: {0}")]
    InvalidPskt(String),

    #[error("Invalid transaction: {0}")]
    InvalidTransaction(String),

    #[error("Invalid address: {0}")]
    InvalidAddress(String),

    #[error("Invalid derivation path: {0}")]
    InvalidDerivationPath(String),

    #[error("Invalid input: {0}")]
    InvalidInput(String),

    #[error("Invalid output: {0}")]
    InvalidOutput(String),
    
    #[error("Invalid index: {0}")]
    InvalidIndex(String),

    #[error("Sign failure: {0}")]
    SignFailure(String),

    #[error("Derive public key error: {0}")]
    DerivePublicKeyError(String),

    #[error("Unsupported: {0}")]
    Unsupported(String),

    #[error("Serialization failed: {0}")]
    SerializationFailed(String),

    #[error("PSKT not finalized")]
    PsktNotFinalized,

    #[error("Unsupported sighash type. Hardware wallet only supports SIG_HASH_ALL (0x01) for security")]
    UnsupportedSighashType,

    #[error("Hex decode error")]
    HexDecodeError,

    #[error("Invalid master fingerprint format")]
    InvalidMasterFingerprint,

    #[error("Invalid transaction hash")]
    InvalidTransactionHash,

    #[error("Invalid script format")]
    InvalidScript,

    #[error("UTF-8 decode error")]
    Utf8DecodeError,

    #[error("JSON parse error")]
    JsonParseError,

    #[error("Invalid path format")]
    InvalidPathFormat,

    #[error("Master key creation failed")]
    MasterKeyCreationFailed,

    #[error("Key derivation failed: {0}")]
    KeyDerivationFailed(String),
}
