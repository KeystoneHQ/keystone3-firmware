mod types;

use alloc::string::ToString;
use alloc::sync::Arc;

use alloc::vec::Vec;
use lazy_static::lazy_static;
pub use types::*;

use crate::vendor::address::TonAddress;
use crate::vendor::cell::{ArcCell, BagOfCells, Cell, StateInit, TonCellError};

pub const DEFAULT_WALLET_ID: i32 = 0x29a9a317;

lazy_static! {
    pub static ref WALLET_V4R2_CODE: BagOfCells = {
        let code = include_str!("../../../resources/wallet/wallet_v4r2.code");
        BagOfCells::parse_base64(code).unwrap()
    };
}

#[derive(PartialEq, Eq, Clone, Hash)]
pub enum WalletVersion {
    V4R2,
}

impl WalletVersion {
    pub fn code(&self) -> Result<&ArcCell, TonCellError> {
        let code: &BagOfCells = match self {
            // reduce firmware size, ignore all other unsupported versions
            WalletVersion::V4R2 => &WALLET_V4R2_CODE,
        };
        code.single_root()
    }

    pub fn initial_data(
        &self,
        public_key: Vec<u8>,
        wallet_id: i32,
    ) -> Result<ArcCell, TonCellError> {
        let public_key: [u8; 32] = public_key
            .clone()
            .try_into()
            .map_err(|_| TonCellError::InternalError("Invalid public key size".to_string()))?;

        let data_cell: Cell = match &self {
            WalletVersion::V4R2 => WalletDataV4 {
                seqno: 0,
                wallet_id,
                public_key,
            }
            .try_into()?,
        };

        Ok(Arc::new(data_cell))
    }

    pub fn has_op(&self) -> bool {
        matches!(self, WalletVersion::V4R2)
    }
}

#[derive(PartialEq, Eq, Clone, Hash)]
pub struct TonWallet {
    pub public_key: Vec<u8>,
    pub version: WalletVersion,
    pub address: TonAddress,
    pub wallet_id: i32,
}

impl TonWallet {
    pub fn derive(
        workchain: i32,
        version: WalletVersion,
        public_key: Vec<u8>,
        wallet_id: i32,
    ) -> Result<TonWallet, TonCellError> {
        let data = version.initial_data(public_key.clone(), wallet_id)?;
        let code = version.code()?;
        let state_init_hash = StateInit::create_account_id(code, &data)?;
        let hash_part = match state_init_hash.as_slice().try_into() {
            Ok(hash_part) => hash_part,
            Err(_) => {
                return Err(TonCellError::InternalError(
                    "StateInit returned hash pof wrong size".to_string(),
                ))
            }
        };
        let addr = TonAddress::new(workchain, &hash_part);
        Ok(TonWallet {
            public_key: public_key.clone(),
            version,
            address: addr,
            wallet_id,
        })
    }

    pub fn derive_default(
        version: WalletVersion,
        public_key: Vec<u8>,
    ) -> Result<TonWallet, TonCellError> {
        let wallet_id = DEFAULT_WALLET_ID;
        let data = version.initial_data(public_key.clone(), wallet_id)?;
        let code = version.code()?;
        let state_init_hash = StateInit::create_account_id(code, &data)?;
        let hash_part = match state_init_hash.as_slice().try_into() {
            Ok(hash_part) => hash_part,
            Err(_) => {
                return Err(TonCellError::InternalError(
                    "StateInit returned hash pof wrong size".to_string(),
                ))
            }
        };
        let addr = TonAddress::new(0, &hash_part);
        Ok(TonWallet {
            public_key: public_key.clone(),
            version,
            address: addr,
            wallet_id,
        })
    }
}
