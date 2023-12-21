use crate::cosmos_sdk_proto as proto;
use crate::proto_wrapper::msg::base::Coin;
use crate::proto_wrapper::msg::common::map_messages;
use crate::proto_wrapper::msg::msg_serialize::{Msg, SerializeJson};
use crate::CosmosError;
use alloc::boxed::Box;
use alloc::string::{String, ToString};
use alloc::vec::Vec;
use serde::{Deserialize, Deserializer, Serialize};
use serde_json::{json, Value};

#[derive(Debug, Serialize, Deserialize)]
pub struct NotSupportMessage {
    pub type_url: String,
    pub err: String,
}

impl SerializeJson for NotSupportMessage {
    fn to_json(&self) -> Result<Value, CosmosError> {
        let value = serde_json::to_value(&self).map_err(|err| {
            CosmosError::ParseTxError(format!(
                "NotSupportMessage serialize failed {}",
                err.to_string()
            ))
        })?;
        let msg = json!({
        "type": Value::String(Self::TYPE_URL.to_string()),
        "value": value,
        });
        Ok(msg)
    }
}

impl NotSupportMessage {
    pub const TYPE_URL: &'static str = "/NotSupportMessage";
}

impl Msg for NotSupportMessage {}

#[derive(Debug, Serialize, Deserialize)]
pub struct MsgSend {
    pub from_address: String,
    pub to_address: String,
    pub amount: Vec<Coin>,
}

impl MsgSend {
    pub const TYPE_URL: &'static str = "/cosmos.bank.v1beta1.MsgSend";
}

impl TryFrom<&proto::cosmos::bank::v1beta1::MsgSend> for MsgSend {
    type Error = CosmosError;

    fn try_from(proto: &proto::cosmos::bank::v1beta1::MsgSend) -> Result<MsgSend, CosmosError> {
        Ok(MsgSend {
            from_address: proto.from_address.clone(),
            to_address: proto.to_address.clone(),
            amount: proto
                .amount
                .iter()
                .map(TryFrom::try_from)
                .collect::<Result<_, _>>()?,
        })
    }
}

impl SerializeJson for MsgSend {
    fn to_json(&self) -> Result<Value, CosmosError> {
        let value = serde_json::to_value(&self).map_err(|err| {
            CosmosError::ParseTxError(format!("MsgSend serialize failed {}", err.to_string()))
        })?;
        let msg = json!({
        "type": Value::String(Self::TYPE_URL.to_string()),
        "value": value,
        });
        Ok(msg)
    }
}

impl Msg for MsgSend {}

#[derive(Debug, Serialize, Deserialize)]
pub struct MsgDelegate {
    pub delegator_address: String,
    pub validator_address: String,
    pub amount: Option<Coin>,
}

impl MsgDelegate {
    pub const TYPE_URL: &'static str = "/cosmos.staking.v1beta1.MsgDelegate";
}

impl TryFrom<&proto::cosmos::staking::v1beta1::MsgDelegate> for MsgDelegate {
    type Error = CosmosError;

    fn try_from(
        proto: &proto::cosmos::staking::v1beta1::MsgDelegate,
    ) -> Result<MsgDelegate, CosmosError> {
        let amount: Option<Coin>;
        match &proto.amount {
            Some(coin) => amount = Some(coin.try_into()?),
            None => amount = None,
        }

        Ok(MsgDelegate {
            delegator_address: proto.delegator_address.clone(),
            validator_address: proto.validator_address.clone(),
            amount,
        })
    }
}

impl SerializeJson for MsgDelegate {
    fn to_json(&self) -> Result<Value, CosmosError> {
        let value = serde_json::to_value(&self).map_err(|err| {
            CosmosError::ParseTxError(format!("MsgDelegate serialize failed {}", err.to_string()))
        })?;
        let msg = json!({
        "type": Value::String(Self::TYPE_URL.to_string()),
        "value": value,
        });
        Ok(msg)
    }
}

impl Msg for MsgDelegate {}

#[derive(Debug, Serialize, Deserialize)]
pub struct MsgUndelegate {
    pub delegator_address: String,
    pub validator_address: String,
    pub amount: Option<Coin>,
}

impl MsgUndelegate {
    pub const TYPE_URL: &'static str = "/cosmos.staking.v1beta1.MsgUndelegate";
}

impl TryFrom<&proto::cosmos::staking::v1beta1::MsgUndelegate> for MsgUndelegate {
    type Error = CosmosError;

    fn try_from(
        proto: &proto::cosmos::staking::v1beta1::MsgUndelegate,
    ) -> Result<MsgUndelegate, CosmosError> {
        let amount: Option<Coin>;
        match &proto.amount {
            Some(coin) => amount = Some(coin.try_into()?),
            None => amount = None,
        }

        Ok(MsgUndelegate {
            delegator_address: proto.delegator_address.clone(),
            validator_address: proto.validator_address.clone(),
            amount,
        })
    }
}

impl SerializeJson for MsgUndelegate {
    fn to_json(&self) -> Result<Value, CosmosError> {
        let value = serde_json::to_value(&self).map_err(|err| {
            CosmosError::ParseTxError(format!(
                "MsgUndelegate serialize failed {}",
                err.to_string()
            ))
        })?;
        let msg = json!({
        "type": Value::String(Self::TYPE_URL.to_string()),
        "value": value,
        });
        Ok(msg)
    }
}

impl Msg for MsgUndelegate {}

#[derive(Debug, Serialize, Deserialize)]
pub struct MsgVote {
    #[serde(deserialize_with = "deserialize_string_u64")]
    pub proposal_id: u64,
    pub voter: String,
    pub option: i32,
}

fn deserialize_string_u64<'de, D>(deserializer: D) -> Result<u64, D::Error>
where
    D: Deserializer<'de>,
{
    let value: Value = serde::Deserialize::deserialize(deserializer)?;
    match value.as_u64() {
        Some(data) => Ok(data),
        None => {
            if let Some(data) = value.as_str().and_then(|v| v.parse::<u64>().ok()) {
                Ok(data)
            } else {
                Err(serde::de::Error::custom(format!(
                    "invalid proposal id {:?}",
                    value
                )))
            }
        }
    }
}

fn deserialize_string_option_u64<'de, D>(deserializer: D) -> Result<Option<u64>, D::Error>
where
    D: Deserializer<'de>,
{
    let value: Value = serde::Deserialize::deserialize(deserializer)?;
    match value.as_u64() {
        Some(data) => Ok(Some(data)),
        None => {
            if let Some(data) = value.as_str().and_then(|v| v.parse::<u64>().ok()) {
                Ok(Some(data))
            } else {
                Err(serde::de::Error::custom(format!(
                    "invalid proposal id {:?}",
                    value
                )))
            }
        }
    }
}

impl MsgVote {
    pub const TYPE_URL: &'static str = "/cosmos.gov.v1beta1.MsgVote";
}

impl TryFrom<&proto::cosmos::gov::v1beta1::MsgVote> for MsgVote {
    type Error = CosmosError;

    fn try_from(proto: &proto::cosmos::gov::v1beta1::MsgVote) -> Result<MsgVote, CosmosError> {
        //let option: proto::cosmos::gov::v1beta1::VoteOption = unsafe { ::std::mem::transmute(proto.option) };

        Ok(MsgVote {
            proposal_id: proto.proposal_id,
            voter: proto.voter.clone(),
            option: proto.option,
        })
    }
}

impl SerializeJson for MsgVote {
    fn to_json(&self) -> Result<Value, CosmosError> {
        let value = serde_json::to_value(&self).map_err(|err| {
            CosmosError::ParseTxError(format!("MsgVote serialize failed {}", err.to_string()))
        })?;
        let msg = json!({
        "type": Value::String(Self::TYPE_URL.to_string()),
        "value": value,
        });
        Ok(msg)
    }
}

impl Msg for MsgVote {}

#[derive(Debug, Serialize, Deserialize)]
pub struct Height {
    /// the revision that the client is currently on
    #[serde(default)]
    #[serde(deserialize_with = "deserialize_string_option_u64")]
    pub revision_number: Option<u64>,
    /// the height within the given revision
    #[serde(deserialize_with = "deserialize_string_u64")]
    pub revision_height: u64,
}

#[derive(Debug, Serialize, Deserialize)]
pub struct MsgTransfer {
    /// the port on which the packet will be sent
    pub source_port: String,
    /// the channel by which the packet will be sent
    pub source_channel: String,
    /// the tokens to be transferred
    pub token: Option<Coin>,
    /// the sender address
    pub sender: String,
    /// the recipient address on the destination chain
    pub receiver: String,
    /// Timeout height relative to the current block height.
    /// The timeout is disabled when set to 0.
    pub timeout_height: Option<Height>,
    /// Timeout timestamp in absolute nanoseconds since unix epoch.
    /// The timeout is disabled when set to 0.
    #[serde(default)]
    #[serde(deserialize_with = "deserialize_string_u64")]
    pub timeout_timestamp: u64,
}

impl MsgTransfer {
    pub const TYPE_URL: &'static str = "/ibc.applications.transfer.v1.MsgTransfer";
}

impl TryFrom<&proto::ibc::applications::transfer::v1::MsgTransfer> for MsgTransfer {
    type Error = CosmosError;

    fn try_from(
        proto: &proto::ibc::applications::transfer::v1::MsgTransfer,
    ) -> Result<MsgTransfer, CosmosError> {
        let token: Option<Coin> = match &proto.token {
            Some(coin) => Some(coin.try_into()?),
            None => None,
        };

        let timeout_height: Option<Height> = match &proto.timeout_height {
            Some(height) => Some(Height {
                revision_number: Some(height.revision_number),
                revision_height: height.revision_height,
            }),
            None => None,
        };

        Ok(MsgTransfer {
            source_port: proto.source_port.clone(),
            source_channel: proto.source_channel.clone(),
            token,
            sender: proto.sender.clone(),
            receiver: proto.receiver.clone(),
            timeout_height,
            timeout_timestamp: proto.timeout_timestamp,
        })
    }
}

impl SerializeJson for MsgTransfer {
    fn to_json(&self) -> Result<Value, CosmosError> {
        let value = serde_json::to_value(&self).map_err(|err| {
            CosmosError::ParseTxError(format!("MsgTransfer serialize failed {}", err.to_string()))
        })?;
        let msg = json!({
        "type": Value::String(Self::TYPE_URL.to_string()),
        "value": value,
        });
        Ok(msg)
    }
}

impl Msg for MsgTransfer {}

#[derive(Debug, Serialize, Deserialize)]
pub struct MsgWithdrawDelegatorReward {
    pub delegator_address: String,
    pub validator_address: String,
}

impl MsgWithdrawDelegatorReward {
    pub const TYPE_URL: &'static str = "/cosmos.distribution.v1beta1.MsgWithdrawDelegatorReward";
}

impl TryFrom<&proto::cosmos::distribution::v1beta1::MsgWithdrawDelegatorReward>
    for MsgWithdrawDelegatorReward
{
    type Error = CosmosError;

    fn try_from(
        proto: &proto::cosmos::distribution::v1beta1::MsgWithdrawDelegatorReward,
    ) -> Result<MsgWithdrawDelegatorReward, CosmosError> {
        Ok(MsgWithdrawDelegatorReward {
            delegator_address: proto.delegator_address.clone(),
            validator_address: proto.validator_address.clone(),
        })
    }
}

impl SerializeJson for MsgWithdrawDelegatorReward {
    fn to_json(&self) -> Result<Value, CosmosError> {
        let value = serde_json::to_value(&self).map_err(|err| {
            CosmosError::ParseTxError(format!(
                "MsgWithdrawDelegatorReward serialize failed {}",
                err.to_string()
            ))
        })?;
        let msg = json!({
        "type": Value::String(Self::TYPE_URL.to_string()),
        "value": value,
        });
        Ok(msg)
    }
}

impl Msg for MsgWithdrawDelegatorReward {}

#[derive(Debug, Serialize, Deserialize)]
pub struct MsgUpdateClient {
    pub client_id: String,
    pub signer: String,
}

impl MsgUpdateClient {
    pub const TYPE_URL: &'static str = "/ibc.core.client.v1.MsgUpdateClient";
}

impl TryFrom<&proto::ibc::core::client::v1::MsgUpdateClient> for MsgUpdateClient {
    type Error = CosmosError;

    fn try_from(
        proto: &proto::ibc::core::client::v1::MsgUpdateClient,
    ) -> Result<MsgUpdateClient, CosmosError> {
        Ok(MsgUpdateClient {
            client_id: proto.client_id.clone(),
            signer: proto.signer.clone(),
        })
    }
}

impl SerializeJson for MsgUpdateClient {
    fn to_json(&self) -> Result<Value, CosmosError> {
        let value = serde_json::to_value(&self).map_err(|err| {
            CosmosError::ParseTxError(format!(
                "MsgUpdateClient serialize failed {}",
                err.to_string()
            ))
        })?;
        let msg = json!({
        "type": Value::String(Self::TYPE_URL.to_string()),
        "value": value,
        });
        Ok(msg)
    }
}

impl Msg for MsgUpdateClient {}

#[derive(Debug, Serialize, Deserialize)]
pub struct MsgBeginRedelegate {
    pub delegator_address: String,
    pub validator_src_address: String,
    pub validator_dst_address: String,
    pub amount: Option<Coin>,
}

impl MsgBeginRedelegate {
    pub const TYPE_URL: &'static str = "/cosmos.staking.v1beta1.MsgBeginRedelegate";
}

impl TryFrom<&proto::cosmos::staking::v1beta1::MsgBeginRedelegate> for MsgBeginRedelegate {
    type Error = CosmosError;

    fn try_from(
        proto: &proto::cosmos::staking::v1beta1::MsgBeginRedelegate,
    ) -> Result<MsgBeginRedelegate, CosmosError> {
        let amount: Option<Coin> = match &proto.amount {
            Some(coin) => Some(coin.try_into()?),
            None => None,
        };
        Ok(MsgBeginRedelegate {
            delegator_address: proto.delegator_address.clone(),
            validator_src_address: proto.validator_src_address.clone(),
            validator_dst_address: proto.validator_dst_address.clone(),
            amount,
        })
    }
}

impl SerializeJson for MsgBeginRedelegate {
    fn to_json(&self) -> Result<Value, CosmosError> {
        let value = serde_json::to_value(&self).map_err(|err| {
            CosmosError::ParseTxError(format!(
                "MsgBeginRedelegate serialize failed {}",
                err.to_string()
            ))
        })?;
        let msg = json!({
        "type": Value::String(Self::TYPE_URL.to_string()),
        "value": value,
        });
        Ok(msg)
    }
}

impl Msg for MsgBeginRedelegate {}

#[derive(Debug, Serialize)]
pub struct MsgExec {
    pub grantee: String,
    /// Authorization Msg requests to execute. Each msg must implement Authorization interface
    /// The x/authz will try to find a grant matching (msg.signers\[0\], grantee, MsgTypeURL(msg))
    /// triple and validate it.
    pub msgs: Vec<Box<dyn Msg>>,
}

impl MsgExec {
    pub const TYPE_URL: &'static str = "/cosmos.authz.v1beta1.MsgExec";
}

impl TryFrom<&proto::cosmos::authz::v1beta1::MsgExec> for MsgExec {
    type Error = CosmosError;

    fn try_from(proto: &proto::cosmos::authz::v1beta1::MsgExec) -> Result<MsgExec, CosmosError> {
        let msgs = map_messages(&proto.msgs)?;
        Ok(MsgExec {
            grantee: proto.grantee.clone(),
            msgs,
        })
    }
}

impl SerializeJson for MsgExec {
    fn to_json(&self) -> Result<Value, CosmosError> {
        let value = serde_json::to_value(&self).map_err(|err| {
            CosmosError::ParseTxError(format!("MsgExec serialize failed {}", err.to_string()))
        })?;
        let msg = json!({
        "type": Value::String(Self::TYPE_URL.to_string()),
        "value": value,
        });
        Ok(msg)
    }
}

impl Msg for MsgExec {}

#[derive(Debug, Serialize, Deserialize)]
pub struct Input {
    pub address: String,
    pub coins: Vec<Coin>,
}

impl TryFrom<&proto::cosmos::bank::v1beta1::Input> for Input {
    type Error = CosmosError;

    fn try_from(proto: &proto::cosmos::bank::v1beta1::Input) -> Result<Input, CosmosError> {
        Ok(Input {
            address: proto.address.clone(),
            coins: proto
                .coins
                .iter()
                .map(TryFrom::try_from)
                .collect::<Result<_, _>>()?,
        })
    }
}

#[derive(Debug, Serialize, Deserialize)]
pub struct Output {
    pub address: String,
    pub coins: Vec<Coin>,
}

impl TryFrom<&proto::cosmos::bank::v1beta1::Output> for Output {
    type Error = CosmosError;

    fn try_from(proto: &proto::cosmos::bank::v1beta1::Output) -> Result<Output, CosmosError> {
        Ok(Output {
            address: proto.address.clone(),
            coins: proto
                .coins
                .iter()
                .map(TryFrom::try_from)
                .collect::<Result<_, _>>()?,
        })
    }
}

#[derive(Debug, Serialize, Deserialize)]
pub struct MsgMultiSend {
    pub inputs: Vec<Input>,
    pub outputs: Vec<Output>,
}

impl MsgMultiSend {
    pub const TYPE_URL: &'static str = "/cosmos.bank.v1beta1.MsgMultiSend";
}

impl TryFrom<&proto::cosmos::bank::v1beta1::MsgMultiSend> for MsgMultiSend {
    type Error = CosmosError;

    fn try_from(
        proto: &proto::cosmos::bank::v1beta1::MsgMultiSend,
    ) -> Result<MsgMultiSend, CosmosError> {
        Ok(MsgMultiSend {
            inputs: proto
                .inputs
                .iter()
                .map(TryFrom::try_from)
                .collect::<Result<_, _>>()?,
            outputs: proto
                .outputs
                .iter()
                .map(TryFrom::try_from)
                .collect::<Result<_, _>>()?,
        })
    }
}

impl SerializeJson for MsgMultiSend {
    fn to_json(&self) -> Result<Value, CosmosError> {
        let value = serde_json::to_value(&self).map_err(|err| {
            CosmosError::ParseTxError(format!("MsgMultiSend serialize failed {}", err.to_string()))
        })?;
        let msg = json!({
        "type": Value::String(Self::TYPE_URL.to_string()),
        "value": value,
        });
        Ok(msg)
    }
}

impl Msg for MsgMultiSend {}
