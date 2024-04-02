use crate::errors::{BitcoinError, Result};
use crate::network::Network;
use alloc::collections::BTreeMap;
use alloc::string::{String, ToString};
use alloc::vec::Vec;
use core::ops::Div;
use third_party::bitcoin::bip32::{DerivationPath, Fingerprint, Xpub};

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
    pub path: Option<String>,
    pub multi_sig_status: Option<String>,
}

#[derive(Debug, Clone, Eq, PartialEq)]
pub struct ParsedOutput {
    pub address: String,
    pub amount: String,
    pub value: u64,
    pub path: Option<String>,
}

#[derive(Debug, Clone, Eq, PartialEq)]
pub struct OverviewTx {
    pub total_output_amount: String,
    pub fee_amount: String,
    pub total_output_sat: String,
    pub fee_sat: String,
    pub from: Vec<String>,
    pub to: Vec<String>,
    pub network: String,
    pub fee_larger_than_amount: bool,
    pub multi_sig_status: Option<String>,
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
    pub network: String,
    pub multi_sig_status: Option<String>,
}

pub struct ParseContext {
    pub master_fingerprint: Fingerprint,
    pub extended_public_keys: BTreeMap<DerivationPath, Xpub>,
}

impl ParseContext {
    pub fn new(
        master_fingerprint: Fingerprint,
        extended_public_keys: BTreeMap<DerivationPath, Xpub>,
    ) -> Self {
        ParseContext {
            master_fingerprint,
            extended_public_keys,
        }
    }
}

pub const DIVIDER: f64 = 100_000_000 as f64;

pub trait TxParser {
    fn format_amount(value: u64, network: &Network) -> String {
        format!("{} {}", (value as f64).div(DIVIDER), network.get_unit())
    }

    fn format_sat(value: u64) -> String {
        format!("{} sats", value)
    }

    fn parse(&self, context: Option<&ParseContext>) -> Result<ParsedTx>;

    fn determine_network(&self) -> Result<Network>;

    fn get_multi_status(parsed_inputs: &[ParsedInput]) -> Option<String> {
        if parsed_inputs.is_empty() {
            return None;
        }
        let first_multi_status = parsed_inputs[0].multi_sig_status.as_ref();
        if parsed_inputs
            .iter()
            .all(|input| input.multi_sig_status.as_ref() == first_multi_status)
        {
            if let Some(value) = first_multi_status {
                if value.starts_with("0") {
                    return Some(String::from("Unsigned"));
                }
            }
            first_multi_status.cloned()
        } else {
            Some(String::from("Partial Signed"))
        }
    }

    fn normalize(
        &self,
        inputs: Vec<ParsedInput>,
        outputs: Vec<ParsedOutput>,
        network: &Network,
    ) -> Result<ParsedTx> {
        let total_input_value = inputs.iter().fold(0, |acc, cur| acc + cur.value);
        let total_output_value = outputs.iter().fold(0, |acc, cur| acc + cur.value);
        let fee = total_input_value - total_output_value;
        if total_input_value < total_output_value {
            return Err(BitcoinError::InvalidTransaction(
                "inputs total value is less than outputs total value".to_string(),
            ));
        }
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
            .filter(|v| v.address.is_some())
            .map(|v| v.address.clone().unwrap_or("Unknown Address".to_string()))
            .collect::<Vec<String>>();
        overview_from.sort();
        overview_from.dedup();
        let mut overview_to = outputs
            .iter()
            .map(|v| v.address.clone())
            .collect::<Vec<String>>();
        overview_to.sort();
        overview_to.dedup();
        let overview = OverviewTx {
            multi_sig_status: Self::get_multi_status(&inputs),
            total_output_amount: Self::format_amount(overview_amount, network),
            fee_amount: Self::format_amount(fee, network),
            total_output_sat: Self::format_sat(overview_amount),
            fee_sat: Self::format_sat(fee),
            from: overview_from,
            to: overview_to,
            network: network.normalize(),
            fee_larger_than_amount: fee > overview_amount,
        };
        let detail = DetailTx {
            multi_sig_status: Self::get_multi_status(&inputs),
            from: inputs,
            to: outputs,
            total_input_amount: Self::format_amount(total_input_value, network),
            total_output_amount: Self::format_amount(total_output_value, network),
            fee_amount: Self::format_amount(fee, network),
            total_input_sat: Self::format_sat(total_input_value),
            total_output_sat: Self::format_sat(total_output_value),
            fee_sat: Self::format_sat(fee),
            network: network.normalize(),
        };
        Ok(ParsedTx { overview, detail })
    }
}
