use crate::errors::{AvaxError, Result};

#[derive(Debug, Copy, Clone, PartialEq)]
pub enum TypeId {
    BaseTx = 0,
    CchainExportTx = 0x0000_0001,
    XchainImportTx = 0x0000_0003,
    XchainExportTx = 0x0000_0004,
    Secp256k1TransferInput = 0x0000_0005,
    Secp256k1MintOutput = 0x0000_0006,
    Secp256k1TransferOutput = 0x0000_0007,
    Secp256k1MintOperation = 0x0000_0008,
    Secp256k1Credential = 0x00000009,
    ValidatorRewardsOwner = 0x0000_000B,
    NftMintOperation = 0x0000000C,
    NftTransferOperation = 0x0000000D,
    PchainImportTx = 0x00000011,
    PchainExportTx = 0x00000012,
    AddPermissionlessValidator = 0x00000019,
    AddPermissionlessDelegator = 0x0000001A,
}

impl TryFrom<u32> for TypeId {
    type Error = AvaxError;

    fn try_from(value: u32) -> Result<Self> {
        match value {
            // the code and documentation are not fully consistent.
            0 | 0x0000_0022 => Ok(TypeId::BaseTx),
            0x0000_0001 => Ok(TypeId::CchainExportTx),
            0x0000_0003 => Ok(TypeId::XchainImportTx),
            0x0000_0004 => Ok(TypeId::XchainExportTx),
            0x0000_0005 => Ok(TypeId::Secp256k1TransferInput),
            0x0000_0006 => Ok(TypeId::Secp256k1MintOutput),
            0x0000_0007 => Ok(TypeId::Secp256k1TransferOutput),
            0x0000_0008 => Ok(TypeId::Secp256k1MintOperation),
            0x0000_0009 => Ok(TypeId::Secp256k1Credential),
            0x0000_000B => Ok(TypeId::ValidatorRewardsOwner),
            0x0000_000C => Ok(TypeId::NftMintOperation),
            0x0000_000D => Ok(TypeId::NftTransferOperation),
            0x0000_0011 => Ok(TypeId::PchainImportTx),
            0x0000_0012 => Ok(TypeId::PchainExportTx),
            0x0000_0019 => Ok(TypeId::AddPermissionlessValidator),
            0x0000_001A => Ok(TypeId::AddPermissionlessDelegator),
            _ => Err(AvaxError::UnknownTypeId(value)),
        }
    }
}
