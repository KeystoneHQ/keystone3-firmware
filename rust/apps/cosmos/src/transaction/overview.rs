use crate::errors::{CosmosError, Result};
use crate::proto_wrapper::fee::{format_amount, format_coin};
use crate::proto_wrapper::msg::msg::{
    MsgBeginRedelegate, MsgDelegate, MsgSend, MsgTransfer, MsgUndelegate, MsgVote,
};
use crate::transaction::structs::CosmosTxDisplayType;
use alloc::string::{String, ToString};
use alloc::vec::Vec;
use serde::Serialize;
use serde_json::{from_value, Value};

#[derive(Debug, Clone, Serialize)]
pub struct OverviewSend {
    #[serde(rename(serialize = "Method"))]
    pub method: String,
    #[serde(rename(serialize = "Value"))]
    pub value: String,
    #[serde(rename(serialize = "From"))]
    pub from: String,
    #[serde(rename(serialize = "To"))]
    pub to: String,
}

impl TryFrom<MsgSend> for OverviewSend {
    type Error = CosmosError;

    fn try_from(value: MsgSend) -> Result<Self> {
        Ok(Self {
            method: "Send".to_string(),
            value: format_amount(value.amount),
            from: value.from_address,
            to: value.to_address,
        })
    }
}

#[derive(Debug, Clone, Serialize)]
pub struct OverviewDelegate {
    #[serde(rename(serialize = "Method"))]
    pub method: String,
    #[serde(skip_serializing_if = "String::is_empty", rename(serialize = "Value"))]
    pub value: String,
    #[serde(rename(serialize = "From"))]
    pub from: String,
    #[serde(rename(serialize = "To"))]
    pub to: String,
}

impl TryFrom<MsgDelegate> for OverviewDelegate {
    type Error = CosmosError;

    fn try_from(data: MsgDelegate) -> Result<Self> {
        let value = data
            .amount
            .and_then(|coin| Some(format_amount(vec![coin])))
            .unwrap_or("".to_string());
        Ok(Self {
            method: "Delegate".to_string(),
            value,
            from: data.delegator_address,
            to: data.validator_address,
        })
    }
}

#[derive(Debug, Clone, Serialize)]
pub struct OverviewUndelegate {
    #[serde(rename(serialize = "Method"))]
    pub method: String,
    #[serde(skip_serializing_if = "String::is_empty", rename(serialize = "Value"))]
    pub value: String,
    #[serde(rename(serialize = "To"))]
    pub to: String,
    #[serde(rename(serialize = "Validator"))]
    pub validator: String,
}

impl TryFrom<MsgUndelegate> for OverviewUndelegate {
    type Error = CosmosError;

    fn try_from(data: MsgUndelegate) -> Result<Self> {
        let value = data
            .amount
            .and_then(|coin| Some(format_amount(vec![coin])))
            .unwrap_or("".to_string());
        Ok(Self {
            method: "Undelegate".to_string(),
            value,
            to: data.delegator_address,
            validator: data.validator_address,
        })
    }
}

#[derive(Debug, Clone, Serialize)]
pub struct OverviewRedelegate {
    #[serde(rename(serialize = "Method"))]
    pub method: String,
    #[serde(skip_serializing_if = "String::is_empty", rename(serialize = "Value"))]
    pub value: String,
    #[serde(rename(serialize = "To"))]
    pub to: String,
    #[serde(rename(serialize = "New Validator"))]
    pub new_validator: String,
}

impl TryFrom<MsgBeginRedelegate> for OverviewRedelegate {
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
        })
    }
}

#[derive(Debug, Clone, Serialize)]
pub struct OverviewWithdrawReward {
    #[serde(rename(serialize = "Method"))]
    pub method: String,
    #[serde(rename(serialize = "To"))]
    pub to: String,
    #[serde(rename(serialize = "Validator"))]
    pub validator: String,
}

impl TryFrom<crate::proto_wrapper::msg::msg::MsgWithdrawDelegatorReward>
    for OverviewWithdrawReward
{
    type Error = CosmosError;

    fn try_from(data: crate::proto_wrapper::msg::msg::MsgWithdrawDelegatorReward) -> Result<Self> {
        Ok(Self {
            method: "Withdraw Reward".to_string(),
            to: data.delegator_address,
            validator: data.validator_address,
        })
    }
}

#[derive(Debug, Clone, Serialize)]
pub struct OverviewTransfer {
    #[serde(rename(serialize = "Method"))]
    pub method: String,
    #[serde(skip_serializing_if = "String::is_empty", rename(serialize = "Value"))]
    pub value: String,
    #[serde(rename(serialize = "From"))]
    pub from: String,
    #[serde(rename(serialize = "To"))]
    pub to: String,
}

impl TryFrom<MsgTransfer> for OverviewTransfer {
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
        })
    }
}

#[derive(Debug, Clone, Serialize)]
pub struct OverviewVote {
    #[serde(rename(serialize = "Method"))]
    pub method: String,
    #[serde(rename(serialize = "Voter"))]
    pub voter: String,
    #[serde(rename(serialize = "Proposal"))]
    pub proposal: String,
    #[serde(rename(serialize = "Voted"))]
    pub voted: String,
}

impl TryFrom<MsgVote> for OverviewVote {
    type Error = CosmosError;

    fn try_from(msg: MsgVote) -> Result<Self> {
        let voted = match msg.option {
            0 => "UNSPECIFIED",
            1 => "YES",
            2 => "ABSTAIN",
            3 => "NO",
            4 => "NO_WITH_VETO",
            _ => "",
        }
        .to_string();
        Ok(Self {
            method: "Vote".to_string(),
            voter: msg.voter,
            proposal: format!("#{}", msg.proposal_id),
            voted,
        })
    }
}

#[derive(Debug, Clone, Serialize)]
#[serde(untagged)]
pub enum MsgOverview {
    Send(OverviewSend),
    Delegate(OverviewDelegate),
    Undelegate(OverviewUndelegate),
    Redelegate(OverviewRedelegate),
    WithdrawReward(OverviewWithdrawReward),
    Transfer(OverviewTransfer),
    Vote(OverviewVote),
}

#[derive(Debug, Clone)]
pub struct CommonOverview {
    pub network: String,
}

#[derive(Debug, Clone)]
pub struct CosmosTxOverview {
    pub display_type: CosmosTxDisplayType,
    pub common: CommonOverview,
    pub kind: Vec<MsgOverview>,
}

impl CosmosTxOverview {
    pub fn from_value(msgs: &Value) -> Result<Vec<MsgOverview>> {
        let mut kind: Vec<MsgOverview> = Vec::new();
        let msg_arr = msgs
            .as_array()
            .ok_or(CosmosError::ParseTxError("empty msg".to_string()))?;
        for each in msg_arr {
            match crate::transaction::utils::detect_msg_type(each["type"].as_str()) {
                "MsgSend" => {
                    let msg = from_value::<MsgSend>(each["value"].clone())?;
                    kind.push(MsgOverview::Send(OverviewSend::try_from(msg)?));
                }
                "MsgDelegate" => {
                    let msg = from_value::<MsgDelegate>(each["value"].clone())?;
                    kind.push(MsgOverview::Delegate(OverviewDelegate::try_from(msg)?));
                }
                "MsgUndelegate" => {
                    let msg = from_value::<MsgUndelegate>(each["value"].clone())?;
                    kind.push(MsgOverview::Undelegate(OverviewUndelegate::try_from(msg)?));
                }
                "MsgBeginRedelegate" => {
                    let msg = from_value::<MsgBeginRedelegate>(each["value"].clone())?;
                    kind.push(MsgOverview::Redelegate(OverviewRedelegate::try_from(msg)?));
                }
                "MsgWithdrawDelegatorReward" | "MsgWithdrawDelegationReward" => {
                    let msg = from_value::<
                        crate::proto_wrapper::msg::msg::MsgWithdrawDelegatorReward,
                    >(each["value"].clone())?;
                    kind.push(MsgOverview::WithdrawReward(
                        OverviewWithdrawReward::try_from(msg)?,
                    ));
                }
                "MsgTransfer" => {
                    let msg = from_value::<MsgTransfer>(each["value"].clone())?;
                    kind.push(MsgOverview::Transfer(OverviewTransfer::try_from(msg)?));
                }
                "MsgVote" => {
                    let msg = from_value::<MsgVote>(each["value"].clone())?;
                    kind.push(MsgOverview::Vote(OverviewVote::try_from(msg)?));
                }
                _ => {}
            };
        }
        Ok(kind)
    }
}
