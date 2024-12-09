use crate::errors::AvaxError;
use alloc::string::ToString;
use bytes::Bytes;
use core::convert::TryFrom;

#[derive(Clone, Debug)]
pub struct AssetId(Bytes);

impl TryFrom<Bytes> for AssetId {
    type Error = AvaxError;

    fn try_from(bytes: Bytes) -> Result<Self, Self::Error> {
        Ok(AssetId(bytes))
    }
}
