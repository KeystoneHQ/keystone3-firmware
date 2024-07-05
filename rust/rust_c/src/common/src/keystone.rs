use crate::errors::{KeystoneError, RustCError};
use crate::extract_ptr_with_type;
use crate::structs::TransactionCheckResult;
use crate::types::{PtrBytes, PtrString, PtrT, PtrUR};
use crate::ur::{QRCodeType, UREncodeResult, FRAGMENT_MAX_LENGTH_DEFAULT};
use crate::utils::recover_c_char;
use alloc::borrow::ToOwned;
use alloc::format;
use alloc::string::{String, ToString};
use alloc::vec::Vec;
use app_bitcoin::addresses::xyzpub::{convert_version, Version};
use app_utils;
use app_wallets::{companion_app::DESCRIPTION, DEVICE_TYPE};
use core::str::FromStr;
use third_party::hex;
use third_party::ur_registry::bytes::Bytes;
#[cfg(feature = "multi-coins")]
use third_party::ur_registry::keystone::keystone_sign_request::KeystoneSignRequest;
#[cfg(feature = "multi-coins")]
use third_party::ur_registry::keystone::keystone_sign_result::KeystoneSignResult;
use third_party::ur_registry::pb::protobuf_parser::{
    parse_protobuf, serialize_protobuf, unzip, zip,
};
use third_party::ur_registry::pb::protoc::base::Content as VersionContent;
use third_party::ur_registry::pb::protoc::payload::Type as PbType;
use third_party::ur_registry::pb::protoc::{payload, Base, Payload, SignTransactionResult};
use third_party::ur_registry::traits::RegistryItem;

pub fn build_payload(ptr: PtrUR, ur_type: QRCodeType) -> Result<Payload, KeystoneError> {
    let bytes = match ur_type {
        #[cfg(feature = "multi-coins")]
        QRCodeType::KeystoneSignRequest => {
            let req = extract_ptr_with_type!(ptr, KeystoneSignRequest);
            req.get_sign_data()
        }
        QRCodeType::Bytes => {
            let bytes = extract_ptr_with_type!(ptr, Bytes);
            bytes.get_bytes()
        }
        _ => return Err(KeystoneError::ProtobufError("invalid ur type".to_string())),
    };
    unzip(bytes)
        .and_then(|unzip_data| parse_protobuf(unzip_data))
        .and_then(|base: Base| Ok(base.data))
        .map_err(|e| KeystoneError::ProtobufError(e.to_string()))?
        .ok_or(KeystoneError::ProtobufError("empty payload".to_string()))
}

pub fn build_parse_context(
    master_fingerprint: PtrBytes,
    x_pub: PtrString,
) -> Result<app_utils::keystone::ParseContext, KeystoneError> {
    let mfp = unsafe { core::slice::from_raw_parts(master_fingerprint, 4) };
    let x_pub = recover_c_char(x_pub);
    let xpub_str = convert_version(x_pub.as_str(), &Version::Xpub)
        .map_err(|e| KeystoneError::InvalidParseContext(e.to_string()))?;
    let master_fingerprint =
        third_party::bitcoin::bip32::Fingerprint::from_str(hex::encode(mfp.to_vec()).as_str())
            .map_err(|_| KeystoneError::InvalidParseContext(format!("invalid mfp")))?;
    let extended_pubkey = third_party::bitcoin::bip32::Xpub::from_str(&xpub_str).map_err(|_| {
        KeystoneError::InvalidParseContext(format!("invalid extended pub key {}", x_pub))
    })?;
    Ok(app_utils::keystone::ParseContext::new(
        master_fingerprint,
        extended_pubkey,
    ))
}

fn get_signed_tx(
    coin_code: String,
    payload: Payload,
    master_fingerprint: PtrBytes,
    x_pub: PtrString,
    seed: &[u8],
) -> Result<(String, String), KeystoneError> {
    build_parse_context(master_fingerprint, x_pub).and_then(|context| {
        if app_bitcoin::network::Network::from_str(coin_code.as_str()).is_ok() {
            return app_bitcoin::sign_raw_tx(payload, context, seed)
                .map_err(|e| KeystoneError::SignTxFailed(e.to_string()));
        }
        match coin_code.as_str() {
            #[cfg(feature = "multi-coins")]
            "TRON" => app_tron::sign_raw_tx(payload, context, seed)
                .map_err(|e| KeystoneError::SignTxFailed(e.to_string())),
            _ => Err(KeystoneError::SignTxFailed(format!(
                "chain is not supported {}",
                coin_code
            ))),
        }
    })
}

pub fn build_check_result(
    ptr: PtrUR,
    ur_type: QRCodeType,
    master_fingerprint: PtrBytes,
    x_pub: PtrString,
) -> Result<(), KeystoneError> {
    let payload = build_payload(ptr, ur_type)?;
    let payload_content = payload.content.clone();
    match payload_content {
        Some(payload::Content::SignTx(sign_tx_content)) => {
            build_parse_context(master_fingerprint, x_pub).and_then(|context| {
                if app_bitcoin::network::Network::from_str(sign_tx_content.coin_code.as_str())
                    .is_ok()
                {
                    return app_bitcoin::check_raw_tx(payload, context)
                        .map_err(|e| KeystoneError::CheckTxFailed(e.to_string()));
                }
                match sign_tx_content.coin_code.as_str() {
                    #[cfg(feature = "multi-coins")]
                    "TRON" => app_tron::check_raw_tx(payload, context)
                        .map_err(|e| KeystoneError::CheckTxFailed(e.to_string())),
                    _ => Err(KeystoneError::CheckTxFailed(format!(
                        "chain is not supported {}",
                        sign_tx_content.coin_code
                    ))),
                }
            })
        }
        _ => {
            return Err(KeystoneError::ProtobufError(
                "empty payload content".to_string(),
            ));
        }
    }
}

pub fn build_sign_result(
    ptr: PtrUR,
    ur_type: QRCodeType,
    master_fingerprint: PtrBytes,
    x_pub: PtrString,
    cold_version: i32,
    seed: &[u8],
) -> Result<Vec<u8>, KeystoneError> {
    let payload = build_payload(ptr, ur_type)?;
    let payload_content = payload.content.clone();
    match payload_content {
        Some(payload::Content::SignTx(sign_tx_content)) => {
            let (raw_tx, tx_id) = get_signed_tx(
                sign_tx_content.coin_code,
                payload.to_owned(),
                master_fingerprint,
                x_pub,
                seed,
            )?;
            let data = Payload {
                content: Some(payload::Content::SignTxResult(SignTransactionResult {
                    sign_id: sign_tx_content.sign_id,
                    tx_id,
                    raw_tx,
                })),
                r#type: PbType::SignTxResult.into(),
                xfp: payload.xfp,
            };
            let base = Base {
                data: Some(data),
                version: 1,
                content: Some(VersionContent::ColdVersion(cold_version)),
                description: DESCRIPTION.to_string(),
                device_type: DEVICE_TYPE.to_string(),
            };
            let data = serialize_protobuf(base);
            zip(&data).map_err(|_| KeystoneError::ProtobufError("zip bytes failed".to_string()))
        }
        _ => {
            return Err(KeystoneError::ProtobufError(
                "empty payload content".to_string(),
            ));
        }
    }
}

pub fn check(
    ptr: PtrUR,
    ur_type: QRCodeType,
    master_fingerprint: PtrBytes,
    length: u32,
    x_pub: PtrString,
) -> PtrT<TransactionCheckResult> {
    if length != 4 {
        return TransactionCheckResult::from(RustCError::InvalidMasterFingerprint).c_ptr();
    }
    let result = build_check_result(ptr, ur_type, master_fingerprint, x_pub);
    match result {
        Ok(_) => TransactionCheckResult::new().c_ptr(),
        Err(e) => TransactionCheckResult::from(e).c_ptr(),
    }
}

pub fn sign(
    ptr: PtrUR,
    ur_type: QRCodeType,
    master_fingerprint: PtrBytes,
    length: u32,
    x_pub: PtrString,
    cold_version: i32,
    seed: &[u8],
) -> *mut UREncodeResult {
    if length != 4 {
        return UREncodeResult::from(RustCError::InvalidMasterFingerprint).c_ptr();
    }
    match ur_type {
        #[cfg(feature = "multi-coins")]
        QRCodeType::KeystoneSignRequest => {
            let result =
                build_sign_result(ptr, ur_type, master_fingerprint, x_pub, cold_version, seed);
            match result.map(|v| KeystoneSignResult::new(v).try_into()) {
                Ok(v) => match v {
                    Ok(data) => UREncodeResult::encode(
                        data,
                        KeystoneSignResult::get_registry_type().get_type(),
                        FRAGMENT_MAX_LENGTH_DEFAULT.clone(),
                    )
                    .c_ptr(),
                    Err(e) => UREncodeResult::from(e).c_ptr(),
                },
                Err(e) => UREncodeResult::from(e).c_ptr(),
            }
        }
        QRCodeType::Bytes => {
            let result =
                build_sign_result(ptr, ur_type, master_fingerprint, x_pub, cold_version, seed);
            match result.map(|v| Bytes::new(v).try_into()) {
                Ok(v) => match v {
                    Ok(data) => UREncodeResult::encode(
                        data,
                        Bytes::get_registry_type().get_type(),
                        FRAGMENT_MAX_LENGTH_DEFAULT.clone(),
                    )
                    .c_ptr(),
                    Err(e) => UREncodeResult::from(e).c_ptr(),
                },
                Err(e) => UREncodeResult::from(e).c_ptr(),
            }
        }
        _ => UREncodeResult::from(RustCError::UnsupportedTransaction(
            "unsupported ur type".to_string(),
        ))
        .c_ptr(),
    }
}
