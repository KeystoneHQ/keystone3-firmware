use crate::cosmos_sdk_proto as proto;
use crate::cosmos_sdk_proto::prost::bytes::Bytes;
use crate::cosmos_sdk_proto::traits::Message;
use crate::proto_wrapper::auth_info::AuthInfo;
use crate::proto_wrapper::body::Body;
use crate::proto_wrapper::fee::Fee;
use crate::proto_wrapper::msg::msg_serialize::Msg;
use crate::{CosmosError, Result};
use alloc::boxed::Box;
use alloc::string::{String, ToString};
use alloc::vec::Vec;
use serde::Serialize;

#[derive(Debug, Serialize)]
pub struct SignDoc {
    pub msgs: Vec<Box<dyn Msg>>,
    pub memo: String,
    pub fee: Option<Fee>,
    pub chain_id: String,
    pub account_number: u64,
}

impl SignDoc {
    fn from(proto: proto::cosmos::tx::v1beta1::SignDoc) -> Result<SignDoc> {
        let tx_body: proto::cosmos::tx::v1beta1::TxBody =
            Message::decode(Bytes::from(proto.body_bytes)).map_err(|e| {
                CosmosError::ParseTxError(format!(
                    "proto TxBody deserialize failed {}",
                    e.to_string()
                ))
            })?;
        let body = Body::try_from(tx_body)?;

        let auth_info: proto::cosmos::tx::v1beta1::AuthInfo =
            Message::decode(Bytes::from(proto.auth_info_bytes)).map_err(|e| {
                CosmosError::ParseTxError(format!(
                    "proto AuthInfo deserialize failed {}",
                    e.to_string()
                ))
            })?;
        let auth_info = AuthInfo::try_from(auth_info)?;

        Ok(SignDoc {
            msgs: body.messages,
            memo: body.memo,
            fee: auth_info.fee,
            chain_id: proto.chain_id,
            account_number: proto.account_number,
        })
    }

    pub fn parse(data: &Vec<u8>) -> Result<SignDoc> {
        let proto_sign_doc: proto::cosmos::tx::v1beta1::SignDoc =
            Message::decode(Bytes::from(data.clone())).map_err(|e| {
                CosmosError::ParseTxError(format!(
                    "proto SignDoc deserialize failed {}",
                    e.to_string()
                ))
            })?;
        SignDoc::from(proto_sign_doc)
    }
}
