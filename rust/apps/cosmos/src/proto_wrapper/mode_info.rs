use crate::cosmos_sdk_proto as proto;
use crate::CosmosError;
use alloc::string::{String, ToString};
use alloc::vec::Vec;
use serde::Serialize;

#[derive(Debug, Serialize)]
pub enum ModeInfo {
    /// single represents a single signer
    Single(Single),
    /// multi represents a nested multisig signer
    Multi(Multi),
}

impl TryFrom<&proto::cosmos::tx::v1beta1::ModeInfo> for ModeInfo {
    type Error = CosmosError;

    fn try_from(proto: &proto::cosmos::tx::v1beta1::ModeInfo) -> Result<ModeInfo, CosmosError> {
        match &proto.sum {
            Some(proto::cosmos::tx::v1beta1::mode_info::Sum::Single(single)) => {
                Ok(ModeInfo::Single(single.into()))
            }
            Some(proto::cosmos::tx::v1beta1::mode_info::Sum::Multi(multi)) => {
                Ok(ModeInfo::Multi(multi.try_into()?))
            }
            None => Err(CosmosError::ParseTxError(
                "option mode_info sum is None".to_string(),
            )),
        }
    }
}

impl TryFrom<proto::cosmos::tx::v1beta1::ModeInfo> for ModeInfo {
    type Error = CosmosError;

    fn try_from(proto: proto::cosmos::tx::v1beta1::ModeInfo) -> Result<ModeInfo, CosmosError> {
        match &proto.sum {
            Some(proto::cosmos::tx::v1beta1::mode_info::Sum::Single(single)) => {
                Ok(ModeInfo::Single(single.into()))
            }
            Some(proto::cosmos::tx::v1beta1::mode_info::Sum::Multi(multi)) => {
                Ok(ModeInfo::Multi(multi.try_into()?))
            }
            None => Err(CosmosError::ParseTxError(
                "option mode_info sum is None".to_string(),
            )),
        }
    }
}

#[derive(Debug, Serialize)]
pub struct CompactBitArray {
    pub extra_bits_stored: u32,
    pub elems: Vec<u8>,
}

impl From<&proto::cosmos::crypto::multisig::v1beta1::CompactBitArray> for CompactBitArray {
    fn from(proto: &proto::cosmos::crypto::multisig::v1beta1::CompactBitArray) -> CompactBitArray {
        CompactBitArray {
            extra_bits_stored: proto.extra_bits_stored,
            elems: proto.elems.to_vec(),
        }
    }
}

#[derive(Debug, Serialize)]
pub struct Multi {
    /// bitarray specifies which keys within the multisig are signing
    pub bitarray: Option<CompactBitArray>,
    /// mode_infos is the corresponding modes of the signers of the multisig
    /// which could include nested multisig public keys
    pub mode_infos: Vec<ModeInfo>,
}

#[derive(Debug, Serialize)]
pub struct Single {
    pub mode: String,
}

impl From<&proto::cosmos::tx::v1beta1::mode_info::Single> for Single {
    fn from(proto: &proto::cosmos::tx::v1beta1::mode_info::Single) -> Single {
        Single {
            mode: proto.mode().as_str_name().to_string(),
        }
    }
}

impl TryFrom<&proto::cosmos::tx::v1beta1::mode_info::Multi> for Multi {
    type Error = CosmosError;

    fn try_from(
        proto: &proto::cosmos::tx::v1beta1::mode_info::Multi,
    ) -> Result<Multi, CosmosError> {
        let bitarray = match &proto.bitarray {
            Some(value) => Some(value.into()),
            None => None,
        };

        Ok(Multi {
            bitarray,
            mode_infos: proto
                .mode_infos
                .clone()
                .into_iter()
                .map(TryInto::try_into)
                .collect::<Result<_, _>>()?,
        })
    }
}
