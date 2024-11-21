use crate::vendor::cell::{Cell, CellBuilder, TonCellError};

/// WalletVersion::V4R1 | WalletVersion::V4R2
pub struct WalletDataV4 {
    pub seqno: u32,
    pub wallet_id: i32,
    pub public_key: [u8; 32],
}

impl TryFrom<Cell> for WalletDataV4 {
    type Error = TonCellError;

    fn try_from(value: Cell) -> Result<Self, Self::Error> {
        let mut parser = value.parser();
        let seqno = parser.load_u32(32)?;
        let wallet_id = parser.load_i32(32)?;
        let mut public_key = [0u8; 32];
        parser.load_slice(&mut public_key)?;
        // TODO: handle plugin dict
        Ok(Self {
            seqno,
            wallet_id,
            public_key,
        })
    }
}

impl TryFrom<WalletDataV4> for Cell {
    type Error = TonCellError;

    fn try_from(value: WalletDataV4) -> Result<Self, Self::Error> {
        CellBuilder::new()
            .store_u32(32, value.seqno)?
            .store_i32(32, value.wallet_id)?
            .store_slice(&value.public_key)?
            // empty plugin dict
            .store_bit(false)?
            .build()
    }
}
