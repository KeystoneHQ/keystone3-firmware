use crate::errors::{AvaxError, Result};
use alloc::{
    format,
    string::{String, ToString},
    vec::Vec,
};
use bytes::{Buf, Bytes};
use core::convert::TryFrom;
#[derive(Debug, Clone)]
pub struct LengthPrefixedVec<T: ParsedSizeAble> {
    pub len: u32,
    pub items: Vec<T>,
}
pub trait ParsedSizeAble {
    fn parsed_size(&self) -> usize;
}

impl<T: ParsedSizeAble> LengthPrefixedVec<T> {
    pub fn get_len(&self) -> u32 {
        self.len
    }

    pub fn get(&self, index: usize) -> Option<&T> {
        self.items.get(index)
    }

    pub fn parsed_size(&self) -> usize {
        4 + self
            .items
            .iter()
            .fold(0, |acc, item| acc + item.parsed_size())
    }
}

impl<T> TryFrom<Bytes> for LengthPrefixedVec<T>
where
    T: TryFrom<Bytes, Error = AvaxError> + ParsedSizeAble,
{
    type Error = AvaxError;

    fn try_from(mut bytes: Bytes) -> Result<Self> {
        if bytes.len() < 4 {
            return Err(AvaxError::InvalidHex(
                "Insufficient data for LengthPrefixedVec".to_string(),
            ));
        }

        let len = bytes.get_u32();

        let mut items = Vec::with_capacity(len as usize);

        for _ in 0..len {
            let item = T::try_from(bytes.clone())?;
            bytes.advance(item.parsed_size());
            items.push(item);
        }

        Ok(LengthPrefixedVec { len, items })
    }
}
