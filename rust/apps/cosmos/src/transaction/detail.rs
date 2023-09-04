use crate::errors::{CosmosError, Result};
use crate::proto_wrapper::fee::{format_amount, format_coin};
use crate::proto_wrapper::msg::msg::{
    MsgBeginRedelegate, MsgDelegate, MsgSend, MsgTransfer, MsgUndelegate, MsgVote,
};
pub use crate::transaction::overview::OverviewDelegate as DetailDelegate;
pub use crate::transaction::overview::OverviewMessage as DetailMessage;
pub use crate::transaction::overview::OverviewSend as DetailSend;
pub use crate::transaction::overview::OverviewUndelegate as DetailUndelegate;
pub use crate::transaction::overview::OverviewVote as DetailVote;
pub use crate::transaction::overview::OverviewWithdrawReward as DetailWithdrawReward;
use crate::transaction::structs::FeeDetail;
use alloc::string::{String, ToString};
use alloc::vec::Vec;
use serde;
use serde::Serialize;
use serde_json::{from_value, Value};

use super::overview::MsgSignData;

#[derive(Debug, Clone, Serialize)]
pub struct DetailRedelegate {
    #[serde(rename(serialize = "Method"))]
    pub method: String,
    #[serde(skip_serializing_if = "String::is_empty", rename(serialize = "Value"))]
    pub value: String,
    #[serde(rename(serialize = "To"))]
    pub to: String,
    #[serde(rename(serialize = "New Validator"))]
    pub new_validator: String,
    #[serde(rename(serialize = "Old Validator"))]
    pub old_validator: String,
}

impl TryFrom<MsgBeginRedelegate> for DetailRedelegate {
    type Error = CosmosError;

    fn try_from(data: MsgBeginRedelegate) -> Result<Self> {
        let value = data
            .amount
            .and_then(|coin| Some(format_amount(vec![coin])))
            .unwrap_or("".to_string());
        Ok(Self {
            method: "Re-delegate".to_string(),
            value,
            to: data.delegator_address,
            new_validator: data.validator_dst_address,
            old_validator: data.validator_src_address,
        })
    }
}

#[derive(Debug, Clone, Serialize)]
pub struct DetailTransfer {
    #[serde(rename(serialize = "Method"))]
    pub method: String,
    #[serde(skip_serializing_if = "String::is_empty", rename(serialize = "Value"))]
    pub value: String,
    #[serde(rename(serialize = "From"))]
    pub from: String,
    #[serde(rename(serialize = "To"))]
    pub to: String,
    #[serde(rename(serialize = "Source Channel"))]
    pub source_channel: String,
}

impl TryFrom<MsgTransfer> for DetailTransfer {
    type Error = CosmosError;

    fn try_from(msg: MsgTransfer) -> Result<Self> {
        let value = msg
            .token
            .and_then(|v| format_coin(v))
            .unwrap_or("".to_string());
        Ok(Self {
            method: "IBC Transfer".to_string(),
            value,
            from: msg.sender,
            to: msg.receiver,
            source_channel: msg.source_channel,
        })
    }
}

#[derive(Debug, Clone, Serialize)]
#[serde(untagged)]
pub enum MsgDetail {
    Send(DetailSend),
    Delegate(DetailDelegate),
    Undelegate(DetailUndelegate),
    Redelegate(DetailRedelegate),
    WithdrawReward(DetailWithdrawReward),
    Transfer(DetailTransfer),
    Vote(DetailVote),
    Message(DetailMessage),
}

#[derive(Debug, Clone, Serialize)]
pub struct CommonDetail {
    #[serde(rename(serialize = "Network"))]
    pub network: String,
    #[serde(rename(serialize = "Chain ID"))]
    pub chain_id: String,
    #[serde(skip_serializing_if = "Option::is_none")]
    #[serde(flatten)]
    pub fee: Option<FeeDetail>,
}

#[derive(Debug, Clone, Serialize)]
pub struct DetailUnknown {
    #[serde(rename(serialize = "Network"))]
    pub network: String,
    #[serde(rename(serialize = "Chain ID"))]
    pub chain_id: String,
    #[serde(skip_serializing_if = "Option::is_none")]
    #[serde(flatten)]
    pub fee: Option<FeeDetail>,
    #[serde(rename(serialize = "Message"))]
    pub message: String,
}

impl CommonDetail {
    pub fn to_unknown(&self) -> DetailUnknown {
        DetailUnknown {
            network: self.network.clone(),
            chain_id: self.chain_id.clone(),
            fee: self.fee.clone(),
            message: "Unknown Data".to_string(),
        }
    }
}
#[derive(Debug, Clone, Serialize)]
pub struct CosmosTxDetail {
    pub common: CommonDetail,
    pub kind: Vec<MsgDetail>,
}

impl CosmosTxDetail {
    pub fn from_value(msgs: &Value) -> Result<Vec<MsgDetail>> {
        let mut kind: Vec<MsgDetail> = Vec::new();
        let msg_arr = msgs
            .as_array()
            .ok_or(CosmosError::ParseTxError("empty msg".to_string()))?;
        for each in msg_arr {
            match crate::transaction::utils::detect_msg_type(each["type"].as_str()) {
                "MsgSend" => {
                    let msg = from_value::<MsgSend>(each["value"].clone())?;
                    kind.push(MsgDetail::Send(msg.try_into()?));
                }
                "MsgDelegate" => {
                    let msg = from_value::<MsgDelegate>(each["value"].clone())?;
                    kind.push(MsgDetail::Delegate(msg.try_into()?));
                }
                "MsgUndelegate" => {
                    let msg = from_value::<MsgUndelegate>(each["value"].clone())?;
                    kind.push(MsgDetail::Undelegate(msg.try_into()?));
                }
                "MsgBeginRedelegate" => {
                    let msg = from_value::<MsgBeginRedelegate>(each["value"].clone())?;
                    kind.push(MsgDetail::Redelegate(msg.try_into()?));
                }
                "MsgWithdrawDelegatorReward" | "MsgWithdrawDelegationReward" => {
                    let msg = from_value::<
                        crate::proto_wrapper::msg::msg::MsgWithdrawDelegatorReward,
                    >(each["value"].clone())?;
                    kind.push(MsgDetail::WithdrawReward(msg.try_into()?));
                }
                "MsgTransfer" => {
                    let msg = from_value::<MsgTransfer>(each["value"].clone())?;
                    kind.push(MsgDetail::Transfer(msg.try_into()?));
                }
                "MsgVote" => {
                    let msg = from_value::<MsgVote>(each["value"].clone())?;
                    kind.push(MsgDetail::Vote(msg.try_into()?));
                }
                "MsgSignData" => {
                    let msg = from_value::<MsgSignData>(each["value"].clone())?;
                    kind.push(MsgDetail::Message(msg.try_into()?));
                }
                _ => {}
            };
        }
        Ok(kind)
    }
}
