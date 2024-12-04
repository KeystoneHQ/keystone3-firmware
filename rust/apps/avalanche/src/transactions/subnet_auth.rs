use crate::constants::*;
use crate::errors::{AvaxError, Result};
use alloc::string::ToString;
use bytes::{Buf, Bytes};
use core::convert::TryFrom;

#[derive(Debug, Clone)]
pub struct SubnetAuth {
    subnet_auth: [u8; SUBNET_AUTH_LEN],
}

impl TryFrom<Bytes> for SubnetAuth {
    type Error = AvaxError;

    fn try_from(mut bytes: Bytes) -> Result<Self> {
        let mut subnet_auth = [0u8; SUBNET_AUTH_LEN];
        bytes.copy_to_slice(&mut subnet_auth);
        Ok(SubnetAuth { subnet_auth })
    }
}
