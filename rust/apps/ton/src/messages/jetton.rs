use alloc::{format, string::{String, ToString}};
use serde::Serialize;
use third_party::hex;

use crate::vendor::{address::TonAddress, cell::{ArcCell, Cell, TonCellError}};

use super::{traits::ParseCell, InternalMessage};

pub const JETTON_TRANSFER: u32 = 0xf8a7ea5;
// pub const JETTON_TRANSFER_NOTIFICATION: u32 = 0x7362d09c;
// pub const JETTON_INTERNAL_TRANSFER: u32 = 0x178d4519;
// pub const JETTON_EXCESSES: u32 = 0xd53276db;
// pub const JETTON_BURN: u32 = 0x595f07bc;
// pub const JETTON_BURN_NOTIFICATION: u32 = 0x7bdd97de;

#[derive(Clone, Debug, Serialize)]
pub enum JettonMessage {
    JettonTransferMessage(JettonTransferMessage),
}

impl ParseCell for JettonMessage {
    fn parse(cell: &ArcCell) -> Result<Self, TonCellError>
    where
        Self: Sized,
    {
        cell.parse_fully(|parser| {
            let op_code = parser.load_u32(32)?;
            match op_code {
                JETTON_TRANSFER => {
                    JettonTransferMessage::parse(cell).map(JettonMessage::JettonTransferMessage)
                }
                _ => Err(TonCellError::InternalError(format!(
                    "Invalid Op Code: {:X}",
                    op_code
                ))),
            }
        })
    }
}

#[derive(Clone, Debug, Serialize)]
pub struct JettonTransferMessage {
    pub query_id: String,
    pub destination: String,
    pub response_destination: Option<String>,
    pub amount: String,
    pub custom_payload: Option<String>,
    pub forward_ton_amount: String,
    pub forward_payload: Option<String>,
}

impl ParseCell for JettonTransferMessage {
    fn parse(cell: &ArcCell) -> Result<Self, TonCellError>
    where
        Self: Sized,
    {
        cell.parse_fully(|parser| {
            let _op_code = parser.load_u32(32)?;
            let query_id = parser.load_u64(64)?.to_string();
            let amount = parser.load_coins()?.to_string();
            let destination = parser.load_address()?.to_base64_std();
            let temp_address = parser.load_address()?;
            let response_destination = if temp_address.eq(&TonAddress::null()) {
                Some(temp_address.to_base64_std())
            } else {
                None
            };
            let mut ref_index = 0;
            let custom_payload = if parser.load_bit()? {
                let payload = Some( hex::encode(cell.reference(ref_index)?.data.clone()));
                ref_index = ref_index + 1;
                payload
            } else {
                None
            };
            let forward_ton_amount = parser.load_coins()?.to_string();
            let forward_payload = if parser.load_bit()? {
                let payload = Some(hex::encode(cell.reference(ref_index)?.data.clone()));
                ref_index = ref_index + 1;
                payload
            } else {
                None
            };

            Ok(Self {
                query_id,
                destination,
                response_destination,
                amount,
                custom_payload,
                forward_ton_amount,
                forward_payload,
            })
        })
    }
}
