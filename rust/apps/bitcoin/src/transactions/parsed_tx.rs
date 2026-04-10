use crate::errors::{BitcoinError, Result};
use crate::multi_sig::wallet::MultiSigWalletConfig;
use crate::network::{Network, NetworkT};
use alloc::collections::BTreeMap;
use alloc::string::{String, ToString};
use alloc::vec::Vec;
use bitcoin::bip32::{DerivationPath, Fingerprint, Xpub};
use core::ops::Div;

#[derive(Debug, Eq, PartialEq)]
pub struct ParsedTx {
    pub overview: OverviewTx,
    pub detail: DetailTx,
}

#[derive(Debug, Clone, Eq, PartialEq)]
pub struct ParsedInput {
    pub address: Option<String>,
    pub amount: String,
    pub value: u64,
    pub input_txid: String,
    pub input_vout: u32,
    pub path: Option<String>,
    pub sign_status: (u32, u32),
    pub is_multisig: bool,
    pub is_external: bool,
    pub need_sign: bool,
    pub ecdsa_sighash_type: u8,
}

#[derive(Debug, Clone, Eq, PartialEq)]
pub struct ParsedOutput {
    pub address: String,
    pub amount: String,
    pub value: u64,
    pub path: Option<String>,
    pub is_external: bool,
    pub is_mine: bool,
}

#[derive(Debug, Clone, Eq, PartialEq, PartialOrd)]
pub struct OverviewTo {
    pub address: String,
    pub is_mine: bool,
    pub is_external: bool,
}

impl Ord for OverviewTo {
    fn cmp(&self, other: &Self) -> core::cmp::Ordering {
        self.address.cmp(&other.address)
    }
}
#[derive(Debug, Clone, Eq, PartialEq)]
pub struct OverviewTx {
    pub total_output_amount: String,
    pub fee_amount: String,
    pub total_output_sat: String,
    pub fee_sat: String,
    pub from: Vec<String>,
    pub to: Vec<OverviewTo>,
    pub network: String,
    pub fee_larger_than_amount: bool,
    pub is_multisig: bool,
    pub sign_status: Option<String>,
    pub need_sign: bool,
    pub has_witness_only_inputs: bool,
    pub fee_is_lower_bound: bool,
    pub fee_is_unknown: bool,
    pub sighash_type: Option<String>,
    pub is_sighash_single: bool,
    pub is_sighash_none: bool,
}

#[derive(Debug, Clone, Eq, PartialEq)]
pub struct DetailTx {
    pub from: Vec<ParsedInput>,
    pub to: Vec<ParsedOutput>,
    pub total_input_amount: String,
    pub total_output_amount: String,
    pub total_input_sat: String,
    pub total_output_sat: String,
    pub fee_amount: String,
    pub fee_sat: String,
    pub fee_is_lower_bound: bool,
    pub fee_is_unknown: bool,
    pub network: String,
    pub sign_status: Option<String>,
}

pub struct ParseContext {
    pub master_fingerprint: Fingerprint,
    pub extended_public_keys: BTreeMap<DerivationPath, Xpub>,
    pub verify_code: Option<String>,
    pub multisig_wallet_config: Option<MultiSigWalletConfig>,
}

impl ParseContext {
    pub fn new(
        master_fingerprint: Fingerprint,
        extended_public_keys: BTreeMap<DerivationPath, Xpub>,
        verify_code: Option<String>,
        multisig_wallet_config: Option<MultiSigWalletConfig>,
    ) -> Self {
        ParseContext {
            master_fingerprint,
            extended_public_keys,
            verify_code,
            multisig_wallet_config,
        }
    }
}

pub const DIVIDER: f64 = 100_000_000_f64;

pub trait TxParser {
    fn get_unified_tx_sighash_type(inputs: &[ParsedInput]) -> Option<u8> {
        let first = inputs.first()?;
        if inputs
            .iter()
            .all(|input| input.ecdsa_sighash_type == first.ecdsa_sighash_type)
        {
            Some(first.ecdsa_sighash_type)
        } else {
            None
        }
    }

    fn format_sighash_type(ecdsa_sighash_type: u8) -> Option<String> {
        let base = match ecdsa_sighash_type & 0x1f {
            0x01 => "ALL",
            0x02 => "NONE",
            0x03 => "SINGLE",
            _ => return None,
        };

        if ecdsa_sighash_type & 0x80 > 0 {
            Some(format!("{base}|ANYONE_CAN_PAY"))
        } else {
            Some(base.to_string())
        }
    }

    fn get_tx_sighash_type(inputs: &[ParsedInput]) -> Option<String> {
        Self::get_unified_tx_sighash_type(inputs).and_then(Self::format_sighash_type)
    }

    fn is_tx_sighash_single(inputs: &[ParsedInput]) -> bool {
        inputs
            .iter()
            .any(|input| matches!(input.ecdsa_sighash_type & 0x1f, 0x03))
    }

    fn is_tx_sighash_none(inputs: &[ParsedInput]) -> bool {
        inputs
            .iter()
            .any(|input| matches!(input.ecdsa_sighash_type & 0x1f, 0x02))
    }

    fn format_amount(value: u64, network: &dyn NetworkT) -> String {
        format!("{} {}", (value as f64).div(DIVIDER), network.get_unit())
    }

    fn format_sat(value: u64) -> String {
        format!("{value} sats")
    }

    fn parse(&self, context: Option<&ParseContext>) -> Result<ParsedTx>;

    fn determine_network(&self) -> Result<Network>;

    fn get_sign_status_text(parsed_inputs: &[ParsedInput]) -> Option<String> {
        //should combine with wrapped_psbt.get_overall_sign_status later;
        if parsed_inputs.is_empty() {
            return None;
        }
        let first_multi_status = parsed_inputs[0].sign_status;
        //none of inputs is signed
        if parsed_inputs.iter().all(|input| input.sign_status.0 == 0) {
            Some(String::from("Unsigned"))
        }
        //or some inputs are signed and completed
        else if parsed_inputs
            .iter()
            .all(|input| input.sign_status.0 >= input.sign_status.1)
        {
            return Some(String::from("Completed"));
        }
        //or inputs are partially signed and all of them are multisig inputs
        else if parsed_inputs.iter().all(|input| {
            input.sign_status.0 == first_multi_status.0
                && input.sign_status.1 == first_multi_status.1
        }) {
            return Some(format!(
                "{}/{} Signed",
                first_multi_status.0, first_multi_status.1
            ));
        } else {
            return Some(String::from("Partly Signed"));
        }
    }

    fn is_need_sign(parsed_inputs: &[ParsedInput]) -> bool {
        for input in parsed_inputs.iter() {
            if input.need_sign {
                return true;
            }
        }
        false
    }

    fn normalize(
        &self,
        inputs: Vec<ParsedInput>,
        outputs: Vec<ParsedOutput>,
        network: &dyn NetworkT,
        has_witness_only_inputs: bool,
    ) -> Result<ParsedTx> {
        let total_input_value = inputs.iter().fold(0, |acc, cur| acc + cur.value);
        let total_output_value = outputs.iter().fold(0, |acc, cur| acc + cur.value);
        let has_anyone_can_pay = inputs.iter().any(|v| v.ecdsa_sighash_type & 0x80 > 0);
        let has_unlocked_outputs = inputs
            .iter()
            .any(|v| !matches!(v.ecdsa_sighash_type & 0x1f, 0x01));
        // in some special cases like Unisat Listing transaction, transaction input will be less than output value
        // for these wallet will reassemble the transaction on their end.
        if !has_anyone_can_pay && (total_input_value < total_output_value) {
            return Err(BitcoinError::InvalidTransaction(
                "inputs total value is less than outputs total value".to_string(),
            ));
        }
        let fee_is_unknown =
            has_unlocked_outputs || (has_anyone_can_pay && total_input_value < total_output_value);
        let fee_is_lower_bound = has_anyone_can_pay && !fee_is_unknown;
        let fee = total_input_value.saturating_sub(total_output_value);
        let fee_amount = Self::format_amount(fee, network);
        let fee_sat = Self::format_sat(fee);
        let overview_amount = outputs.iter().fold(0, |acc, cur| {
            if cur.path.is_none() {
                return acc + cur.value;
            }
            if let Some(hd_path) = &cur.path {
                if hd_path.is_empty() {
                    return acc + cur.value;
                }
            };
            acc
        });
        let mut overview_from = inputs
            .iter()
            .filter_map(|v| v.address.clone())
            .collect::<Vec<String>>();
        overview_from.sort();
        overview_from.dedup();
        let mut overview_to = outputs
            .iter()
            .map(|v| OverviewTo {
                address: v.address.clone(),
                is_mine: v.is_mine,
                is_external: v.is_external,
            })
            .collect::<Vec<OverviewTo>>();
        overview_to.sort();
        overview_to.dedup();
        let overview = OverviewTx {
            has_witness_only_inputs,
            sighash_type: Self::get_tx_sighash_type(&inputs),
            is_sighash_single: Self::is_tx_sighash_single(&inputs),
            is_sighash_none: Self::is_tx_sighash_none(&inputs),
            sign_status: Self::get_sign_status_text(&inputs),
            total_output_amount: Self::format_amount(overview_amount, network),
            fee_amount: fee_amount.clone(),
            total_output_sat: Self::format_sat(overview_amount),
            fee_sat: fee_sat.clone(),
            fee_is_lower_bound,
            fee_is_unknown,
            from: overview_from,
            to: overview_to,
            network: network.normalize(),
            fee_larger_than_amount: fee > overview_amount,
            is_multisig: inputs.iter().any(|v| v.is_multisig),
            need_sign: Self::is_need_sign(&inputs),
        };
        let detail = DetailTx {
            sign_status: Self::get_sign_status_text(&inputs),
            from: inputs,
            to: outputs,
            total_input_amount: Self::format_amount(total_input_value, network),
            total_output_amount: Self::format_amount(total_output_value, network),
            fee_amount,
            total_input_sat: Self::format_sat(total_input_value),
            total_output_sat: Self::format_sat(total_output_value),
            fee_sat,
            fee_is_lower_bound,
            fee_is_unknown,
            network: network.normalize(),
        };
        Ok(ParsedTx { overview, detail })
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    struct DummyParser;

    impl TxParser for DummyParser {
        fn parse(&self, _context: Option<&ParseContext>) -> Result<ParsedTx> {
            unreachable!()
        }

        fn determine_network(&self) -> Result<Network> {
            unreachable!()
        }
    }

    fn build_input(sighash_type: u8) -> ParsedInput {
        ParsedInput {
            address: None,
            amount: "0 BTC".to_string(),
            value: 0,
            input_txid: String::new(),
            input_vout: 0,
            path: None,
            sign_status: (0, 1),
            is_multisig: false,
            is_external: false,
            need_sign: false,
            ecdsa_sighash_type: sighash_type,
        }
    }

    fn build_input_with_value(value: u64, sighash_type: u8) -> ParsedInput {
        ParsedInput {
            value,
            ..build_input(sighash_type)
        }
    }

    fn build_output(value: u64) -> ParsedOutput {
        ParsedOutput {
            address: String::new(),
            amount: "0 BTC".to_string(),
            value,
            path: None,
            is_external: true,
            is_mine: false,
        }
    }

    #[test]
    fn test_format_sighash_type() {
        assert_eq!(
            DummyParser::format_sighash_type(0x01),
            Some("ALL".to_string())
        );
        assert_eq!(
            DummyParser::format_sighash_type(0x02),
            Some("NONE".to_string())
        );
        assert_eq!(
            DummyParser::format_sighash_type(0x03),
            Some("SINGLE".to_string())
        );
        assert_eq!(
            DummyParser::format_sighash_type(0x81),
            Some("ALL|ANYONE_CAN_PAY".to_string())
        );
        assert_eq!(
            DummyParser::format_sighash_type(0x82),
            Some("NONE|ANYONE_CAN_PAY".to_string())
        );
        assert_eq!(
            DummyParser::format_sighash_type(0x83),
            Some("SINGLE|ANYONE_CAN_PAY".to_string())
        );
        assert_eq!(DummyParser::format_sighash_type(0x00), None);
    }

    #[test]
    fn test_get_tx_sighash_type() {
        assert_eq!(
            DummyParser::get_tx_sighash_type(&[build_input(0x83), build_input(0x83)]),
            Some("SINGLE|ANYONE_CAN_PAY".to_string())
        );
        assert_eq!(
            DummyParser::get_tx_sighash_type(&[build_input(0x01), build_input(0x02)]),
            None
        );
        assert_eq!(DummyParser::get_tx_sighash_type(&[]), None);
    }

    #[test]
    fn test_get_tx_sighash_warning_flags() {
        assert!(DummyParser::is_tx_sighash_single(&[
            build_input(0x03),
            build_input(0x03)
        ]));
        assert!(DummyParser::is_tx_sighash_single(&[
            build_input(0x83),
            build_input(0x83)
        ]));
        assert!(!DummyParser::is_tx_sighash_single(&[build_input(0x02)]));

        assert!(DummyParser::is_tx_sighash_none(&[
            build_input(0x02),
            build_input(0x02)
        ]));
        assert!(DummyParser::is_tx_sighash_none(&[
            build_input(0x82),
            build_input(0x82)
        ]));
        assert!(!DummyParser::is_tx_sighash_none(&[build_input(0x03)]));

        assert!(DummyParser::is_tx_sighash_single(&[
            build_input(0x03),
            build_input(0x83)
        ]));
        assert!(DummyParser::is_tx_sighash_none(&[
            build_input(0x02),
            build_input(0x82)
        ]));
        assert!(DummyParser::is_tx_sighash_single(&[
            build_input(0x01),
            build_input(0x03)
        ]));
        assert!(DummyParser::is_tx_sighash_none(&[
            build_input(0x01),
            build_input(0x02)
        ]));
    }

    #[test]
    fn test_anyone_can_pay_fee_is_lower_bound() {
        let parsed = DummyParser
            .normalize(
                vec![build_input_with_value(1_000, 0x81)],
                vec![build_output(600)],
                &Network::Bitcoin,
                false,
            )
            .unwrap();

        assert!(parsed.overview.fee_is_lower_bound);
        assert!(parsed.detail.fee_is_lower_bound);
        assert!(!parsed.overview.fee_is_unknown);
        assert!(!parsed.detail.fee_is_unknown);
        assert_eq!("0.000004 BTC", parsed.overview.fee_amount);
        assert_eq!("400 sats", parsed.overview.fee_sat);
        assert_eq!("0.000004 BTC", parsed.detail.fee_amount);
        assert_eq!("400 sats", parsed.detail.fee_sat);
    }

    #[test]
    fn test_anyone_can_pay_fee_is_unknown_when_outputs_exceed_inputs() {
        let parsed = DummyParser
            .normalize(
                vec![build_input_with_value(600, 0x81)],
                vec![build_output(1_000)],
                &Network::Bitcoin,
                false,
            )
            .unwrap();

        assert!(!parsed.overview.fee_is_lower_bound);
        assert!(!parsed.detail.fee_is_lower_bound);
        assert!(parsed.overview.fee_is_unknown);
        assert!(parsed.detail.fee_is_unknown);
        assert_eq!("0 BTC", parsed.overview.fee_amount);
        assert_eq!("0 sats", parsed.overview.fee_sat);
        assert_eq!("0 BTC", parsed.detail.fee_amount);
        assert_eq!("0 sats", parsed.detail.fee_sat);
    }

    #[test]
    fn test_unlocked_outputs_fee_is_unknown() {
        for sighash_type in [0x02, 0x03, 0x82, 0x83] {
            let parsed = DummyParser
                .normalize(
                    vec![build_input_with_value(1_000, sighash_type)],
                    vec![build_output(600)],
                    &Network::Bitcoin,
                    false,
                )
                .unwrap();

            assert!(!parsed.overview.fee_is_lower_bound);
            assert!(!parsed.detail.fee_is_lower_bound);
            assert!(parsed.overview.fee_is_unknown);
            assert!(parsed.detail.fee_is_unknown);
            assert_eq!("0.000004 BTC", parsed.overview.fee_amount);
            assert_eq!("400 sats", parsed.overview.fee_sat);
            assert_eq!("0.000004 BTC", parsed.detail.fee_amount);
            assert_eq!("400 sats", parsed.detail.fee_sat);
        }
    }
}
