use alloc::boxed::Box;
use alloc::format;
use alloc::string::{String, ToString};
use alloc::vec::Vec;
use core::ptr::null_mut;
use ur_registry::ethereum::eth_batch_sign_requests::EthBatchSignRequest;

use cstr_core::CString;
use cty::c_char;
use keystore::errors::KeystoreError;
use ur_parse_lib::keystone_ur_decoder::{probe_decode, URParseResult as InnerParseResult};
use ur_parse_lib::keystone_ur_encoder::KeystoneUREncoder;
use ur_registry::error::{URError, URResult};
use ur_registry::extend::crypto_multi_accounts::CryptoMultiAccounts;
use ur_registry::registry_types::URType as InnerURType;
use ur_registry::traits::RegistryItem;
use ur_registry::{
    bytes::Bytes, cardano::cardano_sign_cip8_data_request::CardanoSignCip8DataRequest,
};

#[cfg(feature = "aptos")]
use ur_registry::aptos::aptos_sign_request::AptosSignRequest;
#[cfg(feature = "arweave")]
use ur_registry::arweave::arweave_sign_request::ArweaveSignRequest;
#[cfg(feature = "avalanche")]
use ur_registry::avalanche::avax_sign_request::AvaxSignRequest;
#[cfg(feature = "bitcoin")]
use ur_registry::bitcoin::btc_sign_request::BtcSignRequest;
#[cfg(feature = "cardano")]
use ur_registry::cardano::{
    cardano_catalyst_voting_registration::CardanoCatalystVotingRegistrationRequest,
    cardano_sign_data_request::CardanoSignDataRequest, cardano_sign_request::CardanoSignRequest,
    cardano_sign_tx_hash_request::CardanoSignTxHashRequest,
};
#[cfg(feature = "cosmos")]
use ur_registry::cosmos::cosmos_sign_request::CosmosSignRequest;
#[cfg(feature = "ethereum")]
use ur_registry::cosmos::evm_sign_request::EvmSignRequest;
#[cfg(feature = "bitcoin")]
use ur_registry::crypto_account::CryptoAccount;
#[cfg(feature = "bitcoin")]
use ur_registry::crypto_psbt::CryptoPSBT;
#[cfg(feature = "bitcoin")]
use ur_registry::crypto_psbt_extend::CryptoPSBTExtend;
#[cfg(feature = "ethereum")]
use ur_registry::ethereum::eth_sign_request::EthSignRequest;
#[cfg(not(feature = "btc-only"))]
use ur_registry::extend::qr_hardware_call::QRHardwareCall;
#[cfg(feature = "iota")]
use ur_registry::iota::iota_sign_hash_request::IotaSignHashRequest;
#[cfg(feature = "iota")]
use ur_registry::iota::iota_sign_request::IotaSignRequest;
#[cfg(feature = "multi-coins")]
use ur_registry::keystone::keystone_sign_request::KeystoneSignRequest;
#[cfg(feature = "monero")]
use ur_registry::monero::{xmr_output::XmrOutput, xmr_txunsigned::XmrTxUnsigned};
#[cfg(feature = "near")]
use ur_registry::near::near_sign_request::NearSignRequest;
#[cfg(feature = "solana")]
use ur_registry::solana::sol_sign_request::SolSignRequest;
#[cfg(feature = "stellar")]
use ur_registry::stellar::stellar_sign_request::StellarSignRequest;
#[cfg(feature = "sui")]
use ur_registry::sui::sui_sign_hash_request::SuiSignHashRequest;
#[cfg(feature = "sui")]
use ur_registry::sui::sui_sign_request::SuiSignRequest;
#[cfg(feature = "ton")]
use ur_registry::ton::ton_sign_request::TonSignRequest;
#[cfg(feature = "tron")]
use ur_registry::tron::tron_sign_request::TronSignRequest;
#[cfg(feature = "zcash")]
use ur_registry::zcash::zcash_pczt::ZcashPczt;
#[cfg(feature = "zcash_cypherpunk")]
use ur_registry::zcash::zcash_sign_batch::ZcashSignBatch;

use super::errors::{ErrorCodes, RustCError};
use super::free::Free;
use super::types::{PtrDecoder, PtrEncoder, PtrString, PtrUR};

#[cfg(all(not(test), not(feature = "simulator")))]
unsafe extern "C" {
    fn MpuSandboxValidateCbor(
        input: *const u8,
        input_length: usize,
        validation_status: *mut u32,
    ) -> bool;
}

struct KeystoneURDecoder {
    decoder: ur::Decoder,
    budget: super::ur_budget::FragmentBudget,
}

impl KeystoneURDecoder {
    fn from_first_part(part: &str) -> Result<Self, URError> {
        let mut decoder = Self {
            decoder: ur::Decoder::default(),
            budget: Default::default(),
        };
        decoder.receive(part)?;
        Ok(decoder)
    }

    fn receive(&mut self, part: &str) -> Result<(), URError> {
        if let Err(reason) = self.budget.receive(part) {
            self.decoder = ur::Decoder::default();
            return Err(URError::UrDecodeError(reason.to_string()));
        }
        self.decoder
            .receive(part)
            .map_err(|error| URError::UrDecodeError(error.to_string()))
    }

    fn complete(&self) -> bool {
        self.decoder.complete()
    }

    fn progress(&self) -> u8 {
        self.decoder.progress()
    }

    fn message(&self) -> Result<Vec<u8>, URError> {
        self.decoder
            .message()
            .map_err(|error| URError::UrDecodeError(error.to_string()))?
            .ok_or_else(|| URError::UrDecodeError("cbor is none".to_string()))
    }
}

fn validate_reassembled_cbor(cbor: &[u8]) -> Result<(), URError> {
    #[cfg(all(not(test), not(feature = "simulator")))]
    let status = {
        let mut status = sandbox_parser::ValidationStatus::InvalidInput as u32;
        let available = unsafe { MpuSandboxValidateCbor(cbor.as_ptr(), cbor.len(), &mut status) };
        if !available {
            return Err(URError::CborDecodeError(format!(
                "sandbox CBOR validation unavailable ({} bytes)",
                cbor.len()
            )));
        }
        status
    };

    #[cfg(any(test, feature = "simulator"))]
    let status = sandbox_parser::validate_cbor(cbor) as u32;

    if status == sandbox_parser::ValidationStatus::Ok as u32 {
        Ok(())
    } else {
        Err(URError::CborDecodeError(format!(
            "sandbox rejected reconstructed CBOR with status {status}"
        )))
    }
}
use super::ur_ext::InferViewType;
use super::utils::{check_recover_c_char_lossy, convert_c_char, recover_c_char};
use crate::{
    extract_ptr_with_type, free_ptr_with_type, free_str_ptr, impl_c_ptr, impl_new_error,
    impl_response,
};

pub const FRAGMENT_MAX_LENGTH_DEFAULT: usize = 200;
pub const FRAGMENT_UNLIMITED_LENGTH: usize = 11000;

#[repr(C)]
pub struct UREncodeResult {
    is_multi_part: bool,
    pub data: *mut c_char,
    encoder: PtrEncoder,
    error_code: u32,
    error_message: *mut c_char,
}

impl UREncodeResult {
    fn new() -> Self {
        Self {
            is_multi_part: false,
            data: null_mut(),
            encoder: null_mut(),
            error_code: ErrorCodes::Success as u32,
            error_message: null_mut(),
        }
    }

    pub fn multi(data: String, encoder: KeystoneUREncoder) -> Self {
        UREncodeResult {
            is_multi_part: true,
            data: convert_c_char(data.to_uppercase()),
            encoder: Box::into_raw(Box::new(encoder)) as PtrEncoder,
            ..Self::new()
        }
    }

    pub fn single(data: String) -> Self {
        UREncodeResult {
            is_multi_part: false,
            data: convert_c_char(data.to_uppercase()),
            ..Self::new()
        }
    }

    pub fn text(data: String) -> Self {
        UREncodeResult {
            is_multi_part: false,
            data: convert_c_char(data),
            ..Self::new()
        }
    }

    fn encode_with_multipart_policy(
        data: Vec<u8>,
        tag: String,
        max_fragment_length: usize,
        allow_multipart: bool,
    ) -> Self {
        let result =
            ur_parse_lib::keystone_ur_encoder::probe_encode(&data, max_fragment_length, tag);
        match result {
            Ok(result) => {
                if result.is_multi_part {
                    if !allow_multipart {
                        return Self::from(RustCError::UnsupportedTransaction(
                            "encoded UR is too large for a single USB response".to_string(),
                        ));
                    }
                    match result.encoder {
                        Some(v) => Self::multi(result.data.to_uppercase(), v),
                        None => Self::from(RustCError::UnexpectedError(
                            "ur encoder is none".to_string(),
                        )),
                    }
                } else {
                    Self::single(result.data.to_uppercase())
                }
            }
            Err(e) => Self::from(e),
        }
    }

    pub fn encode(data: Vec<u8>, tag: String, max_fragment_length: usize) -> Self {
        Self::encode_with_multipart_policy(data, tag, max_fragment_length, true)
    }

    pub fn encode_full_response(data: Vec<u8>, tag: String) -> Self {
        let max_fragment_length = data.len().max(1);
        Self::encode_with_multipart_policy(data, tag, max_fragment_length, false)
    }
}

impl Free for UREncodeResult {
    unsafe fn free(&self) {
        free_str_ptr!(self.data);
        free_str_ptr!(self.error_message);
        free_ptr_with_type!(self.encoder, KeystoneUREncoder);
    }
}

impl_response!(UREncodeResult);

#[repr(C)]
pub struct UREncodeMultiResult {
    data: *mut c_char,
    error_code: u32,
    error_message: *mut c_char,
}

impl UREncodeMultiResult {
    fn new() -> Self {
        Self {
            data: null_mut(),
            error_code: ErrorCodes::Success.to_u32(),
            error_message: null_mut(),
        }
    }
    fn success(data: String) -> Self {
        Self {
            data: convert_c_char(data),
            ..Self::new()
        }
    }
}

impl Free for UREncodeMultiResult {
    unsafe fn free(&self) {
        free_str_ptr!(self.data);
        free_str_ptr!(self.error_message);
    }
}

impl_response!(UREncodeMultiResult);

#[repr(C)]
#[derive(Debug, Eq, PartialEq)]
pub enum ViewType {
    #[cfg(feature = "bitcoin")]
    BtcNativeSegwitTx,
    #[cfg(feature = "bitcoin")]
    BtcSegwitTx,
    #[cfg(feature = "bitcoin")]
    BtcLegacyTx,
    #[cfg(feature = "bitcoin")]
    BtcTx,
    #[cfg(feature = "bitcoin")]
    BtcMsg,
    #[cfg(feature = "ltc")]
    LtcTx,
    #[cfg(feature = "doge")]
    DogeTx,
    #[cfg(feature = "dash")]
    DashTx,
    #[cfg(feature = "bch")]
    BchTx,
    #[cfg(feature = "ethereum")]
    EthTx,
    #[cfg(feature = "ethereum")]
    EthBatchTx,
    #[cfg(feature = "ethereum")]
    EthPersonalMessage,
    #[cfg(feature = "ethereum")]
    EthTypedData,
    #[cfg(feature = "tron")]
    TronTx,
    #[cfg(feature = "tron")]
    TronPersonalMessage,
    #[cfg(feature = "tron")]
    TronSwapTx,
    #[cfg(feature = "solana")]
    SolanaTx,
    #[cfg(feature = "solana")]
    SolanaMessage,
    #[cfg(feature = "cardano")]
    CardanoTx,
    #[cfg(feature = "cardano")]
    CardanoSignData,
    #[cfg(feature = "cardano")]
    CardanoSignCip8Data,
    #[cfg(feature = "cardano")]
    CardanoCatalystVotingRegistration,
    #[cfg(feature = "cardano")]
    CardanoSignTxHash,
    #[cfg(feature = "near")]
    NearTx,
    #[cfg(feature = "xrp")]
    XRPTx,
    #[cfg(feature = "cosmos")]
    CosmosTx,
    #[cfg(feature = "cosmos")]
    CosmosEvmTx,
    #[cfg(feature = "sui")]
    SuiTx,
    #[cfg(feature = "sui")]
    SuiSignMessageHash,
    #[cfg(feature = "iota")]
    IotaTx,
    #[cfg(feature = "iota")]
    IotaSignMessageHash,

    #[cfg(feature = "arweave")]
    ArweaveTx,
    #[cfg(feature = "arweave")]
    ArweaveMessage,
    #[cfg(feature = "arweave")]
    ArweaveDataItem,
    #[cfg(feature = "stellar")]
    StellarTx,
    #[cfg(feature = "stellar")]
    StellarHash,
    #[cfg(feature = "ton")]
    TonTx,
    #[cfg(feature = "ton")]
    TonSignProof,
    #[cfg(feature = "zcash")]
    ZcashTx,
    #[cfg(feature = "zcash_cypherpunk")]
    ZcashBatchTx,
    #[cfg(feature = "aptos")]
    AptosTx,
    #[cfg(feature = "monero")]
    XmrOutput,
    #[cfg(feature = "monero")]
    XmrTxUnsigned,
    #[cfg(feature = "avalanche")]
    AvaxTx,
    WebAuthResult,
    #[cfg(not(feature = "btc-only"))]
    KeyDerivationRequest,
    #[cfg(feature = "multi-coins")]
    DeriveContextHashRequest,
    #[cfg(feature = "multi-coins")]
    BatchCall,
    #[cfg(feature = "btc-only")]
    MultisigWalletImport,
    #[cfg(feature = "btc-only")]
    MultisigCryptoImportXpub,
    #[cfg(feature = "btc-only")]
    MultisigBytesImportXpub,
    #[cfg(feature = "cypherpunk")]
    ZcashAddressCheck,
    ViewTypeUnKnown,
}

#[repr(C)]
pub enum QRCodeType {
    #[cfg(feature = "bitcoin")]
    CryptoPSBT,
    #[cfg(feature = "multi-coins")]
    CryptoPSBTExtend,
    CryptoMultiAccounts,
    #[cfg(feature = "bitcoin")]
    CryptoAccount,
    Bytes,
    #[cfg(feature = "bitcoin")]
    BtcSignRequest,
    SeedSignerMessage,
    #[cfg(feature = "cypherpunk")]
    ZcashAddressText,
    #[cfg(feature = "multi-coins")]
    KeystoneSignRequest,
    #[cfg(feature = "ethereum")]
    EthSignRequest,
    #[cfg(feature = "ethereum")]
    EthBatchSignRequest,
    #[cfg(feature = "solana")]
    SolSignRequest,
    #[cfg(feature = "tron")]
    TronSignRequest,
    #[cfg(feature = "near")]
    NearSignRequest,
    #[cfg(feature = "cardano")]
    CardanoSignRequest,
    #[cfg(feature = "cardano")]
    CardanoSignTxHashRequest,

    #[cfg(feature = "cardano")]
    CardanoSignDataRequest,
    #[cfg(feature = "cardano")]
    CardanoCatalystVotingRegistrationRequest,

    #[cfg(feature = "cardano")]
    CardanoSignCip8DataRequest,

    #[cfg(feature = "cosmos")]
    CosmosSignRequest,
    #[cfg(feature = "ethereum")]
    EvmSignRequest,
    #[cfg(feature = "sui")]
    SuiSignRequest,
    #[cfg(feature = "sui")]
    SuiSignHashRequest,
    #[cfg(feature = "iota")]
    IotaSignRequest,
    #[cfg(feature = "iota")]
    IotaSignHashRequest,
    #[cfg(feature = "aptos")]
    AptosSignRequest,
    #[cfg(not(feature = "btc-only"))]
    QRHardwareCall,
    #[cfg(feature = "arweave")]
    ArweaveSignRequest,
    #[cfg(feature = "stellar")]
    StellarSignRequest,
    #[cfg(feature = "ton")]
    TonSignRequest,
    #[cfg(feature = "avalanche")]
    AvaxSignRequest,
    #[cfg(feature = "zcash")]
    ZcashPczt,
    #[cfg(feature = "zcash_cypherpunk")]
    ZcashSignBatch,
    #[cfg(feature = "monero")]
    XmrOutputSignRequest,
    #[cfg(feature = "monero")]
    XmrTxUnsignedRequest,
    URTypeUnKnown,
}

impl QRCodeType {
    pub fn from(value: &InnerURType) -> Result<Self, URError> {
        match value {
            #[cfg(feature = "bitcoin")]
            InnerURType::CryptoPsbt(_) => Ok(QRCodeType::CryptoPSBT),
            #[cfg(feature = "multi-coins")]
            InnerURType::CryptoPsbtExtend(_) => Ok(QRCodeType::CryptoPSBTExtend),
            InnerURType::CryptoMultiAccounts(_) => Ok(QRCodeType::CryptoMultiAccounts),
            #[cfg(feature = "bitcoin")]
            InnerURType::CryptoAccount(_) => Ok(QRCodeType::CryptoAccount),
            InnerURType::Bytes(_) => Ok(QRCodeType::Bytes),
            #[cfg(feature = "bitcoin")]
            InnerURType::BtcSignRequest(_) => Ok(QRCodeType::BtcSignRequest),
            #[cfg(feature = "multi-coins")]
            InnerURType::KeystoneSignRequest(_) => Ok(QRCodeType::KeystoneSignRequest),
            #[cfg(feature = "ethereum")]
            InnerURType::EthSignRequest(_) => Ok(QRCodeType::EthSignRequest),
            #[cfg(feature = "ethereum")]
            InnerURType::EthBatchSignRequest(_) => Ok(QRCodeType::EthBatchSignRequest),
            #[cfg(feature = "solana")]
            InnerURType::SolSignRequest(_) => Ok(QRCodeType::SolSignRequest),
            #[cfg(feature = "tron")]
            InnerURType::TronSignRequest(_) => Ok(QRCodeType::TronSignRequest),
            #[cfg(feature = "near")]
            InnerURType::NearSignRequest(_) => Ok(QRCodeType::NearSignRequest),
            #[cfg(feature = "cosmos")]
            InnerURType::CosmosSignRequest(_) => Ok(QRCodeType::CosmosSignRequest),
            #[cfg(feature = "ethereum")]
            InnerURType::EvmSignRequest(_) => Ok(QRCodeType::EvmSignRequest),
            #[cfg(feature = "sui")]
            InnerURType::SuiSignRequest(_) => Ok(QRCodeType::SuiSignRequest),
            #[cfg(feature = "sui")]
            InnerURType::SuiSignHashRequest(_) => Ok(QRCodeType::SuiSignHashRequest),
            #[cfg(feature = "iota")]
            InnerURType::IotaSignRequest(_) => Ok(QRCodeType::IotaSignRequest),
            #[cfg(feature = "iota")]
            InnerURType::IotaSignHashRequest(_) => Ok(QRCodeType::IotaSignHashRequest),
            #[cfg(feature = "stellar")]
            InnerURType::StellarSignRequest(_) => Ok(QRCodeType::StellarSignRequest),
            #[cfg(feature = "arweave")]
            InnerURType::ArweaveSignRequest(_) => Ok(QRCodeType::ArweaveSignRequest),
            #[cfg(feature = "aptos")]
            InnerURType::AptosSignRequest(_) => Ok(QRCodeType::AptosSignRequest),
            #[cfg(feature = "cardano")]
            InnerURType::CardanoSignRequest(_) => Ok(QRCodeType::CardanoSignRequest),
            #[cfg(feature = "cardano")]
            InnerURType::CardanoSignTxHashRequest(_) => Ok(QRCodeType::CardanoSignTxHashRequest),
            #[cfg(feature = "cardano")]
            InnerURType::CardanoSignCip8DataRequest(_) => {
                Ok(QRCodeType::CardanoSignCip8DataRequest)
            }
            #[cfg(feature = "cardano")]
            InnerURType::CardanoSignDataRequest(_) => Ok(QRCodeType::CardanoSignDataRequest),
            #[cfg(feature = "cardano")]
            InnerURType::CardanoCatalystVotingRegistrationRequest(_) => {
                Ok(QRCodeType::CardanoCatalystVotingRegistrationRequest)
            }
            #[cfg(feature = "ton")]
            InnerURType::TonSignRequest(_) => Ok(QRCodeType::TonSignRequest),
            #[cfg(feature = "zcash")]
            InnerURType::ZcashPczt(_) => Ok(QRCodeType::ZcashPczt),
            #[cfg(feature = "zcash_cypherpunk")]
            InnerURType::ZcashSignBatch(_) => Ok(QRCodeType::ZcashSignBatch),
            #[cfg(feature = "monero")]
            InnerURType::XmrTxUnsigned(_) => Ok(QRCodeType::XmrTxUnsignedRequest),
            #[cfg(feature = "monero")]
            InnerURType::XmrOutput(_) => Ok(QRCodeType::XmrOutputSignRequest),
            #[cfg(feature = "avalanche")]
            InnerURType::AvaxSignRequest(_) => Ok(QRCodeType::AvaxSignRequest),
            #[cfg(not(feature = "btc-only"))]
            InnerURType::QRHardwareCall(_) => Ok(QRCodeType::QRHardwareCall),
            _ => Err(URError::NotSupportURTypeError(value.get_type_str())),
        }
    }
}

#[repr(C)]
pub struct URParseResult {
    is_multi_part: bool,
    progress: u32,
    t: ViewType,
    ur_type: QRCodeType,
    data: PtrUR,
    decoder: PtrDecoder,
    error_code: u32,
    error_message: PtrString,
}

impl URParseResult {
    pub fn new() -> Self {
        Self {
            is_multi_part: false,
            progress: 0,
            t: ViewType::ViewTypeUnKnown,
            ur_type: QRCodeType::URTypeUnKnown,
            data: null_mut(),
            decoder: null_mut(),
            error_code: 0,
            error_message: null_mut(),
        }
    }

    pub fn single<T>(t: ViewType, ur_type: QRCodeType, data: T) -> Self {
        let _self = Self::new();
        let data = Box::into_raw(Box::new(data)) as PtrUR;
        Self {
            progress: 100,
            t,
            ur_type,
            data,
            .._self
        }
    }

    fn multi(progress: u32, t: ViewType, ur_type: QRCodeType, decoder: KeystoneURDecoder) -> Self {
        let _self = Self::new();
        let decoder = Box::into_raw(Box::new(decoder)) as PtrUR;
        Self {
            is_multi_part: true,
            progress,
            t,
            ur_type,
            decoder,
            .._self
        }
    }
}

impl Free for URParseResult {
    unsafe fn free(&self) {
        free_str_ptr!(self.error_message);
        free_ptr_with_type!(self.decoder, KeystoneURDecoder);
        free_ur(&self.ur_type, self.data);
    }
}

unsafe fn free_ur(ur_type: &QRCodeType, data: PtrUR) {
    match ur_type {
        #[cfg(feature = "cypherpunk")]
        QRCodeType::ZcashAddressText => {
            free_ptr_with_type!(data, CString);
        }
        #[cfg(feature = "bitcoin")]
        QRCodeType::CryptoPSBT => {
            free_ptr_with_type!(data, CryptoPSBT);
        }
        QRCodeType::CryptoMultiAccounts => {
            free_ptr_with_type!(data, CryptoMultiAccounts);
        }
        #[cfg(feature = "bitcoin")]
        QRCodeType::CryptoAccount => {
            free_ptr_with_type!(data, CryptoAccount);
        }
        #[cfg(feature = "ethereum")]
        QRCodeType::EthSignRequest => {
            free_ptr_with_type!(data, EthSignRequest);
        }
        #[cfg(feature = "ethereum")]
        QRCodeType::EthBatchSignRequest => {
            free_ptr_with_type!(data, EthBatchSignRequest);
        }
        #[cfg(feature = "solana")]
        QRCodeType::SolSignRequest => {
            free_ptr_with_type!(data, SolSignRequest);
        }
        #[cfg(feature = "tron")]
        QRCodeType::TronSignRequest => {
            free_ptr_with_type!(data, TronSignRequest);
        }
        #[cfg(feature = "near")]
        QRCodeType::NearSignRequest => {
            free_ptr_with_type!(data, NearSignRequest);
        }
        QRCodeType::Bytes => {
            free_ptr_with_type!(data, Bytes);
        }
        #[cfg(feature = "bitcoin")]
        QRCodeType::BtcSignRequest => {
            free_ptr_with_type!(data, BtcSignRequest);
        }
        #[cfg(feature = "multi-coins")]
        QRCodeType::KeystoneSignRequest => {
            free_ptr_with_type!(data, KeystoneSignRequest);
        }
        #[cfg(feature = "cosmos")]
        QRCodeType::CosmosSignRequest => {
            free_ptr_with_type!(data, CosmosSignRequest);
        }
        #[cfg(feature = "ethereum")]
        QRCodeType::EvmSignRequest => {
            free_ptr_with_type!(data, EvmSignRequest);
        }
        #[cfg(feature = "sui")]
        QRCodeType::SuiSignRequest => {
            free_ptr_with_type!(data, SuiSignRequest);
        }
        #[cfg(feature = "sui")]
        QRCodeType::SuiSignHashRequest => {
            free_ptr_with_type!(data, SuiSignHashRequest);
        }

        #[cfg(feature = "stellar")]
        QRCodeType::StellarSignRequest => {
            free_ptr_with_type!(data, StellarSignRequest);
        }
        #[cfg(feature = "arweave")]
        QRCodeType::ArweaveSignRequest => {
            free_ptr_with_type!(data, ArweaveSignRequest);
        }
        #[cfg(feature = "aptos")]
        QRCodeType::AptosSignRequest => {
            free_ptr_with_type!(data, AptosSignRequest);
        }
        #[cfg(feature = "cardano")]
        QRCodeType::CardanoSignRequest => {
            free_ptr_with_type!(data, CardanoSignRequest);
        }
        #[cfg(feature = "cardano")]
        QRCodeType::CardanoSignDataRequest => {
            free_ptr_with_type!(data, CardanoSignDataRequest);
        }

        #[cfg(feature = "cardano")]
        QRCodeType::CardanoSignCip8DataRequest => {
            free_ptr_with_type!(data, CardanoSignCip8DataRequest);
        }

        #[cfg(feature = "cardano")]
        QRCodeType::CardanoSignTxHashRequest => {
            free_ptr_with_type!(data, CardanoSignTxHashRequest);
        }
        #[cfg(feature = "cardano")]
        QRCodeType::CardanoCatalystVotingRegistrationRequest => {
            free_ptr_with_type!(data, CardanoCatalystVotingRegistrationRequest);
        }
        #[cfg(feature = "zcash_cypherpunk")]
        QRCodeType::ZcashSignBatch => {
            free_ptr_with_type!(data, ZcashSignBatch);
        }
        #[cfg(feature = "monero")]
        QRCodeType::XmrOutputSignRequest => {
            free_ptr_with_type!(data, XmrOutput);
        }
        #[cfg(feature = "monero")]
        QRCodeType::XmrTxUnsignedRequest => {
            free_ptr_with_type!(data, XmrTxUnsigned);
        }
        #[cfg(feature = "avalanche")]
        // todo
        QRCodeType::AvaxSignRequest => {
            free_ptr_with_type!(data, AvaxSignRequest);
        }
        #[cfg(not(feature = "btc-only"))]
        QRCodeType::QRHardwareCall => {
            free_ptr_with_type!(data, QRHardwareCall);
        }
        _ => {}
    }
}

impl_response!(URParseResult);

#[repr(C)]
pub struct URParseMultiResult {
    is_complete: bool,
    t: ViewType,
    ur_type: QRCodeType,
    progress: u32,
    data: PtrUR,
    error_code: u32,
    error_message: PtrString,
}

impl URParseMultiResult {
    pub fn new() -> Self {
        Self {
            is_complete: false,
            t: ViewType::ViewTypeUnKnown,
            ur_type: QRCodeType::URTypeUnKnown,
            progress: 0,
            data: null_mut(),
            error_code: 0,
            error_message: null_mut(),
        }
    }
    pub fn success<T>(t: ViewType, ur_type: QRCodeType, data: T) -> Self {
        let _self = Self::new();
        let data = Box::into_raw(Box::new(data)) as PtrUR;
        let progress = 100;
        Self {
            is_complete: true,
            t,
            ur_type,
            progress,
            data,
            .._self
        }
    }

    pub fn un_complete(t: ViewType, ur_type: QRCodeType, progress: u8) -> Self {
        let _self = Self::new();
        let progress = progress as u32;
        Self {
            t,
            ur_type,
            progress,
            .._self
        }
    }
}

impl Free for URParseMultiResult {
    unsafe fn free(&self) {
        free_str_ptr!(self.error_message);
        free_ur(&self.ur_type, self.data);
    }
}

impl_response!(URParseMultiResult);

#[cfg(test)]
mod tests {
    #[cfg(feature = "cypherpunk")]
    #[test]
    fn zcash_address_text_qr_owns_and_releases_its_payload() {
        use crate::common::qrcode::parse_qrcode_text;
        use crate::zcash::{verify_zcash_address, zcash_address_qr_text, ZcashAddressVerification};
        let key = keystore::algorithms::zcash::derive_ufvk(
            &zcash_vendor::zcash_protocol::consensus::MainNetwork,
            &[1; 32],
            "m/32'/133'/0'",
        )
        .unwrap();
        let text =
            app_zcash::get_address(&zcash_vendor::zcash_protocol::consensus::MainNetwork, &key)
                .unwrap();
        let input = CString::new(text.as_bytes()).unwrap();
        let key = CString::new(key).unwrap();
        unsafe {
            for _ in 0..8 {
                let parsed = Box::from_raw(parse_qrcode_text(input.as_ptr() as PtrString));
                assert_eq!(parsed.error_code, 0);
                assert_eq!(parsed.t, ViewType::ZcashAddressCheck);
                assert!(matches!(parsed.ur_type, QRCodeType::ZcashAddressText));
                let borrowed = zcash_address_qr_text(parsed.data);
                assert_eq!(cstr_core::CStr::from_ptr(borrowed).to_str().unwrap(), text);
                assert!(matches!(
                    verify_zcash_address(key.as_ptr() as PtrString, borrowed as PtrString),
                    ZcashAddressVerification::ZcashAddressOwned
                ));
                parsed.free();
            }
            for (candidate, owned) in [
                (text.to_ascii_uppercase(), true),
                (format!("U{}", &text[1..]), false),
                ("UTEST1INVALID".to_string(), false),
                ("UREGTEST1INVALID".to_string(), false),
            ] {
                let input = CString::new(candidate.as_bytes()).unwrap();
                let parsed = Box::from_raw(parse_qrcode_text(input.as_ptr() as PtrString));
                assert_eq!(parsed.error_code, 0);
                assert_eq!(parsed.t, ViewType::ZcashAddressCheck);
                let borrowed = zcash_address_qr_text(parsed.data);
                assert_eq!(
                    cstr_core::CStr::from_ptr(borrowed).to_str().unwrap(),
                    candidate
                );
                let result = verify_zcash_address(key.as_ptr() as PtrString, borrowed as PtrString);
                if owned {
                    assert!(matches!(
                        result,
                        ZcashAddressVerification::ZcashAddressOwned
                    ));
                } else {
                    assert!(matches!(
                        result,
                        ZcashAddressVerification::ZcashAddressInvalid
                    ));
                }
                parsed.free();
            }
            for text in [
                "zcash:u1invalid".to_string(),
                format!("u1{}", "q".repeat(1024)),
            ] {
                let text = CString::new(text).unwrap();
                let parsed = Box::from_raw(parse_qrcode_text(text.as_ptr() as PtrString));
                assert_ne!(parsed.error_code, 0);
                parsed.free();
            }
            assert!(matches!(
                verify_zcash_address(core::ptr::null_mut(), core::ptr::null_mut()),
                ZcashAddressVerification::ZcashAddressKeyUnavailable
            ));
        }
    }
    use alloc::vec;

    use super::*;

    #[test]
    fn test_encode_full_response_returns_complete_ur_when_capped_encoder_would_fragment() {
        let capped_result = UREncodeResult::encode(
            vec![0; FRAGMENT_UNLIMITED_LENGTH + 1],
            "bytes".to_string(),
            FRAGMENT_UNLIMITED_LENGTH,
        );
        assert_eq!(capped_result.error_code, ErrorCodes::Success as u32);
        assert!(capped_result.is_multi_part);
        let capped_data = unsafe { recover_c_char(capped_result.data) };
        assert!(capped_data.contains("/1-"));
        unsafe {
            capped_result.free();
        }

        let full_result = UREncodeResult::encode_full_response(
            vec![0; FRAGMENT_UNLIMITED_LENGTH + 1],
            "bytes".to_string(),
        );
        assert_eq!(full_result.error_code, ErrorCodes::Success as u32);
        assert!(!full_result.is_multi_part);
        assert!(full_result.encoder.is_null());
        let full_data = unsafe { recover_c_char(full_result.data) };
        assert!(full_data.starts_with("UR:BYTES/"));
        assert!(!full_data.contains("/1-"));
        unsafe {
            full_result.free();
        }

        let zcash_sign_result = UREncodeResult::encode_full_response(
            vec![0; FRAGMENT_UNLIMITED_LENGTH + 1],
            "zcash-sign-result".to_string(),
        );
        assert_eq!(zcash_sign_result.error_code, ErrorCodes::Success as u32);
        assert!(!zcash_sign_result.is_multi_part);
        assert!(zcash_sign_result.encoder.is_null());
        let zcash_data = unsafe { recover_c_char(zcash_sign_result.data) };
        assert!(zcash_data.starts_with("UR:ZCASH-SIGN-RESULT/"));
        assert!(!zcash_data.contains("/1-"));
        unsafe {
            zcash_sign_result.free();
        }

        let zcash_batch_sig_result = UREncodeResult::encode_full_response(
            vec![0; FRAGMENT_UNLIMITED_LENGTH + 1],
            "zcash-batch-sig-result".to_string(),
        );
        assert_eq!(
            zcash_batch_sig_result.error_code,
            ErrorCodes::Success as u32
        );
        assert!(!zcash_batch_sig_result.is_multi_part);
        assert!(zcash_batch_sig_result.encoder.is_null());
        let zcash_data = unsafe { recover_c_char(zcash_batch_sig_result.data) };
        assert!(zcash_data.starts_with("UR:ZCASH-BATCH-SIG-RESULT/"));
        assert!(!zcash_data.contains("/1-"));
        unsafe {
            zcash_batch_sig_result.free();
        }
    }

    #[test]
    fn single_part_cbor_must_consume_the_entire_payload() {
        let valid = ur::encode(&[0xa0], "bytes");
        assert!(validate_single_part_cbor(&valid).is_ok());

        let trailing = ur::encode(&[0xa0, 0x00], "bytes");
        let error = validate_single_part_cbor(&trailing).unwrap_err();
        assert!(matches!(error, URError::CborDecodeError(_)));
        assert!(error.to_string().contains("trailing data"));
    }

    #[test]
    fn single_part_cbor_enforces_complexity_budgets() {
        let mut at_depth_limit = vec![0x81; sandbox_parser::MAX_NESTING_DEPTH as usize];
        at_depth_limit.push(0xf6);
        assert!(validate_single_part_cbor(&ur::encode(&at_depth_limit, "bytes")).is_ok());

        let mut over_depth_limit = vec![0x81; sandbox_parser::MAX_NESTING_DEPTH as usize + 1];
        over_depth_limit.push(0xf6);
        let error = validate_single_part_cbor(&ur::encode(&over_depth_limit, "bytes")).unwrap_err();
        assert!(error.to_string().contains("nesting limit"));

        let mut over_item_limit = vec![0x99, 0x04, 0x00];
        over_item_limit.extend(core::iter::repeat(0xf6).take(sandbox_parser::MAX_ITEMS as usize));
        let error = validate_single_part_cbor(&ur::encode(&over_item_limit, "bytes")).unwrap_err();
        assert!(error.to_string().contains("item limit"));
    }

    #[test]
    fn reconstructed_cbor_uses_sandbox_complexity_budgets() {
        let mut at_depth_limit = vec![0x81; 32];
        at_depth_limit.push(0xf6);
        assert!(validate_reassembled_cbor(&at_depth_limit).is_ok());

        let mut over_depth_limit = vec![0x81; 33];
        over_depth_limit.push(0xf6);
        let error = validate_reassembled_cbor(&over_depth_limit).unwrap_err();
        assert!(error.to_string().contains("status 11"));
    }
}

fn get_ur_type(ur: &String) -> Result<QRCodeType, URError> {
    let t = ur_parse_lib::keystone_ur_decoder::get_type(ur)?;
    QRCodeType::from(&t)
}

fn sandbox_cbor_error(status: sandbox_parser::ValidationStatus) -> URError {
    let reason = match status {
        sandbox_parser::ValidationStatus::InvalidInput => "invalid input".to_string(),
        sandbox_parser::ValidationStatus::CborUnexpectedEof => "end of input bytes".to_string(),
        sandbox_parser::ValidationStatus::CborInvalid => "invalid CBOR".to_string(),
        sandbox_parser::ValidationStatus::CborNestingLimit => format!(
            "nesting limit {} exceeded",
            sandbox_parser::MAX_NESTING_DEPTH
        ),
        sandbox_parser::ValidationStatus::CborItemLimit => {
            format!("item limit {} exceeded", sandbox_parser::MAX_ITEMS)
        }
        sandbox_parser::ValidationStatus::CborTrailingData => "trailing data".to_string(),
        _ => format!("CBOR validation failed with status {}", status as u32),
    };
    URError::CborDecodeError(reason)
}

fn validate_single_part_cbor(value: &str) -> Result<(), URError> {
    #[cfg(feature = "tron")]
    if super::ur_budget::is_tron(value) {
        super::ur_budget::validate_text_len(value).map_err(URError::UrDecodeError)?;
    }
    let (kind, cbor) =
        ur::decode(value).map_err(|error| URError::UrDecodeError(error.to_string()))?;
    if kind != ur::ur::Kind::SinglePart {
        return Ok(());
    }
    let status = sandbox_parser::validate_cbor(&cbor);
    if status != sandbox_parser::ValidationStatus::Ok {
        return Err(sandbox_cbor_error(status));
    }
    Ok(())
}

fn _decode_ur<T: RegistryItem + TryFrom<Vec<u8>, Error = URError> + InferViewType>(
    ur: String,
    u: QRCodeType,
) -> URParseResult {
    match ur::decode(&ur) {
        Ok((ur::ur::Kind::MultiPart, _)) => {
            return match KeystoneURDecoder::from_first_part(&ur) {
                Ok(decoder) => URParseResult::multi(
                    decoder.progress() as u32,
                    ViewType::ViewTypeUnKnown,
                    u,
                    decoder,
                ),
                Err(error) => URParseResult::from(error),
            };
        }
        Ok((ur::ur::Kind::SinglePart, _)) => {}
        Err(error) => {
            return URParseResult::from(URError::UrDecodeError(error.to_string()));
        }
    }

    let result: URResult<InnerParseResult<T>> = probe_decode(ur);
    match result {
        Ok(parse_result) => match parse_result.data {
            Some(data) => match InferViewType::infer(&data) {
                Ok(t) => URParseResult::single(t, u, data),
                Err(e) => URParseResult::from(e),
            },
            None => URParseResult::from(RustCError::UnexpectedError("ur data is none".to_string())),
        },
        Err(e) => URParseResult::from(e),
    }
}

pub fn decode_ur(ur: String) -> URParseResult {
    let ur = ur.trim().to_lowercase();
    if let Err(error) = validate_single_part_cbor(&ur) {
        return URParseResult::from(error);
    }
    let ur_type = get_ur_type(&ur);
    let ur_type = match ur_type {
        Ok(t) => t,
        Err(e) => return URParseResult::from(e),
    };

    match ur_type {
        #[cfg(feature = "bitcoin")]
        QRCodeType::CryptoPSBT => _decode_ur::<CryptoPSBT>(ur, ur_type),
        #[cfg(feature = "multi-coins")]
        QRCodeType::CryptoPSBTExtend => _decode_ur::<CryptoPSBTExtend>(ur, ur_type),
        #[cfg(feature = "bitcoin")]
        QRCodeType::CryptoAccount => _decode_ur::<CryptoAccount>(ur, ur_type),
        QRCodeType::CryptoMultiAccounts => _decode_ur::<CryptoMultiAccounts>(ur, ur_type),
        QRCodeType::Bytes => _decode_ur::<Bytes>(ur, ur_type),
        #[cfg(feature = "bitcoin")]
        QRCodeType::BtcSignRequest => _decode_ur::<BtcSignRequest>(ur, ur_type),
        #[cfg(feature = "multi-coins")]
        QRCodeType::KeystoneSignRequest => _decode_ur::<KeystoneSignRequest>(ur, ur_type),
        #[cfg(feature = "ethereum")]
        QRCodeType::EthSignRequest => _decode_ur::<EthSignRequest>(ur, ur_type),
        #[cfg(feature = "ethereum")]
        QRCodeType::EthBatchSignRequest => _decode_ur::<EthBatchSignRequest>(ur, ur_type),
        #[cfg(feature = "solana")]
        QRCodeType::SolSignRequest => _decode_ur::<SolSignRequest>(ur, ur_type),
        #[cfg(feature = "tron")]
        QRCodeType::TronSignRequest => _decode_ur::<TronSignRequest>(ur, ur_type),
        #[cfg(feature = "near")]
        QRCodeType::NearSignRequest => _decode_ur::<NearSignRequest>(ur, ur_type),
        #[cfg(feature = "cardano")]
        QRCodeType::CardanoSignRequest => _decode_ur::<CardanoSignRequest>(ur, ur_type),
        #[cfg(feature = "cardano")]
        QRCodeType::CardanoSignTxHashRequest => _decode_ur::<CardanoSignTxHashRequest>(ur, ur_type),
        #[cfg(feature = "cardano")]
        QRCodeType::CardanoSignDataRequest => _decode_ur::<CardanoSignDataRequest>(ur, ur_type),
        #[cfg(feature = "cardano")]
        QRCodeType::CardanoSignCip8DataRequest => {
            _decode_ur::<CardanoSignCip8DataRequest>(ur, ur_type)
        }
        #[cfg(feature = "cardano")]
        QRCodeType::CardanoCatalystVotingRegistrationRequest => {
            _decode_ur::<CardanoCatalystVotingRegistrationRequest>(ur, ur_type)
        }
        #[cfg(feature = "cosmos")]
        QRCodeType::CosmosSignRequest => _decode_ur::<CosmosSignRequest>(ur, ur_type),
        #[cfg(feature = "ethereum")]
        QRCodeType::EvmSignRequest => _decode_ur::<EvmSignRequest>(ur, ur_type),
        #[cfg(feature = "sui")]
        QRCodeType::SuiSignRequest => _decode_ur::<SuiSignRequest>(ur, ur_type),
        #[cfg(feature = "sui")]
        QRCodeType::SuiSignHashRequest => _decode_ur::<SuiSignHashRequest>(ur, ur_type),
        #[cfg(feature = "iota")]
        QRCodeType::IotaSignRequest => _decode_ur::<IotaSignRequest>(ur, ur_type),
        #[cfg(feature = "iota")]
        QRCodeType::IotaSignHashRequest => _decode_ur::<IotaSignHashRequest>(ur, ur_type),
        #[cfg(feature = "stellar")]
        QRCodeType::StellarSignRequest => _decode_ur::<StellarSignRequest>(ur, ur_type),
        #[cfg(feature = "arweave")]
        QRCodeType::ArweaveSignRequest => _decode_ur::<ArweaveSignRequest>(ur, ur_type),
        #[cfg(feature = "aptos")]
        QRCodeType::AptosSignRequest => _decode_ur::<AptosSignRequest>(ur, ur_type),
        #[cfg(feature = "ton")]
        QRCodeType::TonSignRequest => _decode_ur::<TonSignRequest>(ur, ur_type),
        #[cfg(feature = "zcash")]
        QRCodeType::ZcashPczt => _decode_ur::<ZcashPczt>(ur, ur_type),
        #[cfg(feature = "zcash_cypherpunk")]
        QRCodeType::ZcashSignBatch => _decode_ur::<ZcashSignBatch>(ur, ur_type),
        #[cfg(feature = "monero")]
        QRCodeType::XmrOutputSignRequest => _decode_ur::<XmrOutput>(ur, ur_type),
        #[cfg(feature = "monero")]
        QRCodeType::XmrTxUnsignedRequest => _decode_ur::<XmrTxUnsigned>(ur, ur_type),
        #[cfg(feature = "avalanche")]
        QRCodeType::AvaxSignRequest => _decode_ur::<AvaxSignRequest>(ur, ur_type),
        #[cfg(not(feature = "btc-only"))]
        QRCodeType::QRHardwareCall => _decode_ur::<QRHardwareCall>(ur, ur_type),
        #[cfg(feature = "cypherpunk")]
        QRCodeType::ZcashAddressText => URParseResult::from(RustCError::InvalidMessage),
        QRCodeType::URTypeUnKnown | QRCodeType::SeedSignerMessage => URParseResult::from(
            URError::NotSupportURTypeError("UnKnown ur type".to_string()),
        ),
    }
}

fn _receive_ur<T: RegistryItem + TryFrom<Vec<u8>, Error = URError> + InferViewType>(
    ur: String,
    u: QRCodeType,
    decoder: &mut KeystoneURDecoder,
) -> URParseMultiResult {
    if let Err(error) = decoder.receive(&ur) {
        return URParseMultiResult::from(error);
    }
    if !decoder.complete() {
        return URParseMultiResult::un_complete(ViewType::ViewTypeUnKnown, u, decoder.progress());
    }

    let cbor = match decoder.message() {
        Ok(cbor) => cbor,
        Err(error) => return URParseMultiResult::from(error),
    };
    if let Err(error) = validate_reassembled_cbor(&cbor) {
        return URParseMultiResult::from(error);
    }
    match T::try_from(cbor) {
        Ok(data) => match InferViewType::infer(&data) {
            Ok(t) => URParseMultiResult::success(t, u, data),
            Err(error) => URParseMultiResult::from(error),
        },
        Err(error) => URParseMultiResult::from(error),
    }
}

fn receive_ur(ur: String, decoder: &mut KeystoneURDecoder) -> URParseMultiResult {
    let ur = ur.trim().to_lowercase();
    let ur_type = get_ur_type(&ur);
    let ur_type = match ur_type {
        Ok(t) => t,
        Err(e) => return URParseMultiResult::from(e),
    };
    match ur_type {
        #[cfg(feature = "bitcoin")]
        QRCodeType::CryptoPSBT => _receive_ur::<CryptoPSBT>(ur, ur_type, decoder),
        #[cfg(feature = "multi-coins")]
        QRCodeType::CryptoPSBTExtend => _receive_ur::<CryptoPSBTExtend>(ur, ur_type, decoder),
        #[cfg(feature = "bitcoin")]
        QRCodeType::CryptoAccount => _receive_ur::<CryptoAccount>(ur, ur_type, decoder),
        QRCodeType::CryptoMultiAccounts => _receive_ur::<CryptoMultiAccounts>(ur, ur_type, decoder),
        QRCodeType::Bytes => _receive_ur::<Bytes>(ur, ur_type, decoder),
        #[cfg(feature = "bitcoin")]
        QRCodeType::BtcSignRequest => _receive_ur::<BtcSignRequest>(ur, ur_type, decoder),
        #[cfg(feature = "multi-coins")]
        QRCodeType::KeystoneSignRequest => _receive_ur::<KeystoneSignRequest>(ur, ur_type, decoder),
        #[cfg(feature = "ethereum")]
        QRCodeType::EthSignRequest => _receive_ur::<EthSignRequest>(ur, ur_type, decoder),
        #[cfg(feature = "ethereum")]
        QRCodeType::EthBatchSignRequest => _receive_ur::<EthBatchSignRequest>(ur, ur_type, decoder),
        #[cfg(feature = "solana")]
        QRCodeType::SolSignRequest => _receive_ur::<SolSignRequest>(ur, ur_type, decoder),
        #[cfg(feature = "tron")]
        QRCodeType::TronSignRequest => _receive_ur::<TronSignRequest>(ur, ur_type, decoder),
        #[cfg(feature = "near")]
        QRCodeType::NearSignRequest => _receive_ur::<NearSignRequest>(ur, ur_type, decoder),
        #[cfg(feature = "cardano")]
        QRCodeType::CardanoSignRequest => _receive_ur::<CardanoSignRequest>(ur, ur_type, decoder),
        #[cfg(feature = "cardano")]
        QRCodeType::CardanoSignTxHashRequest => {
            _receive_ur::<CardanoSignTxHashRequest>(ur, ur_type, decoder)
        }
        #[cfg(feature = "cardano")]
        QRCodeType::CardanoSignDataRequest => {
            _receive_ur::<CardanoSignDataRequest>(ur, ur_type, decoder)
        }
        #[cfg(feature = "cardano")]
        QRCodeType::CardanoSignCip8DataRequest => {
            _receive_ur::<CardanoSignCip8DataRequest>(ur, ur_type, decoder)
        }
        #[cfg(feature = "cardano")]
        QRCodeType::CardanoCatalystVotingRegistrationRequest => {
            _receive_ur::<CardanoCatalystVotingRegistrationRequest>(ur, ur_type, decoder)
        }
        #[cfg(feature = "cosmos")]
        QRCodeType::CosmosSignRequest => _receive_ur::<CosmosSignRequest>(ur, ur_type, decoder),
        #[cfg(feature = "ethereum")]
        QRCodeType::EvmSignRequest => _receive_ur::<EvmSignRequest>(ur, ur_type, decoder),
        #[cfg(feature = "sui")]
        QRCodeType::SuiSignRequest => _receive_ur::<SuiSignRequest>(ur, ur_type, decoder),
        #[cfg(feature = "sui")]
        QRCodeType::SuiSignHashRequest => _receive_ur::<SuiSignHashRequest>(ur, ur_type, decoder),
        #[cfg(feature = "iota")]
        QRCodeType::IotaSignRequest => _receive_ur::<IotaSignRequest>(ur, ur_type, decoder),
        #[cfg(feature = "iota")]
        QRCodeType::IotaSignHashRequest => _receive_ur::<IotaSignHashRequest>(ur, ur_type, decoder),
        #[cfg(feature = "arweave")]
        QRCodeType::ArweaveSignRequest => _receive_ur::<ArweaveSignRequest>(ur, ur_type, decoder),
        #[cfg(feature = "stellar")]
        QRCodeType::StellarSignRequest => _receive_ur::<StellarSignRequest>(ur, ur_type, decoder),
        #[cfg(feature = "aptos")]
        QRCodeType::AptosSignRequest => _receive_ur::<AptosSignRequest>(ur, ur_type, decoder),
        #[cfg(not(feature = "btc-only"))]
        QRCodeType::QRHardwareCall => _receive_ur::<QRHardwareCall>(ur, ur_type, decoder),
        #[cfg(feature = "ton")]
        QRCodeType::TonSignRequest => _receive_ur::<TonSignRequest>(ur, ur_type, decoder),
        #[cfg(feature = "zcash")]
        QRCodeType::ZcashPczt => _receive_ur::<ZcashPczt>(ur, ur_type, decoder),
        #[cfg(feature = "zcash_cypherpunk")]
        QRCodeType::ZcashSignBatch => _receive_ur::<ZcashSignBatch>(ur, ur_type, decoder),
        #[cfg(feature = "monero")]
        QRCodeType::XmrOutputSignRequest => _receive_ur::<XmrOutput>(ur, ur_type, decoder),
        #[cfg(feature = "monero")]
        QRCodeType::XmrTxUnsignedRequest => _receive_ur::<XmrTxUnsigned>(ur, ur_type, decoder),
        #[cfg(feature = "avalanche")]
        QRCodeType::AvaxSignRequest => _receive_ur::<AvaxSignRequest>(ur, ur_type, decoder),
        #[cfg(feature = "cypherpunk")]
        QRCodeType::ZcashAddressText => URParseMultiResult::from(RustCError::InvalidMessage),
        QRCodeType::URTypeUnKnown | QRCodeType::SeedSignerMessage => URParseMultiResult::from(
            URError::NotSupportURTypeError("UnKnown ur type".to_string()),
        ),
    }
}

#[no_mangle]
pub extern "C" fn get_next_part(ptr: PtrEncoder) -> *mut UREncodeMultiResult {
    let keystone_ur_encoder_ptr = ptr as *mut ur_parse_lib::keystone_ur_encoder::KeystoneUREncoder;
    let encoder = unsafe { &mut *keystone_ur_encoder_ptr };
    match encoder.next_part() {
        Ok(result) => UREncodeMultiResult::success(result.to_uppercase()).c_ptr(),
        Err(e) => UREncodeMultiResult::from(e).c_ptr(),
    }
}

#[no_mangle]
pub extern "C" fn get_next_cyclic_part(ptr: PtrEncoder) -> *mut UREncodeMultiResult {
    let keystone_ur_encoder_ptr = ptr as *mut ur_parse_lib::keystone_ur_encoder::KeystoneUREncoder;
    let encoder = unsafe { &mut *keystone_ur_encoder_ptr };
    match encoder.next_cyclic_part() {
        Ok(result) => UREncodeMultiResult::success(result.to_lowercase()).c_ptr(),
        Err(e) => UREncodeMultiResult::from(e).c_ptr(),
    }
}

#[no_mangle]
pub unsafe extern "C" fn parse_ur(ur: PtrString) -> *mut URParseResult {
    if ur.is_null() {
        return URParseResult::from(RustCError::InvalidData("UR payload is null".to_string()))
            .c_ptr();
    }
    let (is_valid_utf8, ur) = check_recover_c_char_lossy(ur);
    if !is_valid_utf8 {
        return URParseResult::from(RustCError::InvalidData(
            "UR payload is not valid UTF-8".to_string(),
        ))
        .c_ptr();
    }
    decode_ur(ur).c_ptr()
}

#[no_mangle]
pub unsafe extern "C" fn receive(ur: PtrString, decoder: PtrDecoder) -> *mut URParseMultiResult {
    if ur.is_null() {
        return URParseMultiResult::from(RustCError::InvalidData("UR payload is null".to_string()))
            .c_ptr();
    }
    let (is_valid_utf8, ur) = check_recover_c_char_lossy(ur);
    if !is_valid_utf8 {
        return URParseMultiResult::from(RustCError::InvalidData(
            "UR payload is not valid UTF-8".to_string(),
        ))
        .c_ptr();
    }
    let decoder = extract_ptr_with_type!(decoder, KeystoneURDecoder);
    receive_ur(ur, decoder).c_ptr()
}

#[cfg(all(test, feature = "tron"))]
mod tron_limit_tests {
    use super::*;
    use alloc::vec;

    fn fragment(parts: u32, message_len: u32, data: &[u8]) -> String {
        let mut e = minicbor::Encoder::new(Vec::new());
        e.array(5)
            .unwrap()
            .u32(1)
            .unwrap()
            .u32(parts)
            .unwrap()
            .u32(message_len)
            .unwrap()
            .u32(0)
            .unwrap()
            .bytes(data)
            .unwrap();
        ur::encode(&e.into_writer(), "tron-sign-request").replacen(
            "tron-sign-request/",
            &format!("tron-sign-request/1-{parts}/"),
            1,
        )
    }

    #[test]
    fn tron_limits_enforce_frame_text_length() {
        for (payload_len, accepted) in [(750, true), (751, false)] {
            let mut encoder = minicbor::Encoder::new(Vec::new());
            encoder.bytes(&vec![0; payload_len]).unwrap();
            let single = ur::encode(&encoder.into_writer(), "tron-sign-request");
            assert_eq!(single.len(), if accepted { 1535 } else { 1537 });
            assert_eq!(validate_single_part_cbor(&single).is_ok(), accepted);
            assert_eq!(
                super::super::ur_budget::validate_text_len(&single.to_uppercase()).is_ok(),
                accepted
            );
        }
        let oversized_part = fragment(2, 2000, &[0; 1000]);
        assert!(validate_single_part_cbor(&oversized_part).is_err());
        assert!(KeystoneURDecoder::from_first_part(&oversized_part).is_err());
    }

    #[test]
    fn tron_limits_reject_oversized_single_and_fountain_metadata() {
        let max_cbor_bytes = app_tron::MAX_SIGN_DATA_LEN + 1024;
        let too_big = ur::encode(&vec![0; max_cbor_bytes + 1], "tron-sign-request");
        assert!(validate_single_part_cbor(&too_big)
            .unwrap_err()
            .to_string()
            .contains("1536-byte limit"));
        for bad in [
            fragment(100, max_cbor_bytes as u32 + 1, &[0; 200]),
            fragment(129, 129, &[0]),
            fragment(u32::MAX, 1, &[0]),
            fragment(0, 1, &[0]),
            fragment(2, 1, &[0]),
            fragment(2, 3, &[0]),
        ] {
            assert!(KeystoneURDecoder::from_first_part(&bad).is_err());
        }
    }

    #[test]
    fn tron_limits_reassemble_full_size_request_with_reordered_parts() {
        use ur_registry::crypto_key_path::CryptoKeyPath;
        use ur_registry::tron::tron_sign_request::DataType;
        let req = TronSignRequest::new(
            None,
            vec![b'a'; app_tron::MAX_SIGN_DATA_LEN],
            DataType::PersonalMessage,
            CryptoKeyPath::from_path("m/44'/195'/0'/0/0".into(), Some([0x73, 0xc5, 0xda, 0x0a]))
                .unwrap(),
            None,
            None,
        );
        let cbor = minicbor::to_vec(req).unwrap();
        let mut encoder = ur::Encoder::new(&cbor, 200, "tron-sign-request").unwrap();
        let mut parts: Vec<String> = (0..encoder.fragment_count())
            .map(|_| encoder.next_part().unwrap())
            .collect();
        parts.reverse();
        let first = encoder.next_part().unwrap();
        let mut decoder = KeystoneURDecoder::from_first_part(&first).unwrap();
        for part in parts {
            decoder.receive(&part).unwrap();
        }
        assert!(decoder.complete());
        assert_eq!(decoder.message().unwrap(), cbor);
        let decoded = TronSignRequest::try_from(decoder.message().unwrap()).unwrap();
        assert_eq!(decoded.get_sign_data().len(), app_tron::MAX_SIGN_DATA_LEN);
        assert!(validate_single_part_cbor(&ur::encode(&cbor, "tron-sign-request")).is_err());
    }

    #[test]
    fn tron_limits_bound_redundant_fragments_and_prevent_type_switching() {
        let cbor = vec![0; 1024];
        let mut encoder = ur::Encoder::new(&cbor, 200, "tron-sign-request").unwrap();
        let part = encoder.next_part().unwrap();
        let mut decoder = KeystoneURDecoder::from_first_part(&part).unwrap();
        for _ in 0..2000 {
            decoder.receive(&part).unwrap();
        }
        let mut rejected = false;
        for _ in 0..2000 {
            if decoder.receive(&encoder.next_part().unwrap()).is_err() {
                rejected = true;
                break;
            }
        }
        assert!(rejected);
        assert!(!decoder.complete());
        assert!(decoder.message().is_err());
        let other = part.replacen("tron-sign-request", "bytes", 1);
        let mut decoder = KeystoneURDecoder::from_first_part(&other).unwrap();
        assert!(decoder.receive(&part).is_err());
        let mut decoder = KeystoneURDecoder::from_first_part(&part).unwrap();
        assert!(decoder.receive(&other).is_err());
    }
}

#[cfg(all(test, feature = "monero"))]
mod monero_budget_tests {
    use super::*;

    #[test]
    fn monero_reassembly_boundaries() {
        let cbor = vec![0; app_monero::utils::io::MAX_REQUEST_BYTES];
        let mut encoder = ur::Encoder::new(&cbor, 200, "xmr-txunsigned").unwrap();
        let mut parts: Vec<String> = (0..encoder.fragment_count())
            .map(|_| encoder.next_part().unwrap())
            .collect();
        parts.reverse();
        let mut decoder = KeystoneURDecoder::from_first_part(&parts[0]).unwrap();
        for part in &parts[1..] {
            decoder.receive(part).unwrap();
        }
        assert!(decoder.complete());
        assert_eq!(decoder.message().unwrap(), cbor);
        let too_large = vec![0; cbor.len() + 1];
        let mut encoder = ur::Encoder::new(&too_large, 200, "xmr-txunsigned").unwrap();
        assert!(KeystoneURDecoder::from_first_part(&encoder.next_part().unwrap()).is_err());
    }
}
