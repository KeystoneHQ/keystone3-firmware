use crate::address::Address;
use crate::constants::*;
use crate::errors::{AvaxError, Result};
use crate::transactions::structs::{
    AvaxFromToInfo, AvaxMethodInfo, AvaxTxInfo, LengthPrefixedVec, ParsedSizeAble,
};
use crate::transactions::transferable::TransferableOutput;
use crate::transactions::tx_header::Header;
use crate::transactions::{asset_id::AssetId, type_id::TypeId};

use alloc::{
    format,
    string::{String, ToString},
    vec::Vec,
};
use bytes::{Buf, Bytes};
use core::convert::TryFrom;

#[derive(Debug, Clone)]
pub struct EvmInput {
    address: Address,
    amount: u64,
    asset_id: AssetId,
    nonce: u64,
}

impl TryFrom<Bytes> for EvmInput {
    type Error = AvaxError;

    fn try_from(mut bytes: Bytes) -> Result<Self> {
        let address = Address::try_from(bytes.split_to(C_CHAIN_ADDRESS_LEN))?;
        let amount = bytes.get_u64();
        let asset_id = AssetId::try_from(bytes.split_to(ASSET_ID_LEN))?;
        let nonce = bytes.get_u64();
        Ok(EvmInput {
            address,
            amount,
            asset_id,
            nonce,
        })
    }
}

impl ParsedSizeAble for EvmInput {
    fn parsed_size(&self) -> usize {
        C_CHAIN_ADDRESS_LEN + 8 + ASSET_ID_LEN + 8
    }
}

impl EvmInput {
    pub fn get_amount(&self) -> u64 {
        self.amount
    }
}

#[derive(Debug, Clone)]
pub struct ExportTx {
    pub codec_id: u16,
    pub type_id: TypeId,
    pub tx_header: Header,
    pub dest_chain: [u8; BLOCKCHAIN_ID_LEN],
    inputs: LengthPrefixedVec<EvmInput>,
    outputs: LengthPrefixedVec<TransferableOutput>,
}

impl AvaxTxInfo for ExportTx {
    fn get_total_input_amount(&self) -> u64 {
        self.inputs
            .iter()
            .fold(0, |acc, item| acc + item.get_amount())
    }

    fn get_total_output_amount(&self) -> u64 {
        self.outputs
            .iter()
            .fold(0, |acc, item| acc + item.get_amount())
    }

    fn get_outputs_addresses(&self) -> Vec<AvaxFromToInfo> {
        self.outputs
            .iter()
            .map(|output| {
                AvaxFromToInfo::from(
                    format!("{} AVAX", output.get_amount() as f64 / NAVAX_TO_AVAX_RATIO),
                    output.get_addresses(),
                )
            })
            .collect()
    }

    fn get_method_info(&self) -> Option<AvaxMethodInfo> {
        let method = match (self.type_id, self.dest_chain) {
            (TypeId::XchainExportTx, chain) | (TypeId::PchainExportTx, chain) => {
                let source = if matches!(self.type_id, TypeId::XchainExportTx) {
                    "X"
                } else {
                    "P"
                };
                let dest = match chain {
                    X_BLOCKCHAIN_ID => "X",
                    P_BLOCKCHAIN_ID => "P",
                    C_BLOCKCHAIN_ID => "C",
                    _ => "Unknown",
                };
                format!("{} to {} Export", source, dest)
            }
            _ => "Unknown".to_string(),
        };
        Some(AvaxMethodInfo::from_string(method).with_method_key("Export Tx".to_string()))
    }

    fn get_network_key(&self) -> String {
        "Export Tx".to_string()
    }
}

impl TryFrom<Bytes> for ExportTx {
    type Error = AvaxError;

    fn try_from(mut bytes: Bytes) -> Result<Self> {
        let codec_id = bytes.get_u16();
        let type_id = TypeId::try_from(bytes.get_u32())?;
        let tx_header = Header::try_from(bytes.clone())?;
        bytes.advance(tx_header.parsed_size());

        let mut dest_chain = [0u8; 32];
        bytes.copy_to_slice(&mut dest_chain);

        let inputs = LengthPrefixedVec::<EvmInput>::try_from(bytes.clone())?;
        bytes.advance(inputs.parsed_size());

        let outputs = LengthPrefixedVec::<TransferableOutput>::try_from(bytes.clone())?;
        bytes.advance(outputs.parsed_size());

        Ok(ExportTx {
            codec_id,
            type_id,
            tx_header,
            dest_chain,
            inputs,
            outputs,
        })
    }
}

#[cfg(test)]
mod tests {
    use crate::transactions::transferable::OutputTrait;

    use super::*;
    extern crate std;
    use std::println;

    #[test]
    fn test_avax_c_export() {
        // f22ATVQT11pSdMwvWLwXgeHNshauTDZxzErFKT5nd64esWXjU
        let input_bytes = "000000000001000000057fc93d85c6d62c5b2ac0b519c87010ea5294012d1e407030d6acd0021cac10d5000000000000000000000000000000000000000000000000000000000000000000000001a9b548da818607e83cbcf6802370691948cbd4160000000005f60cde3d9bdac0ed1d761330cf680efdeb1a42159eb387d6d2950c96f7d28f61bbe2aa0000000000000000000000013d9bdac0ed1d761330cf680efdeb1a42159eb387d6d2950c96f7d28f61bbe2aa000000070000000005f5e1000000000000000000000000010000000169bc9b5b6cbbbd490abbd79a37ad6cd643be87ab000000010000000900000001683728d1c682680aa80e702dcc8e349f79b5da8cac4b7fb59b28750f1c4c977014a70f69d5ef97dec616a21d69e3560cf2db78257710af3c03f959a487bcab04000218c13e";
        let binary_data = hex::decode(input_bytes).expect("Failed to decode hex string");
        let mut bytes = Bytes::from(binary_data);
        let result = ExportTx::try_from(bytes.clone()).unwrap();
        println!("{:?}", result);
        println!("asset id = {:?}", result.outputs.get(0).unwrap());
        println!(
            "get_outputs_addresses = {:?}",
            result.get_outputs_addresses()
        );
        println!("evm address = {:?}", result.inputs.get(0).unwrap().address.to_evm_address());
        assert!(false);
    }
}
