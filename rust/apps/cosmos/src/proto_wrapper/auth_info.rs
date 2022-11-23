use crate::cosmos_sdk_proto as proto;
use crate::proto_wrapper::fee::Fee;
use crate::proto_wrapper::signer_info::SignerInfo;
use crate::CosmosError;
use alloc::vec::Vec;
use serde::Serialize;

#[derive(Debug, Serialize)]
pub struct AuthInfo {
    /// signer_infos defines the signing modes for the required signers. The number
    /// and order of elements must match the required signers from TxBody's
    /// messages. The first element is the primary signer and the one which pays
    /// the fee.
    pub signer_infos: Vec<SignerInfo>,
    /// Fee is the fee and gas limit for the transaction. The first signer is the
    /// primary signer and the one which pays the fee. The fee can be calculated
    /// based on the cost of evaluating the body and doing signature verification
    /// of the signers. This can be estimated via simulation.
    pub fee: Option<Fee>,
}

impl TryFrom<proto::cosmos::tx::v1beta1::AuthInfo> for AuthInfo {
    type Error = CosmosError;

    fn try_from(proto: proto::cosmos::tx::v1beta1::AuthInfo) -> Result<AuthInfo, CosmosError> {
        let mut signer_info_vec: Vec<SignerInfo> = Vec::new();
        for signer_info in proto.signer_infos.iter() {
            let signer_info_inner = SignerInfo::try_from(signer_info)?;
            signer_info_vec.push(signer_info_inner)
        }
        let fee_inner: Option<Fee> = match &proto.fee {
            Some(fee) => Some(Fee::try_from(fee)?),
            None => None,
        };

        Ok(AuthInfo {
            signer_infos: signer_info_vec,
            fee: fee_inner,
        })
    }
}
