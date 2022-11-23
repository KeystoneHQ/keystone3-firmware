use crate::addresses::cashaddr::{Base58Codec, CashAddrCodec};
use crate::addresses::get_address;
use crate::errors::Result;
use crate::network::Network;
use crate::transactions::legacy::input::TxIn;
use crate::transactions::legacy::output::TxOut;
use crate::transactions::legacy::tx_data::TxData;
use crate::transactions::parsed_tx::{ParseContext, ParsedInput, ParsedOutput, ParsedTx, TxParser};
use alloc::string::ToString;
use alloc::vec::Vec;
use core::str::FromStr;

impl TxParser for TxData {
    fn parse(&self, _context: Option<&ParseContext>) -> Result<ParsedTx> {
        let mapped_inputs: Result<Vec<ParsedInput>> = self
            .inputs
            .iter()
            .map(|each| self.parse_raw_tx_input(each))
            .collect();
        let mapped_outputs: Result<Vec<ParsedOutput>> = self
            .outputs
            .iter()
            .map(|each| self.parse_raw_tx_output(each))
            .collect();
        let network = Network::from_str(&self.network)?;
        self.normalize(mapped_inputs?, mapped_outputs?, &network)
    }
    fn determine_network(&self) -> Result<Network> {
        Network::from_str(&self.network)
    }
}

impl TxData {
    fn parse_raw_tx_input(&self, input: &TxIn) -> Result<ParsedInput> {
        let network = &self.network;
        let address =
            get_address(input.hd_path.to_string(), &self.extended_pubkey.to_string()).ok();
        let path = &input.hd_path;
        Ok(ParsedInput {
            address,
            amount: Self::format_amount(input.value, &Network::from_str(network)?),
            value: input.value,
            path: Some(path.to_string()),
        })
    }
    fn parse_raw_tx_output(&self, output: &TxOut) -> Result<ParsedOutput> {
        let mut address = output.address.to_string();
        if output.is_change
            && self.network == Network::BitcoinCash.get_unit()
            && output.address.starts_with("1")
        {
            let decoded = Base58Codec::decode(address.as_str())?;
            address = CashAddrCodec::encode(decoded)?;
        }
        Ok(ParsedOutput {
            address,
            amount: Self::format_amount(output.value, &Network::from_str(&self.network)?),
            value: output.value,
            path: Some(output.change_address_path.to_string()),
        })
    }
}
