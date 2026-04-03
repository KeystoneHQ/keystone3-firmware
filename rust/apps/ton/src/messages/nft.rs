use alloc::{
    format,
    string::{String, ToString},
};
use hex;
use serde::Serialize;

use super::{traits::ParseCell, Comment};

pub const NFT_TRANSFER: u32 = 0x5fcc3d14;

#[derive(Clone, Debug, Serialize)]
#[non_exhaustive]
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
                    "Invalid Op Code: {op_code:X}"
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
                ref_index += 1;
                payload
            } else {
                None
            };
            let forward_ton_amount = parser.load_coins()?.to_string();
            let (forward_payload, comment) = if parser.load_bit()? {
                let child = cell.reference(ref_index)?;
                let comment = Comment::parse(child);

                let payload = Some(hex::encode(child.data.clone()));
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

#[cfg(test)]
mod tests {
    extern crate std;

    use alloc::string::ToString;

    use anyhow::Result;
    use num_bigint::BigUint;

    use super::{NFTMessage, NFTTransferMessage, NFT_TRANSFER};
    use crate::messages::traits::ParseCell;
    use crate::vendor::address::TonAddress;
    use crate::vendor::cell::{ArcCell, CellBuilder, TonCellError};

    fn test_address(seed: u8) -> TonAddress {
        TonAddress::new(0, &[seed; 32])
    }

    fn build_payload_cell(bytes: &[u8]) -> Result<ArcCell, TonCellError> {
        let mut builder = CellBuilder::new();
        builder.store_slice(bytes)?;
        Ok(builder.build()?.to_arc())
    }

    fn build_comment_cell(text: &str) -> Result<ArcCell, TonCellError> {
        let mut builder = CellBuilder::new();
        builder.store_u32(32, 0)?;
        builder.store_string(text)?;
        Ok(builder.build()?.to_arc())
    }

    fn build_nft_transfer_cell(
        custom_payload: Option<&ArcCell>,
        forward_payload: Option<&ArcCell>,
        forward_ton_amount: u32,
    ) -> Result<ArcCell, TonCellError> {
        let mut builder = CellBuilder::new();
        builder.store_u32(32, NFT_TRANSFER)?;
        builder.store_u64(64, 9)?;
        builder.store_address(&test_address(1))?;
        builder.store_address(&test_address(2))?;
        builder.store_bit(custom_payload.is_some())?;
        if let Some(custom_payload) = custom_payload {
            builder.store_reference(custom_payload)?;
        }
        builder.store_coins(&BigUint::from(forward_ton_amount))?;
        builder.store_bit(forward_payload.is_some())?;
        if let Some(forward_payload) = forward_payload {
            builder.store_reference(forward_payload)?;
        }
        Ok(builder.build()?.to_arc())
    }

    #[test]
    fn test_parse_nft_transfer_with_payloads() -> Result<()> {
        let custom_payload = build_payload_cell(&[0xAB, 0xCD])?;
        let forward_payload = build_comment_cell("memo")?;
        let cell = build_nft_transfer_cell(Some(&custom_payload), Some(&forward_payload), 5)?;

        let parsed = NFTTransferMessage::parse(&cell)?;
        assert_eq!(parsed.query_id, "9");
        assert_eq!(
            parsed.new_owner_address,
            test_address(1).to_base64_url_flags(true, false)
        );
        assert_eq!(
            parsed.response_address,
            test_address(2).to_base64_url_flags(true, false)
        );
        assert_eq!(
            parsed.custom_payload,
            Some(hex::encode(custom_payload.data.clone()))
        );
        assert_eq!(parsed.forward_ton_amount, "5");
        assert_eq!(
            parsed.forward_payload,
            Some(hex::encode(forward_payload.data.clone()))
        );
        assert_eq!(parsed.comment, Some("memo".to_string()));

        Ok(())
    }

    #[test]
    fn test_parse_nft_transfer_without_optional_fields() -> Result<()> {
        let cell = build_nft_transfer_cell(None, None, 0)?;

        let parsed = NFTTransferMessage::parse(&cell)?;
        assert_eq!(parsed.custom_payload, None);
        assert_eq!(parsed.forward_ton_amount, "0");
        assert_eq!(parsed.forward_payload, None);
        assert_eq!(parsed.comment, None);

        let message = NFTMessage::parse(&cell)?;
        match message {
            NFTMessage::NFTTransferMessage(inner) => assert_eq!(inner.query_id, "9"),
        }

        Ok(())
    }

    #[test]
    fn test_parse_nft_message_invalid_opcode() -> Result<()> {
        let mut builder = CellBuilder::new();
        builder.store_u32(32, 0x01020304)?;
        let cell = builder.build()?.to_arc();

        let err = NFTMessage::parse(&cell).unwrap_err();
        assert!(
            matches!(err, TonCellError::InternalError(message) if message.contains("Invalid Op Code"))
        );

        Ok(())
    }
}
