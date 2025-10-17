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
