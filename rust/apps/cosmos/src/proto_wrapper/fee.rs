use crate::cosmos_sdk_proto as proto;
use crate::errors::{CosmosError, Result};
use crate::proto_wrapper::msg::base::Coin;
use crate::transaction::structs::FeeDetail;

use alloc::string::{String, ToString};
use alloc::vec::Vec;
use core::ops::Div;
use serde::Serialize;

pub const ATOM_TO_UATOM_UNIT: f64 = 1000000f64;
pub const DYM_TO_ADYM_UNIT: f64 = 1e18f64;
pub const INJ_TO_INJ_UNIT: f64 = 1e18f64;

#[derive(Debug, Serialize)]
pub struct Fee {
    /// amount is the amount of coins to be paid as a fee
    pub amount: Vec<Coin>,
    /// gas_limit is the maximum gas that can be used in transaction processing
    /// before an out of gas error occurs
    #[serde(rename = "gas")]
    pub gas_limit: u64,
    /// if unset, the first signer is responsible for paying the fees. If set, the specified account must pay the fees.
    /// the payer must be a tx signer (and thus have signed this field in AuthInfo).
    /// setting this field does *not* change the ordering of required signers for the transaction.
    pub payer: String,
    /// if set, the fee payer (either the first signer or the value of the payer field) requests that a fee grant be used
    /// to pay fees instead of the fee payer's own balance. If an appropriate fee grant does not exist or the chain does
    /// not support fee grants, this will fail
    pub granter: String,
}

impl TryFrom<&proto::cosmos::tx::v1beta1::Fee> for Fee {
    type Error = CosmosError;

    fn try_from(proto: &proto::cosmos::tx::v1beta1::Fee) -> Result<Fee> {
        Ok(Fee {
            amount: proto
                .amount
                .iter()
                .map(TryFrom::try_from)
                .collect::<Result<_>>()?,
            gas_limit: proto.gas_limit,
            payer: proto.payer.clone(),
            granter: proto.granter.clone(),
        })
    }
}

pub fn format_amount(amounts: Vec<Coin>) -> String {
    let mut result = vec![];
    amounts.into_iter().for_each(|coin| {
        let amount = format_coin(coin);
        if amount.is_some() {
            result.push(amount.unwrap_or("".to_string()));
        }
    });
    result.join(",")
}

pub fn format_coin(coin: Coin) -> Option<String> {
    if coin.denom.to_lowercase().eq("uatom") {
        if let Ok(value) = coin.amount.as_str().parse::<f64>() {
            return Some(format!("{} {}", value.div(ATOM_TO_UATOM_UNIT), "ATOM"));
        }
    } else if coin.denom.to_lowercase().eq("adym") {
        if let Ok(value) = coin.amount.as_str().parse::<f64>() {
            return Some(format!("{} {}", value.div(DYM_TO_ADYM_UNIT), "DYM"));
        }
    } else if coin.denom.eq("inj") {
        if let Ok(value) = coin.amount.as_str().parse::<f64>() {
            return Some(format!("{} {}", value.div(INJ_TO_INJ_UNIT), "INJ"));
        }
    } else {
        return Some(format!("{} {}", coin.amount, coin.denom));
    }
    return None;
}

pub fn parse_gas_limit(gas: &serde_json::Value) -> Result<f64> {
    if let Some(gas_limit) = gas.as_str() {
        let result = gas_limit.parse::<f64>()?;
        return Ok(result);
    }
    if let Some(gas_limit) = gas.as_f64() {
        return Ok(gas_limit);
    }
    return Err(CosmosError::InvalidData(format!(
        "failed to parse gas {:?}",
        gas
    )));
}

pub fn format_fee_from_value(data: serde_json::Value) -> Result<FeeDetail> {
    let gas_limit = parse_gas_limit(&data["gas"])?;
    let mut max_fee: Vec<String> = Vec::new();
    let mut fee: Vec<String> = Vec::new();
    if let Some(amounts) = data["amount"].as_array() {
        for each in amounts {
            if let (Some(amount), Some(denom)) = (each["amount"].as_str(), each["denom"].as_str()) {
                if let Ok(value) = amount.parse::<f64>() {
                    if denom.to_lowercase().eq("uatom") {
                        fee.push(format!("{} {}", value.div(ATOM_TO_UATOM_UNIT), "ATOM"));
                        max_fee.push(format!(
                            "{} {}",
                            value.div(ATOM_TO_UATOM_UNIT) * gas_limit,
                            "ATOM"
                        ))
                    } else if denom.to_lowercase().eq("adym") {
                        fee.push(format!("{} {}", value.div(DYM_TO_ADYM_UNIT), "DYM"));
                        max_fee.push(format!(
                            "{} {}",
                            value.div(DYM_TO_ADYM_UNIT) * gas_limit,
                            "DYM"
                        ))
                    } else if denom.eq("inj") {
                        fee.push(format!("{} {}", value.div(INJ_TO_INJ_UNIT), "INJ"));
                        max_fee.push(format!(
                            "{} {}",
                            value.div(INJ_TO_INJ_UNIT) * gas_limit,
                            "INJ"
                        ))
                    } else {
                        max_fee.push(format!("{} {}", value * gas_limit, denom));
                        fee.push(format!("{} {}", value, denom));
                    };
                }
            }
        }
        return Ok(FeeDetail {
            max_fee: max_fee.join(","),
            fee: fee.join(","),
            gas_limit: gas_limit.to_string(),
        });
    }
    return Err(CosmosError::InvalidData("can not parse fee".to_string()));
}
