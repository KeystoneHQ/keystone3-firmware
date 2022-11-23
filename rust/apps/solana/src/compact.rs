use crate::errors::{Result, SolanaError};
use crate::read::Read;
use alloc::vec::Vec;
use alloc::{format, vec};

pub struct Compact<T> {
    compact_length: u32,
    pub(crate) data: Vec<T>,
}

impl<T: Read<T>> Compact<T> {
    fn new(raw: &mut Vec<u8>) -> Result<Compact<T>> {
        let length: u32 = Compact::<T>::read_length(raw)?;
        let mut compact = Compact {
            compact_length: length,
            data: vec![],
        };
        for _ in 0..compact.compact_length {
            compact.data.push(T::read(raw)?);
        }
        Ok(compact)
    }

    fn read_length(raw: &mut Vec<u8>) -> Result<u32> {
        let mut len: u32 = 0;
        let mut size: u32 = 0;
        loop {
            if raw.len() < 1 {
                return Err(SolanaError::InvalidData(format!("compact length")));
            }
            let element: u32 = raw.remove(0) as u32;
            len |= (element & 0x7f) << (size * 7);
            size += 1;
            if (element & 0x80) == 0 {
                break;
            }
        }
        Ok(len)
    }
}

impl<T: Read<T>> Read<Compact<T>> for Compact<T> {
    fn read(raw: &mut Vec<u8>) -> Result<Compact<T>> {
        Compact::new(raw)
    }
}
