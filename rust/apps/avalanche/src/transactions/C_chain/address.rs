use crate::constants::*;
use crate::errors::{AvaxError, Result};
use alloc::string::ToString;
use bytes::{Buf, Bytes};
use core::convert::TryFrom;

#[derive(Debug, Clone)]
pub struct Address([u8; C_CHAIN_ADDRESS_LEN]);

impl TryFrom<Bytes> for Address {
    type Error = AvaxError;

    fn try_from(mut bytes: Bytes) -> Result<Self> {
        Ok(Address(bytes[..C_CHAIN_ADDRESS_LEN].try_into().map_err(
            |_| {
                AvaxError::InvalidHex(format!(
                    "Expected {} bytes, got {} bytes",
                    C_CHAIN_ADDRESS_LEN,
                    bytes.len()
                ))
            },
        )?))
    }
}
