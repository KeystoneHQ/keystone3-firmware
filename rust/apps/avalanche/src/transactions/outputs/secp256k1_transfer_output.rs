use crate::address::Address;
use crate::errors::{AvaxError, Result};
use crate::transactions::structs::{LengthPrefixedVec, ParsedSizeAble};
use crate::transactions::transferable::OutputTrait;
use alloc::string::{String, ToString};
use alloc::vec::Vec;
use bytes::{Buf, BufMut, Bytes, BytesMut};
use core::{convert::TryFrom, fmt, str::FromStr};

#[derive(Debug, Clone)]
pub struct SECP256K1TransferOutput {
    pub type_id: u32,
    pub amount: u64,
    pub locktime: u64,
    pub threshold: u32,
    addresses: LengthPrefixedVec<Address>,
}

impl OutputTrait for SECP256K1TransferOutput {
    fn get_addresses(&self) -> Vec<String> {
        self.addresses.iter().map(|item| item.encode()).collect()
    }

    fn get_addresses_len(&self) -> usize {
        self.addresses.get_len()
    }

    fn get_transfer_output_len(&self) -> usize {
        28 + self.addresses.get_len() * 20
    }

    fn get_amount(&self) -> u64 {
        self.amount
    }
}

impl TryFrom<Bytes> for SECP256K1TransferOutput {
    type Error = AvaxError;

    fn try_from(mut bytes: Bytes) -> Result<Self> {
        if bytes.remaining() < 28 {
            return Err(AvaxError::InvalidHex("Insufficient data".to_string()));
        }

        let type_id = bytes.get_u32();
        let amount = bytes.get_u64();
        let locktime = bytes.get_u64();
        let threshold = bytes.get_u32();

        let addresses = LengthPrefixedVec::<Address>::try_from(bytes.clone())?;
        bytes.advance(addresses.parsed_size());

        Ok(SECP256K1TransferOutput {
            type_id,
            amount,
            locktime,
            threshold,
            addresses,
        })
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    extern crate std;
    use std::println;

    #[test]
    fn test_secp256k1_transfer_output() {
        let input_bytes = "000000070000000005f5e100000000000000000000000001000000018771921301d5bffff592dae86695a615bdb4a441";
        let result = SECP256K1TransferOutput::try_from(Bytes::from(
            hex::decode(input_bytes).expect("Failed to decode hex string"),
        ));
        println!("Result: {:?}", result.unwrap());
        assert!(false);
    }
}
