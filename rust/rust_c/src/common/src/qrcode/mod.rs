use alloc::{format, string::ToString};
use seed_signer_message::{MessageEncoding, SeedSignerMessage};
use third_party::bech32::primitives::encode;

pub mod seed_signer_message;

use crate::{
    errors::RustCError,
    structs::Response,
    types::{Ptr, PtrString},
    ur::URParseResult,
    utils::recover_c_char,
};

#[repr(C)]
pub enum QRProtocol {
    QRCodeTypeText,
    QRCodeTypeUR,
}

#[no_mangle]
pub extern "C" fn infer_qrcode_type(qrcode: PtrString) -> QRProtocol {
    let value = recover_c_char(qrcode);
    if value.to_uppercase().starts_with("UR:") {
        QRProtocol::QRCodeTypeUR
    } else {
        QRProtocol::QRCodeTypeText
    }
}

#[no_mangle]
pub extern "C" fn parse_qrcode_text(qr: PtrString) -> Ptr<URParseResult> {
    let value = recover_c_char(qr).to_lowercase();
    if value.starts_with("signmessage") {
        let mut pieces = value.split_ascii_whitespace();
        let _ = pieces.next(); //drop "signmessage"
        let path = pieces.next();
        let encode_and_message = pieces.next();
        if let (Some(path), Some(encode_and_message)) = (path, encode_and_message) {
            let mut pieces = encode_and_message.split(":");
            let encode = pieces.next();
            let message = pieces.next();
            if let (Some(encode), Some(message)) = (encode, message) {
                match encode {
                    "ascii" => {
                        let data = SeedSignerMessage::new(
                            path.replace("h", "'").to_string(),
                            message.to_string(),
                            MessageEncoding::ASCII,
                        );
                        return URParseResult::single(
                            crate::ur::ViewType::BtcMsg,
                            crate::ur::QRCodeType::SeedSignerMessage,
                            data,
                        )
                        .c_ptr();
                    }
                    _ => {
                        return URParseResult::from(RustCError::UnsupportedTransaction(format!(
                            "message encode not supported: {}",
                            encode
                        )))
                        .c_ptr()
                    }
                }
            }
        }
        return URParseResult::from(RustCError::UnsupportedTransaction(format!(
            "Invalid seed signer message format"
        )))
        .c_ptr();
    }
    URParseResult::from(RustCError::UnsupportedTransaction(format!("plain text"))).c_ptr()
}
