use super::transferable::{TransferableInput, TransferableOutput};
use super::tx_header::{Header, BLOCKCHAIN_ID_LEN};
use super::type_id::TypeId;
use super::structs::LengthPrefixedVec;
use crate::errors::{AvaxError, Result};
use alloc::{
    format,
    string::{String, ToString},
    vec::Vec,
};
use bytes::{Buf, Bytes};
use core::convert::TryFrom;

extern crate std;
use std::{println, vec};

#[derive(Debug)]
pub struct BaseTx {
    codec_id: u16,
    pub type_id: TypeId,
    pub tx_header: Header,
    pub output_len: u32,
    pub outputs: Vec<TransferableOutput>,
    pub input_len: u32,
    pub inputs: Vec<TransferableInput>,
    pub memo_len: u32,
    pub memo: Vec<u8>,
    tx_size: usize,
}

impl BaseTx {
    pub fn get_type_id(&self) -> TypeId {
        self.type_id
    }

    pub fn get_network_id(&self) -> u32 {
        self.tx_header.get_network_id()
    }

    pub fn get_blockchain_id(&self) -> [u8; BLOCKCHAIN_ID_LEN] {
        self.tx_header.get_blockchain_id()
    }

    pub fn get_inputs_len(&self) -> u32 {
        self.inputs.len() as u32
    }
    pub fn get_outputs_len(&self) -> u32 {
        self.outputs.len() as u32
    }

    pub fn get_inputs(&self) -> Vec<TransferableInput> {
        self.inputs.clone()
    }

    pub fn get_input_by_index(&self, index: usize) -> Option<&TransferableInput> {
        self.inputs.get(index)
    }

    pub fn get_outputs(&self) -> Vec<TransferableOutput> {
        self.outputs.clone()
    }

    pub fn get_memo(&self) -> Vec<u8> {
        self.memo.clone()
    }

    pub fn parsed_size(&self) -> usize {
        self.tx_size
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
        let output_len = bytes.get_u32();
        let mut output_bytes = bytes.clone();
        let outputs = (0..output_len)
            .map(|_| {
                let output = TransferableOutput::try_from(output_bytes.clone())?;
                output_bytes.advance(output.parsed_size());
                bytes.advance(output.parsed_size());
                Ok(output)
            })
            .collect::<Result<Vec<TransferableOutput>>>()?;

        let input_len = bytes.get_u32();
        let mut input_bytes = bytes.clone();
        let inputs: Vec<TransferableInput> = (0..input_len)
            .map(|_| {
                let output = TransferableInput::try_from(input_bytes.clone())?;
                input_bytes.advance(output.parsed_size());
                bytes.advance(output.parsed_size());
                Ok(output)
            })
            .collect::<Result<Vec<TransferableInput>>>()?;

        let memo_len = bytes.get_u32();
        let memo = bytes.split_to(memo_len as usize).to_vec();
        let tx_size = initial_len - bytes.len();

        Ok(BaseTx {
            codec_id,
            type_id,
            tx_header,
            output_len,
            outputs,
            input_len,
            inputs,
            memo_len,
            memo,
            tx_size,
        })
    }
}

mod tests {
    use super::*;
    extern crate std;
    use std::println;

    #[test]
    fn test_avax_base_transaction() {
        {
            let input_bytes = "000000000022000000050000000000000000000000000000000000000000000000000000000000000000000000023d9bdac0ed1d761330cf680efdeb1a42159eb387d6d2950c96f7d28f61bbe2aa000000070000000005f5e100000000000000000000000001000000018771921301d5bffff592dae86695a615bdb4a4413d9bdac0ed1d761330cf680efdeb1a42159eb387d6d2950c96f7d28f61bbe2aa000000070000000017c771d2000000000000000000000001000000010969ea62e2bb30e66d82e82fe267edf6871ea5f70000000157d5e23e2e1f460b618bba1b55913ff3ceb315f0d1acc41fe6408edc4de9facd000000003d9bdac0ed1d761330cf680efdeb1a42159eb387d6d2950c96f7d28f61bbe2aa00000005000000001dbd670d000000010000000000000000";
            let mut bytes =
                Bytes::from(hex::decode(input_bytes).expect("Failed to decode hex string"));
            match BaseTx::try_from(bytes) {
                Ok(result) => {
                    assert_eq!(result.get_type_id(), TypeId::BaseTx);
                    assert_eq!(result.get_network_id(), 5);
                    assert_eq!(result.get_blockchain_id(), [0; BLOCKCHAIN_ID_LEN]);
                    assert_eq!(result.get_inputs_len(), 1);
                    assert_eq!(result.get_outputs_len(), 2);
                    assert_eq!(result.get_memo(), vec![0u8]);
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

        // x chain base tx
        {
            let input_bytes = "00000000000000000005ab68eb1ee142a05cfe768c36e11f0b596db5a3c6c77aabe665dad9e638ca94f7000000023d9bdac0ed1d761330cf680efdeb1a42159eb387d6d2950c96f7d28f61bbe2aa000000070000000001312d00000000000000000000000001000000018771921301d5bffff592dae86695a615bdb4a4413d9bdac0ed1d761330cf680efdeb1a42159eb387d6d2950c96f7d28f61bbe2aa000000070000000004b571c0000000000000000000000001000000010969ea62e2bb30e66d82e82fe267edf6871ea5f7000000019eae34633c2103aaee5253bb3ca3046c2ab4718a109ffcdb77b51d0427be6bb7000000003d9bdac0ed1d761330cf680efdeb1a42159eb387d6d2950c96f7d28f61bbe2aa000000050000000005f5e100000000010000000000000000";
            let mut bytes =
                Bytes::from(hex::decode(input_bytes).expect("Failed to decode hex string"));
            match BaseTx::try_from(bytes) {
                Ok(result) => {
                    println!("{:?}", result);
                    assert_eq!(result.get_type_id(), TypeId::BaseTx);
                    assert_eq!(result.get_network_id(), 5);
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
