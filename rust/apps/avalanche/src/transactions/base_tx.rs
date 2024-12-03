use super::transferable::{TransferableInput, TransferableOutput};
use super::type_id::TypeId;
use crate::errors::{AvaxError, Result};
use crate::transactions::outputs;
use alloc::{
    format,
    string::{String, ToString},
    vec::Vec,
};
use bytes::{Buf, Bytes};
use core::convert::TryFrom;

pub const BLOCKCHAIN_ID_LEN: usize = 32;
extern crate std;
use std::{println, vec};

#[derive(Debug)]

pub struct BaseTx {
    codec_id: u16,
    pub type_id: TypeId,
    pub network_id: u32,
    pub blockchain_id: [u8; BLOCKCHAIN_ID_LEN],
    pub output_len: u32,
    pub outputs: Vec<TransferableOutput>,
    pub input_len: u32,
    pub inputs: Vec<TransferableInput>,
    pub memo: String,
}

impl BaseTx {
    pub fn get_type_id(&self) -> TypeId {
        self.type_id
    }

    pub fn get_network_id(&self) -> u32 {
        self.network_id
    }

    pub fn get_blockchain_id(&self) -> [u8; BLOCKCHAIN_ID_LEN] {
        self.blockchain_id
    }

    pub fn get_inputs_len(&self) -> u32 {
        self.inputs.len() as u32
    }
    pub fn get_outputs_len(&self) -> u32 {
        self.outputs.len() as u32
    }

    // pub fn get_inputs(&self) -> Vec<TransferableInput> {
    //     self.inputs.clone()
    // }

    // pub fn get_outputs(&self) -> Vec<TransferableOutput> {
    //     self.outputs.clone()
    // }

    pub fn get_memo(&self) -> String {
        self.memo.clone()
    }
}

impl TryFrom<Bytes> for BaseTx {
    type Error = AvaxError;

    fn try_from(mut bytes: Bytes) -> Result<Self> {
        let codec_id = bytes.get_u16();
        println!("codec_id: {}", codec_id);

        let type_id = TypeId::try_from(bytes.get_u32())?;
        println!("type_id: {:?}", type_id);

        let network_id = bytes.get_u32();
        println!("network_id: {}", network_id);

        let blockchain_id = bytes.split_to(BLOCKCHAIN_ID_LEN)[..]
            .try_into()
            .map_err(|_| AvaxError::InvalidHex(format!("error data to tx_id")))?;

        let output_len = bytes.get_u32();
        println!("output_len: {}", output_len);

        // let outputs = (0..output_len)
        //     .map(|_| TransferableOutput::try_from(bytes.clone()))
        //     .collect::<Result<Vec<TransferableOutput>>>()?;
        
        let mut outputs = vec![];
        for _ in 0..output_len {
            let output = TransferableOutput::try_from(bytes.clone())?;
            outputs.push(output);
        }
        println!("outputs: {:?}", outputs);

        let input_len = bytes.get_u32();
        println!("input_len: {}", input_len);

        let inputs = (0..input_len)
            .map(|_| TransferableInput::try_from(bytes.clone()))
            .collect::<Result<Vec<TransferableInput>>>()?;

        let memo = String::from_utf8(bytes.to_vec())
            .map_err(|_| AvaxError::InvalidHex(format!("error data to memo")))?;
        Ok(BaseTx {
            codec_id,
            type_id,
            network_id,
            blockchain_id,
            output_len,
            outputs,
            input_len,
            inputs,
            memo,
        })
        // Ok(BaseTx {
        //     codec_id: bytes.get_u16(),
        //     type_id: TypeId::try_from(bytes.get_u32())?,
        //     network_id: bytes.get_u32(),
        //     blockchain_id: bytes.split_to(BLOCKCHAIN_ID_LEN)[..]
        //         .try_into()
        //         .map_err(|_| AvaxError::InvalidHex(format!("error data to tx_id")))?,
        //     input_len: bytes.get_u32(),
        //     inputs: (0..bytes.get_u32())
        //         .map(|_| TransferableInput::try_from(bytes.clone()))
        //         .collect::<Result<Vec<TransferableInput>>>()?,
        //     output_len: bytes.get_u32(),
        //     outputs: (0..bytes.get_u32())
        //         .map(|_| TransferableOutput::try_from(bytes.clone()))
        //         .collect::<Result<Vec<TransferableOutput>>>()?,
        //     memo: String::from_utf8(bytes.to_vec())
        //         .map_err(|_| AvaxError::InvalidHex(format!("error data to memo")))?,
        // })
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
            let result = BaseTx::try_from(bytes);
            println!("Result: {:?}", result.unwrap());
            // match result {
            //     Ok(parse_result) => {
            //         println!("parse_result = {:?}", parse_result);
            //     }
            //     Err(e) => match e {
            //         AvaxError::InvalidHex(msg) => {
            //             assert_eq!(
            //                 msg, "Unsupported output type found in input bytes.",
            //                 "Unexpected error message"
            //             );
            //         }
            //         _ => {}
            //     },
            // }
            assert!(false);
        }
    }
}
