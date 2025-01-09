#![no_std]
#![feature(error_in_core)]

#[allow(unused_imports)]
#[macro_use]
extern crate alloc;
use alloc::{
    format,
    string::{String, ToString},
    vec::Vec,
};

pub use address::get_address;
use bitcoin::secp256k1::ecdsa::{RecoverableSignature, RecoveryId};
use bytes::{Buf, Bytes};
use hex;
use transactions::base_tx::BaseTx;
use transactions::tx_header::Header;
use ur_registry::pb::protoc;

use crate::errors::{AvaxError, Result};
use core::{
    convert::TryFrom,
    fmt::{self, Debug},
};

pub mod constants;
pub mod errors;
pub mod transactions;
pub mod encode {
    pub mod cb58;
}
mod address;
pub mod network;
mod ripple_keypair;
pub struct PsbtSignStatus {
    pub sign_status: Option<String>,
    pub is_completed: bool,
}

use transactions::type_id::TypeId;

pub fn parse_avax_tx<T>(data: Vec<u8>) -> Result<T>
where
    T: TryFrom<Bytes>,
{
    let bytes = Bytes::from(data);
    match T::try_from(bytes) {
        Ok(data) => Ok(data),
        Err(e) => Err(AvaxError::InvalidInput),
    }
}

pub fn get_avax_tx_type_id(data: Vec<u8>) -> Result<TypeId> {
    let mut bytes = Bytes::from(data);
    // codec_id 2 bytes
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
