use alloc::sync::Arc;

use alloc::vec;
use alloc::vec::Vec;
use base64::engine::general_purpose::STANDARD;

use crate::vendor::cell::raw_boc_from_boc::convert_to_raw_boc;
use crate::vendor::cell::*;

#[derive(PartialEq, Eq, Debug, Clone, Hash)]
pub struct BagOfCells {
    pub roots: Vec<ArcCell>,
}

impl BagOfCells {
    pub fn new(roots: &[ArcCell]) -> BagOfCells {
        BagOfCells {
            roots: roots.to_vec(),
        }
    }

    pub fn from_root(root: Cell) -> BagOfCells {
        let arc = Arc::new(root);
        BagOfCells { roots: vec![arc] }
    }

    pub fn add_root(&mut self, root: Cell) {
        let arc = Arc::new(root);
        self.roots.push(arc)
    }

    pub fn num_roots(&self) -> usize {
        self.roots.len()
    }

    pub fn root(&self, idx: usize) -> Result<&ArcCell, TonCellError> {
        self.roots.get(idx).ok_or_else(|| {
            TonCellError::boc_deserialization_error(format!(
                "Invalid root index: {}, BoC contains {} roots",
                idx,
                self.roots.len()
            ))
        })
    }

    pub fn single_root(&self) -> Result<&ArcCell, TonCellError> {
        let root_count = self.roots.len();
        if root_count == 1 {
            Ok(&self.roots[0])
        } else {
            Err(TonCellError::CellParserError(format!(
                "Single root expected, got {root_count}"
            )))
        }
    }

    pub fn parse(serial: &[u8]) -> Result<BagOfCells, TonCellError> {
        let raw = RawBagOfCells::parse(serial)?;
        let num_cells = raw.cells.len();
        let mut cells: Vec<ArcCell> = Vec::with_capacity(num_cells);

        for (cell_index, raw_cell) in raw.cells.into_iter().enumerate().rev() {
            let mut references = Vec::with_capacity(raw_cell.references.len());
            for ref_index in &raw_cell.references {
                if *ref_index <= cell_index {
                    return Err(TonCellError::boc_deserialization_error(
                        "References to previous cells are not supported",
                    ));
                }
                references.push(cells[num_cells - 1 - ref_index].clone());
            }

            let cell = Cell::new(
                raw_cell.data,
                raw_cell.bit_len,
                references,
                raw_cell.is_exotic,
            )
            .map_err(|e| TonCellError::boc_deserialization_error(e.to_string()))?;
            cells.push(cell.to_arc());
        }

        let roots = raw
            .roots
            .into_iter()
            .map(|r| &cells[num_cells - 1 - r])
            .map(Arc::clone)
            .collect();

        Ok(BagOfCells { roots })
    }

    pub fn parse_hex(hex: &str) -> Result<BagOfCells, TonCellError> {
        let str: String = hex.chars().filter(|c| !c.is_whitespace()).collect();
        let bin = hex::decode(str.as_str())
            .map_err(|e| TonCellError::boc_serialization_error(e.to_string()))?;
        Self::parse(&bin)
    }

    pub fn parse_base64(base64: &str) -> Result<BagOfCells, TonCellError> {
        let bin = STANDARD
            .decode(base64)
            .map_err(|e| TonCellError::boc_deserialization_error(e.to_string()))?;
        Self::parse(&bin)
    }

    pub fn serialize(&self, has_crc32: bool) -> Result<Vec<u8>, TonCellError> {
        let raw = convert_to_raw_boc(self)?;
        raw.serialize(has_crc32)
    }
}
