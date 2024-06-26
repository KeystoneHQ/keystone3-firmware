use crate::vendor::cell::{ArcCell, TonCellError};

pub trait ParseCell {
    fn parse(cell: &ArcCell) -> Result<Self, TonCellError>
    where
        Self: Sized;
}