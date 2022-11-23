use crate::cosmos_sdk_proto::cosmos::tx::v1beta1::TxBody;
use crate::cosmos_sdk_proto::Any;
use crate::proto_wrapper::msg::common::map_messages;
use crate::proto_wrapper::msg::msg_serialize::Msg;
use crate::CosmosError;
use alloc::boxed::Box;
use alloc::string::String;
use alloc::vec::Vec;
use core::fmt::Debug;
use serde::Serialize;

#[derive(Debug, Serialize)]
pub struct Body {
    #[serde(rename = "msgs")]
    pub messages: Vec<Box<dyn Msg>>,

    /// `memo` is any arbitrary memo to be added to the transaction.
    pub memo: String,

    /// `timeout` is the block height after which this transaction will not
    /// be processed by the chain
    pub timeout_height: u64,

    /// `extension_options` are arbitrary options that can be added by chains
    /// when the default options are not sufficient. If any of these are present
    /// and can't be handled, the transaction will be rejected
    #[serde(skip_serializing)]
    pub extension_options: Vec<Any>,

    /// `extension_options` are arbitrary options that can be added by chains
    /// when the default options are not sufficient. If any of these are present
    /// and can't be handled, they will be ignored
    #[serde(skip_serializing)]
    pub non_critical_extension_options: Vec<Any>,
}

impl TryFrom<TxBody> for Body {
    type Error = CosmosError;

    fn try_from(proto: TxBody) -> Result<Body, CosmosError> {
        let message_vec = map_messages(&proto.messages)?;
        Ok(Body {
            messages: message_vec,
            memo: proto.memo,
            timeout_height: proto.timeout_height,
            extension_options: proto.extension_options,
            non_critical_extension_options: proto.non_critical_extension_options,
        })
    }
}
