use crate::errors::{AvaxError, Result};
use crate::constants::*;
use bytes::{Buf, Bytes};
use core::convert::TryFrom;

pub type BlockChainId = [u8; BLOCKCHAIN_ID_LEN];

#[derive(Debug, Clone)]
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

    pub fn parsed_size(&self) -> usize {
        4 + BLOCKCHAIN_ID_LEN
    }
}

impl TryFrom<Bytes> for Header {
    type Error = AvaxError;

    fn try_from(mut bytes: Bytes) -> Result<Self> {
        Ok(Header {
            network_id: bytes.get_u32(),
            blockchain_id: bytes[..32]
                .try_into()
                .map_err(|_| AvaxError::InvalidHex(format!("error data to blockchain_id")))?,
        })
    }
}
