use crate::transactions::type_id::TypeId;
use crate::constants::*;
use crate::errors::{AvaxError, Result};
use crate::encode::cb58::Cb58Encodable;
use alloc::string::ToString;
use bytes::{Buf, Bytes};
use core::convert::TryFrom;

#[derive(Debug, Clone)]
pub struct OutputOwner {
    type_id: TypeId,
    locktime: u64,
    threshold: u32,
    
}

impl Cb58Encodable for NodeId {
    fn get_prefix(&self) -> &'static str {
        "fuji"
    }

    fn get_data(&self) -> &[u8] {
        &self.node_id
    }
}

impl TryFrom<Bytes> for NodeId {
    type Error = AvaxError;

    fn try_from(mut bytes: Bytes) -> Result<Self> {
        let mut node_id = [0u8; NODE_ID_LEN];
        bytes.copy_to_slice(&mut node_id);
        Ok(NodeId { node_id })
    }
}