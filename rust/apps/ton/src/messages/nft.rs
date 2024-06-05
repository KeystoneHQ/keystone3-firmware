use alloc::{
    format,
    string::{String, ToString},
};
use serde::Serialize;
use third_party::hex;

use super::{traits::ParseCell, Comment};

pub const NFT_TRANSFER: u32 = 0x5fcc3d14;

#[derive(Clone, Debug, Serialize)]
pub enum NFTMessage {
    NFTTransferMessage(NFTTransferMessage),
}

impl ParseCell for NFTMessage {
    fn parse(cell: &crate::vendor::cell::ArcCell) -> Result<Self, crate::vendor::cell::TonCellError>
    where
        Self: Sized,
    {
        cell.parse(|parser| {
            let op_code = parser.load_u32(32)?;
            match op_code {
                NFT_TRANSFER => NFTTransferMessage::parse(cell).map(NFTMessage::NFTTransferMessage),
                _ => Err(crate::vendor::cell::TonCellError::InternalError(format!(
                    "Invalid Op Code: {:X}",
                    op_code
                ))),
            }
        })
    }
}

#[derive(Clone, Debug, Serialize)]
pub struct NFTTransferMessage {
    pub query_id: String,
    pub new_owner_address: String,
    pub response_address: String,
    pub custom_payload: Option<String>,
    pub forward_ton_amount: String,
    pub forward_payload: Option<String>,
    pub comment: Option<String>,
}

impl ParseCell for NFTTransferMessage {
    fn parse(cell: &crate::vendor::cell::ArcCell) -> Result<Self, crate::vendor::cell::TonCellError>
    where
        Self: Sized,
    {
        cell.parse_fully(|parser| {
            let _op_code = parser.load_u32(32)?;
            let query_id = parser.load_u64(64)?.to_string();
            let new_owner_address = parser.load_address()?.to_base64_url_flags(true, false);
            let response_address = parser.load_address()?.to_base64_url_flags(true, false);
            let mut ref_index = 0;
            let custom_payload = if parser.load_bit()? {
                let payload = Some(hex::encode(cell.reference(ref_index)?.data.clone()));
                ref_index = ref_index + 1;
                payload
            } else {
                None
            };
            let forward_ton_amount = parser.load_coins()?.to_string();
            let (forward_payload, comment) = if parser.load_bit()? {
                let child = cell.reference(ref_index)?;
                let comment = Comment::parse(child);

                let payload = Some(hex::encode(child.data.clone()));
                ref_index = ref_index + 1;
                (payload, comment.ok())
            } else {
                (None, None)
            };

            Ok(Self {
                query_id,
                new_owner_address,
                response_address,
                custom_payload,
                forward_ton_amount,
                forward_payload,
                comment,
            })
        })
    }
}
