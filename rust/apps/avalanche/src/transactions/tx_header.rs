use crate::errors::{AvaxError, Result};

pub const BLOCKCHAIN_ID_LEN: usize = 32;

#[derive(Clone, Copy, PartialEq, Eq)]
pub struct Header<'a> {
    pub network_id: u32,
    pub blockchain_id: &'a [u8; BLOCKCHAIN_ID_LEN],
}