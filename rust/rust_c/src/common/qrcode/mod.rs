use alloc::{format, string::ToString};
use seed_signer_message::{MessageEncoding, SeedSignerMessage};

pub mod seed_signer_message;

use super::{
    errors::RustCError,
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
pub unsafe extern "C" fn infer_qrcode_type(qrcode: PtrString) -> QRProtocol {
    let value = recover_c_char(qrcode);
    if value.to_uppercase().starts_with("UR:") {
        QRProtocol::QRCodeTypeUR
    } else {
        QRProtocol::QRCodeTypeText
    }
}

#[no_mangle]
pub unsafe extern "C" fn parse_qrcode_text(qr: PtrString) -> Ptr<URParseResult> {
    let value = recover_c_char(qr);
    if value.to_lowercase().starts_with("signmessage") {
        if let Some((headers, message)) = value.split_once(':') {
            if message.is_empty() {
                return URParseResult::from(RustCError::UnsupportedTransaction(
                    "Invalid seed signer message format".to_string(),
                ))
                .c_ptr();
            }
            let mut pieces = headers.split_ascii_whitespace();
            let _ = pieces.next(); //drop "signmessage"
            let path = pieces.next();
            let encode = pieces.next();
            if let (Some(encode), Some(path)) = (encode, path) {
                match encode {
                    "ascii" => {
                        let data = SeedSignerMessage::new(
                            path.replace('h', "'").to_string(),
                            message.to_string(),
                            MessageEncoding::ASCII,
                        );
                        return URParseResult::single(
                            super::ur::ViewType::BtcMsg,
                            super::ur::QRCodeType::SeedSignerMessage,
                            data,
                        )
                        .c_ptr();
                    }
                    _ => {
                        return URParseResult::from(RustCError::UnsupportedTransaction(format!(
                            "message encode not supported: {encode}"
                        )))
                        .c_ptr()
                    }
                }
            }
        }
        return URParseResult::from(RustCError::UnsupportedTransaction(
            "Invalid seed signer message format".to_string(),
        ))
        .c_ptr();
    }
    URParseResult::from(RustCError::UnsupportedTransaction("plain text".to_string())).c_ptr()
}
