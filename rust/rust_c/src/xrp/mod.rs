use alloc::format;
use alloc::string::ToString;
use alloc::vec::Vec;
use core::slice;
use core::str::FromStr;

use app_xrp::errors::XRPError;
use bitcoin::bip32::{DerivationPath, Xpub};
use bitcoin::secp256k1;
use cty::c_char;

use serde_json::Value;
use ur_registry::bytes::Bytes;
use ur_registry::pb;
use ur_registry::pb::protoc::base::Content::ColdVersion;
use ur_registry::pb::protoc::payload::Content;
use ur_registry::pb::protoc::sign_transaction::Transaction::XrpTx;
use ur_registry::traits::RegistryItem;
use ur_registry::xrp::xrp_batch_sign_request::XrpBatchSignRequest;
use ur_registry::xrp::xrp_batch_signature::XrpBatchSignature;

use crate::common::errors::{ErrorCodes, KeystoneError, RustCError};
use crate::common::keystone::build_payload;
use crate::common::structs::{SimpleResponse, TransactionCheckResult, TransactionParseResult};
use crate::common::types::{PtrBytes, PtrString, PtrT, PtrUR};
use crate::common::ur::{QRCodeType, UREncodeResult, FRAGMENT_MAX_LENGTH_DEFAULT};
use crate::common::utils::{convert_c_char, recover_c_char};
use crate::extract_array;
use crate::extract_ptr_with_type;

use structs::DisplayXrpTx;

pub mod structs;

#[no_mangle]
pub unsafe extern "C" fn xrp_get_address(
    hd_path: PtrString,
    root_x_pub: PtrString,
    root_path: PtrString,
) -> *mut SimpleResponse<c_char> {
    let hd_path = recover_c_char(hd_path);
    let root_x_pub = recover_c_char(root_x_pub);
    let root_path = recover_c_char(root_path);
    if !hd_path.starts_with(root_path.as_str()) {
        return SimpleResponse::from(XRPError::InvalidHDPath(format!(
            "{hd_path} does not match {root_path}"
        )))
        .simple_c_ptr();
    }
    let address = app_xrp::address::get_address(&hd_path, &root_x_pub, &root_path);
    match address {
        Ok(result) => SimpleResponse::success(convert_c_char(result) as *mut c_char).simple_c_ptr(),
        Err(e) => SimpleResponse::from(e).simple_c_ptr(),
    }
}

#[no_mangle]
pub unsafe extern "C" fn xrp_parse_tx(ptr: PtrUR) -> PtrT<TransactionParseResult<DisplayXrpTx>> {
    let crypto_bytes = extract_ptr_with_type!(ptr, Bytes);
    match app_xrp::parse(crypto_bytes.get_bytes().as_slice()) {
        Ok(v) => TransactionParseResult::success(DisplayXrpTx::from(v).c_ptr()).c_ptr(),
        Err(e) => TransactionParseResult::from(e).c_ptr(),
    }
}

#[no_mangle]
pub unsafe extern "C" fn xrp_parse_batch_tx(
    ptr: PtrUR,
) -> PtrT<TransactionParseResult<DisplayXrpTx>> {
    let crypto_bytes = extract_ptr_with_type!(ptr, XrpBatchSignRequest);
    let requests = crypto_bytes.get_sign_data();
    if requests.len() != 2 {
        return TransactionParseResult::from(RustCError::InvalidData(
            "xaman format batch transaction is not 2 requests".to_string(),
        ))
        .c_ptr();
    }

    match app_xrp::parse_batch(requests[0].as_slice(), requests[1].as_slice()) {
        Ok(v) => TransactionParseResult::success(DisplayXrpTx::from(v).c_ptr()).c_ptr(),
        Err(e) => TransactionParseResult::from(e).c_ptr(),
    }
}

#[no_mangle]
pub unsafe extern "C" fn xrp_sign_tx_bytes(
    ptr: PtrUR,
    seed: PtrBytes,
    seed_len: u32,
    mfp: PtrBytes,
    mfp_len: u32,
    root_xpub: PtrString,
) -> PtrT<UREncodeResult> {
    let seed = extract_array!(seed, u8, seed_len);
    let mfp = extract_array!(mfp, u8, mfp_len);
    let payload = build_payload(ptr, QRCodeType::Bytes).unwrap();
    let content = payload.content.unwrap();
    let sign_tx = match content {
        Content::SignTx(sign_tx) => sign_tx,
        _ => {
            return UREncodeResult::from(RustCError::InvalidData(
                "Cant get sign tx struct data".to_string(),
            ))
            .c_ptr();
        }
    };
    let tx = sign_tx.transaction.unwrap();
    let hd_path = sign_tx.hd_path;
    let xrp_tx = match tx {
        XrpTx(tx) => tx,
        _ => {
            return UREncodeResult::from(RustCError::InvalidData(
                "Cant get xrp tx struct data".to_string(),
            ))
            .c_ptr();
        }
    };
    let root_xpub = recover_c_char(root_xpub);
    let xpub = Xpub::from_str(&root_xpub).unwrap();
    let k1 = secp256k1::Secp256k1::new();
    // M/44'/144'/0'/0/0 -> 0/0
    let split_hd_path: Vec<&str> = hd_path.split('/').collect();
    let derive_hd_path = format!("{}/{}", split_hd_path[4], split_hd_path[5]);
    let five_level_xpub = xpub
        .derive_pub(
            &k1,
            &DerivationPath::from_str(format!("m/{derive_hd_path}").as_str()).unwrap(),
        )
        .unwrap();
    let key = five_level_xpub.public_key.serialize();
    let tx_str = format!(
        r#"{{
            "Account": "{}",
            "Amount": "{}",
            "Destination":"{}",
            "Fee": "{}",
            "Flags": 2147483648,
            "Sequence": {},
            "TransactionType": "Payment",
            "SigningPubKey": "{}",
            "DestinationTag":{}
        }}"#,
        xrp_tx.change_address,
        xrp_tx.amount,
        xrp_tx.to,
        xrp_tx.fee,
        xrp_tx.sequence,
        hex::encode(key).to_uppercase(),
        xrp_tx.tag
    );

    let v: Value = serde_json::from_str(tx_str.as_str()).unwrap();
    let input_bytes = v.to_string().into_bytes();

    let sign_result = app_xrp::sign_tx(input_bytes.as_slice(), &hd_path, seed);
    let tx_hash = app_xrp::get_tx_hash(input_bytes.as_slice()).unwrap();
    let raw_tx = sign_result.unwrap();
    let raw_tx_hex = hex::encode(raw_tx);
    // generate a qr code
    let sign_tx_result = ur_registry::pb::protoc::SignTransactionResult {
        sign_id: sign_tx.sign_id,
        tx_id: tx_hash.to_uppercase().to_string(),
        raw_tx: raw_tx_hex.clone().to_string(),
    };
    let content = ur_registry::pb::protoc::payload::Content::SignTxResult(sign_tx_result);
    let payload = ur_registry::pb::protoc::Payload {
        // type is ur_registry::pb::protoc::payload::Type::SignTxResult
        r#type: 9,
        xfp: hex::encode(mfp),
        content: Some(content),
    };
    let base = ur_registry::pb::protoc::Base {
        version: 1,
        description: "keystone qrcode".to_string(),
        data: Some(payload),
        device_type: "keystone Pro".to_string(),
        content: Some(ColdVersion(31206)),
    };
    let base_vec = ur_registry::pb::protobuf_parser::serialize_protobuf(base);
    // zip data can reduce the size of the data
    let zip_data = pb::protobuf_parser::zip(&base_vec).unwrap();
    // data --> protobuf --> zip protobuf data --> cbor bytes data
    UREncodeResult::encode(
        ur_registry::bytes::Bytes::new(zip_data).try_into().unwrap(),
        ur_registry::bytes::Bytes::get_registry_type().get_type(),
        FRAGMENT_MAX_LENGTH_DEFAULT,
    )
    .c_ptr()
}

#[no_mangle]
pub unsafe extern "C" fn xrp_sign_tx(
    ptr: PtrUR,
    hd_path: PtrString,
    seed: PtrBytes,
    seed_len: u32,
) -> PtrT<UREncodeResult> {
    let seed = extract_array!(seed, u8, seed_len);
    let transaction = extract_ptr_with_type!(ptr, Bytes);
    let hd_path = recover_c_char(hd_path);
    let result = app_xrp::sign_tx(transaction.get_bytes().as_slice(), &hd_path, seed);
    match result.map(|v| Bytes::new(v).try_into()) {
        Ok(v) => match v {
            Ok(data) => UREncodeResult::encode(
                data,
                Bytes::get_registry_type().get_type(),
                FRAGMENT_MAX_LENGTH_DEFAULT,
            )
            .c_ptr(),
            Err(e) => UREncodeResult::from(e).c_ptr(),
        },
        Err(e) => UREncodeResult::from(e).c_ptr(),
    }
}

#[no_mangle]
pub unsafe extern "C" fn xrp_sign_batch_tx(
    ptr: PtrUR,
    hd_path: PtrString,
    seed: PtrBytes,
    seed_len: u32,
) -> PtrT<UREncodeResult> {
    let seed = extract_array!(seed, u8, seed_len);
    let batch_transaction = extract_ptr_with_type!(ptr, XrpBatchSignRequest);
    let hd_path = recover_c_char(hd_path);

    let signatures: Result<Vec<Vec<u8>>, XRPError> = batch_transaction
        .get_sign_data()
        .iter()
        .map(|data| app_xrp::sign_tx(data.as_slice(), &hd_path, seed))
        .collect();
    match signatures {
        Ok(signatures) => {
            let signature = XrpBatchSignature::new(signatures);
            match signature.try_into() {
                Ok(signature_bytes) => UREncodeResult::encode(
                    signature_bytes,
                    XrpBatchSignature::get_registry_type().get_type(),
                    FRAGMENT_MAX_LENGTH_DEFAULT,
                )
                .c_ptr(),
                Err(e) => UREncodeResult::from(RustCError::InvalidData(
                    format!("Failed to convert signature: {}", e),
                ))
                .c_ptr(),
            }
        }
        Err(e) => UREncodeResult::from(RustCError::InvalidData(
            format!("Failed to sign transaction: {}", e),
        ))
        .c_ptr(),
    }
}

#[no_mangle]
pub unsafe extern "C" fn xrp_check_batch_tx(
    ptr: PtrUR,
    root_xpub: PtrString,
    cached_pubkey: PtrString,
) -> PtrT<TransactionCheckResult> {
    let batch_transaction = extract_ptr_with_type!(ptr, XrpBatchSignRequest);
    let crypto_bytes = batch_transaction.get_sign_data();
    let root_xpub = recover_c_char(root_xpub);
    let cached_pubkey = recover_c_char(cached_pubkey);
    // xaman format batch transaction is 2 requests
    if crypto_bytes.len() != 2 {
        return TransactionCheckResult::from(RustCError::InvalidData(
            "xaman format batch transaction is not 2 requests".to_string(),
        ))
        .c_ptr();
    }
    // The length has already been measured.
    xrp_check_raw_tx(crypto_bytes[0].as_slice(), &root_xpub, &cached_pubkey)
}

#[no_mangle]
pub unsafe extern "C" fn xrp_check_tx(
    ptr: PtrUR,
    root_xpub: PtrString,
    cached_pubkey: PtrString,
) -> PtrT<TransactionCheckResult> {
    let crypto_bytes = extract_ptr_with_type!(ptr, Bytes);
    let root_xpub = recover_c_char(root_xpub);
    let cached_pubkey = recover_c_char(cached_pubkey);
    xrp_check_raw_tx(
        crypto_bytes.get_bytes().as_slice(),
        &root_xpub,
        &cached_pubkey,
    )
}

unsafe fn xrp_check_raw_tx(
    raw_tx: &[u8],
    root_xpub: &str,
    cached_pubkey: &str,
) -> PtrT<TransactionCheckResult> {
    match app_xrp::check_tx(raw_tx, &root_xpub, &cached_pubkey) {
        Ok(p) => TransactionCheckResult::error(ErrorCodes::Success, p).c_ptr(),
        Err(e) => TransactionCheckResult::from(e).c_ptr(),
    }
}

#[no_mangle]
pub unsafe extern "C" fn is_keystone_xrp_tx(ur_data_ptr: PtrUR) -> bool {
    // if data can be parsed by protobuf, it is a keyston hot app version2 tx or it is a xrp tx
    let payload = build_payload(ur_data_ptr, QRCodeType::Bytes);
    payload.is_ok()
}

#[no_mangle]
pub unsafe extern "C" fn xrp_check_tx_bytes(
    ptr: PtrUR,
    master_fingerprint: PtrBytes,
    length: u32,
    ur_type: QRCodeType,
) -> PtrT<TransactionCheckResult> {
    if length != 4 {
        return TransactionCheckResult::from(RustCError::InvalidMasterFingerprint).c_ptr();
    }
    let payload = build_payload(ptr, ur_type);
    match payload {
        Ok(payload) => {
            let mfp = extract_array!(master_fingerprint, u8, 4);
            let mfp: [u8; 4] = mfp.to_vec().try_into().unwrap();

            let xfp = payload.xfp;
            let xfp_vec: [u8; 4] = hex::decode(xfp).unwrap().try_into().unwrap();
            if mfp == xfp_vec {
                TransactionCheckResult::error(ErrorCodes::Success, "".to_string()).c_ptr()
            } else {
                TransactionCheckResult::from(RustCError::MasterFingerprintMismatch).c_ptr()
            }
        }
        Err(e) => TransactionCheckResult::from(KeystoneError::ProtobufError(e.to_string())).c_ptr(),
    }
}

#[no_mangle]
pub unsafe extern "C" fn xrp_parse_bytes_tx(
    ptr: PtrUR,
) -> PtrT<TransactionParseResult<DisplayXrpTx>> {
    let payload = build_payload(ptr, QRCodeType::Bytes).unwrap();
    let content = payload.content.unwrap();
    let sign_tx = match content {
        Content::SignTx(sign_tx) => sign_tx,
        _ => {
            return TransactionParseResult::from(RustCError::InvalidData(
                "Cant get sign tx struct data".to_string(),
            ))
            .c_ptr();
        }
    };
    let tx = sign_tx.transaction.unwrap();
    let xrp_tx = match tx {
        XrpTx(tx) => tx,
        _ => {
            return TransactionParseResult::from(RustCError::InvalidData(
                "Cant get xrp tx struct data".to_string(),
            ))
            .c_ptr();
        }
    };

    let display_xrp = DisplayXrpTx::try_from(xrp_tx).unwrap();
    TransactionParseResult::success(display_xrp.c_ptr()).c_ptr()
}
