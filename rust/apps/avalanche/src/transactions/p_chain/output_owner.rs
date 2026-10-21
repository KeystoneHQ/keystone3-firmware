use crate::address::Address;
use crate::errors::{AvaxError, Result};
use crate::transactions::{
    structs::{LengthPrefixedVec, ParsedSizeAble},
    type_id::TypeId,
};
use bytes::{Buf, Bytes};
use core::convert::TryFrom;

#[derive(Debug, Clone)]
pub struct OutputOwner {
    #[allow(
        dead_code,
        reason = "Retained as part of the parsed wire format and debug representation"
    )]
    type_id: TypeId,
    #[allow(
        dead_code,
        reason = "Retained as part of the parsed wire format and debug representation"
    )]
    locktime: u64,
    #[allow(
        dead_code,
        reason = "Retained as part of the parsed wire format and debug representation"
    )]
    threshold: u32,
    pub addresses: LengthPrefixedVec<Address>,
}

impl ParsedSizeAble for OutputOwner {
    fn parsed_size(&self) -> usize {
        4 + 8 + 4 + self.addresses.parsed_size()
    }
}

impl TryFrom<Bytes> for OutputOwner {
    type Error = AvaxError;

    fn try_from(mut bytes: Bytes) -> Result<Self> {
        let type_id = TypeId::try_from(bytes.get_u32())?;
        let locktime = bytes.get_u64();
        let threshold = bytes.get_u32();
        let addresses = LengthPrefixedVec::<Address>::try_from(bytes.clone())?;
        bytes.advance(addresses.parsed_size());
        Ok(OutputOwner {
            type_id,
            locktime,
            threshold,
            addresses,
        })
    }
}
