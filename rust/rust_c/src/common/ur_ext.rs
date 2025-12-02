use alloc::format;
use alloc::string::{String, ToString};
use alloc::vec::Vec;

use serde_json::{from_slice, from_value, Value};

#[cfg(feature = "aptos")]
use ur_registry::aptos::aptos_sign_request::AptosSignRequest;
#[cfg(feature = "arweave")]
use ur_registry::arweave::arweave_sign_request::{ArweaveSignRequest, SignType};
#[cfg(feature = "avalanche")]
use ur_registry::avalanche::avax_sign_request::AvaxSignRequest;
#[cfg(feature = "bitcoin")]
use ur_registry::bitcoin::btc_sign_request::BtcSignRequest;
use ur_registry::bytes::Bytes;
#[cfg(feature = "cardano")]
use ur_registry::cardano::cardano_catalyst_voting_registration::CardanoCatalystVotingRegistrationRequest;
#[cfg(feature = "cardano")]
use ur_registry::cardano::cardano_sign_cip8_data_request::CardanoSignCip8DataRequest;
#[cfg(feature = "cardano")]
use ur_registry::cardano::cardano_sign_data_request::CardanoSignDataRequest;
#[cfg(feature = "cardano")]
use ur_registry::cardano::cardano_sign_request::CardanoSignRequest;
#[cfg(feature = "cardano")]
use ur_registry::cardano::cardano_sign_tx_hash_request::CardanoSignTxHashRequest;
#[cfg(feature = "cosmos")]
use ur_registry::cosmos::cosmos_sign_request::CosmosSignRequest;
#[cfg(feature = "cosmos")]
use ur_registry::cosmos::evm_sign_request::EvmSignRequest;
use ur_registry::crypto_account::CryptoAccount;
use ur_registry::crypto_psbt::CryptoPSBT;
use ur_registry::crypto_psbt_extend::CryptoPSBTExtend;
use ur_registry::error::URError;
#[cfg(feature = "ethereum")]
use ur_registry::ethereum::eth_batch_sign_requests::EthBatchSignRequest;
#[cfg(feature = "ethereum")]
use ur_registry::ethereum::eth_sign_request;
#[cfg(feature = "ethereum")]
use ur_registry::ethereum::eth_sign_request::EthSignRequest;
use ur_registry::extend::crypto_multi_accounts::CryptoMultiAccounts;
#[cfg(not(feature = "btc-only"))]
use ur_registry::extend::qr_hardware_call::{CallType, QRHardwareCall};
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
use ur_registry::pb::protobuf_parser::{parse_protobuf, unzip};
use ur_registry::pb::protoc;
use ur_registry::pb::protoc::Base;
#[cfg(feature = "solana")]
use ur_registry::solana::sol_sign_request::SolSignRequest;
#[cfg(feature = "stellar")]
use ur_registry::stellar::stellar_sign_request::{SignType as StellarSignType, StellarSignRequest};
#[cfg(feature = "sui")]
use ur_registry::sui::sui_sign_hash_request::SuiSignHashRequest;
#[cfg(feature = "sui")]
use ur_registry::sui::sui_sign_request::SuiSignRequest;
#[cfg(feature = "ton")]
use ur_registry::ton::ton_sign_request::{DataType, TonSignRequest};
#[cfg(feature = "xrp")]
use ur_registry::xrp::xrp_batch_sign_request::XrpBatchSignRequest;
#[cfg(feature = "zcash")]
use ur_registry::zcash::zcash_pczt::ZcashPczt;

use super::ur::ViewType;

pub trait InferViewType {
    fn infer(&self) -> Result<ViewType, URError> {
        Ok(ViewType::ViewTypeUnKnown)
    }
}

impl InferViewType for CryptoPSBT {
    fn infer(&self) -> Result<ViewType, URError> {
        Ok(ViewType::BtcTx)
    }
}

#[cfg(feature = "multi-coins")]
impl InferViewType for CryptoPSBTExtend {
    fn infer(&self) -> Result<ViewType, URError> {
        match self.get_coin_id() {
            Some(2) => Ok(ViewType::LtcTx),
            Some(3) => Ok(ViewType::DogeTx),
            Some(4) => Ok(ViewType::DashTx),
            Some(145) => Ok(ViewType::BchTx),
            _ => Ok(ViewType::BtcTx),
        }
    }
}

impl InferViewType for CryptoMultiAccounts {
    // ToDo
    fn infer(&self) -> Result<ViewType, URError> {
        Ok(ViewType::ViewTypeUnKnown)
    }
}

#[cfg(feature = "btc-only")]
impl InferViewType for CryptoAccount {
    fn infer(&self) -> Result<ViewType, URError> {
        Ok(ViewType::MultisigCryptoImportXpub)
    }
}

#[cfg(not(feature = "btc-only"))]
impl InferViewType for CryptoAccount {
    fn infer(&self) -> Result<ViewType, URError> {
        Ok(ViewType::ViewTypeUnKnown)
    }
}

#[cfg(feature = "ethereum")]
impl InferViewType for EthSignRequest {
    fn infer(&self) -> Result<ViewType, URError> {
        match self.get_data_type() {
            eth_sign_request::DataType::Transaction
            | eth_sign_request::DataType::TypedTransaction => Ok(ViewType::EthTx),
            eth_sign_request::DataType::TypedData => Ok(ViewType::EthTypedData),
            eth_sign_request::DataType::PersonalMessage => Ok(ViewType::EthPersonalMessage),
        }
    }
}

#[cfg(feature = "ethereum")]
impl InferViewType for EthBatchSignRequest {
    fn infer(&self) -> Result<ViewType, URError> {
        Ok(ViewType::EthBatchTx)
    }
}

#[cfg(feature = "cosmos")]
impl InferViewType for CosmosSignRequest {
    fn infer(&self) -> Result<ViewType, URError> {
        Ok(ViewType::CosmosTx)
    }
}

#[cfg(feature = "cosmos")]
impl InferViewType for EvmSignRequest {
    fn infer(&self) -> Result<ViewType, URError> {
        Ok(ViewType::CosmosEvmTx)
    }
}

#[cfg(feature = "sui")]
impl InferViewType for SuiSignRequest {
    fn infer(&self) -> Result<ViewType, URError> {
        Ok(ViewType::SuiTx)
    }
}

#[cfg(feature = "iota")]
impl InferViewType for IotaSignRequest {
    fn infer(&self) -> Result<ViewType, URError> {
        Ok(ViewType::IotaTx)
    }
}

#[cfg(feature = "iota")]
impl InferViewType for IotaSignHashRequest {
    fn infer(&self) -> Result<ViewType, URError> {
        Ok(ViewType::IotaSignMessageHash)
    }
}

#[cfg(feature = "sui")]
impl InferViewType for SuiSignHashRequest {
    fn infer(&self) -> Result<ViewType, URError> {
        Ok(ViewType::SuiSignMessageHash)
    }
}

#[cfg(feature = "stellar")]
impl InferViewType for StellarSignRequest {
    fn infer(&self) -> Result<ViewType, URError> {
        match self.get_sign_type() {
            StellarSignType::Transaction => Ok(ViewType::StellarTx),
            StellarSignType::TransactionHash => Ok(ViewType::StellarHash),
            _ => Ok(ViewType::StellarTx),
        }
    }
}

#[cfg(feature = "arweave")]
impl InferViewType for ArweaveSignRequest {
    fn infer(&self) -> Result<ViewType, URError> {
        match self.get_sign_type() {
            SignType::Transaction => Ok(ViewType::ArweaveTx),
            SignType::Message => Ok(ViewType::ArweaveMessage),
            SignType::DataItem => Ok(ViewType::ArweaveDataItem),
        }
    }
}

#[cfg(feature = "ton")]
impl InferViewType for TonSignRequest {
    fn infer(&self) -> Result<ViewType, URError> {
        match self.get_data_type() {
            DataType::Transaction => Ok(ViewType::TonTx),
            DataType::SignProof => Ok(ViewType::TonSignProof),
        }
    }
}

#[cfg(feature = "xrp")]
impl InferViewType for XrpBatchSignRequest {
    fn infer(&self) -> Result<ViewType, URError> {
        Ok(ViewType::XRPTx)
    }
}

#[cfg(feature = "aptos")]
impl InferViewType for AptosSignRequest {
    fn infer(&self) -> Result<ViewType, URError> {
        Ok(ViewType::AptosTx)
    }
}

#[cfg(feature = "zcash")]
impl InferViewType for ZcashPczt {
    fn infer(&self) -> Result<ViewType, URError> {
        Ok(ViewType::ZcashTx)
    }
}

#[cfg(feature = "avalanche")]
impl InferViewType for AvaxSignRequest {
    fn infer(&self) -> Result<ViewType, URError> {
        Ok(ViewType::AvaxTx)
    }
}

fn get_view_type_from_keystone(bytes: Vec<u8>) -> Result<ViewType, URError> {
    let unzip_data = unzip(bytes)
        .map_err(|_| URError::NotSupportURTypeError("bytes can not unzip".to_string()))?;
    let base: Base = parse_protobuf(unzip_data)
        .map_err(|_| URError::NotSupportURTypeError("invalid protobuf".to_string()))?;
    let payload = base
        .data
        .ok_or(URError::NotSupportURTypeError("empty payload".to_string()))?;
    let result = match payload.content {
        Some(protoc::payload::Content::SignTx(sign_tx_content)) => {
            match sign_tx_content.coin_code.as_str() {
                "BTC_NATIVE_SEGWIT" => ViewType::BtcNativeSegwitTx,
                "BTC_SEGWIT" => ViewType::BtcSegwitTx,
                "BTC_LEGACY" => ViewType::BtcLegacyTx,
                "BTC" => ViewType::BtcSegwitTx,
                #[cfg(feature = "ltc")]
                "LTC" => ViewType::LtcTx,
                #[cfg(feature = "doge")]
                "DOGE" => ViewType::DogeTx,
                #[cfg(feature = "dash")]
                "DASH" => ViewType::DashTx,
                #[cfg(feature = "bch")]
                "BCH" => ViewType::BchTx,
                #[cfg(feature = "ethereum")]
                "ETH" => ViewType::EthTx,
                #[cfg(feature = "tron")]
                "TRON" => ViewType::TronTx,
                #[cfg(feature = "xrp")]
                "XRP" => ViewType::XRPTx,
                _ => {
                    return Err(URError::ProtobufDecodeError(format!(
                        "invalid coin_code {:?}",
                        sign_tx_content.coin_code
                    )));
                }
            }
        }
        _ => {
            return Err(URError::ProtobufDecodeError(
                "invalid payload content".to_string(),
            ));
        }
    };
    Ok(result)
}

#[cfg(feature = "multi-coins")]
impl InferViewType for KeystoneSignRequest {
    fn infer(&self) -> Result<ViewType, URError> {
        get_view_type_from_keystone(self.get_sign_data())
    }
}

impl InferViewType for Bytes {
    fn infer(&self) -> Result<ViewType, URError> {
        match from_slice::<Value>(self.get_bytes().as_slice()) {
            // XRPTx or WebAuth
            Ok(_v) => {
                if let Some(_type) = _v.pointer("/data/type") {
                    let contract_name: String = from_value(_type.clone())
                        .map_err(|e| URError::UrDecodeError(format!("invalid data, {e}")))?;
                    if contract_name.eq("webAuth") {
                        return Ok(ViewType::WebAuthResult);
                    }
                }
                #[cfg(feature = "xrp")]
                return Ok(ViewType::XRPTx);
                #[cfg(not(feature = "xrp"))]
                return Err(URError::UrDecodeError("invalid data".to_string()));
            }
            #[cfg(feature = "multi-coins")]
            Err(_e) => get_view_type_from_keystone(self.get_bytes()),
            #[cfg(feature = "btc-only")]
            Err(_e) => {
                if app_bitcoin::multi_sig::wallet::is_valid_xpub_config(&self) {
                    return Ok(ViewType::MultisigBytesImportXpub);
                }
                if app_bitcoin::multi_sig::wallet::is_valid_wallet_config(&self) {
                    return Ok(ViewType::MultisigWalletImport);
                }
                get_view_type_from_keystone(self.get_bytes())
            }
            #[cfg(not(any(feature = "btc-only", feature = "multi-coins")))]
            Err(_e) => Err(URError::UrDecodeError("invalid data".to_string())),
        }
    }
}

#[cfg(feature = "bitcoin")]
impl InferViewType for BtcSignRequest {
    fn infer(&self) -> Result<ViewType, URError> {
        Ok(ViewType::BtcMsg)
    }
}

#[cfg(feature = "solana")]
impl InferViewType for SolSignRequest {
    fn infer(&self) -> Result<ViewType, URError> {
        if app_solana::validate_tx(&mut self.get_sign_data()) {
            return Ok(ViewType::SolanaTx);
        }
        Ok(ViewType::SolanaMessage)
    }
}

#[cfg(feature = "near")]
impl InferViewType for NearSignRequest {
    fn infer(&self) -> Result<ViewType, URError> {
        Ok(ViewType::NearTx)
    }
}

#[cfg(feature = "cardano")]
impl InferViewType for CardanoSignRequest {
    fn infer(&self) -> Result<ViewType, URError> {
        Ok(ViewType::CardanoTx)
    }
}

#[cfg(feature = "cardano")]
impl InferViewType for CardanoSignTxHashRequest {
    fn infer(&self) -> Result<ViewType, URError> {
        Ok(ViewType::CardanoSignTxHash)
    }
}

#[cfg(feature = "cardano")]
impl InferViewType for CardanoSignDataRequest {
    fn infer(&self) -> Result<ViewType, URError> {
        Ok(ViewType::CardanoSignData)
    }
}

#[cfg(feature = "cardano")]
impl InferViewType for CardanoCatalystVotingRegistrationRequest {
    fn infer(&self) -> Result<ViewType, URError> {
        Ok(ViewType::CardanoCatalystVotingRegistration)
    }
}

#[cfg(feature = "cardano")]
impl InferViewType for CardanoSignCip8DataRequest {
    fn infer(&self) -> Result<ViewType, URError> {
        Ok(ViewType::CardanoSignCip8Data)
    }
}

#[cfg(feature = "monero")]
impl InferViewType for XmrOutput {
    fn infer(&self) -> Result<ViewType, URError> {
        Ok(ViewType::XmrOutput)
    }
}

#[cfg(feature = "monero")]
impl InferViewType for XmrTxUnsigned {
    fn infer(&self) -> Result<ViewType, URError> {
        Ok(ViewType::XmrTxUnsigned)
    }
}

#[cfg(not(feature = "btc-only"))]
impl InferViewType for QRHardwareCall {
    fn infer(&self) -> Result<ViewType, URError> {
        match self.get_call_type() {
            CallType::KeyDerivation => Ok(ViewType::KeyDerivationRequest),
        }
    }
}

#[cfg(test)]
mod tests {
    use alloc::vec::Vec;

    use hex::FromHex;
    use ur_registry::bytes::Bytes;

    use super::InferViewType;
    use crate::common::ur::ViewType;

    #[cfg(feature = "ltc")]
    #[test]
    fn test_parse_ur_type() {
        {
            //ltc legacy
            let crypto = Bytes::new(
                Vec::from_hex("1f8b0800000000000003558dbb4a03411846b36be192266baa902a2c8212583233ffdc162ccc0d63349268306837333b2b1875558c0979061fc0c242ec051b0b0b5b0b3bc156b0147d005bd30a1f070e1cf83c379feb9dd7d3d896bae7e9456ad2a3e2a7ebb9794f20d16c36783d7873b3739bfd7a7e9131ce12442124dcaa902a2dc32851101a3086608b2dc4b498293d7e3dddfda2654f5fbbdeeb82ff5e2e66825b27bbaa58a48d564a598cf54c4052a096c4334a42c1320b11610c63c60d5560a5b442c70669a264c239f84e713d5b43444422a20a4b6c1281ad8a88c51a04274c01235672c18d4418255881e1d6628230301dc78831008349e1e5fedb0b72c7151a2d55c85205cd5641e5301b74d6b8185407fbfcb0795c8dc4e660d4dc6ef787b59a386d75d2dde4e0d0ff7cb8720a9920535e99e583eaeede683c9d801e9eb5b6366abd8bbdc664e7723a1df346efa43d4efd9b9f8ff98213e43affcf4acfdd3f9997819c79010000").unwrap()
            );
            let view_type = InferViewType::infer(&crypto).unwrap();
            assert_eq!(ViewType::LtcTx, view_type);
        }
    }
}
