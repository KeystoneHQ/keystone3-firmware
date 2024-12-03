use crate::errors::{AvaxError, Result};
use crate::transactions::transferable::OutputTrait;
use alloc::string::ToString;
use alloc::vec::Vec;
use bytes::{Buf, BufMut, Bytes, BytesMut};
use core::{convert::TryFrom, fmt, str::FromStr};

extern crate std;
use std::println;

#[derive(Debug, Clone)]
pub struct SECP256K1TransferOutput {
    pub type_id: u32,
    pub amount: u64,
    pub locktime: u64,
    pub threshold: u32,
    pub addresses_len: u32,
    pub addresses: Vec<[u8; 20]>,
}

impl OutputTrait for SECP256K1TransferOutput {
    fn display(&self) {
        #[cfg(feature = "std")]
        {
            extern crate std;
            use std::println;
            println!("SECP256K1TransferOutput:");
            println!("  Type ID: {}", self.type_id);
            println!("  Amount: {}", self.amount);
            println!("  Locktime: {}", self.locktime);
            println!("  Threshold: {}", self.threshold);
            println!("  Addresses Length: {}", self.addresses_len);
            println!("  Addresses: ");
            for (index, address) in self.addresses.iter().enumerate() {
                println!("    Address {}: {:?}", index, address);
            }
        }
    }

    fn get_addresses(&self) -> Vec<[u8; 20]> {
        self.addresses.clone()
    }

    fn get_addresses_len(&self) -> u32 {
        self.addresses_len
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
        let addresses_len = bytes.get_u32();

        let mut addresses = Vec::with_capacity(addresses_len as usize);
        for _ in 0..addresses_len {
            if bytes.remaining() < 20 {
                return Err(AvaxError::InvalidOutput);
            }
            let mut address = [0u8; 20];
            bytes.copy_to_slice(&mut address);
            addresses.push(address);
        }

        Ok(SECP256K1TransferOutput {
            type_id,
            amount,
            locktime,
            threshold,
            addresses_len,
            addresses,
        })
    }
}

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
