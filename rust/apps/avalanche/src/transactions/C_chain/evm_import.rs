use super::address::Address;
use crate::constants::*;
use crate::errors::{AvaxError, Result};
use crate::transactions::structs::{LengthPrefixedVec, ParsedSizeAble};
use crate::transactions::transferable::TransferableInput;
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
pub struct EvmOutput {
    address: Address,
    amount: u64,
    asset_id: AssetId,
}

impl TryFrom<Bytes> for EvmOutput {
    type Error = AvaxError;

    fn try_from(mut bytes: Bytes) -> Result<Self> {
        let address = Address::try_from(bytes.split_to(C_CHAIN_ADDRESS_LEN))?;
        let amount = bytes.get_u64();
        let asset_id = AssetId::try_from(bytes.split_to(ASSET_ID_LEN))?;
        Ok(EvmOutput {
            address,
            amount,
            asset_id,
        })
    }
}

impl ParsedSizeAble for EvmOutput {
    fn parsed_size(&self) -> usize {
        C_CHAIN_ADDRESS_LEN + 8 + ASSET_ID_LEN + 8
    }
}

#[derive(Debug, Clone)]
pub struct ExportTx {
    pub codec_id: u16,
    pub type_id: TypeId,
    pub tx_header: Header,
    pub source_chain: [u8; BLOCKCHAIN_ID_LEN],
    inputs: LengthPrefixedVec<TransferableInput>,
    outputs: LengthPrefixedVec<EvmOutput>,
}

impl TryFrom<Bytes> for ExportTx {
    type Error = AvaxError;

    fn try_from(mut bytes: Bytes) -> Result<Self> {
        let codec_id = bytes.get_u16();
        let type_id = TypeId::try_from(bytes.get_u32())?;
        let tx_header = Header::try_from(bytes.clone())?;
        bytes.advance(tx_header.parsed_size());

        let mut source_chain = [0u8; 32];
        bytes.copy_to_slice(&mut source_chain);

        let inputs = LengthPrefixedVec::<TransferableInput>::try_from(bytes.clone())?;
        bytes.advance(inputs.parsed_size());

        let outputs = LengthPrefixedVec::<EvmOutput>::try_from(bytes.clone())?;
        bytes.advance(outputs.parsed_size());

        Ok(ExportTx {
            codec_id,
            type_id,
            tx_header,
            source_chain,
            inputs,
            outputs,
        })
    }
}

mod tests {
    use crate::transactions::transferable::OutputTrait;

    use super::*;
    extern crate std;
    use std::println;

    #[test]
    fn test_avax_c_import() {
        let input_bytes = "000000000000000000057fc93d85c6d62c5b2ac0b519c87010ea5294012d1e407030d6acd0021cac10d5ab68eb1ee142a05cfe768c36e11f0b596db5a3c6c77aabe665dad9e638ca94f7000000013f407c793a9a31b5f281623e479bcaee760be3da54969b85f44ffc51a67c1c64000000013d9bdac0ed1d761330cf680efdeb1a42159eb387d6d2950c96f7d28f61bbe2aa000000050000000005f5e1000000000100000000000000013fe51338992d913b6a999693d7c345646ca3bb4c0000000005f5b5223d9bdac0ed1d761330cf680efdeb1a42159eb387d6d2950c96f7d28f61bbe2aa0000000100000009000000017e79e5f2fba29e48d9d329f23e8e27bf20d79714555bb04a175e57136b95b282528654583b1fd568e7f8322962fb31e4174896a50114511d124bf74a92559c1f0048c457c5";
        let binary_data = hex::decode(input_bytes).expect("Failed to decode hex string");
        let mut bytes = Bytes::from(binary_data);
        let result = ExportTx::try_from(bytes.clone()).unwrap();
        println!("{:?}", result);
        assert!(false);
    }
}
