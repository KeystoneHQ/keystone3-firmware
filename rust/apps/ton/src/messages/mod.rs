use alloc::format;
use alloc::string::{String, ToString};
use alloc::vec::Vec;

use serde::Serialize;

use self::jetton::JettonMessage;
use self::nft::{NFTMessage, NFT_TRANSFER};
use self::traits::ParseCell;
use crate::vendor::cell::{ArcCell, CellBuilder, TonCellError};
use crate::vendor::message::JETTON_TRANSFER;

pub mod jetton;
pub mod nft;
pub mod traits;

#[derive(Debug, Clone, Serialize)]
pub struct SigningMessage {
    pub wallet_id: Option<u32>,
    pub timeout: u32,
    pub seq_no: u32,
    pub messages: Vec<TransferMessage>,
}

impl ParseCell for SigningMessage {
    fn parse(cell: &ArcCell) -> Result<Self, TonCellError>
    where
        Self: Sized,
    {
        cell.parse(|parser| {
            let wallet_id = parser.load_u32(32).ok();
            let timeout = parser.load_u32(32)?;
            let seq_no = parser.load_u32(32)?;
            let _order = parser.load_u8(8).unwrap();
            let _send_mode = parser.load_u8(8).unwrap();
            let messages: Result<Vec<TransferMessage>, TonCellError> =
                cell.references.iter().map(TransferMessage::parse).collect();
            Ok(Self {
                wallet_id,
                timeout,
                seq_no,
                messages: messages?,
            })
        })
    }
}

#[derive(Clone, Debug, Serialize)]
pub struct TransferMessage {
    pub ihr_disabled: bool,
    pub bounce: bool,
    pub bounced: bool,
    pub dest_addr: String,
    pub dest_addr_legacy: String,
    pub value: String,
    pub currency_coll: bool,
    pub ihr_fees: String,
    pub fwd_fees: String,
    pub state_init: Option<String>,
    pub data: Option<InternalMessage>,
}

impl ParseCell for TransferMessage {
    fn parse(cell: &ArcCell) -> Result<Self, TonCellError>
    where
        Self: Sized,
    {
        cell.parse(|parser| {
            let _flag = parser.load_bit()?;
            let ihr_disabled = parser.load_bit()?;
            let bounce = parser.load_bit()?;
            let bounced = parser.load_bit()?;
            let _src_addr = parser.load_address()?;
            let addr = parser.load_address()?;
            let dest_addr = addr.to_base64_url_flags(true, false);
            let dest_addr_legacy = addr.to_base64_std();
            let value = parser.load_coins()?.to_string();
            let coins = u64::from_str_radix(&value, 10)
                .map_err(|_e| TonCellError::InternalError("Invalid value".to_string()))?;
            let ton_value = format!("{} Ton", (coins as f64) / 1_000_000_000f64);
            let currency_coll = parser.load_bit()?;
            let ihr_fees = parser.load_coins()?.to_string();
            let fwd_fees = parser.load_coins()?.to_string();
            let _created_lt = parser.load_u64(64)?;
            let _created_at = parser.load_u32(32)?;
            let mut ref_index = 0;
            let state_init = if parser.load_bit()? {
                let init = Some(hex::encode(cell.reference(ref_index)?.data.clone()));
                ref_index += 1;
                init
            } else {
                None
            };
            let data = if parser.load_bit()? {
                Some(InternalMessage::parse(cell.reference(ref_index)?))
            } else if parser.remaining_bits() > 0 {
                let mut builder = CellBuilder::new();
                let remaining_bits = parser.remaining_bits();
                builder.store_bits(remaining_bits, &parser.load_bits(remaining_bits)?)?;
                Some(InternalMessage::parse(&builder.build()?.to_arc()))
            } else {
                None
            };
            Ok(Self {
                ihr_disabled,
                bounce,
                bounced,
                dest_addr,
                value: ton_value,
                currency_coll,
                ihr_fees,
                fwd_fees,
                state_init,
                dest_addr_legacy,
                data: data.transpose().ok().flatten(),
            })
        })
    }
}

#[derive(Clone, Debug, Serialize)]
pub struct InternalMessage {
    pub op_code: String,
    pub action: Option<String>,
    pub operation: Operation,
}

#[derive(Clone, Debug, Serialize)]
pub enum Operation {
    Comment(Comment),
    JettonMessage(JettonMessage),
    NFTMessage(NFTMessage),
    OtherMessage(OtherMessage),
}

fn infer_action(op_code: u32) -> Option<String> {
    match op_code {
        JETTON_TRANSFER => Some("Jetton Transfer".to_string()),
        NFT_TRANSFER => Some("NFT Transfer".to_string()),
        _ => None,
    }
}

impl ParseCell for InternalMessage {
    fn parse(cell: &ArcCell) -> Result<Self, TonCellError>
    where
        Self: Sized,
    {
        cell.parse(|parser| {
            let op_code = parser.load_u32(32)?;
            match op_code {
                JETTON_TRANSFER => Ok(Self {
                    op_code: format!("{op_code:x}"),
                    action: infer_action(op_code),
                    operation: Operation::JettonMessage(JettonMessage::parse(cell)?),
                }),
                NFT_TRANSFER => Ok(Self {
                    op_code: format!("{op_code:x}"),
                    action: infer_action(op_code),
                    operation: Operation::NFTMessage(NFTMessage::parse(cell)?),
                }),
                0x00000000 => {
                    let remaining_bytes = parser.remaining_bytes();
                    let mut comment = parser.load_utf8(remaining_bytes)?;
                    let mut child = cell.reference(0);
                    while child.is_ok() {
                        let t = child.unwrap();
                        let child_comment = t.parse_fully(|child_parser| {
                            let child_remaining_bytes = child_parser.remaining_bytes();
                            child_parser.load_utf8(child_remaining_bytes)
                        })?;
                        comment.push_str(&child_comment);
                        child = t.reference(0);
                    }
                    Ok(Self {
                        op_code: format!("{op_code:x}"),
                        action: None,
                        operation: Operation::Comment(comment),
                    })
                }
                _ => {
                    let remaining_bytes = parser.remaining_bytes();
                    Ok(Self {
                        op_code: format!("{op_code:x}"),
                        action: infer_action(op_code),
                        operation: Operation::OtherMessage(OtherMessage {
                            payload: hex::encode(parser.load_bytes(remaining_bytes)?),
                        }),
                    })
                }
            }
        })
    }
}

#[derive(Clone, Debug, Serialize)]
pub struct OtherMessage {
    pub payload: String,
}

pub type Comment = String;

impl ParseCell for Comment {
    fn parse(cell: &ArcCell) -> Result<Self, TonCellError>
    where
        Self: Sized,
    {
        cell.parse_fully(|parser| {
            if parser.remaining_bits() < 32 {
                return Err(TonCellError::CellParserError(
                    "payload is not a comment".to_string(),
                ));
            }
            let op = parser.load_u32(32)?;
            if op != 0x00000000 {
                return Err(TonCellError::CellParserError(
                    "payload is not a comment".to_string(),
                ));
            }
            let remaining_bytes = parser.remaining_bytes();
            String::from_utf8(parser.load_bytes(remaining_bytes)?)
                .map_err(|_e| TonCellError::CellParserError("payload is not a comment".to_string()))
        })
    }
}

#[cfg(test)]
mod tests {
    extern crate std;

    use anyhow::Result;

    use super::nft::NFT_TRANSFER;
    use super::traits::ParseCell;
    use super::{infer_action, Comment, InternalMessage, Operation};
    use crate::vendor::cell::{ArcCell, CellBuilder, TonCellError};
    use crate::vendor::message::JETTON_TRANSFER;

    fn build_utf8_cell(payload: &str, child: Option<&ArcCell>) -> Result<ArcCell, TonCellError> {
        let mut builder = CellBuilder::new();
        builder.store_string(payload)?;
        if let Some(child) = child {
            builder.store_reference(child)?;
        }
        Ok(builder.build()?.to_arc())
    }

    fn build_internal_comment_cell(
        payload: &str,
        child: Option<&ArcCell>,
    ) -> Result<ArcCell, TonCellError> {
        let mut builder = CellBuilder::new();
        builder.store_u32(32, 0)?;
        builder.store_string(payload)?;
        if let Some(child) = child {
            builder.store_reference(child)?;
        }
        Ok(builder.build()?.to_arc())
    }

    fn assert_invalid_comment_error(cell: &ArcCell) {
        let err = Comment::parse(cell).unwrap_err();
        assert!(
            matches!(err, TonCellError::CellParserError(ref message) if message == "payload is not a comment")
                || matches!(err, TonCellError::NonEmptyReader(_))
        );
    }

    #[test]
    fn test_infer_action_known_and_unknown() {
        assert_eq!(infer_action(JETTON_TRANSFER).as_deref(), Some("Jetton Transfer"));
        assert_eq!(infer_action(NFT_TRANSFER).as_deref(), Some("NFT Transfer"));
        assert_eq!(infer_action(0xDEADBEEF), None);
    }

    #[test]
    fn test_parse_internal_comment_with_child_chain() -> Result<()> {
        let tail = build_utf8_cell("world", None)?;
        let mid = build_utf8_cell(" ", Some(&tail))?;
        let root = build_internal_comment_cell("hello", Some(&mid))?;

        let parsed = InternalMessage::parse(&root)?;
        assert_eq!(parsed.op_code, "0");
        assert_eq!(parsed.action, None);

        match parsed.operation {
            Operation::Comment(comment) => assert_eq!(comment, "hello world"),
            _ => panic!("expected comment operation"),
        }

        Ok(())
    }

    #[test]
    fn test_parse_internal_other_message() -> Result<()> {
        let mut builder = CellBuilder::new();
        builder.store_u32(32, 0x12345678)?;
        builder.store_slice(&[0xAA, 0xBB, 0xCC])?;
        let cell = builder.build()?.to_arc();

        let parsed = InternalMessage::parse(&cell)?;
        assert_eq!(parsed.op_code, "12345678");
        assert_eq!(parsed.action, None);

        match parsed.operation {
            Operation::OtherMessage(other) => assert_eq!(other.payload, "aabbcc"),
            _ => panic!("expected other message operation"),
        }

        Ok(())
    }

    #[test]
    fn test_parse_comment_success() -> Result<()> {
        let cell = build_internal_comment_cell("memo", None)?;
        let comment = Comment::parse(&cell)?;
        assert_eq!(comment, "memo");
        Ok(())
    }

    #[test]
    fn test_parse_comment_rejects_short_payload() -> Result<()> {
        let mut builder = CellBuilder::new();
        builder.store_u8(8, 0xFF)?;
        let cell = builder.build()?.to_arc();
        assert_invalid_comment_error(&cell);
        Ok(())
    }

    #[test]
    fn test_parse_comment_rejects_non_zero_opcode() -> Result<()> {
        let mut builder = CellBuilder::new();
        builder.store_u32(32, 1)?;
        let cell = builder.build()?.to_arc();
        assert_invalid_comment_error(&cell);
        Ok(())
    }

    #[test]
    fn test_parse_comment_rejects_invalid_utf8() -> Result<()> {
        let mut builder = CellBuilder::new();
        builder.store_u32(32, 0)?;
        builder.store_byte(0xFF)?;
        let cell = builder.build()?.to_arc();
        assert_invalid_comment_error(&cell);
        Ok(())
    }
}


