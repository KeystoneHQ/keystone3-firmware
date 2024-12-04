use crate::constants::*;
use crate::errors::{AvaxError, Result};
use alloc::string::ToString;
use bytes::{Buf, Bytes};
use core::convert::TryFrom;

#[derive(Debug, Clone)]
pub struct SubnetId {
    subnet_id: [u8; SUBNET_ID_LEN],
}

impl TryFrom<Bytes> for SubnetId {
    type Error = AvaxError;

    fn try_from(mut bytes: Bytes) -> Result<Self> {
        let mut subnet_id = [0u8; SUBNET_ID_LEN];
        bytes.copy_to_slice(&mut subnet_id);
        Ok(SubnetId { subnet_id })
    }
}


