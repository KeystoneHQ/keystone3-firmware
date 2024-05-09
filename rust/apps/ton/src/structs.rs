use alloc::vec::Vec;
use crate::vendor::cell::{BagOfCells, Cell};
use crate::errors::Result;
use crate::messages::{SigningMessage};

pub struct TonTransaction {
    signing_message: SigningMessage,
    buffer_to_sign: Vec<u8>,
}

impl TonTransaction {
    fn parse(boc: BagOfCells)-> Result<Self> {
        let root = boc.single_root()?;
        let buffer_to_sign = root.cell_hash()?;
        unimplemented!()
    }
}