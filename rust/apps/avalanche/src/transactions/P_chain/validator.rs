use super::node_id::{self, NodeId};
use crate::constants::*;
use crate::encode::cb58::Cb58Encodable;
use crate::errors::{AvaxError, Result};
use crate::transactions::base_tx::BaseTx;
use alloc::{
    format,
    string::{String, ToString},
    vec::Vec,
};
use bytes::{Buf, Bytes};
use core::convert::TryFrom;

#[derive(Debug, Clone)]
pub struct Validator {
    pub node_id: NodeId,
    pub start_time: i64,
    pub endtime: i64,
    pub weight: u64,
}

impl Validator {
    pub fn parsed_size(&self) -> usize {
        20 + 8 + 8 + 8
    }
}

impl TryFrom<Bytes> for Validator {
    type Error = AvaxError;

    fn try_from(mut bytes: Bytes) -> Result<Self> {
        let node_id = NodeId::try_from(bytes.clone())?;
        bytes.advance(NODE_ID_LEN);
        Ok(Validator {
            node_id,
            start_time: bytes.get_i64(),
            endtime: bytes.get_i64(),
            weight: bytes.get_u64(),
        })
    }
}

mod tests {
    use super::*;
    extern crate std;
    use std::println;

    #[test]
    fn test_validator_parse() {
        let input_bytes = "000000023d9bdac0ed1d761330cf680efdeb1a42159eb387d6d2950c96f7d28f61bbe2aa000000070000000005f5e100000000000000000000000001000000018771921301d5bffff592dae86695a615bdb4a4413d9bdac0ed1d761330cf680efdeb1a42159eb387d6d2950c96f7d28f61bbe2aa000000070000000017c771d2000000000000000000000001000000010969ea62e2bb30e66d82e82fe267edf6871ea5f7";
        let binary_data = hex::decode(input_bytes).expect("Failed to decode hex string");
        let mut bytes = Bytes::from(binary_data);
        let result = Validator::try_from(bytes.clone()).unwrap();
        assert_eq!(
            "Node-1118zVrK8tN1ic5wUb5dcsECsf2BtfQ",
            result.node_id.to_cb58()
        );
    }
}
