use super::{ArcCell, CellHash};
use super::{Cell, CellBuilder, TonCellError};

pub struct StateInitBuilder {
    code: Option<ArcCell>,
    data: Option<ArcCell>,
    split_depth: bool,
    tick_tock: bool,
    library: bool,
}
pub struct StateInit {
    pub code: Option<ArcCell>,
    pub data: Option<ArcCell>,
}

impl StateInitBuilder {
    pub fn new(code: &ArcCell, data: &ArcCell) -> StateInitBuilder {
        StateInitBuilder {
            code: Some(code.clone()),
            data: Some(data.clone()),
            split_depth: false,
            tick_tock: false,
            library: false,
        }
    }

    pub fn with_split_depth(&mut self, split_depth: bool) -> &mut Self {
        self.split_depth = split_depth;
        self
    }

    pub fn with_tick_tock(&mut self, tick_tock: bool) -> &mut Self {
        self.tick_tock = tick_tock;
        self
    }

    pub fn with_library(&mut self, library: bool) -> &mut Self {
        self.library = library;
        self
    }

    pub fn build(&self) -> Result<Cell, TonCellError> {
        let mut builder = CellBuilder::new();
        builder
            .store_bit(self.split_depth)? //Split depth
            .store_bit(self.tick_tock)? //Tick tock
            .store_bit(self.code.is_some())? //Code
            .store_bit(self.data.is_some())? //Data
            .store_bit(self.library)?; //Library
        if let Some(code) = &self.code {
            builder.store_reference(code)?;
        }
        if let Some(data) = &self.data {
            builder.store_reference(data)?;
        }
        builder.build()
    }
}

impl StateInit {
    pub fn create_account_id(code: &ArcCell, data: &ArcCell) -> Result<CellHash, TonCellError> {
        Ok(StateInitBuilder::new(code, data).build()?.cell_hash())
    }
}
