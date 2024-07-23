#![no_std]

extern crate alloc;

use alloc::format;
use alloc::string::ToString;
use alloc::vec::Vec;
use core::slice;
use core::str::FromStr;

use app_xrp::errors::XRPError;
use cty::c_char;
use third_party::bitcoin::bip32::{DerivationPath, Xpub};
use third_party::serde_json::Value;
use third_party::ur_registry::bytes::Bytes;
use third_party::ur_registry::pb;
use third_party::ur_registry::pb::protoc::base::Content::ColdVersion;
use third_party::ur_registry::pb::protoc::payload::Content;
use third_party::ur_registry::pb::protoc::sign_transaction::Transaction::XrpTx;
use third_party::ur_registry::traits::RegistryItem;
use third_party::{hex, secp256k1};

use common_rust_c::errors::{ErrorCodes, KeystoneError, RustCError};
use common_rust_c::extract_ptr_with_type;
use common_rust_c::keystone::build_payload;
use common_rust_c::structs::{SimpleResponse, TransactionCheckResult, TransactionParseResult};
use common_rust_c::types::{PtrBytes, PtrString, PtrT, PtrUR};
use common_rust_c::ur::{QRCodeType, UREncodeResult, FRAGMENT_MAX_LENGTH_DEFAULT};
use common_rust_c::utils::{convert_c_char, recover_c_char};

use crate::structs::DisplayXrpTx;

pub mod structs;

#[no_mangle]
pub extern "C" fn xrp_get_address(
    hd_path: PtrString,
    root_x_pub: PtrString,
    root_path: PtrString,
) -> *mut SimpleResponse<c_char> {
    let hd_path = recover_c_char(hd_path);
    let root_x_pub = recover_c_char(root_x_pub);
    let root_path = recover_c_char(root_path);
    if !hd_path.starts_with(root_path.as_str()) {
        return SimpleResponse::from(XRPError::InvalidHDPath(format!(
            "{} does not match {}",
            hd_path, root_path
        )))
        .simple_c_ptr();
    }
    let address = app_xrp::address::get_address(&hd_path, &root_x_pub, &root_path);
    match address {
        Ok(result) => SimpleResponse::success(convert_c_char(result) as *mut c_char).simple_c_ptr(),
        Err(e) => SimpleResponse::from(e).simple_c_ptr(),
    }
}

fn build_sign_result(ptr: PtrUR, hd_path: PtrString, seed: &[u8]) -> Result<Vec<u8>, XRPError> {
    let crypto_bytes = extract_ptr_with_type!(ptr, Bytes);
    let hd_path = recover_c_char(hd_path);
    app_xrp::sign_tx(crypto_bytes.get_bytes().as_slice(), &hd_path, seed)
}

#[no_mangle]
pub extern "C" fn xrp_parse_tx(ptr: PtrUR) -> PtrT<TransactionParseResult<DisplayXrpTx>> {
    let crypto_bytes = extract_ptr_with_type!(ptr, Bytes);
    match app_xrp::parse(crypto_bytes.get_bytes().as_slice()) {
        Ok(v) => TransactionParseResult::success(DisplayXrpTx::from(v).c_ptr()).c_ptr(),
        Err(e) => TransactionParseResult::from(e).c_ptr(),
    }
}

#[no_mangle]
pub extern "C" fn xrp_sign_tx_bytes(
    ptr: PtrUR,
    seed: PtrBytes,
    seed_len: u32,
    mfp: PtrBytes,
    mfp_len: u32,
    root_xpub: PtrString,
) -> PtrT<UREncodeResult> {
    let seed = unsafe { slice::from_raw_parts(seed, seed_len as usize) };
    let mfp = unsafe { slice::from_raw_parts(mfp, mfp_len as usize) };
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
    let split_hd_path: Vec<&str> = hd_path.split("/").collect();
    let derive_hd_path = format!("{}/{}", split_hd_path[4], split_hd_path[5]);
    let five_level_xpub = xpub
        .derive_pub(
            &k1,
            &DerivationPath::from_str(format!("m/{}", derive_hd_path).as_str()).unwrap(),
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

    let v: Value = third_party::serde_json::from_str(tx_str.as_str()).unwrap();
    let input_bytes = v.to_string().into_bytes();

    let sign_result = app_xrp::sign_tx(input_bytes.as_slice(), &hd_path, seed);
    let tx_hash = app_xrp::get_tx_hash(input_bytes.as_slice()).unwrap();
    let raw_tx = sign_result.unwrap();
    let raw_tx_hex = hex::encode(raw_tx);
    // generate a qr code
    let sign_tx_result = third_party::ur_registry::pb::protoc::SignTransactionResult {
        sign_id: sign_tx.sign_id,
        tx_id: format!("{}", tx_hash.to_uppercase()),
        raw_tx: format!("{}", raw_tx_hex.clone()),
    };
    let content =
        third_party::ur_registry::pb::protoc::payload::Content::SignTxResult(sign_tx_result);
    let payload = third_party::ur_registry::pb::protoc::Payload {
        ///  type is third_party::ur_registry::pb::protoc::payload::Type::SignTxResult
        r#type: 9,
        xfp: hex::encode(mfp),
        content: Some(content),
    };
    let base = third_party::ur_registry::pb::protoc::Base {
        version: 1,
        description: "keystone qrcode".to_string(),
        data: Some(payload),
        device_type: "keystone Pro".to_string(),
        content: Some(ColdVersion(31206)),
    };
    let base_vec = third_party::ur_registry::pb::protobuf_parser::serialize_protobuf(base);
    // zip data can reduce the size of the data
    let zip_data = pb::protobuf_parser::zip(&base_vec).unwrap();
    // data --> protobuf --> zip protobuf data --> cbor bytes data
    UREncodeResult::encode(
        third_party::ur_registry::bytes::Bytes::new(zip_data)
            .try_into()
            .unwrap(),
        third_party::ur_registry::bytes::Bytes::get_registry_type().get_type(),
        FRAGMENT_MAX_LENGTH_DEFAULT.clone(),
    )
    .c_ptr()
}

#[no_mangle]
pub extern "C" fn xrp_sign_tx(
    ptr: PtrUR,
    hd_path: PtrString,
    seed: PtrBytes,
    seed_len: u32,
) -> PtrT<UREncodeResult> {
    let seed = unsafe { slice::from_raw_parts(seed, seed_len as usize) };
    let result = build_sign_result(ptr, hd_path, seed);
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

#[no_mangle]
pub extern "C" fn xrp_check_tx(
    ptr: PtrUR,
    root_xpub: PtrString,
    cached_pubkey: PtrString,
) -> PtrT<TransactionCheckResult> {
    let crypto_bytes = extract_ptr_with_type!(ptr, Bytes);
    let root_xpub = recover_c_char(root_xpub);
    let cached_pubkey = recover_c_char(cached_pubkey);
    match app_xrp::check_tx(
        crypto_bytes.get_bytes().as_slice(),
        &root_xpub,
        &cached_pubkey,
    ) {
        Ok(p) => TransactionCheckResult::error(ErrorCodes::Success, p).c_ptr(),
        Err(e) => TransactionCheckResult::from(e).c_ptr(),
    }
}

#[no_mangle]
pub extern "C" fn is_keystone_xrp_tx(ur_data_ptr: PtrUR) -> bool {
    // if data can be parsed by protobuf, it is a keyston hot app version2 tx or it is a xrp tx
    let payload = build_payload(ur_data_ptr, QRCodeType::Bytes);
    match payload {
        Ok(_) => true,
        Err(_) => false,
    }
}

#[no_mangle]
pub extern "C" fn xrp_check_tx_bytes(
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
            let mfp = unsafe { core::slice::from_raw_parts(master_fingerprint, 4) };
            let mfp: [u8; 4] = mfp.to_vec().try_into().unwrap();

            let xfp = payload.xfp;
            let xfp_vec: [u8; 4] = hex::decode(xfp).unwrap().try_into().unwrap();
            if mfp == xfp_vec {
                return TransactionCheckResult::error(ErrorCodes::Success, "".to_string()).c_ptr();
            } else {
                return TransactionCheckResult::from(RustCError::MasterFingerprintMismatch).c_ptr();
            }
        }
        Err(e) => TransactionCheckResult::from(KeystoneError::ProtobufError(e.to_string())).c_ptr(),
    }
}

#[no_mangle]
pub extern "C" fn xrp_parse_bytes_tx(ptr: PtrUR) -> PtrT<TransactionParseResult<DisplayXrpTx>> {
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
