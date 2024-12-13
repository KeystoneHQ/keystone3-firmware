use super::address::Address;
use crate::constants::*;
use crate::errors::{AvaxError, Result};
use crate::transactions::structs::{LengthPrefixedVec, ParsedSizeAble};
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

#[derive(Debug, Clone)]
pub struct ExportTx {
    pub codec_id: u16,
    pub type_id: TypeId,
    pub tx_header: Header,
    pub dest_chain: [u8; BLOCKCHAIN_ID_LEN],
    pub inputs: LengthPrefixedVec<EvmInput>,
    outputs: LengthPrefixedVec<TransferableOutput>,
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
        let input_bytes = "000000000001000000057fc93d85c6d62c5b2ac0b519c87010ea5294012d1e407030d6acd0021cac10d5ab68eb1ee142a05cfe768c36e11f0b596db5a3c6c77aabe665dad9e638ca94f700000001dba1d159ccdf86fb35e55c0b29a5a41428153ae70000000005f60cde3d9bdac0ed1d761330cf680efdeb1a42159eb387d6d2950c96f7d28f61bbe2aa0000000000000003000000013d9bdac0ed1d761330cf680efdeb1a42159eb387d6d2950c96f7d28f61bbe2aa000000070000000005f5e100000000000000000000000001000000014effb776db45b22c20b4fd8bed9c315a305ecf86000000010000000900000001974e78137a82401d4acec39c6400a20ecbed4d1f91451678d73aca115bf18a316b0045b6845ba09a6ef0d7743d300c52126cdc3a3bc105efb5bf6444f10002c700771b2ace";
        let binary_data = hex::decode(input_bytes).expect("Failed to decode hex string");
        let mut bytes = Bytes::from(binary_data);
        let result = ExportTx::try_from(bytes.clone()).unwrap();
        println!("{:?}", result);
        println!(
            "asset id = {:?}",
            result.outputs.get(0).unwrap()
        );
        println!("{}", hex::encode([78, 255, 183, 118, 219, 69, 178, 44, 32, 180, 253, 139, 237, 156, 49, 90, 48, 94, 207, 134]));
        assert!(false);
    }
}
