use crate::transactions::tx_header::Header;
use crate::transactions::type_id::TypeId;
use crate::errors::{AvaxError, Result};
use crate::constants::*;
use alloc::{
    format,
    string::{String, ToString},
    vec::Vec,
};
use bytes::{Buf, Bytes};
use core::convert::TryFrom;

#[derive(Debug, Clone)]
pub struct EvmInput {
    pub input_len: u32,
    pub input: Bytes,
}

impl TryFrom<Bytes> for EvmInput {
    type Error = AvaxError;

    fn try_from(mut bytes: Bytes) -> Result<Self> {
        Ok(EvmInput {
            input_len: bytes.get_u32(),
            input: bytes.split_to(25),
        })
    }
}

#[derive(Debug, Clone)]
pub struct ExportTx {
    pub codec_id: u16,
    pub type_id: TypeId,
    pub tx_header: Header,
    pub dest_chain: [u8; BLOCKCHAIN_ID_LEN],
    pub inputs: EvmInput,
}

impl TryFrom<Bytes> for ExportTx {
    type Error = AvaxError;

    fn try_from(mut bytes: Bytes) -> Result<Self> {
        let codec_id = bytes.get_u16();
        let type_id = TypeId::try_from(bytes.get_u32())?;
        let tx_header = Header::try_from(bytes.clone())?;

        let mut dest_chain = [0u8; 32];
        bytes.copy_to_slice(&mut dest_chain);

        Ok(ExportTx {
            codec_id,
            type_id,
            tx_header,
            dest_chain,
            inputs: EvmInput::try_from(bytes)?,
        })
    }
}

mod tests {
    use super::*;
    extern crate std;
    use std::println;

    #[test]
    fn test_avax_c_export() {
        let input_bytes = "000000000001000000057fc93d85c6d62c5b2ac0b519c87010ea5294012d1e407030d6acd0021cac10d5ab68eb1ee142a05cfe768c36e11f0b596db5a3c6c77aabe665dad9e638ca94f7000000019858effd232b4033e47d90003d41ec34ecaeda94000000003b9af5de3d9bdac0ed1d761330cf680efdeb1a42159eb387d6d2950c96f7d28f61bbe2aa0000000000000010000000013d9bdac0ed1d761330cf680efdeb1a42159eb387d6d2950c96f7d28f61bbe2aa00000007000000003b9aca00000000000000000000000001000000010969ea62e2bb30e66d82e82fe267edf6871ea5f70000000100000009000000017c9bebce7c5d3f234de2991ccc1a036816312cdb58a3544962e576aabb9a48095d8c83a662d0b5f7208c8a4d0b9ac69bdffc2f42344cb6b03d1e822644b925b600ed8119af";
        let binary_data = hex::decode(input_bytes).expect("Failed to decode hex string");
        let mut bytes = Bytes::from(binary_data);
        let output_len = bytes.get_u32();
        for _ in 0..output_len {
            let result = ExportTx::try_from(bytes.clone()).unwrap();
            println!("{:?}", result);
        }
        assert!(false);
    }
}
