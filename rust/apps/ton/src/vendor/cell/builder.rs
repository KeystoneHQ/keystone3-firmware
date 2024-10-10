use alloc::sync::Arc;
use core::ops::Add;

use alloc::format;
use alloc::string::ToString;
use alloc::vec;
use alloc::vec::Vec;
use bitstream_io::{BigEndian, BitWrite, BitWriter};
use num_bigint::{BigInt, BigUint, Sign};
use num_traits::{One, Zero};

use crate::vendor::address::TonAddress;
use crate::vendor::cell::error::{MapTonCellError, TonCellError};
use crate::vendor::cell::{ArcCell, Cell, CellParser};

const MAX_CELL_BITS: usize = 1023;
const MAX_CELL_REFERENCES: usize = 4;

pub struct CellBuilder {
    bit_writer: BitWriter<Vec<u8>, BigEndian>,
    references: Vec<ArcCell>,
    is_cell_exotic: bool,
}

impl CellBuilder {
    pub fn new() -> CellBuilder {
        let bit_writer = BitWriter::endian(Vec::new(), BigEndian);
        CellBuilder {
            bit_writer,
            references: Vec::new(),
            is_cell_exotic: false,
        }
    }

    pub fn set_cell_is_exotic(&mut self, val: bool) {
        self.is_cell_exotic = val;
    }

    pub fn store_bit(&mut self, val: bool) -> Result<&mut Self, TonCellError> {
        self.bit_writer.write_bit(val).map_cell_builder_error()?;
        Ok(self)
    }

    pub fn store_u8(&mut self, bit_len: usize, val: u8) -> Result<&mut Self, TonCellError> {
        self.bit_writer
            .write(bit_len as u32, val)
            .map_cell_builder_error()?;
        Ok(self)
    }

    pub fn store_i8(&mut self, bit_len: usize, val: i8) -> Result<&mut Self, TonCellError> {
        self.bit_writer
            .write(bit_len as u32, val)
            .map_cell_builder_error()?;
        Ok(self)
    }

    pub fn store_u32(&mut self, bit_len: usize, val: u32) -> Result<&mut Self, TonCellError> {
        self.bit_writer
            .write(bit_len as u32, val)
            .map_cell_builder_error()?;
        Ok(self)
    }

    pub fn store_i32(&mut self, bit_len: usize, val: i32) -> Result<&mut Self, TonCellError> {
        self.bit_writer
            .write(bit_len as u32, val)
            .map_cell_builder_error()?;
        Ok(self)
    }

    pub fn store_u64(&mut self, bit_len: usize, val: u64) -> Result<&mut Self, TonCellError> {
        self.bit_writer
            .write(bit_len as u32, val)
            .map_cell_builder_error()?;
        Ok(self)
    }

    pub fn store_i64(&mut self, bit_len: usize, val: i64) -> Result<&mut Self, TonCellError> {
        self.bit_writer
            .write(bit_len as u32, val)
            .map_cell_builder_error()?;
        Ok(self)
    }

    pub fn store_uint(&mut self, bit_len: usize, val: &BigUint) -> Result<&mut Self, TonCellError> {
        if val.bits() as usize > bit_len {
            return Err(TonCellError::cell_builder_error(format!(
                "Value {} doesn't fit in {} bits (takes {} bits)",
                val,
                bit_len,
                val.bits()
            )));
        }
        // example: bit_len=13, val=5. 5 = 00000101, we must store 0000000000101
        // leading_zeros_bits = 10
        // leading_zeros_bytes = 10 / 8 = 1
        let leading_zero_bits = bit_len - val.bits() as usize;
        let leading_zeros_bytes = leading_zero_bits / 8;
        for _ in 0..leading_zeros_bytes {
            self.store_byte(0)?;
        }
        // we must align high byte of val to specified bit_len, 00101 in our case
        let extra_zeros = leading_zero_bits % 8;
        for _ in 0..extra_zeros {
            self.store_bit(false)?;
        }
        // and then store val's high byte in minimum number of bits
        let val_bytes = val.to_bytes_be();
        let high_bits_cnt = {
            let cnt = val.bits() % 8;
            if cnt == 0 {
                8
            } else {
                cnt
            }
        };
        let high_byte = val_bytes[0];
        for i in 0..high_bits_cnt {
            self.store_bit(high_byte & (1 << (high_bits_cnt - i - 1)) != 0)?;
        }
        // store the rest of val
        for byte in val_bytes.iter().skip(1) {
            self.store_byte(*byte)?;
        }
        Ok(self)
    }

    pub fn store_int(&mut self, bit_len: usize, val: &BigInt) -> Result<&mut Self, TonCellError> {
        let (sign, mag) = val.clone().into_parts();
        let bit_len = bit_len - 1; // reserve 1 bit for sign
        if bit_len < mag.bits() as usize {
            return Err(TonCellError::cell_builder_error(format!(
                "Value {} doesn't fit in {} bits (takes {} bits)",
                val,
                bit_len,
                mag.bits()
            )));
        }
        if sign == Sign::Minus {
            self.store_byte(1)?;
            self.store_uint(bit_len, &extend_and_invert_bits(bit_len, &mag)?)?;
        } else {
            self.store_byte(0)?;
            self.store_uint(bit_len, &mag)?;
        };
        Ok(self)
    }

    pub fn store_byte(&mut self, val: u8) -> Result<&mut Self, TonCellError> {
        self.store_u8(8, val)
    }

    pub fn store_slice(&mut self, slice: &[u8]) -> Result<&mut Self, TonCellError> {
        for val in slice {
            self.store_byte(*val)?;
        }
        Ok(self)
    }

    pub fn store_bits(&mut self, bit_len: usize, slice: &[u8]) -> Result<&mut Self, TonCellError> {
        let full_bytes = bit_len / 8;
        self.store_slice(&slice[0..full_bytes])?;
        let last_byte_len = bit_len % 8;
        if last_byte_len != 0 {
            let last_byte = slice[full_bytes] >> (8 - last_byte_len);
            self.store_u8(last_byte_len, last_byte)?;
        }
        Ok(self)
    }

    pub fn store_string(&mut self, val: &str) -> Result<&mut Self, TonCellError> {
        self.store_slice(val.as_bytes())
    }

    pub fn store_coins(&mut self, val: &BigUint) -> Result<&mut Self, TonCellError> {
        if val.is_zero() {
            self.store_u8(4, 0)
        } else {
            let num_bytes = (val.bits() as usize + 7) / 8;
            self.store_u8(4, num_bytes as u8)?;
            self.store_uint(num_bytes * 8, val)
        }
    }

    /// Stores address without optimizing hole address
    pub fn store_raw_address(&mut self, val: &TonAddress) -> Result<&mut Self, TonCellError> {
        self.store_u8(2, 0b10u8)?;
        self.store_bit(false)?;
        let wc = (val.workchain & 0xff) as u8;
        self.store_u8(8, wc)?;
        self.store_slice(&val.hash_part)?;
        Ok(self)
    }

    /// Stores address optimizing hole address two to bits
    pub fn store_address(&mut self, val: &TonAddress) -> Result<&mut Self, TonCellError> {
        if val == &TonAddress::NULL {
            self.store_u8(2, 0)?;
        } else {
            self.store_raw_address(val)?;
        }
        Ok(self)
    }

    /// Adds reference to an existing `Cell`.
    ///
    /// The reference is passed as `ArcCell` so it might be references from other cells.
    pub fn store_reference(&mut self, cell: &ArcCell) -> Result<&mut Self, TonCellError> {
        let ref_count = self.references.len() + 1;
        if ref_count > 4 {
            return Err(TonCellError::cell_builder_error(format!(
                "Cell must contain at most 4 references, got {}",
                ref_count
            )));
        }
        self.references.push(cell.clone());
        Ok(self)
    }

    pub fn store_references(&mut self, refs: &[ArcCell]) -> Result<&mut Self, TonCellError> {
        for r in refs {
            self.store_reference(r)?;
        }
        Ok(self)
    }

    /// Adds a reference to a newly constructed `Cell`.
    ///
    /// The cell is wrapped it the `Arc`.
    pub fn store_child(&mut self, cell: Cell) -> Result<&mut Self, TonCellError> {
        self.store_reference(&Arc::new(cell))
    }

    pub fn store_remaining_bits(
        &mut self,
        parser: &mut CellParser,
    ) -> Result<&mut Self, TonCellError> {
        let num_full_bytes = parser.remaining_bits() / 8;
        let bytes = parser.load_bytes(num_full_bytes)?;
        self.store_slice(bytes.as_slice())?;
        let num_bits = parser.remaining_bits() % 8;
        let tail = parser.load_u8(num_bits)?;
        self.store_u8(num_bits, tail)?;
        Ok(self)
    }

    pub fn store_cell_data(&mut self, cell: &Cell) -> Result<&mut Self, TonCellError> {
        let mut parser = cell.parser();
        self.store_remaining_bits(&mut parser)?;
        Ok(self)
    }

    pub fn store_cell(&mut self, cell: &Cell) -> Result<&mut Self, TonCellError> {
        self.store_cell_data(cell)?;
        self.store_references(cell.references.as_slice())?;
        Ok(self)
    }

    pub fn build(&mut self) -> Result<Cell, TonCellError> {
        let mut trailing_zeros = 0;
        while !self.bit_writer.byte_aligned() {
            self.bit_writer.write_bit(false).map_cell_builder_error()?;
            trailing_zeros += 1;
        }

        if let Some(vec) = self.bit_writer.writer() {
            let bit_len = vec.len() * 8 - trailing_zeros;
            if bit_len > MAX_CELL_BITS {
                return Err(TonCellError::cell_builder_error(format!(
                    "Cell must contain at most {} bits, got {}",
                    MAX_CELL_BITS, bit_len
                )));
            }
            let ref_count = self.references.len();
            if ref_count > MAX_CELL_REFERENCES {
                return Err(TonCellError::cell_builder_error(format!(
                    "Cell must contain at most 4 references, got {}",
                    ref_count
                )));
            }

            Cell::new(
                vec.clone(),
                bit_len,
                self.references.clone(),
                self.is_cell_exotic,
            )
        } else {
            Err(TonCellError::CellBuilderError(
                "Stream is not byte-aligned".to_string(),
            ))
        }
    }
}

fn extend_and_invert_bits(bits_cnt: usize, src: &BigUint) -> Result<BigUint, TonCellError> {
    if bits_cnt < src.bits() as usize {
        return Err(TonCellError::cell_builder_error(format!(
            "Can't invert bits: value {} doesn't fit in {} bits",
            src, bits_cnt
        )));
    }

    let src_bytes = src.to_bytes_be();
    let inverted_bytes_cnt = (bits_cnt + 7) / 8;
    let mut inverted = vec![0xffu8; inverted_bytes_cnt];
    // can be optimized
    for (pos, byte) in src_bytes.iter().rev().enumerate() {
        let inverted_pos = inverted.len() - 1 - pos;
        inverted[inverted_pos] ^= byte;
    }
    let mut inverted_val_bytes = BigUint::from_bytes_be(&inverted)
        .add(BigUint::one())
        .to_bytes_be();
    let leading_zeros = inverted_bytes_cnt * 8 - bits_cnt;
    inverted_val_bytes[0] &= 0xffu8 >> leading_zeros;
    Ok(BigUint::from_bytes_be(&inverted_val_bytes))
}

impl Default for CellBuilder {
    fn default() -> Self {
        Self::new()
    }
}
