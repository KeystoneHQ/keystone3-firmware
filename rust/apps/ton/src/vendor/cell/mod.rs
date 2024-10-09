use alloc::sync::Arc;
use alloc::{
    collections::BTreeMap,
    format,
    string::{String, ToString},
    vec::Vec,
};

use alloc::borrow::ToOwned;
use core::fmt;
use core::fmt::Debug;
use core::fmt::Formatter;
use core::hash::Hash;
use core::ops::Deref;
use third_party::core2::io;
use third_party::core2::io::Cursor;

pub use crate::vendor::cell::bag_of_cells::*;
use crate::vendor::cell::bit_string::*;
pub use crate::vendor::cell::builder::*;
pub use crate::vendor::cell::error::*;
pub use crate::vendor::cell::parser::*;
pub use crate::vendor::cell::raw::*;
pub use crate::vendor::cell::slice::*;
pub use crate::vendor::cell::state_init::*;
pub use crate::vendor::cell::util::*;
use base64::engine::general_purpose::URL_SAFE_NO_PAD;
use base64::Engine;
use bitstream_io::{BigEndian, BitReader, BitWrite, BitWriter};
use num_bigint::BigUint;
use num_traits::{One, ToPrimitive};
use sha2::Digest;
use sha2::Sha256;

use crate::vendor::cell::cell_type::CellType;
use crate::vendor::cell::level_mask::LevelMask;

mod bag_of_cells;
mod bit_string;
mod builder;
mod cell_type;
mod error;
mod level_mask;
mod parser;
mod raw;
mod raw_boc_from_boc;
mod slice;
mod state_init;
mod util;

const HASH_BYTES: usize = 32;
const DEPTH_BYTES: usize = 2;
const MAX_LEVEL: u8 = 3;

pub type CellHash = [u8; HASH_BYTES];
pub type ArcCell = Arc<Cell>;

pub type SnakeFormattedDict = BTreeMap<CellHash, Vec<u8>>;

#[derive(PartialEq, Eq, Clone, Hash)]
pub struct Cell {
    pub data: Vec<u8>,
    pub bit_len: usize,
    pub references: Vec<ArcCell>,
    pub cell_type: CellType,
    pub level_mask: LevelMask,
    pub hashes: [CellHash; 4],
    pub depths: [u16; 4],
}

impl Cell {
    pub fn new(
        data: Vec<u8>,
        bit_len: usize,
        references: Vec<ArcCell>,
        is_exotic: bool,
    ) -> Result<Self, TonCellError> {
        let cell_type = if is_exotic {
            CellType::determine_exotic_cell_type(&data)?
        } else {
            CellType::Ordinary
        };

        cell_type.validate(&data, bit_len, &references)?;
        let level_mask = cell_type.level_mask(&data, bit_len, &references)?;
        let (hashes, depths) =
            calculate_hashes_and_depths(cell_type, &data, bit_len, &references, level_mask)?;

        let result = Self {
            data,
            bit_len,
            references,
            level_mask,
            cell_type,
            hashes,
            depths,
        };

        Ok(result)
    }

    pub fn parser(&self) -> CellParser {
        let bit_len = self.bit_len;
        let cursor = Cursor::new(&self.data);
        let bit_reader: BitReader<Cursor<&Vec<u8>>, BigEndian> =
            BitReader::endian(cursor, BigEndian);

        CellParser {
            bit_len,
            bit_reader,
        }
    }

    #[allow(clippy::let_and_return)]
    pub fn parse<F, T>(&self, parse: F) -> Result<T, TonCellError>
    where
        F: FnOnce(&mut CellParser) -> Result<T, TonCellError>,
    {
        let mut parser = self.parser();
        let res = parse(&mut parser);
        res
    }

    pub fn parse_fully<F, T>(&self, parse: F) -> Result<T, TonCellError>
    where
        F: FnOnce(&mut CellParser) -> Result<T, TonCellError>,
    {
        let mut reader = self.parser();
        let res = parse(&mut reader);
        reader.ensure_empty()?;
        res
    }

    pub fn reference(&self, idx: usize) -> Result<&ArcCell, TonCellError> {
        self.references.get(idx).ok_or(TonCellError::InvalidIndex {
            idx,
            ref_count: self.references.len(),
        })
    }

    pub fn data(&self) -> &[u8] {
        self.data.as_slice()
    }

    pub fn bit_len(&self) -> usize {
        self.bit_len
    }

    pub fn references(&self) -> &[ArcCell] {
        self.references.as_slice()
    }

    pub(crate) fn get_level_mask(&self) -> u32 {
        self.level_mask.mask()
    }

    pub fn cell_depth(&self) -> u16 {
        self.get_depth(MAX_LEVEL)
    }

    pub fn get_depth(&self, level: u8) -> u16 {
        self.depths[level.min(3) as usize]
    }

    pub fn cell_hash(&self) -> CellHash {
        self.get_hash(MAX_LEVEL)
    }

    pub fn get_hash(&self, level: u8) -> CellHash {
        self.hashes[level.min(3) as usize]
    }

    pub fn is_exotic(&self) -> bool {
        self.cell_type != CellType::Ordinary
    }

    pub fn cell_hash_base64(&self) -> String {
        URL_SAFE_NO_PAD.encode(self.cell_hash())
    }

    pub fn to_arc(self) -> ArcCell {
        Arc::new(self)
    }

    pub fn expect_reference_count(&self, expected_refs: usize) -> Result<(), TonCellError> {
        let ref_count = self.references.len();
        if ref_count != expected_refs {
            Err(TonCellError::CellParserError(format!(
                "Cell should contain {} reference cells, actual: {}",
                expected_refs, ref_count
            )))
        } else {
            Ok(())
        }
    }
}

impl Debug for Cell {
    fn fmt(&self, f: &mut Formatter<'_>) -> fmt::Result {
        let t = match self.cell_type {
            CellType::Ordinary | CellType::Library => 'x',
            CellType::PrunedBranch | CellType::MerkleProof => 'p',
            CellType::MerkleUpdate => 'u',
        };
        writeln!(
            f,
            "Cell {}{{ data: [{}], bit_len: {}, references: [\n",
            t,
            self.data
                .iter()
                .map(|&byte| format!("{:02X}", byte))
                .collect::<Vec<_>>()
                .join(""),
            self.bit_len,
        )?;

        for reference in &self.references {
            writeln!(
                f,
                "    {}\n",
                format!("{:?}", reference).replace('\n', "\n    ")
            )?;
        }

        write!(f, "] }}")
    }
}

fn get_repr_for_data(
    (original_data, original_data_bit_len): (&[u8], usize),
    (data, data_bit_len): (&[u8], usize),
    refs: &[ArcCell],
    level_mask: LevelMask,
    level: u8,
    cell_type: CellType,
) -> Result<Vec<u8>, TonCellError> {
    // Allocate
    let data_len = data.len();
    // descriptors + data + (hash + depth) * refs_count
    let buffer_len = 2 + data_len + (32 + 2) * refs.len();

    let mut writer = BitWriter::endian(Vec::with_capacity(buffer_len), BigEndian);
    let d1 = get_refs_descriptor(cell_type, refs, level_mask.apply(level).mask());
    let d2 = get_bits_descriptor(original_data, original_data_bit_len);

    // Write descriptors
    writer.write(8, d1).map_cell_parser_error()?;
    writer.write(8, d2).map_cell_parser_error()?;
    // Write main data
    write_data(&mut writer, data, data_bit_len).map_cell_parser_error()?;
    // Write ref data
    write_ref_depths(&mut writer, refs, cell_type, level)?;
    write_ref_hashes(&mut writer, refs, cell_type, level)?;

    let result = writer
        .writer()
        .ok_or_else(|| TonCellError::cell_builder_error("Stream for cell repr is not byte-aligned"))
        .map(|b| b.to_vec());

    result
}

/// This function replicates unknown logic of resolving cell data
/// https://github.com/ton-blockchain/ton/blob/24dc184a2ea67f9c47042b4104bbb4d82289fac1/crypto/vm/cells/DataCell.cpp#L214
fn calculate_hashes_and_depths(
    cell_type: CellType,
    data: &[u8],
    bit_len: usize,
    references: &[ArcCell],
    level_mask: LevelMask,
) -> Result<([CellHash; 4], [u16; 4]), TonCellError> {
    let hash_count = if cell_type == CellType::PrunedBranch {
        1
    } else {
        level_mask.hash_count()
    };

    let total_hash_count = level_mask.hash_count();
    let hash_i_offset = total_hash_count - hash_count;

    let mut depths: Vec<u16> = Vec::with_capacity(hash_count);
    let mut hashes: Vec<CellHash> = Vec::with_capacity(hash_count);

    // Iterate through significant levels
    for (hash_i, level_i) in (0..=level_mask.level())
        .filter(|&i| level_mask.is_significant(i))
        .enumerate()
    {
        if hash_i < hash_i_offset {
            continue;
        }

        let (current_data, current_bit_len) = if hash_i == hash_i_offset {
            (data, bit_len)
        } else {
            let previous_hash = hashes
                .get(hash_i - hash_i_offset - 1)
                .ok_or_else(|| TonCellError::InternalError("Can't get right hash".to_owned()))?;
            (previous_hash.as_slice(), 256)
        };

        // Calculate Depth
        let depth = if references.is_empty() {
            0
        } else {
            let max_ref_depth = references.iter().fold(0, |max_depth, reference| {
                let child_depth = cell_type.child_depth(reference, level_i);
                max_depth.max(child_depth)
            });

            max_ref_depth + 1
        };

        // Calculate Hash
        let repr = get_repr_for_data(
            (data, bit_len),
            (current_data, current_bit_len),
            references,
            level_mask,
            level_i,
            cell_type,
        )?;
        let hash = Sha256::new_with_prefix(repr).finalize()[..]
            .try_into()
            .map_err(|error| {
                TonCellError::InternalError(format!(
                    "Can't get [u8; 32] from finalized hash with error: {error}"
                ))
            })?;

        depths.push(depth);
        hashes.push(hash);
    }

    cell_type.resolve_hashes_and_depths(hashes, depths, data, bit_len, level_mask)
}

fn get_refs_descriptor(cell_type: CellType, references: &[ArcCell], level_mask: u32) -> u8 {
    let cell_type_var = (cell_type != CellType::Ordinary) as u8;
    references.len() as u8 + 8 * cell_type_var + level_mask as u8 * 32
}

fn get_bits_descriptor(data: &[u8], bit_len: usize) -> u8 {
    let rest_bits = bit_len % 8;
    let full_bytes = rest_bits == 0;
    data.len() as u8 * 2 - !full_bytes as u8 // subtract 1 if the last byte is not full
}

fn write_data(
    writer: &mut BitWriter<Vec<u8>, BigEndian>,
    data: &[u8],
    bit_len: usize,
) -> Result<(), io::Error> {
    let data_len = data.len();
    let rest_bits = bit_len % 8;
    let full_bytes = rest_bits == 0;

    if !full_bytes {
        writer.write_bytes(&data[..data_len - 1])?;
        let last_byte = data[data_len - 1];
        let l = last_byte | 1 << (8 - rest_bits - 1);
        writer.write(8, l)?;
    } else {
        writer.write_bytes(data)?;
    }

    Ok(())
}

fn write_ref_depths(
    writer: &mut BitWriter<Vec<u8>, BigEndian>,
    refs: &[ArcCell],
    parent_cell_type: CellType,
    level: u8,
) -> Result<(), TonCellError> {
    for reference in refs {
        let child_depth = if matches!(
            parent_cell_type,
            CellType::MerkleProof | CellType::MerkleUpdate
        ) {
            reference.get_depth(level + 1)
        } else {
            reference.get_depth(level)
        };

        writer.write(8, child_depth / 256).map_cell_parser_error()?;
        writer.write(8, child_depth % 256).map_cell_parser_error()?;
    }

    Ok(())
}

fn write_ref_hashes(
    writer: &mut BitWriter<Vec<u8>, BigEndian>,
    refs: &[ArcCell],
    parent_cell_type: CellType,
    level: u8,
) -> Result<(), TonCellError> {
    for reference in refs {
        let child_hash = if matches!(
            parent_cell_type,
            CellType::MerkleProof | CellType::MerkleUpdate
        ) {
            reference.get_hash(level + 1)
        } else {
            reference.get_hash(level)
        };

        writer.write_bytes(&child_hash).map_cell_parser_error()?;
    }

    Ok(())
}
