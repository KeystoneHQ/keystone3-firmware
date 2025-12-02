use alloc::format;
use alloc::string::{String, ToString};
use core::ops::Div;
use core::ptr::null_mut;

use serde_json;
use serde_json::Value;

use crate::errors::{XRPError, R};
use crate::parser::overview::{XrpTxOverview, XrpTxOverviewGeneral, XrpTxOverviewPayment};
use crate::parser::structs::{ParsedXrpTx, XrpTxDisplayType};

pub mod overview;
pub mod structs;

pub const DIVIDER: f64 = 1_000_000f64;

impl ParsedXrpTx {
    pub fn build_batch(tx: &Value, service_fee: &Value) -> R<Self> {
        let display_type = Self::detect_display_type(&tx)?;
        let parsed_overview = Self::build_overview(&display_type, &tx)?;
        let parsed_detail = Self::build_detail(&tx)?;
        let parsed_service_fee_detail = Self::build_detail(&service_fee)?;
        Ok(Self {
            display_type,
            overview: parsed_overview,
            detail: parsed_detail,
            service_fee_detail: Some(parsed_service_fee_detail),
            network: "XRP Mainnet".to_string(),
            signing_pubkey: Self::format_field(&tx["SigningPubKey"])?,
        })
    }

    pub fn build(tx: Value) -> R<Self> {
        let display_type = Self::detect_display_type(&tx)?;
        let parsed_overview = Self::build_overview(&display_type, &tx)?;
        let parsed_detail = Self::build_detail(&tx)?;
        Ok(Self {
            display_type,
            overview: parsed_overview,
            detail: parsed_detail,
            network: "XRP Mainnet".to_string(),
            signing_pubkey: Self::format_field(&tx["SigningPubKey"])?,
            service_fee_detail: None,
        })
    }

    fn detect_display_type(tx: &Value) -> R<XrpTxDisplayType> {
        if let Some(tx_type) = tx["TransactionType"].as_str() {
            return if tx_type.eq("Payment") {
                Ok(XrpTxDisplayType::Payment)
            } else {
                Ok(XrpTxDisplayType::General)
            };
        }
        Err(XRPError::ParseTxError(format!(
            "invalid transaction type {:?}",
            tx["TransactionType"]
        )))
    }
    fn build_detail(tx: &Value) -> R<String> {
        Ok(serde_json::to_string_pretty(tx)?)
    }

    fn build_overview(display_type: &XrpTxDisplayType, tx: &Value) -> R<XrpTxOverview> {
        match display_type {
            XrpTxDisplayType::Payment => Self::build_transfer_overview(tx),
            XrpTxDisplayType::General => Self::build_general_overview(tx),
        }
    }

    fn format_amount(amount: &Value) -> R<String> {
        if let Some(value) = amount.as_str().and_then(|v| v.parse::<f64>().ok()) {
            return Ok(format!("{} {}", value.div(DIVIDER), "XRP"));
        }
        if let Some(value) = amount.as_object() {
            if let (Some(currency), Some(value)) =
                (value["currency"].as_str(), value["value"].as_str())
            {
                return Ok(format!("{value} {currency}"));
            }
        }
        Err(XRPError::ParseTxError(format!(
            "format amount failed {amount:?}"
        )))
    }

    fn format_field(field: &Value) -> R<String> {
        if let Some(v) = field.as_str() {
            return Ok(v.to_string());
        }
        Err(XRPError::ParseTxError(format!(
            "format field failed {field:?}"
        )))
    }

    fn format_sequence(sequence: &Value) -> R<u64> {
        if let Some(value) = sequence.as_u64() {
            return Ok(value);
        }
        Err(XRPError::ParseTxError(format!(
            "format field failed {sequence:?}"
        )))
    }

    fn build_transfer_overview(tx: &Value) -> R<XrpTxOverview> {
        Ok(XrpTxOverview::Payment(XrpTxOverviewPayment {
            value: Self::format_amount(&tx["Amount"])?,
            transaction_type: "Payment".to_string(),
            from: Self::format_field(&tx["Account"])?,
            to: Self::format_field(&tx["Destination"])?,
            fee: Self::format_amount(&tx["Fee"])?,
        }))
    }

    fn build_general_overview(tx: &Value) -> R<XrpTxOverview> {
        Ok(XrpTxOverview::General(XrpTxOverviewGeneral {
            transaction_type: Self::format_field(&tx["TransactionType"])?,
            from: Self::format_field(&tx["Account"])?,
            fee: Self::format_amount(&tx["Fee"])?,
            sequence: Self::format_sequence(&tx["Sequence"])?,
        }))
    }
}

#[cfg(test)]
mod tests {
    use serde_json::from_str;

    use crate::transaction::WrappedTxData;

    use super::*;

    #[test]
    fn test_parse_tx_1() {
        // USD payment
        let tx_str = r#"{
            "TransactionType" : "Payment",
            "Account" : "rf1BiGeXwwQoi8Z2ueFYTEXSwuJYfV2Jpn",
            "Destination" : "ra5nK24KXen9AHvsdFTKHSANinZseWnPcX",
            "Amount" : {
                "currency" : "USD",
                "value" : "1",
                "issuer" : "rf1BiGeXwwQoi8Z2ueFYTEXSwuJYfV2Jpn"
            },
            "Fee": "12",
            "Flags": 2147483648,
            "Sequence": 2,
            "SigningPubKey": "03F5C5BB1D19EC710D3D7FAD199AF10CF8BC1D11348E5B3765C0B0B9C0BEC32879"
        }"#;
        let v: Value = from_str(tx_str).unwrap();
        let input_bytes = v.to_string().into_bytes();
        let wrapped_tx = WrappedTxData::from_raw(input_bytes.as_slice()).unwrap();
        let parsed = ParsedXrpTx::build(wrapped_tx.tx_data).unwrap();
        assert_eq!("XRP Mainnet", parsed.network.as_str());
        assert_eq!(XrpTxDisplayType::Payment, parsed.display_type);
        match parsed.overview {
            XrpTxOverview::Payment(overview) => {
                assert_eq!("0.000012 XRP", overview.fee);
                assert_eq!("ra5nK24KXen9AHvsdFTKHSANinZseWnPcX", overview.to);
                assert_eq!("rf1BiGeXwwQoi8Z2ueFYTEXSwuJYfV2Jpn", overview.from);
                assert_eq!("1 USD", overview.value);
                assert_eq!("Payment", overview.transaction_type);
            }
            _ => panic!("transaction parse error!"),
        };
        let parsed_detail: Value = serde_json::from_str(parsed.detail.as_str()).unwrap();
        let expected_detail: Value = serde_json::from_str(tx_str).unwrap();
        assert_eq!(expected_detail, parsed_detail);
    }

    #[test]
    fn test_parse_tx_2() {
        // XRP payment
        let tx_str = r#"{
            "TransactionType": "Payment",
            "Amount": "10000000",
            "Destination": "rDxQoYzcQrpzVHuT4Wx6bacJYXyGTEtbvm",
            "Flags": 2147483648,
            "Account": "rGUmkyLbvqGF3hwX4qwGHdrzLdY2Qpskum",
            "Fee": "12",
            "Sequence": 79991865,
            "LastLedgerSequence": 80815479,
            "SigningPubKey": "03F5C5BB1D19EC710D3D7FAD199AF10CF8BC1D11348E5B3765C0B0B9C0BEC32879"
        }"#;
        let v: Value = from_str(tx_str).unwrap();
        let input_bytes = v.to_string().into_bytes();
        let wrapped_tx = WrappedTxData::from_raw(input_bytes.as_slice()).unwrap();
        let parsed = ParsedXrpTx::build(wrapped_tx.tx_data).unwrap();
        assert_eq!("XRP Mainnet", parsed.network.as_str());
        assert_eq!(XrpTxDisplayType::Payment, parsed.display_type);
        match parsed.overview {
            XrpTxOverview::Payment(overview) => {
                assert_eq!("0.000012 XRP", overview.fee);
                assert_eq!("rDxQoYzcQrpzVHuT4Wx6bacJYXyGTEtbvm", overview.to);
                assert_eq!("rGUmkyLbvqGF3hwX4qwGHdrzLdY2Qpskum", overview.from);
                assert_eq!("10 XRP", overview.value);
                assert_eq!("Payment", overview.transaction_type);
            }
            _ => panic!("transaction parse error!"),
        };
        let parsed_detail: Value = serde_json::from_str(parsed.detail.as_str()).unwrap();
        let expected_detail: Value = serde_json::from_str(tx_str).unwrap();
        assert_eq!(expected_detail, parsed_detail);
    }
    #[test]
    fn test_parse_tx_3() {
        // AccountDelete
        let tx_str = r#"{
            "Account": "rfkE1aSy9G8Upk4JssnwBxhEv5p4mn2KTy",
            "TransactionType": "CheckCash",
            "Amount": "100000000",
            "CheckID": "838766BA2B995C00744175F69A1B11E32C3DBC40E64801A4056FCBD657F57334",
            "Sequence": 5,
            "Fee": "12",
            "SigningPubKey": "03F5C5BB1D19EC710D3D7FAD199AF10CF8BC1D11348E5B3765C0B0B9C0BEC32879"
        }"#;
        let v: Value = from_str(tx_str).unwrap();
        let input_bytes = v.to_string().into_bytes();
        let wrapped_tx = WrappedTxData::from_raw(input_bytes.as_slice()).unwrap();
        let parsed = ParsedXrpTx::build(wrapped_tx.tx_data).unwrap();
        assert_eq!("XRP Mainnet", parsed.network.as_str());
        assert_eq!(XrpTxDisplayType::General, parsed.display_type);
        match parsed.overview {
            XrpTxOverview::General(overview) => {
                assert_eq!("rfkE1aSy9G8Upk4JssnwBxhEv5p4mn2KTy", overview.from);
                assert_eq!("CheckCash", overview.transaction_type);
                assert_eq!(5, overview.sequence);
            }
            _ => panic!("transaction parse error!"),
        };
        let parsed_detail: Value = serde_json::from_str(parsed.detail.as_str()).unwrap();
        let expected_detail: Value = serde_json::from_str(tx_str).unwrap();
        assert_eq!(expected_detail, parsed_detail);
    }

    #[test]
    fn test_parse_tx_4() {
        // AccountSet
        let tx_str = r#"{
            "TransactionType": "AccountSet",
            "Account" : "rf1BiGeXwwQoi8Z2ueFYTEXSwuJYfV2Jpn",
            "Fee": "12",
            "Sequence": 5,
            "Domain": "6578616D706C652E636F6D",
            "SetFlag": 5,
            "MessageKey": "03AB40A0490F9B7ED8DF29D246BF2D6269820A0EE7742ACDD457BEA7C7D0931EDB",
            "SigningPubKey":"03F5C5BB1D19EC710D3D7FAD199AF10CF8BC1D11348E5B3765C0B0B9C0BEC32879"
        }"#;
        let v: Value = from_str(tx_str).unwrap();
        let input_bytes = v.to_string().into_bytes();
        let wrapped_tx = WrappedTxData::from_raw(input_bytes.as_slice()).unwrap();
        let parsed = ParsedXrpTx::build(wrapped_tx.tx_data).unwrap();
        assert_eq!("XRP Mainnet", parsed.network.as_str());
        assert_eq!(XrpTxDisplayType::General, parsed.display_type);
        match parsed.overview {
            XrpTxOverview::General(overview) => {
                assert_eq!("0.000012 XRP", overview.fee);
                assert_eq!("rf1BiGeXwwQoi8Z2ueFYTEXSwuJYfV2Jpn", overview.from);
                assert_eq!("AccountSet", overview.transaction_type);
                assert_eq!(5, overview.sequence);
            }
            _ => panic!("transaction parse error!"),
        };
        let parsed_detail: Value = serde_json::from_str(parsed.detail.as_str()).unwrap();
        let expected_detail: Value = serde_json::from_str(tx_str).unwrap();
        assert_eq!(expected_detail, parsed_detail);
    }

    #[test]
    fn test_parse_tx_5() {
        // CheckCancel
        let tx_str = r#"{
            "Account": "rUn84CUYbNjRoTQ6mSW7BVJPSVJNLb1QLo",
            "TransactionType": "CheckCancel",
            "CheckID": "49647F0D748DC3FE26BDACBC57F251AADEFFF391403EC9BF87C97F67E9977FB0",
            "Fee": "12",
            "Sequence": 5,
            "SigningPubKey":"03F5C5BB1D19EC710D3D7FAD199AF10CF8BC1D11348E5B3765C0B0B9C0BEC32879"
        }"#;
        let v: Value = from_str(tx_str).unwrap();
        let input_bytes = v.to_string().into_bytes();
        let wrapped_tx = WrappedTxData::from_raw(input_bytes.as_slice()).unwrap();
        let parsed = ParsedXrpTx::build(wrapped_tx.tx_data).unwrap();
        assert_eq!("XRP Mainnet", parsed.network.as_str());
        assert_eq!(XrpTxDisplayType::General, parsed.display_type);
        match parsed.overview {
            XrpTxOverview::General(overview) => {
                assert_eq!("0.000012 XRP", overview.fee);
                assert_eq!("rUn84CUYbNjRoTQ6mSW7BVJPSVJNLb1QLo", overview.from);
                assert_eq!("CheckCancel", overview.transaction_type);
                assert_eq!(5, overview.sequence);
            }
            _ => panic!("transaction parse error!"),
        };
        let parsed_detail: Value = serde_json::from_str(parsed.detail.as_str()).unwrap();
        let expected_detail: Value = serde_json::from_str(tx_str).unwrap();
        assert_eq!(expected_detail, parsed_detail);
    }
}
