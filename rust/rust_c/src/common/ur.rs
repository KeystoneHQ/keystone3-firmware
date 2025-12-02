use alloc::boxed::Box;
use alloc::format;
use alloc::string::{String, ToString};
use alloc::vec::Vec;
use core::ptr::null_mut;
use ur_registry::ethereum::eth_batch_sign_requests::EthBatchSignRequest;

use cstr_core::CString;
use cty::c_char;
use keystore::errors::KeystoreError;
use ur_parse_lib::keystone_ur_decoder::{
    probe_decode, KeystoneURDecoder, MultiURParseResult as InnerMultiParseResult,
    URParseResult as InnerParseResult,
};
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
#[cfg(feature = "xrp")]
use ur_registry::xrp::xrp_batch_sign_request::XrpBatchSignRequest;
#[cfg(feature = "zcash")]
use ur_registry::zcash::zcash_pczt::ZcashPczt;

use super::errors::{ErrorCodes, RustCError};
use super::free::Free;
use super::types::{PtrDecoder, PtrEncoder, PtrString, PtrUR};
use super::ur_ext::InferViewType;
use super::utils::{convert_c_char, recover_c_char};
use crate::{
    extract_ptr_with_type, free_ptr_with_type, free_str_ptr, impl_c_ptr, impl_new_error,
    impl_response,
};

#[no_mangle]
pub static FRAGMENT_MAX_LENGTH_DEFAULT: usize = 200;
pub static FRAGMENT_UNLIMITED_LENGTH: usize = 11000;

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

    pub fn encode(data: Vec<u8>, tag: String, max_fragment_length: usize) -> Self {
        let result =
            ur_parse_lib::keystone_ur_encoder::probe_encode(&data, max_fragment_length, tag);
        match result {
            Ok(result) => {
                if result.is_multi_part {
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
    BatchCall,
    #[cfg(feature = "btc-only")]
    MultisigWalletImport,
    #[cfg(feature = "btc-only")]
    MultisigCryptoImportXpub,
    #[cfg(feature = "btc-only")]
    MultisigBytesImportXpub,
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
    #[cfg(feature = "multi-coins")]
    KeystoneSignRequest,
    #[cfg(feature = "ethereum")]
    EthSignRequest,
    #[cfg(feature = "ethereum")]
    EthBatchSignRequest,
    #[cfg(feature = "solana")]
    SolSignRequest,
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
    #[cfg(feature = "xrp")]
    XrpBatchSignRequest,
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
            #[cfg(feature = "xrp")]
            InnerURType::XrpBatchSignRequest(_) => Ok(QRCodeType::XrpBatchSignRequest),
            #[cfg(feature = "zcash")]
            InnerURType::ZcashPczt(_) => Ok(QRCodeType::ZcashPczt),
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

    pub fn multi(
        progress: u32,
        t: ViewType,
        ur_type: QRCodeType,
        decoder: KeystoneURDecoder,
    ) -> Self {
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
        #[cfg(feature = "xrp")]
        QRCodeType::XrpBatchSignRequest => {
            free_ptr_with_type!(data, XrpBatchSignRequest);
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

fn get_ur_type(ur: &String) -> Result<QRCodeType, URError> {
    let t = ur_parse_lib::keystone_ur_decoder::get_type(ur)?;
    QRCodeType::from(&t)
}

fn _decode_ur<T: RegistryItem + TryFrom<Vec<u8>, Error = URError> + InferViewType>(
    ur: String,
    u: QRCodeType,
) -> URParseResult {
    let result: URResult<InnerParseResult<T>> = probe_decode(ur);
    match result {
        Ok(parse_result) => {
            if parse_result.is_multi_part {
                match parse_result.decoder {
                    Some(decoder) => URParseResult::multi(
                        parse_result.progress as u32,
                        ViewType::ViewTypeUnKnown,
                        u,
                        decoder,
                    ),
                    None => URParseResult::from(RustCError::UnexpectedError(
                        "ur decoder is none".to_string(),
                    )),
                }
            } else {
                match parse_result.data {
                    Some(data) => match InferViewType::infer(&data) {
                        Ok(t) => URParseResult::single(t, u, data),
                        Err(e) => URParseResult::from(e),
                    },
                    None => URParseResult::from(RustCError::UnexpectedError(
                        "ur data is none".to_string(),
                    )),
                }
            }
        }
        Err(e) => URParseResult::from(e),
    }
}

pub fn decode_ur(ur: String) -> URParseResult {
    let ur = ur.trim().to_lowercase();
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
        #[cfg(feature = "xrp")]
        QRCodeType::XrpBatchSignRequest => _decode_ur::<XrpBatchSignRequest>(ur, ur_type),
        #[cfg(feature = "zcash")]
        QRCodeType::ZcashPczt => _decode_ur::<ZcashPczt>(ur, ur_type),
        #[cfg(feature = "monero")]
        QRCodeType::XmrOutputSignRequest => _decode_ur::<XmrOutput>(ur, ur_type),
        #[cfg(feature = "monero")]
        QRCodeType::XmrTxUnsignedRequest => _decode_ur::<XmrTxUnsigned>(ur, ur_type),
        #[cfg(feature = "avalanche")]
        QRCodeType::AvaxSignRequest => _decode_ur::<AvaxSignRequest>(ur, ur_type),
        #[cfg(not(feature = "btc-only"))]
        QRCodeType::QRHardwareCall => _decode_ur::<QRHardwareCall>(ur, ur_type),
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
    let result: URResult<InnerMultiParseResult<T>> = decoder.parse_ur(ur);
    match result {
        Ok(parse_result) => {
            if parse_result.is_complete {
                match parse_result.data {
                    Some(data) => match InferViewType::infer(&data) {
                        Ok(t) => URParseMultiResult::success(t, u, data),
                        Err(e) => URParseMultiResult::from(e),
                    },
                    None => URParseMultiResult::from(RustCError::UnexpectedError(
                        "UR parsed completely but data is none".to_string(),
                    )),
                }
            } else {
                URParseMultiResult::un_complete(ViewType::ViewTypeUnKnown, u, parse_result.progress)
            }
        }
        Err(e) => URParseMultiResult::from(e),
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
        #[cfg(feature = "xrp")]
        QRCodeType::XrpBatchSignRequest => _receive_ur::<XrpBatchSignRequest>(ur, ur_type, decoder),
        #[cfg(feature = "zcash")]
        QRCodeType::ZcashPczt => _receive_ur::<ZcashPczt>(ur, ur_type, decoder),
        #[cfg(feature = "monero")]
        QRCodeType::XmrOutputSignRequest => _receive_ur::<XmrOutput>(ur, ur_type, decoder),
        #[cfg(feature = "monero")]
        QRCodeType::XmrTxUnsignedRequest => _receive_ur::<XmrTxUnsigned>(ur, ur_type, decoder),
        #[cfg(feature = "avalanche")]
        QRCodeType::AvaxSignRequest => _receive_ur::<AvaxSignRequest>(ur, ur_type, decoder),
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
    decode_ur(recover_c_char(ur)).c_ptr()
}

#[no_mangle]
pub unsafe extern "C" fn receive(ur: PtrString, decoder: PtrDecoder) -> *mut URParseMultiResult {
    let decoder = extract_ptr_with_type!(decoder, KeystoneURDecoder);
    receive_ur(recover_c_char(ur), decoder).c_ptr()
}
