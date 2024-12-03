use crate::errors::AvaxError;
use alloc::string::ToString;
use bytes::Bytes;
use core::convert::TryFrom;

pub const ASSET_ID_LEN: usize = 32;

#[derive(Clone, Debug)]
pub struct AssetId(Bytes);

impl TryFrom<Bytes> for AssetId {
    type Error = AvaxError;

    fn try_from(bytes: Bytes) -> Result<Self, Self::Error> {
        if bytes.len() != ASSET_ID_LEN {
            return Err(AvaxError::InvalidHex(
                "Invalid length for AssetId".to_string(),
            ));
        }
        Ok(AssetId(bytes))
    }
}
