use crate::cosmos_sdk_proto as proto;
use crate::cosmos_sdk_proto::traits::{Message, MessageExt};
use crate::cosmos_sdk_proto::Any;
use crate::proto_wrapper::msg::msg::{
    MsgBeginRedelegate as MsgBeginRedelegateWrapper, MsgDelegate as MsgDelegateWrapper,
    MsgExec as MsgExecWrapper, MsgMultiSend as MsgMultiSendWrapper, MsgSend as MsgSendWrapper,
    MsgTransfer as MsgTransferWrapper, MsgUndelegate as MsgUnDelegateWrapper,
    MsgUpdateClient as MsgUpdateClientWrapper, MsgVote as MsgVoteWrapper,
    MsgWithdrawDelegatorReward as MsgWithdrawDelegatorRewardWrapper, NotSupportMessage,
};
use crate::proto_wrapper::msg::msg_serialize::Msg;
use alloc::boxed::Box;
use alloc::string::ToString;
use alloc::vec::Vec;

use crate::CosmosError;

pub fn map_messages(messages: &Vec<Any>) -> Result<Vec<Box<dyn Msg>>, CosmosError> {
    let mut message_vec: Vec<Box<dyn Msg>> = Vec::new();
    for message in messages.iter() {
        match message.type_url.as_str() {
            MsgSendWrapper::TYPE_URL => {
                let unpacked: proto::cosmos::bank::v1beta1::MsgSend = MessageExt::from_any(message)
                    .map_err(|e| {
                        CosmosError::ParseTxError(format!(
                            "proto MsgSend deserialize failed {}",
                            e.to_string()
                        ))
                    })?;
                let msg_send = MsgSendWrapper::try_from(&unpacked).map_err(|e| {
                    CosmosError::ParseTxError(format!(
                        "proto MsgSend deserialize failed {}",
                        e.to_string()
                    ))
                })?;
                message_vec.push(Box::new(msg_send));
            }
            MsgDelegateWrapper::TYPE_URL => {
                let unpacked: proto::cosmos::staking::v1beta1::MsgDelegate =
                    MessageExt::from_any(message).map_err(|e| {
                        CosmosError::ParseTxError(format!(
                            "proto MsgDelegate deserialize failed {}",
                            e.to_string()
                        ))
                    })?;
                let msg_delegate = MsgDelegateWrapper::try_from(&unpacked).map_err(|e| {
                    CosmosError::ParseTxError(format!(
                        "proto MsgDelegate deserialize failed {}",
                        e.to_string()
                    ))
                })?;
                message_vec.push(Box::new(msg_delegate));
            }
            MsgUnDelegateWrapper::TYPE_URL => {
                let unpacked: proto::cosmos::staking::v1beta1::MsgUndelegate =
                    MessageExt::from_any(message).map_err(|e| {
                        CosmosError::ParseTxError(format!(
                            "proto MsgUndelegate deserialize failed {}",
                            e.to_string()
                        ))
                    })?;
                let msg_undelegate = MsgUnDelegateWrapper::try_from(&unpacked).map_err(|e| {
                    CosmosError::ParseTxError(format!(
                        "proto MsgUndelegate deserialize failed {}",
                        e.to_string()
                    ))
                })?;
                message_vec.push(Box::new(msg_undelegate));
            }
            MsgTransferWrapper::TYPE_URL => {
                let unpacked: proto::ibc::applications::transfer::v1::MsgTransfer =
                    MessageExt::from_any(message).map_err(|e| {
                        CosmosError::ParseTxError(format!(
                            "proto MsgTransfer deserialize failed {}",
                            e.to_string()
                        ))
                    })?;
                let msg_transfer = MsgTransferWrapper::try_from(&unpacked).map_err(|e| {
                    CosmosError::ParseTxError(format!(
                        "proto MsgTransfer deserialize failed {}",
                        e.to_string()
                    ))
                })?;
                message_vec.push(Box::new(msg_transfer));
            }
            MsgVoteWrapper::TYPE_URL => {
                let unpacked: proto::cosmos::gov::v1beta1::MsgVote =
                    proto::cosmos::gov::v1beta1::MsgVote::decode(&*message.value).map_err(|e| {
                        CosmosError::ParseTxError(format!(
                            "proto MsgVote deserialize failed {}",
                            e.to_string()
                        ))
                    })?;
                let msg_vote = MsgVoteWrapper::try_from(&unpacked).map_err(|e| {
                    CosmosError::ParseTxError(format!(
                        "proto MsgVote deserialize failed {}",
                        e.to_string()
                    ))
                })?;
                message_vec.push(Box::new(msg_vote));
            }
            MsgWithdrawDelegatorRewardWrapper::TYPE_URL => {
                let unpacked: proto::cosmos::distribution::v1beta1::MsgWithdrawDelegatorReward =
                    MessageExt::from_any(message).map_err(|e| {
                        CosmosError::ParseTxError(format!(
                            "proto MsgTransfer deserialize failed {}",
                            e.to_string()
                        ))
                    })?;
                let msg_withdraw_reward = MsgWithdrawDelegatorRewardWrapper::try_from(&unpacked)
                    .map_err(|e| {
                        CosmosError::ParseTxError(format!(
                            "proto MsgTransfer deserialize failed {}",
                            e.to_string()
                        ))
                    })?;
                message_vec.push(Box::new(msg_withdraw_reward));
            }
            MsgBeginRedelegateWrapper::TYPE_URL => {
                let unpacked: proto::cosmos::staking::v1beta1::MsgBeginRedelegate =
                    MessageExt::from_any(message).map_err(|e| {
                        CosmosError::ParseTxError(format!(
                            "proto MsgTransfer deserialize failed {}",
                            e.to_string()
                        ))
                    })?;
                let msg_redelegate =
                    MsgBeginRedelegateWrapper::try_from(&unpacked).map_err(|e| {
                        CosmosError::ParseTxError(format!(
                            "proto MsgTransfer deserialize failed {}",
                            e.to_string()
                        ))
                    })?;
                message_vec.push(Box::new(msg_redelegate));
            }
            MsgMultiSendWrapper::TYPE_URL => {
                let unpacked: proto::cosmos::bank::v1beta1::MsgMultiSend =
                    MessageExt::from_any(message).map_err(|e| {
                        CosmosError::ParseTxError(format!(
                            "proto MsgMultiSend deserialize failed {}",
                            e.to_string()
                        ))
                    })?;
                let msg_multi_send = MsgMultiSendWrapper::try_from(&unpacked).map_err(|e| {
                    CosmosError::ParseTxError(format!(
                        "proto MsgMultiSend deserialize failed {}",
                        e.to_string()
                    ))
                })?;
                message_vec.push(Box::new(msg_multi_send));
            }
            MsgUpdateClientWrapper::TYPE_URL => {
                let unpacked: proto::ibc::core::client::v1::MsgUpdateClient =
                    proto::ibc::core::client::v1::MsgUpdateClient::decode(&*message.value)
                        .map_err(|e| {
                            CosmosError::ParseTxError(format!(
                                "proto MsgMultiSend deserialize failed {}",
                                e.to_string()
                            ))
                        })?;
                let msg_update_client =
                    MsgUpdateClientWrapper::try_from(&unpacked).map_err(|e| {
                        CosmosError::ParseTxError(format!(
                            "proto MsgMultiSend deserialize failed {}",
                            e.to_string()
                        ))
                    })?;
                message_vec.push(Box::new(msg_update_client));
            }
            MsgExecWrapper::TYPE_URL => {
                let unpacked: proto::cosmos::authz::v1beta1::MsgExec =
                    proto::cosmos::authz::v1beta1::MsgExec::decode(&*message.value).map_err(
                        |e| {
                            CosmosError::ParseTxError(format!(
                                "proto MsgMultiSend deserialize failed {}",
                                e.to_string()
                            ))
                        },
                    )?;
                let msg_exec = MsgExecWrapper::try_from(&unpacked).map_err(|e| {
                    CosmosError::ParseTxError(format!(
                        "proto MsgMultiSend deserialize failed {}",
                        e.to_string()
                    ))
                })?;
                message_vec.push(Box::new(msg_exec));
            }
            other => {
                message_vec.push(Box::new(NotSupportMessage {
                    type_url: other.to_string(),
                    err: "the type is not support!".to_string(),
                }));
            }
        }
    }
    Ok(message_vec)
}
