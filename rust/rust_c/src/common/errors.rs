use alloc::string::String;
use keystore::errors::KeystoreError;
use thiserror;
use thiserror::Error;
use ur_registry::error::URError;

#[cfg(feature = "aptos")]
use app_aptos::errors::AptosError;
#[cfg(feature = "arweave")]
use app_arweave::errors::ArweaveError;
#[cfg(feature = "avalanche")]
use app_avalanche::errors::AvaxError;
#[cfg(feature = "bitcoin")]
use app_bitcoin::errors::BitcoinError;
#[cfg(feature = "cardano")]
use app_cardano::errors::CardanoError;
#[cfg(feature = "cosmos")]
use app_cosmos::errors::CosmosError;
#[cfg(feature = "ergo")]
use app_ergo::errors::ErgoError;
#[cfg(feature = "ethereum")]
use app_ethereum::errors::EthereumError;
#[cfg(feature = "monero")]
use app_monero::errors::MoneroError;
#[cfg(feature = "near")]
use app_near::errors::NearError;
#[cfg(feature = "solana")]
use app_solana::errors::SolanaError;
#[cfg(feature = "stellar")]
use app_stellar::errors::StellarError;
#[cfg(feature = "sui")]
use app_sui::errors::SuiError;
#[cfg(feature = "ton")]
use app_ton::errors::TonError;
#[cfg(feature = "tron")]
use app_tron::errors::TronError;
#[cfg(feature = "xrp")]
use app_xrp::errors::XRPError;
#[cfg(feature = "zcash")]
use app_zcash::errors::ZcashError;

#[derive(Debug, Clone)]
#[repr(C)]
pub enum ErrorCodes {
    Success = 0,

    // common errors,
    InvalidMasterFingerprint,
    MasterFingerprintMismatch,
    InvalidHDPath,
    UnsupportedTransaction,
    InvalidXPub,
    SignFailure,
    UnexpectedError,
    InvalidHex,
    WebAuthFailed,
    InvalidData,

    //UR errors
    CborDecodeError = 20,
    CborEncodeError,
    URDecodeError,
    UREncodeError,
    NotSupportURTypeError,
    NotAnUr,
    URTypeUnspecified,
    URProtobufDecodeError,
    URProtobufEncodeError,
    URGzipDecodeError,
    URGzipEnCodeError,

    //Keystore errors
    KeystoreSeedError = 40,
    KeystoreDerivationError,
    KeystoreXPubError,
    KeystoreInvalidDerivationPath,
    KeystoreDeivePubkey,
    KeystoreGenerateSigningKeyError,
    KeystoreRSASignError,
    KeystoreRSAVerifyError,
    KeystoreInvalidDataError,
    KeystoreZcashOrchardSignError,

    //Bitcoin errors
    BitcoinInvalidInput = 100,
    BitcoinInvalidOutput,
    BitcoinInvalidPsbt,
    BitcoinInvalidTransaction,
    BitcoinNoInputs,
    BitcoinNoOutputs,
    BitcoinNoMyInputs,
    BitcoinInputValueTampered,
    BitcoinSignFailure,
    BitcoinAddressError,
    BitcoinGetKeyError,
    BitcoinSignLegacyTxError,
    BitcoinUnsupportedTransaction,
    BitcoinUnsupportedNetwork,
    BitcoinTransactionConsensusEncodeError,
    BitcoinPushBytesFailed,
    BitcoinInvalidHex,
    BitcoinBase58Error,
    BitcoinKeystoreError,
    BitcoinInvalidParseContext,
    BitcoinInvalidRawTxCryptoBytes,
    BitcoinInvalidTxData,
    BitcoinUnsupportedScriptType,
    BitcoinBech32DecodeError,
    BitcoinWitnessProgramError,
    BitcoinMultiSigWalletParseError,
    BitcoinMultiSigWalletNotMyWallet,
    BitcoinMultiSigWalletAddressCalError,
    BitcoinMultiSigWalletImportXpubError,
    BitcoinMultiSigWalletCrateError,
    BitcoinMultiSigWalletFormatError,
    BitcoinMultiSigNetworkError,
    BitcoinMultiSigInputError,
    BitcoinDerivePublicKeyError,
    BitcoinWalletTypeError,

    //Ethereum
    EthereumRlpDecodingError = 200,
    EthereumInvalidTransaction,
    EthereumSignFailure,
    EthereumInvalidHDPath,
    EthereumKeystoreError,
    EthereumInvalidAddressError,
    EthereumInvalidUtf8Error,
    EthereumInvalidContractABI,
    EthereumDecodeContractDataError,
    EthereumInvalidTypedData,
    EthereumHashTypedDataError,

    //Tron
    TronInvalidRawTxCryptoBytes = 300,
    TronInvalidParseContext,
    TronBase58Error,
    TronSignFailure,
    TronProtobufError,
    TronParseNumberError,
    TronNoMyInputs,

    //CompanionApp
    CompanionAppProtobufError = 400,
    CompanionAppInvalidParseContext,
    CompanionAppSignTxFailed,
    CompanionAppCheckTxFailed,

    //Cardano
    CardanoInvalidTransaction = 500,
    CardanoAddressEncodingError,

    //Solana
    SolanaAddressEncodingError = 600,
    SolanaKeystoreError,
    SolanaUnsupportedProgram,
    SolanaInvalidData,
    SolanaAccountNotFound,
    SolanaProgramError,
    SolanaParseTxError,

    // Near
    NearKeystoreError = 700,
    NearSignFailure,
    NearParseTxError,

    // XRP
    XRPSignFailure = 800,
    XRPInvalidData,
    XRPParseTxError,

    // Cosmos
    CosmosSignFailure = 900,
    CosmosKeystoreError,
    CosmosInvalidData,
    CosmosParseTxError,

    //Aptos
    AptosSignFailure = 1000,
    AptosKeystoreError,
    AptosInvalidData,
    AptosParseTxError,

    // Sui
    SuiSignFailure = 1100,
    SuiInvalidData,
    SuiParseTxError,

    // Arweave
    ArweaveSignFailure = 1200,
    ArweaveKeystoreError,
    ArweaveInvalidData,
    ArweaveParseTxError,
    ArweaveParseAOTxError,

    //Ton
    TonUnknownError = 1300,
    TonMnemonicError,
    TonTransactionError,
    InvalidProof,
    TonTransactionJsonError,
    AddressError,

    // Stellar
    StellarAddressError = 1400,
    StellarInvalidData,
    StellarParseTxError,
    StellarKeystoreError,

    // Zcash
    ZcashGenerateAddressError = 1500,
    ZcashSigningError,
    ZcashInvalidPczt,

    // Monero
    MoneroUnknownError = 1600,

    //Avax
    AvaxInvalidInput = 1700,
    AvaxInvalidOutput,
    AvaxInvalidTransaction,
    AvaxSignFailure,
    AvaxAddressError,
    AvaxUnsupportedTransaction,
    AvaxGetKeyError,
    AvaxUnsupportedNetwork,
    AvaxTransactionConsensusEncodeError,
    AvaxInvalidHex,
    AvaxBech32DecodeError,
    AvaxKeystoreError,
    AvaxDerivePublicKeyError,
    AvaxUnknownTypeId,
    AvaxInvalidHDPath,
    AvaxBech32Error,

    // Ergo
    ErgoUnknownError = 1800,
    ErgoMnemonicError,
    ErgoDerivationError,
    ErgoParseTxError,
    ErgoSignFailure,
    ErgoInvalidTransaction,
}

impl ErrorCodes {
    pub fn to_u32(self) -> u32 {
        self as u32
    }
}

impl From<&BitcoinError> for ErrorCodes {
    fn from(value: &BitcoinError) -> Self {
        match value {
            BitcoinError::InvalidInput => Self::BitcoinInvalidInput,
            BitcoinError::InvalidOutput => Self::BitcoinInvalidOutput,
            BitcoinError::InvalidPsbt(_) => Self::BitcoinInvalidPsbt,
            BitcoinError::InvalidTransaction(_) => Self::BitcoinInvalidTransaction,
            BitcoinError::NoInputs => Self::BitcoinNoInputs,
            BitcoinError::NoOutputs => Self::BitcoinNoOutputs,
            BitcoinError::NoMyInputs => Self::BitcoinNoMyInputs,
            BitcoinError::InputValueTampered(_) => Self::BitcoinInputValueTampered,
            BitcoinError::SignFailure(_) => Self::BitcoinSignFailure,
            BitcoinError::AddressError(_) => Self::BitcoinAddressError,
            BitcoinError::GetKeyError(_) => Self::BitcoinGetKeyError,
            BitcoinError::SignLegacyTxError(_) => Self::BitcoinSignLegacyTxError,
            BitcoinError::UnsupportedTransaction(_) => Self::BitcoinUnsupportedTransaction,
            BitcoinError::UnsupportedNetwork(_) => Self::BitcoinUnsupportedNetwork,
            BitcoinError::TransactionConsensusEncodeError(_) => {
                Self::BitcoinTransactionConsensusEncodeError
            }
            BitcoinError::PushBytesFailed(_) => Self::BitcoinPushBytesFailed,
            BitcoinError::InvalidHex(_) => Self::BitcoinInvalidHex,
            BitcoinError::Base58Error(_) => Self::BitcoinBase58Error,
            BitcoinError::Bech32DecodeError(_) => Self::BitcoinBech32DecodeError,
            BitcoinError::WitnessProgramError(_) => Self::BitcoinWitnessProgramError,
            BitcoinError::KeystoreError(_) => Self::BitcoinKeystoreError,
            BitcoinError::InvalidParseContext(_) => Self::BitcoinInvalidParseContext,
            BitcoinError::InvalidRawTxCryptoBytes(_) => Self::BitcoinInvalidRawTxCryptoBytes,
            BitcoinError::InvalidTxData(_) => Self::BitcoinInvalidTxData,
            BitcoinError::UnsupportedScriptType(_) => Self::BitcoinUnsupportedScriptType,
            BitcoinError::MultiSigWalletParseError(_) => Self::BitcoinMultiSigWalletParseError,
            BitcoinError::MultiSigWalletNotMyWallet => Self::BitcoinMultiSigWalletNotMyWallet,
            BitcoinError::MultiSigWalletAddressCalError(_) => {
                Self::BitcoinMultiSigWalletAddressCalError
            }
            BitcoinError::MultiSigWalletImportXpubError(_) => {
                Self::BitcoinMultiSigWalletImportXpubError
            }
            BitcoinError::MultiSigWalletCrateError(_) => Self::BitcoinMultiSigWalletCrateError,
            BitcoinError::MultiSigWalletFormatError(_) => Self::BitcoinMultiSigWalletFormatError,
            BitcoinError::MultiSigNetworkError(_) => Self::BitcoinMultiSigNetworkError,
            BitcoinError::MultiSigInputError(_) => Self::BitcoinMultiSigInputError,
            BitcoinError::DerivePublicKeyError(_) => Self::BitcoinDerivePublicKeyError,
            BitcoinError::WalletTypeError(_) => Self::BitcoinWalletTypeError,
        }
    }
}

impl From<&URError> for ErrorCodes {
    fn from(value: &URError) -> Self {
        match value {
            URError::CborEncodeError(_) => Self::CborEncodeError,
            URError::CborDecodeError(_) => Self::CborDecodeError,
            URError::UrEncodeError(_) => Self::UREncodeError,
            URError::UrDecodeError(_) => Self::URDecodeError,
            URError::NotAUr => Self::NotAnUr,
            URError::NotSupportURTypeError(_) => Self::NotSupportURTypeError,
            URError::TypeUnspecified => Self::URTypeUnspecified,
            URError::ProtobufDecodeError(_) => Self::URProtobufDecodeError,
            URError::ProtobufEncodeError(_) => Self::URProtobufEncodeError,
            URError::GzipDecodeError(_) => Self::URGzipDecodeError,
            URError::GzipEncodeError(_) => Self::URGzipEnCodeError,
        }
    }
}

impl From<&RustCError> for ErrorCodes {
    fn from(value: &RustCError) -> Self {
        match value {
            RustCError::InvalidMasterFingerprint => Self::InvalidMasterFingerprint,
            RustCError::UnsupportedTransaction(_) => Self::UnsupportedTransaction,
            RustCError::InvalidHDPath => Self::InvalidHDPath,
            RustCError::MasterFingerprintMismatch => Self::MasterFingerprintMismatch,
            RustCError::InvalidXPub => Self::InvalidXPub,
            RustCError::UnexpectedError(_) => Self::UnexpectedError,
            RustCError::InvalidHex(_) => Self::InvalidHex,
            RustCError::WebAuthFailed(_) => Self::WebAuthFailed,
            RustCError::InvalidData(_) => Self::InvalidData,
        }
    }
}

#[cfg(feature = "ethereum")]
impl From<&EthereumError> for ErrorCodes {
    fn from(value: &EthereumError) -> Self {
        match value {
            EthereumError::RlpDecodingError(_) => Self::EthereumRlpDecodingError,
            EthereumError::InvalidTransaction => Self::EthereumInvalidTransaction,
            EthereumError::SignFailure(_) => Self::EthereumSignFailure,
            EthereumError::InvalidHDPath(_) => Self::EthereumInvalidHDPath,
            EthereumError::KeystoreError(_) => Self::EthereumKeystoreError,
            EthereumError::InvalidAddressError(_) => Self::EthereumInvalidAddressError,
            EthereumError::InvalidUtf8Error(_) => Self::EthereumInvalidUtf8Error,
            EthereumError::InvalidContractABI => Self::EthereumInvalidContractABI,
            EthereumError::DecodeContractDataError(_) => Self::EthereumDecodeContractDataError,
            EthereumError::InvalidTypedData(_, _) => Self::EthereumInvalidTypedData,
            EthereumError::HashTypedDataError(_) => Self::EthereumHashTypedDataError,
        }
    }
}

impl From<&KeystoreError> for ErrorCodes {
    fn from(value: &KeystoreError) -> Self {
        match value {
            KeystoreError::DerivationError(_) => Self::KeystoreDerivationError,
            KeystoreError::SeedError(_) => Self::KeystoreSeedError,
            KeystoreError::XPubError(_) => Self::KeystoreXPubError,
            KeystoreError::InvalidDerivationPath(_) => Self::KeystoreInvalidDerivationPath,
            KeystoreError::DerivePubKey(_) => Self::KeystoreDeivePubkey,
            KeystoreError::GenerateSigningKeyError(_) => Self::KeystoreGenerateSigningKeyError,
            KeystoreError::RSAVerifyError => Self::KeystoreRSAVerifyError,
            KeystoreError::RSASignError => Self::KeystoreRSASignError,
            KeystoreError::InvalidDataError(_) => Self::KeystoreInvalidDataError,
            KeystoreError::ZcashOrchardSign(_) => Self::KeystoreZcashOrchardSignError,
        }
    }
}

#[cfg(feature = "cardano")]
impl From<&CardanoError> for ErrorCodes {
    fn from(value: &CardanoError) -> Self {
        match value {
            CardanoError::InvalidTransaction(_) => Self::CardanoInvalidTransaction,
            CardanoError::AddressEncodingError(_) => Self::CardanoAddressEncodingError,
            CardanoError::DerivationError(_) => Self::KeystoreDerivationError,
            CardanoError::UnsupportedTransaction(_) => Self::UnsupportedTransaction,
            CardanoError::SigningFailed(_) => Self::SignFailure,
        }
    }
}

#[cfg(feature = "solana")]
impl From<&SolanaError> for ErrorCodes {
    fn from(value: &SolanaError) -> Self {
        match value {
            SolanaError::KeystoreError(_) => Self::SolanaKeystoreError,
            SolanaError::UnsupportedProgram(_) => Self::SolanaUnsupportedProgram,
            SolanaError::InvalidData(_) => Self::SolanaInvalidData,
            SolanaError::ProgramError(_) => Self::SolanaProgramError,
            SolanaError::AccountNotFound(_) => Self::SolanaAccountNotFound,
            SolanaError::AddressError(_) => Self::SolanaAddressEncodingError,
            SolanaError::ParseTxError(_) => Self::SolanaParseTxError,
        }
    }
}

#[cfg(feature = "near")]
impl From<&NearError> for ErrorCodes {
    fn from(value: &NearError) -> Self {
        match value {
            NearError::InvalidHDPath(_) => Self::InvalidHDPath,
            NearError::KeystoreError(_) => Self::NearKeystoreError,
            NearError::SignFailure(_) => Self::NearSignFailure,
            NearError::ParseTxError(_) => Self::NearParseTxError,
        }
    }
}

#[cfg(feature = "arweave")]
impl From<&ArweaveError> for ErrorCodes {
    fn from(value: &ArweaveError) -> Self {
        match value {
            ArweaveError::InvalidHDPath(_) => Self::InvalidHDPath,
            ArweaveError::KeystoreError(_) => Self::ArweaveKeystoreError,
            ArweaveError::SignFailure(_) => Self::ArweaveSignFailure,
            ArweaveError::ParseTxError(_) => Self::ArweaveParseTxError,
            ArweaveError::AvroError(_) => Self::ArweaveParseTxError,
            ArweaveError::NotSupportedError => Self::UnsupportedTransaction,
            ArweaveError::NotAOTransaction => Self::ArweaveParseAOTxError,
        }
    }
}

#[cfg(feature = "stellar")]
impl From<&StellarError> for ErrorCodes {
    fn from(value: &StellarError) -> Self {
        match value {
            StellarError::AddressError(_) => Self::StellarAddressError,
            StellarError::InvalidData(_) => Self::StellarInvalidData,
            StellarError::ParseTxError(_) => Self::StellarParseTxError,
            StellarError::KeystoreError(_) => Self::StellarKeystoreError,
        }
    }
}

#[cfg(feature = "cosmos")]
impl From<&CosmosError> for ErrorCodes {
    fn from(value: &CosmosError) -> Self {
        match value {
            CosmosError::InvalidHDPath(_) => Self::InvalidHDPath,
            CosmosError::KeystoreError(_) => Self::CosmosKeystoreError,
            CosmosError::SignFailure(_) => Self::CosmosSignFailure,
            CosmosError::InvalidAddressError(_) => Self::CosmosInvalidData,
            CosmosError::ParseTxError(_) => Self::CosmosParseTxError,
            CosmosError::InvalidData(_) => Self::CosmosInvalidData,
        }
    }
}

#[cfg(feature = "aptos")]
impl From<&AptosError> for ErrorCodes {
    fn from(value: &AptosError) -> Self {
        match value {
            AptosError::ParseTxError(_) => Self::AptosParseTxError,
            AptosError::KeystoreError(_) => Self::AptosKeystoreError,
            AptosError::SignFailure(_) => Self::AptosSignFailure,
            AptosError::InvalidData(_) => Self::AptosInvalidData,
        }
    }
}

#[cfg(feature = "tron")]
impl From<&TronError> for ErrorCodes {
    fn from(value: &TronError) -> Self {
        match value {
            TronError::ParseNumberError(_) => Self::TronParseNumberError,
            TronError::ProtobufError(_) => Self::TronProtobufError,
            TronError::SignFailure(_) => Self::TronSignFailure,
            TronError::Base58Error(_) => Self::TronBase58Error,
            TronError::InvalidParseContext(_) => Self::TronInvalidParseContext,
            TronError::InvalidRawTxCryptoBytes(_) => Self::TronInvalidRawTxCryptoBytes,
            TronError::InvalidHDPath(_) => Self::InvalidHDPath,
            TronError::KeystoreError(_) => Self::KeystoreDeivePubkey,
            TronError::NoMyInputs => Self::TronNoMyInputs,
        }
    }
}

#[cfg(feature = "xrp")]
impl From<&XRPError> for ErrorCodes {
    fn from(value: &XRPError) -> Self {
        match value {
            XRPError::InvalidHDPath(_) => Self::InvalidHDPath,
            XRPError::KeystoreError(_) => Self::KeystoreDeivePubkey,
            XRPError::SignFailure(_) => Self::XRPSignFailure,
            XRPError::InvalidData(_) => Self::XRPInvalidData,
            XRPError::ParseTxError(_) => Self::XRPParseTxError,
        }
    }
}

#[cfg(feature = "sui")]
impl From<&SuiError> for ErrorCodes {
    fn from(value: &SuiError) -> Self {
        match value {
            SuiError::SignFailure(_) => Self::SuiSignFailure,
            SuiError::InvalidData(_) => Self::SuiInvalidData,
            SuiError::ParseTxError(_) => Self::SuiParseTxError,
            SuiError::InvalidXPub(_) => Self::InvalidXPub,
        }
    }
}

#[cfg(feature = "ton")]
impl From<&TonError> for ErrorCodes {
    fn from(value: &TonError) -> Self {
        match value {
            TonError::UnknownError => Self::TonUnknownError,
            TonError::MnemonicError(_) => Self::TonMnemonicError,
            TonError::TransactionError(_) => Self::TonTransactionError,
            TonError::TransactionJsonError(_) => Self::TonTransactionJsonError,
            TonError::AddressError(_) => Self::AddressError,
            TonError::InvalidTransaction(_) => Self::TonTransactionError,
            TonError::InvalidProof(_) => Self::InvalidProof,
        }
    }
}

#[cfg(feature = "avalanche")]
impl From<&AvaxError> for ErrorCodes {
    fn from(value: &AvaxError) -> Self {
        match value {
            AvaxError::InvalidInput => Self::AvaxInvalidInput,
            AvaxError::InvalidOutput => Self::AvaxInvalidOutput,
            AvaxError::InvalidTransaction(_) => Self::AvaxInvalidTransaction,
            AvaxError::SignFailure(_) => Self::AvaxSignFailure,
            AvaxError::AddressError(_) => Self::AvaxAddressError,
            AvaxError::GetKeyError(_) => Self::AvaxGetKeyError,
            AvaxError::UnsupportedTransaction(_) => Self::AvaxUnsupportedTransaction,
            AvaxError::UnsupportedNetwork(_) => Self::AvaxUnsupportedNetwork,
            AvaxError::TransactionConsensusEncodeError(_) => {
                Self::AvaxTransactionConsensusEncodeError
            }
            AvaxError::InvalidHex(_) => Self::AvaxInvalidHex,
            AvaxError::Bech32DecodeError(_) => Self::AvaxBech32DecodeError,
            AvaxError::KeystoreError(_) => Self::AvaxAddressError,
            AvaxError::DerivePublicKeyError(_) => Self::AvaxDerivePublicKeyError,
            AvaxError::UnknownTypeId(_) => Self::AvaxUnknownTypeId,
            AvaxError::InvalidHDPath(_) => Self::AvaxInvalidHDPath,
            AvaxError::Bech32Error => Self::AvaxBech32Error,
        }
    }
}

#[cfg(feature = "zcash")]
impl From<&ZcashError> for ErrorCodes {
    fn from(value: &ZcashError) -> Self {
        match value {
            ZcashError::GenerateAddressError(_) => Self::ZcashGenerateAddressError,
            ZcashError::InvalidDataError(_) => Self::InvalidData,
            ZcashError::SigningError(_) => Self::ZcashSigningError,
            ZcashError::InvalidPczt(_) => Self::ZcashInvalidPczt,
        }
    }
}

#[cfg(feature = "monero")]
impl From<&MoneroError> for ErrorCodes {
    fn from(value: &MoneroError) -> Self {
        match value {
            _ => Self::MoneroUnknownError,
        }
    }
}

#[cfg(feature = "ergo")]
impl From<&ErgoError> for ErrorCodes {
    fn from(value: &ErgoError) -> Self {
        match value {
            ErgoError::UnknownError => Self::ErgoUnknownError,
            ErgoError::MnemonicError(_) => Self::ErgoMnemonicError,
            ErgoError::DerivationError(_) => Self::ErgoDerivationError,
            ErgoError::TransactionParseError(_) => Self::ErgoParseTxError,
            ErgoError::SigningFailed(_) => Self::ErgoSignFailure,
            ErgoError::InvalidTransaction(_) => Self::ErgoInvalidTransaction,
        }
    }
}

pub type R<T> = Result<T, RustCError>;

#[derive(Error, Debug, PartialEq)]
pub enum RustCError {
    #[error("invalid master fingerprint")]
    InvalidMasterFingerprint,
    #[error("master finger print mismatch")]
    MasterFingerprintMismatch,
    #[error("invalid path")]
    InvalidHDPath,
    #[error("invalid xpub")]
    InvalidXPub,
    #[error("this kind of transaction is not supported yet, {0}")]
    UnsupportedTransaction(String),
    #[error("Unexpected error: {0}")]
    UnexpectedError(String),
    #[error("invalid hex value: {0}")]
    InvalidHex(String),
    #[error("web auth failed: {0}")]
    WebAuthFailed(String),
    #[error("invalid data: {0}")]
    InvalidData(String),
}

#[derive(Error, Debug, PartialEq)]
pub enum KeystoneError {
    #[error("protobuf operation failed, reason: {0}")]
    ProtobufError(String),
    #[error("parse context invalid: {0}")]
    InvalidParseContext(String),
    #[error("sign tx failed: {0}")]
    SignTxFailed(String),
    #[error("check tx failed: {0}")]
    CheckTxFailed(String),
}

impl From<&KeystoneError> for ErrorCodes {
    fn from(value: &KeystoneError) -> Self {
        match value {
            KeystoneError::ProtobufError(_) => Self::CompanionAppProtobufError,
            KeystoneError::InvalidParseContext(_) => Self::CompanionAppInvalidParseContext,
            KeystoneError::SignTxFailed(_) => Self::CompanionAppSignTxFailed,
            KeystoneError::CheckTxFailed(_) => Self::CompanionAppCheckTxFailed,
        }
    }
}
