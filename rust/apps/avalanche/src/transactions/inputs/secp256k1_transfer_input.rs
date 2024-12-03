use crate::errors::{AvaxError, Result};
use crate::transactions::transferable::InputTrait;
use alloc::string::ToString;
use alloc::vec::Vec;
use bytes::{Buf, BufMut, Bytes, BytesMut};
use core::{convert::TryFrom, fmt, str::FromStr};

extern crate std;
use std::println;

#[derive(Debug, Clone)]
pub struct SECP256K1TransferInput {
    pub type_id: u32,
    pub amount: u64,
    pub addresses_len: u32,
    pub address_indices: Vec<u32>,
}

impl InputTrait for SECP256K1TransferInput {
    fn display(&self) {
        #[cfg(feature = "std")]
        {
            extern crate std;
            use std::println;
            println!("SECP256K1TransferInput:");
            println!("  Type ID: {}", self.type_id);
            println!("  Amount: {}", self.amount);
            println!("  Addresses Length: {}", self.addresses_len);
            println!("  Addresses: ");
        }
    }
}

impl TryFrom<Bytes> for SECP256K1TransferInput {
    type Error = AvaxError;

    fn try_from(mut bytes: Bytes) -> Result<Self> {
        if bytes.remaining() < 20 {
            return Err(AvaxError::InvalidHex("Insufficient data".to_string()));
        }

        let type_id = bytes.get_u32();
        let amount = bytes.get_u64();
        let addresses_len = bytes.get_u32();
        let mut addresses_indices = Vec::with_capacity(addresses_len as usize);
        for _ in 0..addresses_len {
            addresses_indices.push(bytes.get_u32());
        }

        Ok(SECP256K1TransferInput {
            type_id,
            amount,
            addresses_len,
            address_indices: addresses_indices,
        })
    }
}

mod tests {
    use super::*;
    extern crate std;
    use std::println;

    #[test]
    fn test_secp256k1_transfer_input_try_from() {
        let input_bytes = "00000005000000001dbd670d0000000100000000";
        let binary_data = hex::decode(input_bytes).expect("Failed to decode hex string");
        let mut bytes = Bytes::from(binary_data);
        let result = SECP256K1TransferInput::try_from(bytes.clone());
        println!("Result: {:?}", result.unwrap());
        assert!(false);
    }
}
