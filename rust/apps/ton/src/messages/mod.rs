use alloc::format;
use alloc::string::{String, ToString};
use alloc::vec::Vec;
use serde::Serialize;
use third_party::hex;

use self::jetton::JettonMessage;
use self::nft::{NFTMessage, NFT_TRANSFER};
use self::traits::ParseCell;
use crate::vendor::cell::{ArcCell, TonCellError};
use crate::vendor::message::JETTON_TRANSFER;

pub mod jetton;
pub mod nft;
pub mod traits;

#[derive(Debug, Clone, Serialize)]
pub struct SigningMessage {
    wallet_id: Option<u32>,
    timeout: u32,
    seq_no: u32,
    messages: Vec<TransferMessage>,
}

impl ParseCell for SigningMessage {
    fn parse(cell: &ArcCell) -> Result<Self, TonCellError>
    where
        Self: Sized,
    {
        cell.parse_fully(|parser| {
            let wallet_id = parser.load_u32(32).ok();
            let timeout = parser.load_u32(32)?;
            let seq_no = parser.load_u32(32)?;
            let _order = parser.load_u8(8).unwrap();
            let _send_mode = parser.load_u8(8).unwrap();
            let messages: Result<Vec<TransferMessage>, TonCellError> = cell
                .references
                .iter()
                .map(|cell| TransferMessage::parse(cell))
                .collect();
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
    ihr_disabled: bool,
    bounce: bool,
    bounced: bool,
    dest_addr: String,
    value: String,
    currency_coll: bool,
    ihr_fees: String,
    fwd_fees: String,
    state_init: Option<String>,
    data: Option<InternalMessage>,
}

impl ParseCell for TransferMessage {
    fn parse(cell: &ArcCell) -> Result<Self, TonCellError>
    where
        Self: Sized,
    {
        cell.parse_fully(|parser| {
            let _flag = parser.load_bit()?;
            let ihr_disabled = parser.load_bit()?;
            let bounce = parser.load_bit()?;
            let bounced = parser.load_bit()?;
            let _src_addr = parser.load_address()?;
            let dest_addr = parser.load_address()?.to_base64_std();
            let value = parser.load_coins()?.to_string();
            let currency_coll = parser.load_bit()?;
            let ihr_fees = parser.load_coins()?.to_string();
            let fwd_fees = parser.load_coins()?.to_string();
            let _created_lt = parser.load_u64(64)?;
            let _created_at = parser.load_u32(32)?;
            let mut ref_index = 0;
            let state_init = if parser.load_bit()? {
                let init = Some(hex::encode(cell.reference(ref_index)?.data.clone()));
                ref_index = ref_index + 1;
                init
            } else {
                None
            };
            let data = if parser.load_bit()? {
                Some(InternalMessage::parse(cell.reference(ref_index)?))
            } else {
                None
            };
            Ok(Self {
                ihr_disabled,
                bounce,
                bounced,
                dest_addr,
                value,
                currency_coll,
                ihr_fees,
                fwd_fees,
                state_init,
                data: data.transpose()?,
            })
        })
    }
}

#[derive(Clone, Debug, Serialize)]
pub struct InternalMessage {
    op_code: String,
    action: Option<String>,
    operation: Operation,
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
        cell.parse_fully(|parser| {
            let op_code = parser.load_u32(32)?;
            match op_code {
                JETTON_TRANSFER => Ok(Self {
                    op_code: format!("{:x}", op_code),
                    action: infer_action(op_code),
                    operation: Operation::JettonMessage(JettonMessage::parse(cell)?),
                }),
                NFT_TRANSFER => Ok(Self {
                    op_code: format!("{:x}", op_code),
                    action: infer_action(op_code),
                    operation: Operation::NFTMessage(NFTMessage::parse(cell)?),
                }),
                0x00000000 => {
                    let remaining_bytes = parser.remaining_bytes();
                    let comment = parser.load_utf8(remaining_bytes)?;
                    Ok(Self {
                        op_code: format!("{:x}", op_code),
                        action: None,
                        operation: Operation::Comment(comment),
                    })
                }
                _ => {
                    let remaining_bytes = parser.remaining_bytes();
                    Ok(Self {
                        op_code: format!("{:x}", op_code),
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
    payload: String,
}

pub type Comment = String;
