use super::base_tx::BaseTx;
use super::structs::{AvaxFromToInfo, AvaxMethodInfo, AvaxTxInfo, LengthPrefixedVec};
use crate::constants::*;
use crate::errors::{AvaxError, Result};
use crate::transactions::{transferable::TransferableOutput, type_id::TypeId};
use alloc::{
    format,
    string::{String, ToString},
    vec::Vec,
};
use bytes::{Buf, Bytes};
use core::convert::TryFrom;

#[derive(Debug)]
pub struct ExportTx {
    base_tx: BaseTx,
    dest_chain: [u8; BLOCKCHAIN_ID_LEN],
    transfer_out: LengthPrefixedVec<TransferableOutput>,
}

impl AvaxTxInfo for ExportTx {
    fn get_total_input_amount(&self) -> u64 {
        self.base_tx.get_total_input_amount()
    }

    fn get_total_output_amount(&self) -> u64 {
        self.base_tx.get_total_output_amount()
            + self
                .transfer_out
                .iter()
                .fold(0, |acc, item| acc + item.get_amount())
    }

    fn get_outputs_addresses(&self) -> Vec<AvaxFromToInfo> {
        self.base_tx
            .get_outputs_addresses()
            .into_iter()
            .chain(self.transfer_out.iter().map(|output| {
                AvaxFromToInfo::from(
                    format!("{} AVAX", output.get_amount() as f64 / NAVAX_TO_AVAX_RATIO),
                    output.get_addresses(),
                )
            }))
            .collect()
    }

    fn get_method_info(&self) -> Option<AvaxMethodInfo> {
        let method = match (self.base_tx.type_id, self.dest_chain) {
            (TypeId::XchainExportTx, chain) | (TypeId::PchainExportTx, chain) => {
                let source = if matches!(self.base_tx.type_id, TypeId::XchainExportTx) {
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

impl ExportTx {
    fn get_base_tx(&self) -> &BaseTx {
        &self.base_tx
    }

    fn get_dest_chain(&self) -> [u8; BLOCKCHAIN_ID_LEN] {
        self.dest_chain
    }

    fn get_transfer_out(&self) -> &LengthPrefixedVec<TransferableOutput> {
        &self.transfer_out
    }
}

impl TryFrom<Bytes> for ExportTx {
    type Error = AvaxError;

    fn try_from(mut bytes: Bytes) -> Result<Self> {
        let base_tx = BaseTx::try_from(bytes.clone())?;
        bytes.advance(base_tx.parsed_size());
        let mut dest_chain = [0u8; BLOCKCHAIN_ID_LEN];
        bytes.copy_to_slice(&mut dest_chain);

        Ok(ExportTx {
            base_tx,
            dest_chain,
            transfer_out: LengthPrefixedVec::<TransferableOutput>::try_from(bytes.clone())?,
        })
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::transactions::type_id::TypeId;
    extern crate std;
    use std::println;

    #[test]
    fn test_avax_base_export_tx() {
        {
            // export to C
            let input_bytes = "00000000000400000005ab68eb1ee142a05cfe768c36e11f0b596db5a3c6c77aabe665dad9e638ca94f7000000013d9bdac0ed1d761330cf680efdeb1a42159eb387d6d2950c96f7d28f61bbe2aa000000070000000057eab6c0000000000000000000000001000000013d200933d192f824fc532519e8ae826990fee64000000001d3f3594499a1e5ce4fcf9aeb139dafb51c1ba18b89c5dc5cefe380a39ca9ba04000000003d9bdac0ed1d761330cf680efdeb1a42159eb387d6d2950c96f7d28f61bbe2aa000000050000000058928f800000000100000000000000007fc93d85c6d62c5b2ac0b519c87010ea5294012d1e407030d6acd0021cac10d5000000013d9bdac0ed1d761330cf680efdeb1a42159eb387d6d2950c96f7d28f61bbe2aa00000007000000000098968000000000000000000000000100000001d6178ba78d772ea8a01782488a6d1937a4a5cc2b000000010000000900000001d2c31dc8565f8b70438abd550dc7ecfc8f33e23c7bd2b3954c31572b991eeb8d6c2990148f8a59b33b3481719c759355c6036f1eaf2e6210313cfae6d6cfe87600d44e2649";

            // x export to p
            let input_bytes = "00000000000400000005ab68eb1ee142a05cfe768c36e11f0b596db5a3c6c77aabe665dad9e638ca94f7000000013d9bdac0ed1d761330cf680efdeb1a42159eb387d6d2950c96f7d28f61bbe2aa00000007000000005742de00000000000000000000000001000000013d200933d192f824fc532519e8ae826990fee64000000001bb4002f89f58f40649a197e557549cc9a2481f54c62b720e5289d5b8d44e2649000000003d9bdac0ed1d761330cf680efdeb1a42159eb387d6d2950c96f7d28f61bbe2aa000000050000000057eab6c00000000100000000000000000000000000000000000000000000000000000000000000000000000000000000000000013d9bdac0ed1d761330cf680efdeb1a42159eb387d6d2950c96f7d28f61bbe2aa000000070000000000989680000000000000000000000001000000013d200933d192f824fc532519e8ae826990fee640000000010000000900000001a0d23a3d847854238fdc20fcd55f280f144dead9b4ac5e7704efcedce789970e1cce1656d5d603bbb9fe268bff5b99b13a6390b0eec8608fed7696372d1a2f58007d974fba";

            // p chain export to x
            let input_bytes = "000000000012000000050000000000000000000000000000000000000000000000000000000000000000000000013d9bdac0ed1d761330cf680efdeb1a42159eb387d6d2950c96f7d28f61bbe2aa0000000700000000001e848000000000000000000000000100000001cc4822028594f008045b77cf8342e8f62db1a1da000000011ebc7d69c8e7d50ce59dac9c0de8c1f22a76274d8454d3427be44d5776335156000000003d9bdac0ed1d761330cf680efdeb1a42159eb387d6d2950c96f7d28f61bbe2aa0000000500000000004c4b400000000100000000000000007fc93d85c6d62c5b2ac0b519c87010ea5294012d1e407030d6acd0021cac10d5000000013d9bdac0ed1d761330cf680efdeb1a42159eb387d6d2950c96f7d28f61bbe2aa0000000700000000001e84800000000000000000000000010000000100739a08d627492cea5abec92998ddc891cd6177000000010000000900000001cda36ebb7b114f0479bfa2eec2ccda8030fd9fd088a1ba19109971c0ffccc9293e21f827502d0210dc7a2a8af11ff7d63f1f858777709e9053b6296287f7abde00e89a6479";
            let mut bytes =
                Bytes::from(hex::decode(input_bytes).expect("Failed to decode hex string"));
            let result = ExportTx::try_from(bytes).unwrap();
            assert_eq!(result.get_dest_chain(), X_BLOCKCHAIN_ID);
        }
    }
}
