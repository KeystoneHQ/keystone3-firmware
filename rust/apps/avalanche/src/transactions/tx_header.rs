use crate::errors::{AvaxError, Result};
use alloc::string::ToString;
use bytes::{Buf, Bytes};
use core::convert::TryFrom;

pub const BLOCKCHAIN_ID_LEN: usize = 32;
#[derive(Clone, Debug)]
pub struct BlockChainId(Bytes);

#[derive(Clone)]
pub struct Header {
    pub network_id: u32,
    pub blockchain_id: BlockChainId,
}

impl Header {
    pub fn get_network_id(&self) -> u32 {
        self.network_id
    }

    pub fn get_blockchain_id(&self) -> BlockChainId {
        self.blockchain_id.clone()
    }
}

impl TryFrom<Bytes> for Header {
    type Error = AvaxError;

    fn try_from(mut bytes: Bytes) -> Result<Self> {
        if bytes.len() != BLOCKCHAIN_ID_LEN + 4 {
            return Err(AvaxError::InvalidHex(
                "Invalid length for AssetId".to_string(),
            ));
        }

        Ok(Header {
            network_id: bytes.get_u32(),
            blockchain_id: BlockChainId(bytes),
        })
    }
}
