use super::structs::{
    AvaxFromToInfo, AvaxMethodInfo, AvaxTxInfo, LengthPrefixedVec, ParsedSizeAble,
};
use super::transferable::{TransferableInput, TransferableOutput};
use super::tx_header::Header;
use super::type_id::TypeId;
use crate::constants::*;
use crate::errors::{AvaxError, Result};
use alloc::{
    format,
    string::{String, ToString},
    vec::Vec,
};
use bytes::{Buf, Bytes};
use core::convert::TryFrom;

#[derive(Debug, Clone)]
pub struct BaseTx {
    codec_id: u16,
    pub type_id: TypeId,
    pub tx_header: Header,
    outputs: LengthPrefixedVec<TransferableOutput>,
    inputs: LengthPrefixedVec<TransferableInput>,
    pub memo_len: u32,
    pub memo: Vec<u8>,
    tx_size: usize,
}

impl BaseTx {
    pub fn get_blockchain_id(&self) -> [u8; BLOCKCHAIN_ID_LEN] {
        self.tx_header.get_blockchain_id()
    }

    pub fn get_inputs_len(&self) -> u32 {
        self.inputs.get_len() as u32
    }
    pub fn get_outputs_len(&self) -> u32 {
        self.outputs.get_len() as u32
    }

    pub fn get_inputs_addresses(&self) -> Vec<String> {
        vec!["".to_string()]
    }

    pub fn get_input_by_index(&self, index: usize) -> Option<&TransferableInput> {
        self.inputs.get(index)
    }

    pub fn get_memo(&self) -> Vec<u8> {
        self.memo.clone()
    }

    pub fn parsed_size(&self) -> usize {
        self.tx_size
    }
}

impl AvaxTxInfo for BaseTx {
    fn get_total_output_amount(&self) -> u64 {
        self.outputs
            .iter()
            .fold(0, |acc, item| acc + item.get_amount())
    }

    fn get_total_input_amount(&self) -> u64 {
        self.inputs
            .iter()
            .fold(0, |acc, item| acc + item.get_amount())
    }

    fn get_network(&self) -> Option<String> {
        match self.get_blockchain_id() {
            X_BLOCKCHAIN_ID => Some("Avalanche X-Chain".to_string()),
            C_BLOCKCHAIN_ID => Some("Avalanche C-Chain".to_string()),
            P_BLOCKCHAIN_ID => Some("Avalanche P-Chain".to_string()),
            _ => None,
        }
    }

    fn get_method_info(&self) -> Option<AvaxMethodInfo> {
        match self.type_id {
            TypeId::BaseTx => Some(AvaxMethodInfo::from_string("Send".to_string())),
            _ => Some(AvaxMethodInfo::from_string("Unknown".to_string())),
        }
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
}

impl TryFrom<Bytes> for BaseTx {
    type Error = AvaxError;

    fn try_from(mut bytes: Bytes) -> Result<Self> {
        let initial_len = bytes.len();
        let codec_id = bytes.get_u16();
        let type_id = TypeId::try_from(bytes.get_u32())?;
        let tx_header = Header::try_from(bytes.clone())?;
        bytes.advance(tx_header.parsed_size());

        let outputs = LengthPrefixedVec::<TransferableOutput>::try_from(bytes.clone())?;
        bytes.advance(outputs.parsed_size());

        let inputs = LengthPrefixedVec::<TransferableInput>::try_from(bytes.clone())?;
        bytes.advance(inputs.parsed_size());

        let memo_len = bytes.get_u32();
        let memo = bytes.split_to(memo_len as usize).to_vec();
        let tx_size = initial_len - bytes.len();

        Ok(BaseTx {
            codec_id,
            type_id,
            tx_header,
            outputs,
            inputs,
            memo_len,
            memo,
            tx_size,
        })
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    extern crate std;
    use std::println;

    #[test]
    fn test_avax_base_transaction() {
        {
            // x-chain fuji test case
            let input_bytes = "00000000000000000005ab68eb1ee142a05cfe768c36e11f0b596db5a3c6c77aabe665dad9e638ca94f7000000023d9bdac0ed1d761330cf680efdeb1a42159eb387d6d2950c96f7d28f61bbe2aa00000007000000000bbdfb400000000000000000000000010000000169bc9b5b6cbbbd490abbd79a37ad6cd643be87ab3d9bdac0ed1d761330cf680efdeb1a42159eb387d6d2950c96f7d28f61bbe2aa00000007000000002faf08000000000000000000000000010000000132336f8715dd313a426155cccc15ba27c3033dae0000000163c5b29498bf6a9f1e2a5d20f8eeddaf92096c0ce1c9c2cf6b93fd9a0d12f725000000003d9bdac0ed1d761330cf680efdeb1a42159eb387d6d2950c96f7d28f61bbe2aa00000005000000003b7c4580000000010000000000000000";
            let input_bytes = "00000000000000000005ab68eb1ee142a05cfe768c36e11f0b596db5a3c6c77aabe665dad9e638ca94f7000000023d9bdac0ed1d761330cf680efdeb1a42159eb387d6d2950c96f7d28f61bbe2aa00000007000000000bbdfb400000000000000000000000010000000169bc9b5b6cbbbd490abbd79a37ad6cd643be87ab3d9bdac0ed1d761330cf680efdeb1a42159eb387d6d2950c96f7d28f61bbe2aa00000007000000002faf0800000000000000000000000001000000016498cb45e255f5937b816a59c34a7559a2d437b10000000163c5b29498bf6a9f1e2a5d20f8eeddaf92096c0ce1c9c2cf6b93fd9a0d12f725000000003d9bdac0ed1d761330cf680efdeb1a42159eb387d6d2950c96f7d28f61bbe2aa00000005000000003b7c4580000000010000000000000000";

            // x-chain mainnet test case
            // let input_bytes = "00000000000000000001ed5f38341e436e5d46e2bb00b45d62ae97d1b050c64bc634ae10626739e35c4b0000000221e67317cbc4be2aeb00677ad6462778a8f52274b9d605df2591b23027a87dff000000070000000218711a00000000000000000000000001000000017c949a8013befa47e992078764ff735b18a26b5b21e67317cbc4be2aeb00677ad6462778a8f52274b9d605df2591b23027a87dff0000000700000003cf87a80c00000000000000000000000100000001d5ae9a7d5b31660f08c0aefc1547fb195fbfc85d000000021ddbc2d7d67f14df1e36111bbeef2adae97067c4ceb9db94b73e8883a5a6dd640000000121e67317cbc4be2aeb00677ad6462778a8f52274b9d605df2591b23027a87dff000000050000000395e95a000000000100000000885eea33e82eff5130de90152c0ebb98f5cfdc7c7529596fe2473a35654aac830000000021e67317cbc4be2aeb00677ad6462778a8f52274b9d605df2591b23027a87dff0000000500000002522030ec00000001000000000000000400000000000000020000000900000001a6810c96af6f4e4281031b795f78c37f3395b6d35806179d37b40603d547e2f262969f5363e168c064712607679b01ed13a76daab84addc94a3745b0549a53e5000000000900000001cefe480034588db7b5e0993410b6dbdd2e37e3ec94e75b450dd4c56c32f3b4c61cd9dab507232eb1211a846165336a7d7d975b39612df8d88174e1a92c27535f004a454d1e";
            let mut bytes =
                Bytes::from(hex::decode(input_bytes).expect("Failed to decode hex string"));
            let result = BaseTx::try_from(bytes).unwrap();
            println!("Result: {:?}", result);
            println!("total output amount: {}", result.get_total_output_amount());
            println!("total input amount: {}", result.get_total_input_amount());
            println!("fee amount: {}", result.get_fee_amount());
            // println!("to address: {:?}", result.get_outputs_addresses());

            // assert_eq!(result.get_type_id(), TypeId::BaseTx);
            // assert_eq!(result.get_network_id(), 1);
            assert_eq!(result.get_blockchain_id(), [0; BLOCKCHAIN_ID_LEN]);
            assert_eq!(result.get_inputs_len(), 1);
            assert_eq!(result.get_outputs_len(), 2);
            assert_eq!(result.get_memo(), vec![0u8]);
            assert!(false);
        }

        // x chain base tx
        {
            let input_bytes = "00000000000000000005ab68eb1ee142a05cfe768c36e11f0b596db5a3c6c77aabe665dad9e638ca94f7000000023d9bdac0ed1d761330cf680efdeb1a42159eb387d6d2950c96f7d28f61bbe2aa000000070000000001312d00000000000000000000000001000000018771921301d5bffff592dae86695a615bdb4a4413d9bdac0ed1d761330cf680efdeb1a42159eb387d6d2950c96f7d28f61bbe2aa000000070000000004b571c0000000000000000000000001000000010969ea62e2bb30e66d82e82fe267edf6871ea5f7000000019eae34633c2103aaee5253bb3ca3046c2ab4718a109ffcdb77b51d0427be6bb7000000003d9bdac0ed1d761330cf680efdeb1a42159eb387d6d2950c96f7d28f61bbe2aa000000050000000005f5e100000000010000000000000000";
            let mut bytes =
                Bytes::from(hex::decode(input_bytes).expect("Failed to decode hex string"));
            match BaseTx::try_from(bytes) {
                Ok(result) => {
                    println!("{:?}", result);
                    assert_eq!(
                        result.get_network().unwrap(),
                        "Avalanche X-Chain".to_string()
                    );
                    assert_eq!(
                        result.get_blockchain_id(),
                        [
                            171, 104, 235, 30, 225, 66, 160, 92, 254, 118, 140, 54, 225, 31, 11,
                            89, 109, 181, 163, 198, 199, 122, 171, 230, 101, 218, 217, 230, 56,
                            202, 148, 247
                        ]
                    );
                    assert_eq!(result.get_inputs_len(), 1);
                    assert_eq!(result.get_outputs_len(), 2);
                    assert_eq!(result.get_memo(), Vec::<u8>::new());
                }
                Err(e) => match e {
                    AvaxError::InvalidHex(msg) => {
                        assert_eq!(
                            msg, "Unsupported output type found in input bytes.",
                            "Unexpected error message"
                        );
                    }
                    _ => {}
                },
            }
        }
    }
}
