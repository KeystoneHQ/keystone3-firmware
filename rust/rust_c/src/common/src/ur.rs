use alloc::boxed::Box;
use alloc::format;
use alloc::string::{String, ToString};
use alloc::vec::Vec;
use ur_registry::zcash::zcash_pczt::ZcashPczt;
use core::ptr::null_mut;
#[cfg(feature = "multi-coins")]
use ur_registry::aptos::aptos_sign_request::AptosSignRequest;
#[cfg(feature = "multi-coins")]
use ur_registry::extend::qr_hardware_call::QRHardwareCall;

use app_bitcoin::errors::BitcoinError;
#[cfg(feature = "multi-coins")]
use app_ethereum::errors::EthereumError;
use cstr_core::CString;
use cty::c_char;
use keystore::errors::KeystoreError;
use ur_parse_lib::keystone_ur_decoder::{
    probe_decode, KeystoneURDecoder, MultiURParseResult as InnerMultiParseResult,
    URParseResult as InnerParseResult,
};
use ur_parse_lib::keystone_ur_encoder::KeystoneUREncoder;
#[cfg(feature = "multi-coins")]
use ur_registry::arweave::arweave_sign_request::ArweaveSignRequest;
use ur_registry::bitcoin::btc_sign_request::BtcSignRequest;
use ur_registry::bytes::Bytes;
#[cfg(feature = "multi-coins")]
use ur_registry::cardano::{
    cardano_catalyst_voting_registration::CardanoCatalystVotingRegistrationRequest,
    cardano_sign_data_request::CardanoSignDataRequest, cardano_sign_request::CardanoSignRequest,
};
#[cfg(feature = "multi-coins")]
use ur_registry::cosmos::cosmos_sign_request::CosmosSignRequest;
#[cfg(feature = "multi-coins")]
use ur_registry::cosmos::evm_sign_request::EvmSignRequest;
use ur_registry::crypto_account::CryptoAccount;
use ur_registry::crypto_psbt::CryptoPSBT;
use ur_registry::error::{URError, URResult};
#[cfg(feature = "multi-coins")]
use ur_registry::ethereum::eth_sign_request::EthSignRequest;
use ur_registry::extend::crypto_multi_accounts::CryptoMultiAccounts;
#[cfg(feature = "multi-coins")]
use ur_registry::keystone::keystone_sign_request::KeystoneSignRequest;
#[cfg(feature = "multi-coins")]
use ur_registry::near::near_sign_request::NearSignRequest;
use ur_registry::registry_types::URType as InnerURType;
#[cfg(feature = "multi-coins")]
use ur_registry::solana::sol_sign_request::SolSignRequest;
#[cfg(feature = "multi-coins")]
use ur_registry::stellar::stellar_sign_request::StellarSignRequest;
#[cfg(feature = "multi-coins")]
use ur_registry::sui::sui_sign_hash_request::SuiSignHashRequest;
#[cfg(feature = "multi-coins")]
use ur_registry::sui::sui_sign_request::SuiSignRequest;
#[cfg(feature = "multi-coins")]
use ur_registry::ton::ton_sign_request::TonSignRequest;
use ur_registry::traits::RegistryItem;

use crate::errors::{ErrorCodes, RustCError};
use crate::free::Free;
use crate::types::{PtrDecoder, PtrEncoder, PtrString, PtrUR};
use crate::ur_ext::InferViewType;
use crate::utils::{convert_c_char, recover_c_char};
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
                        None => {
                            Self::from(RustCError::UnexpectedError(format!("ur encoder is none")))
                        }
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
    fn free(&self) {
        unsafe {
            free_str_ptr!(self.data);
            free_str_ptr!(self.error_message);
            free_ptr_with_type!(self.encoder, KeystoneUREncoder);
        }
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
            data: convert_c_char(data.to_uppercase()),
            ..Self::new()
        }
    }
}

impl Free for UREncodeMultiResult {
    fn free(&self) {
        free_str_ptr!(self.data);
        free_str_ptr!(self.error_message);
    }
}

impl_response!(UREncodeMultiResult);

#[repr(C)]
#[derive(Debug, Eq, PartialEq)]
pub enum ViewType {
    BtcNativeSegwitTx,
    BtcSegwitTx,
    BtcLegacyTx,
    BtcTx,
    BtcMsg,
    #[cfg(feature = "multi-coins")]
    LtcTx,
    #[cfg(feature = "multi-coins")]
    DashTx,
    #[cfg(feature = "multi-coins")]
    BchTx,
    #[cfg(feature = "multi-coins")]
    EthTx,
    #[cfg(feature = "multi-coins")]
    EthPersonalMessage,
    #[cfg(feature = "multi-coins")]
    EthTypedData,
    #[cfg(feature = "multi-coins")]
    TronTx,
    #[cfg(feature = "multi-coins")]
    SolanaTx,
    #[cfg(feature = "multi-coins")]
    SolanaMessage,
    #[cfg(feature = "multi-coins")]
    CardanoTx,
    #[cfg(feature = "multi-coins")]
    CardanoSignData,
    #[cfg(feature = "multi-coins")]
    CardanoCatalystVotingRegistration,
    #[cfg(feature = "multi-coins")]
    NearTx,
    #[cfg(feature = "multi-coins")]
    XRPTx,
    #[cfg(feature = "multi-coins")]
    CosmosTx,
    #[cfg(feature = "multi-coins")]
    CosmosEvmTx,
    #[cfg(feature = "multi-coins")]
    SuiTx,
    #[cfg(feature = "multi-coins")]
    SuiSignMessageHash,
    #[cfg(feature = "multi-coins")]
    ArweaveTx,
    #[cfg(feature = "multi-coins")]
    ArweaveMessage,
    #[cfg(feature = "multi-coins")]
    ArweaveDataItem,
    #[cfg(feature = "multi-coins")]
    StellarTx,
    #[cfg(feature = "multi-coins")]
    StellarHash,
    #[cfg(feature = "multi-coins")]
    TonTx,
    #[cfg(feature = "multi-coins")]
    TonSignProof,
    #[cfg(feature = "multi-coins")]
    ZcashTx,
    #[cfg(feature = "multi-coins")]
    AptosTx,
    WebAuthResult,
    #[cfg(feature = "multi-coins")]
    KeyDerivationRequest,
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
    CryptoPSBT,
    CryptoMultiAccounts,
    CryptoAccount,
    Bytes,
    BtcSignRequest,
    SeedSignerMessage,
    #[cfg(feature = "multi-coins")]
    KeystoneSignRequest,
    #[cfg(feature = "multi-coins")]
    EthSignRequest,
    #[cfg(feature = "multi-coins")]
    SolSignRequest,
    #[cfg(feature = "multi-coins")]
    NearSignRequest,
    #[cfg(feature = "multi-coins")]
    CardanoSignRequest,
    #[cfg(feature = "multi-coins")]
    CardanoSignDataRequest,
    #[cfg(feature = "multi-coins")]
    CardanoCatalystVotingRegistrationRequest,
    #[cfg(feature = "multi-coins")]
    CosmosSignRequest,
    #[cfg(feature = "multi-coins")]
    EvmSignRequest,
    #[cfg(feature = "multi-coins")]
    SuiSignRequest,
    #[cfg(feature = "multi-coins")]
    SuiSignHashRequest,
    #[cfg(feature = "multi-coins")]
    AptosSignRequest,
    #[cfg(feature = "multi-coins")]
    QRHardwareCall,
    #[cfg(feature = "multi-coins")]
    ArweaveSignRequest,
    #[cfg(feature = "multi-coins")]
    StellarSignRequest,
    #[cfg(feature = "multi-coins")]
    TonSignRequest,
    #[cfg(feature = "multi-coins")]
    ZcashPczt,
    URTypeUnKnown,
}

impl QRCodeType {
    pub fn from(value: &InnerURType) -> Result<Self, URError> {
        match value {
            InnerURType::CryptoPsbt(_) => Ok(QRCodeType::CryptoPSBT),
            InnerURType::CryptoMultiAccounts(_) => Ok(QRCodeType::CryptoMultiAccounts),
            InnerURType::CryptoAccount(_) => Ok(QRCodeType::CryptoAccount),
            InnerURType::Bytes(_) => Ok(QRCodeType::Bytes),
            InnerURType::BtcSignRequest(_) => Ok(QRCodeType::BtcSignRequest),
            #[cfg(feature = "multi-coins")]
            InnerURType::KeystoneSignRequest(_) => Ok(QRCodeType::KeystoneSignRequest),
            #[cfg(feature = "multi-coins")]
            InnerURType::EthSignRequest(_) => Ok(QRCodeType::EthSignRequest),
            #[cfg(feature = "multi-coins")]
            InnerURType::SolSignRequest(_) => Ok(QRCodeType::SolSignRequest),
            #[cfg(feature = "multi-coins")]
            InnerURType::NearSignRequest(_) => Ok(QRCodeType::NearSignRequest),
            #[cfg(feature = "multi-coins")]
            InnerURType::CosmosSignRequest(_) => Ok(QRCodeType::CosmosSignRequest),
            #[cfg(feature = "multi-coins")]
            InnerURType::EvmSignRequest(_) => Ok(QRCodeType::EvmSignRequest),
            #[cfg(feature = "multi-coins")]
            InnerURType::SuiSignRequest(_) => Ok(QRCodeType::SuiSignRequest),
            #[cfg(feature = "multi-coins")]
            InnerURType::SuiSignHashRequest(_) => Ok(QRCodeType::SuiSignHashRequest),
            #[cfg(feature = "multi-coins")]
            InnerURType::StellarSignRequest(_) => Ok(QRCodeType::StellarSignRequest),
            #[cfg(feature = "multi-coins")]
            InnerURType::ArweaveSignRequest(_) => Ok(QRCodeType::ArweaveSignRequest),
            #[cfg(feature = "multi-coins")]
            InnerURType::AptosSignRequest(_) => Ok(QRCodeType::AptosSignRequest),
            #[cfg(feature = "multi-coins")]
            InnerURType::CardanoSignRequest(_) => Ok(QRCodeType::CardanoSignRequest),
            #[cfg(feature = "multi-coins")]
            InnerURType::CardanoSignDataRequest(_) => Ok(QRCodeType::CardanoSignDataRequest),
            #[cfg(feature = "multi-coins")]
            InnerURType::CardanoCatalystVotingRegistrationRequest(_) => {
                Ok(QRCodeType::CardanoCatalystVotingRegistrationRequest)
            }
            #[cfg(feature = "multi-coins")]
            InnerURType::TonSignRequest(_) => Ok(QRCodeType::TonSignRequest),
            #[cfg(feature = "multi-coins")]
            InnerURType::ZcashPczt(_) => Ok(QRCodeType::ZcashPczt),
            #[cfg(feature = "multi-coins")]
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
    fn free(&self) {
        free_str_ptr!(self.error_message);
        free_ptr_with_type!(self.decoder, KeystoneURDecoder);
        free_ur(&self.ur_type, self.data);
    }
}

fn free_ur(ur_type: &QRCodeType, data: PtrUR) {
    match ur_type {
        QRCodeType::CryptoPSBT => {
            free_ptr_with_type!(data, CryptoPSBT);
        }
        QRCodeType::CryptoMultiAccounts => {
            free_ptr_with_type!(data, CryptoMultiAccounts);
        }
        QRCodeType::CryptoAccount => {
            free_ptr_with_type!(data, CryptoAccount);
        }
        #[cfg(feature = "multi-coins")]
        QRCodeType::EthSignRequest => {
            free_ptr_with_type!(data, EthSignRequest);
        }
        #[cfg(feature = "multi-coins")]
        QRCodeType::SolSignRequest => {
            free_ptr_with_type!(data, SolSignRequest);
        }
        #[cfg(feature = "multi-coins")]
        QRCodeType::NearSignRequest => {
            free_ptr_with_type!(data, NearSignRequest);
        }
        QRCodeType::Bytes => {
            free_ptr_with_type!(data, Bytes);
        }
        QRCodeType::BtcSignRequest => {
            free_ptr_with_type!(data, BtcSignRequest);
        }
        #[cfg(feature = "multi-coins")]
        QRCodeType::KeystoneSignRequest => {
            free_ptr_with_type!(data, KeystoneSignRequest);
        }
        #[cfg(feature = "multi-coins")]
        QRCodeType::CosmosSignRequest => {
            free_ptr_with_type!(data, CosmosSignRequest);
        }
        #[cfg(feature = "multi-coins")]
        QRCodeType::EvmSignRequest => {
            free_ptr_with_type!(data, EvmSignRequest);
        }
        #[cfg(feature = "multi-coins")]
        QRCodeType::SuiSignRequest => {
            free_ptr_with_type!(data, SuiSignRequest);
        }
        #[cfg(feature = "multi-coins")]
        QRCodeType::SuiSignHashRequest => {
            free_ptr_with_type!(data, SuiSignHashRequest);
        }

        #[cfg(feature = "multi-coins")]
        QRCodeType::StellarSignRequest => {
            free_ptr_with_type!(data, StellarSignRequest);
        }
        #[cfg(feature = "multi-coins")]
        QRCodeType::ArweaveSignRequest => {
            free_ptr_with_type!(data, ArweaveSignRequest);
        }
        #[cfg(feature = "multi-coins")]
        QRCodeType::AptosSignRequest => {
            free_ptr_with_type!(data, AptosSignRequest);
        }
        #[cfg(feature = "multi-coins")]
        QRCodeType::CardanoSignRequest => {
            free_ptr_with_type!(data, CardanoSignRequest);
        }
        #[cfg(feature = "multi-coins")]
        QRCodeType::CardanoSignDataRequest => {
            free_ptr_with_type!(data, CardanoSignDataRequest);
        }
        #[cfg(feature = "multi-coins")]
        QRCodeType::CardanoCatalystVotingRegistrationRequest => {
            free_ptr_with_type!(data, CardanoCatalystVotingRegistrationRequest);
        }
        #[cfg(feature = "multi-coins")]
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
    fn free(&self) {
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
                    Some(data) => {
                        return match InferViewType::infer(&data) {
                            Ok(t) => URParseResult::single(t, u, data),
                            Err(e) => URParseResult::from(e),
                        };
                    }
                    None => URParseResult::from(RustCError::UnexpectedError(
                        "ur data is none".to_string(),
                    )),
                }
            }
        }
        Err(e) => {
            return URParseResult::from(e);
        }
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
        QRCodeType::CryptoPSBT => _decode_ur::<CryptoPSBT>(ur, ur_type),
        QRCodeType::CryptoAccount => _decode_ur::<CryptoAccount>(ur, ur_type),
        QRCodeType::CryptoMultiAccounts => _decode_ur::<CryptoMultiAccounts>(ur, ur_type),
        QRCodeType::Bytes => _decode_ur::<Bytes>(ur, ur_type),
        QRCodeType::BtcSignRequest => _decode_ur::<BtcSignRequest>(ur, ur_type),
        #[cfg(feature = "multi-coins")]
        QRCodeType::KeystoneSignRequest => _decode_ur::<KeystoneSignRequest>(ur, ur_type),
        #[cfg(feature = "multi-coins")]
        QRCodeType::EthSignRequest => _decode_ur::<EthSignRequest>(ur, ur_type),
        #[cfg(feature = "multi-coins")]
        QRCodeType::SolSignRequest => _decode_ur::<SolSignRequest>(ur, ur_type),
        #[cfg(feature = "multi-coins")]
        QRCodeType::NearSignRequest => _decode_ur::<NearSignRequest>(ur, ur_type),
        #[cfg(feature = "multi-coins")]
        QRCodeType::CardanoSignRequest => _decode_ur::<CardanoSignRequest>(ur, ur_type),
        #[cfg(feature = "multi-coins")]
        QRCodeType::CardanoSignDataRequest => _decode_ur::<CardanoSignDataRequest>(ur, ur_type),
        #[cfg(feature = "multi-coins")]
        QRCodeType::CardanoCatalystVotingRegistrationRequest => {
            _decode_ur::<CardanoCatalystVotingRegistrationRequest>(ur, ur_type)
        }
        #[cfg(feature = "multi-coins")]
        QRCodeType::CosmosSignRequest => _decode_ur::<CosmosSignRequest>(ur, ur_type),
        #[cfg(feature = "multi-coins")]
        QRCodeType::EvmSignRequest => _decode_ur::<EvmSignRequest>(ur, ur_type),
        #[cfg(feature = "multi-coins")]
        QRCodeType::SuiSignRequest => _decode_ur::<SuiSignRequest>(ur, ur_type),
        #[cfg(feature = "multi-coins")]
        QRCodeType::SuiSignHashRequest => _decode_ur::<SuiSignHashRequest>(ur, ur_type),
        #[cfg(feature = "multi-coins")]
        QRCodeType::StellarSignRequest => _decode_ur::<StellarSignRequest>(ur, ur_type),
        #[cfg(feature = "multi-coins")]
        QRCodeType::ArweaveSignRequest => _decode_ur::<ArweaveSignRequest>(ur, ur_type),
        #[cfg(feature = "multi-coins")]
        QRCodeType::AptosSignRequest => _decode_ur::<AptosSignRequest>(ur, ur_type),
        #[cfg(feature = "multi-coins")]
        QRCodeType::TonSignRequest => _decode_ur::<TonSignRequest>(ur, ur_type),
        #[cfg(feature = "multi-coins")]
        QRCodeType::QRHardwareCall => _decode_ur::<QRHardwareCall>(ur, ur_type),
        #[cfg(feature = "multi-coins")]
        QRCodeType::ZcashPczt => _decode_ur::<ZcashPczt>(ur, ur_type),
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
                    Some(data) => {
                        return match InferViewType::infer(&data) {
                            Ok(t) => URParseMultiResult::success(t, u, data),
                            Err(e) => URParseMultiResult::from(e),
                        };
                    }
                    None => URParseMultiResult::from(RustCError::UnexpectedError(
                        "UR parsed completely but data is none".to_string(),
                    )),
                }
            } else {
                URParseMultiResult::un_complete(ViewType::ViewTypeUnKnown, u, parse_result.progress)
            }
        }
        Err(e) => {
            return URParseMultiResult::from(e);
        }
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
        QRCodeType::CryptoPSBT => _receive_ur::<CryptoPSBT>(ur, ur_type, decoder),
        QRCodeType::CryptoAccount => _receive_ur::<CryptoAccount>(ur, ur_type, decoder),
        QRCodeType::CryptoMultiAccounts => _receive_ur::<CryptoMultiAccounts>(ur, ur_type, decoder),
        QRCodeType::Bytes => _receive_ur::<Bytes>(ur, ur_type, decoder),
        QRCodeType::BtcSignRequest => _receive_ur::<BtcSignRequest>(ur, ur_type, decoder),
        #[cfg(feature = "multi-coins")]
        QRCodeType::KeystoneSignRequest => _receive_ur::<KeystoneSignRequest>(ur, ur_type, decoder),
        #[cfg(feature = "multi-coins")]
        QRCodeType::EthSignRequest => _receive_ur::<EthSignRequest>(ur, ur_type, decoder),
        #[cfg(feature = "multi-coins")]
        QRCodeType::SolSignRequest => _receive_ur::<SolSignRequest>(ur, ur_type, decoder),
        #[cfg(feature = "multi-coins")]
        QRCodeType::NearSignRequest => _receive_ur::<NearSignRequest>(ur, ur_type, decoder),
        #[cfg(feature = "multi-coins")]
        QRCodeType::CardanoSignRequest => _receive_ur::<CardanoSignRequest>(ur, ur_type, decoder),
        #[cfg(feature = "multi-coins")]
        QRCodeType::CardanoSignDataRequest => {
            _receive_ur::<CardanoSignDataRequest>(ur, ur_type, decoder)
        }
        #[cfg(feature = "multi-coins")]
        QRCodeType::CardanoCatalystVotingRegistrationRequest => {
            _receive_ur::<CardanoCatalystVotingRegistrationRequest>(ur, ur_type, decoder)
        }
        #[cfg(feature = "multi-coins")]
        QRCodeType::CosmosSignRequest => _receive_ur::<CosmosSignRequest>(ur, ur_type, decoder),
        #[cfg(feature = "multi-coins")]
        QRCodeType::EvmSignRequest => _receive_ur::<EvmSignRequest>(ur, ur_type, decoder),
        #[cfg(feature = "multi-coins")]
        QRCodeType::SuiSignRequest => _receive_ur::<SuiSignRequest>(ur, ur_type, decoder),
        #[cfg(feature = "multi-coins")]
        QRCodeType::SuiSignHashRequest => _receive_ur::<SuiSignHashRequest>(ur, ur_type, decoder),
        #[cfg(feature = "multi-coins")]
        QRCodeType::ArweaveSignRequest => _receive_ur::<ArweaveSignRequest>(ur, ur_type, decoder),
        #[cfg(feature = "multi-coins")]
        QRCodeType::StellarSignRequest => _receive_ur::<StellarSignRequest>(ur, ur_type, decoder),
        #[cfg(feature = "multi-coins")]
        QRCodeType::AptosSignRequest => _receive_ur::<AptosSignRequest>(ur, ur_type, decoder),
        #[cfg(feature = "multi-coins")]
        QRCodeType::QRHardwareCall => _receive_ur::<QRHardwareCall>(ur, ur_type, decoder),
        #[cfg(feature = "multi-coins")]
        QRCodeType::TonSignRequest => _receive_ur::<TonSignRequest>(ur, ur_type, decoder),
        #[cfg(feature = "multi-coins")]
        QRCodeType::ZcashPczt => _receive_ur::<ZcashPczt>(ur, ur_type, decoder),
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
        Ok(result) => UREncodeMultiResult::success(result).c_ptr(),
        Err(e) => UREncodeMultiResult::from(e).c_ptr(),
    }
}

#[no_mangle]
pub extern "C" fn parse_ur(ur: PtrString) -> *mut URParseResult {
    decode_ur(recover_c_char(ur)).c_ptr()
}

#[no_mangle]
pub extern "C" fn receive(ur: PtrString, decoder: PtrDecoder) -> *mut URParseMultiResult {
    let decoder = extract_ptr_with_type!(decoder, KeystoneURDecoder);
    receive_ur(recover_c_char(ur), decoder).c_ptr()
}
