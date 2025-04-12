use alloc::format;
use alloc::string::{String, ToString};

use ethereum_types::{U256,H160};

use rlp::{Decodable, DecoderError, Rlp};

pub struct EIP7702Authorization {
    pub chain_id: u64,
    pub delegate: H160,
    pub nonce: U256
}

impl EIP7702Authorization {
    pub fn decode_raw(bytes: &[u8]) -> Result<EIP7702Authorization, DecoderError> {
        rlp::decode(bytes)
    }
}
impl Decodable for EIP7702Authorization {
    fn decode(rlp: &Rlp) -> Result<Self, DecoderError> {
        Ok(Self {
            chain_id: rlp.val_at(0)?,
            delegate: rlp.val_at(1)?,
            nonce: rlp.val_at(2)?
        })
    }
}
