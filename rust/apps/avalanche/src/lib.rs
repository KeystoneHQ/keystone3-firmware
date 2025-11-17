#![no_std]

#[allow(unused_imports)]
#[macro_use]
extern crate alloc;
use alloc::{string::ToString, vec::Vec};

pub use address::get_address;
use bytes::{Buf, Bytes};
use transactions::tx_header::Header;

use crate::errors::{AvaxError, Result};
use core::convert::TryFrom;

pub mod constants;
pub mod errors;
pub mod transactions;
pub mod encode {
    pub mod cb58;
}
mod address;
pub mod network;
mod ripple_keypair;

use transactions::type_id::TypeId;

pub fn parse_avax_tx<T>(data: Vec<u8>) -> Result<T>
where
    T: TryFrom<Bytes>,
{
    let bytes = Bytes::from(data);
    match T::try_from(bytes) {
        Ok(data) => Ok(data),
        Err(_) => Err(AvaxError::InvalidInput),
    }
}

pub fn get_avax_tx_type_id(data: Vec<u8>) -> Result<TypeId> {
    let mut bytes = Bytes::from(data);
    if bytes.remaining() < 6 {
        return Err(AvaxError::InvalidTransaction(
            "Insufficient data".to_string(),
        ));
    }
    bytes.advance(2);
    let type_id = TypeId::try_from(bytes.get_u32())?;
    Ok(type_id)
}

pub fn get_avax_tx_header(data: Vec<u8>) -> Result<Header> {
    let mut bytes = Bytes::from(data);
    // codec_id 2 bytes
    bytes.advance(2);

    // type id
    bytes.advance(4);

    // tx header
    let tx_header = Header::try_from(bytes.clone())?;
    bytes.advance(tx_header.parsed_size());

    Ok(tx_header)
}
