use crate::errors::{AvaxError, Result};
use crate::transactions::transferable::InputTrait;
use alloc::string::ToString;
use alloc::vec::Vec;
use bytes::{Buf, Bytes};
use core::convert::TryFrom;

#[derive(Debug, Clone)]
pub struct SECP256K1TransferInput {
    pub type_id: u32,
    pub amount: u64,
    pub addresses_len: u32,
    pub address_indices: Vec<u32>,
}

impl InputTrait for SECP256K1TransferInput {
    fn get_transfer_input_len(&self) -> usize {
        16 + self.addresses_len as usize * 4
    }

    fn get_amount(&self) -> u64 {
        self.amount
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

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_secp256k1_transfer_input_try_from() {
        let input_bytes = "00000005000000001dbd670d0000000100000000";
        let binary_data = hex::decode(input_bytes).expect("Failed to decode hex string");
        let mut bytes = Bytes::from(binary_data);
        let result = SECP256K1TransferInput::try_from(bytes.clone()).unwrap();
        assert_eq!(result.type_id, 5);
        assert_eq!(result.amount, 498951949);
        assert_eq!(result.addresses_len, 1);
    }
}
