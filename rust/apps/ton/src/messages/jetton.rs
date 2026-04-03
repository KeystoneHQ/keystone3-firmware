use alloc::{
    format,
    string::{String, ToString},
};

use hex;
use serde::Serialize;

use crate::vendor::{
    address::TonAddress,
    cell::{ArcCell, TonCellError},
};

use super::{traits::ParseCell, Comment};

pub const JETTON_TRANSFER: u32 = 0xf8a7ea5;
// pub const JETTON_TRANSFER_NOTIFICATION: u32 = 0x7362d09c;
// pub const JETTON_INTERNAL_TRANSFER: u32 = 0x178d4519;
// pub const JETTON_EXCESSES: u32 = 0xd53276db;
// pub const JETTON_BURN: u32 = 0x595f07bc;
// pub const JETTON_BURN_NOTIFICATION: u32 = 0x7bdd97de;

#[derive(Clone, Debug, Serialize)]
#[non_exhaustive]
pub enum JettonMessage {
    JettonTransferMessage(JettonTransferMessage),
}

impl ParseCell for JettonMessage {
    fn parse(cell: &ArcCell) -> Result<Self, TonCellError>
    where
        Self: Sized,
    {
        cell.parse(|parser| {
            let op_code = parser.load_u32(32)?;
            match op_code {
                JETTON_TRANSFER => {
                    JettonTransferMessage::parse(cell).map(JettonMessage::JettonTransferMessage)
                }
                _ => Err(TonCellError::InternalError(format!(
                    "Invalid Op Code: {op_code:X}"
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
    pub comment: Option<String>,
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
            let destination = parser.load_address()?.to_base64_url_flags(true, false);
            let temp_address = parser.load_address()?;
            let response_destination = if temp_address.eq(&TonAddress::null()) {
                Some(temp_address.to_base64_url_flags(true, false))
            } else {
                None
            };
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
                destination,
                response_destination,
                amount,
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

    use super::{JettonMessage, JettonTransferMessage, JETTON_TRANSFER};
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

    fn build_jetton_transfer_cell(
        response_destination: &TonAddress,
        custom_payload: Option<&ArcCell>,
        forward_payload: Option<&ArcCell>,
        forward_ton_amount: u32,
    ) -> Result<ArcCell, TonCellError> {
        let mut builder = CellBuilder::new();
        builder.store_u32(32, JETTON_TRANSFER)?;
        builder.store_u64(64, 7)?;
        builder.store_coins(&BigUint::from(42u32))?;
        builder.store_address(&test_address(1))?;
        builder.store_address(response_destination)?;
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
    fn test_parse_jetton_transfer_with_payloads() -> Result<()> {
        let custom_payload = build_payload_cell(&[0xDE, 0xAD])?;
        let forward_payload = build_comment_cell("note")?;
        let cell = build_jetton_transfer_cell(
            &TonAddress::null(),
            Some(&custom_payload),
            Some(&forward_payload),
            100,
        )?;

        let parsed = JettonTransferMessage::parse(&cell)?;
        assert_eq!(parsed.query_id, "7");
        assert_eq!(parsed.amount, "42");
        assert_eq!(
            parsed.destination,
            test_address(1).to_base64_url_flags(true, false)
        );
        assert_eq!(
            parsed.response_destination,
            Some(TonAddress::null().to_base64_url_flags(true, false))
        );
        assert_eq!(
            parsed.custom_payload,
            Some(hex::encode(custom_payload.data.clone()))
        );
        assert_eq!(parsed.forward_ton_amount, "100");
        assert_eq!(
            parsed.forward_payload,
            Some(hex::encode(forward_payload.data.clone()))
        );
        assert_eq!(parsed.comment, Some("note".to_string()));

        Ok(())
    }

    #[test]
    fn test_parse_jetton_transfer_without_optional_fields() -> Result<()> {
        let response_destination = test_address(2);
        let cell = build_jetton_transfer_cell(&response_destination, None, None, 0)?;

        let parsed = JettonTransferMessage::parse(&cell)?;
        assert_eq!(parsed.response_destination, None);
        assert_eq!(parsed.custom_payload, None);
        assert_eq!(parsed.forward_ton_amount, "0");
        assert_eq!(parsed.forward_payload, None);
        assert_eq!(parsed.comment, None);

        let message = JettonMessage::parse(&cell)?;
        match message {
            JettonMessage::JettonTransferMessage(inner) => assert_eq!(inner.query_id, "7"),
        }

        Ok(())
    }

    #[test]
    fn test_parse_jetton_message_invalid_opcode() -> Result<()> {
        let mut builder = CellBuilder::new();
        builder.store_u32(32, 0xDEADBEEF)?;
        let cell = builder.build()?.to_arc();

        let err = JettonMessage::parse(&cell).unwrap_err();
        assert!(matches!(err, TonCellError::InternalError(message) if message.contains("Invalid Op Code")));

        Ok(())
    }
}
