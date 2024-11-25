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
    pub is_multisig: bool,
    pub sign_status: Option<String>,
    pub need_sign: bool,
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

pub const DIVIDER: f64 = 100_000_000 as f64;

pub trait TxParser {
    fn format_amount(value: u64, network: &dyn NetworkT) -> String {
        format!("{} {}", (value as f64).div(DIVIDER), network.get_unit())
    }

    fn format_sat(value: u64) -> String {
        format!("{} sats", value)
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
            return Some(String::from("Unsigned"));
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
        for (_index, input) in parsed_inputs.iter().enumerate() {
            if input.need_sign {
                return true;
            }
        }
        return false;
    }

    fn normalize(
        &self,
        inputs: Vec<ParsedInput>,
        outputs: Vec<ParsedOutput>,
        network: &dyn NetworkT,
    ) -> Result<ParsedTx> {
        let total_input_value = inputs.iter().fold(0, |acc, cur| acc + cur.value);
        let total_output_value = outputs.iter().fold(0, |acc, cur| acc + cur.value);
        let has_anyone_can_pay = inputs
            .iter()
            .find(|v| v.ecdsa_sighash_type & 0x80 > 0)
            .is_some();
        let fee = if has_anyone_can_pay {
            0
        } else {
            total_input_value - total_output_value
        };
        // in some special cases like Unisat Listing transaction, transaction input will be less than output value
        // for these wallet will reassemble the transaction on their end.
        if !has_anyone_can_pay && (total_input_value < total_output_value) {
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
            sign_status: Self::get_sign_status_text(&inputs),
            total_output_amount: Self::format_amount(overview_amount, network),
            fee_amount: Self::format_amount(fee, network),
            total_output_sat: Self::format_sat(overview_amount),
            fee_sat: Self::format_sat(fee),
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
            fee_amount: Self::format_amount(fee, network),
            total_input_sat: Self::format_sat(total_input_value),
            total_output_sat: Self::format_sat(total_output_value),
            fee_sat: Self::format_sat(fee),
            network: network.normalize(),
        };
        Ok(ParsedTx { overview, detail })
    }
}
