use crate::constants::NAVAX_TO_AVAX_RATIO;
use crate::errors::{AvaxError, Result};
use crate::get_address;
use alloc::{
    string::{String, ToString},
    vec::Vec,
};
use bytes::{Buf, Bytes};
use core::convert::TryFrom;
#[derive(Debug, Clone)]
pub struct LengthPrefixedVec<T: ParsedSizeAble> {
    len: usize,
    items: Vec<T>,
}

pub trait ParsedSizeAble {
    fn parsed_size(&self) -> usize;
}

impl<T: ParsedSizeAble> LengthPrefixedVec<T> {
    pub fn get_len(&self) -> usize {
        self.len
    }

    pub fn get(&self, index: usize) -> Option<&T> {
        self.items.get(index)
    }

    pub fn parsed_size(&self) -> usize {
        4 + self
            .items
            .iter()
            .fold(0, |acc, item| acc + item.parsed_size())
    }

    pub fn iter(&self) -> impl Iterator<Item = &T> {
        self.items.iter()
    }
}

impl<T> TryFrom<Bytes> for LengthPrefixedVec<T>
where
    T: TryFrom<Bytes, Error = AvaxError> + ParsedSizeAble,
{
    type Error = AvaxError;

    fn try_from(mut bytes: Bytes) -> Result<Self> {
        if bytes.len() < 4 {
            return Err(AvaxError::InvalidHex(
                "Insufficient data for LengthPrefixedVec".to_string(),
            ));
        }

        let len = bytes.get_u32() as usize;

        let mut items = Vec::with_capacity(len);

        for _ in 0..len {
            let item = T::try_from(bytes.clone())?;
            bytes.advance(item.parsed_size());
            items.push(item);
        }

        Ok(LengthPrefixedVec { len, items })
    }
}

pub trait AvaxTxInfo {
    fn get_total_input_amount(&self) -> u64;
    fn get_total_output_amount(&self) -> u64;

    fn get_output_amount(&self, address: String) -> u64 {
        self.get_outputs_addresses()
            .iter()
            .find(|info| {info.address[0] == address})
            .map(|info| info.amount)
            .unwrap_or(0)
    }

    fn get_fee_amount(&self) -> u64 {
        self.get_total_input_amount() - self.get_total_output_amount()
    }
    fn get_outputs_addresses(&self) -> Vec<AvaxFromToInfo>;
    fn get_network(&self) -> Option<String> {
        None
    }
    fn get_network_key(&self) -> String {
        "Network".to_string()
    }

    fn get_subnet_id(&self) -> Option<String> {
        None
    }

    fn get_method_info(&self) -> Option<AvaxMethodInfo> {
        None
    }

    fn get_reward_address(&self) -> Option<String> {
        None
    }
}

pub struct AvaxMethodInfo {
    pub method_key: String,
    pub method: String,
    pub start_time: i64,
    pub end_time: i64,
}

impl AvaxMethodInfo {
    pub fn from_string(method: String) -> Self {
        AvaxMethodInfo {
            method_key: "Method".to_string(),
            method,
            start_time: 0,
            end_time: 0,
        }
    }

    pub fn with_method_key(mut self, method_key: String) -> Self {
        self.method_key = method_key;
        self
    }

    pub fn from(method: String, start_time: i64, end_time: i64) -> Self {
        AvaxMethodInfo {
            method_key: "Method".to_string(),
            method,
            start_time,
            end_time,
        }
    }
}

#[derive(Debug, Clone)]
pub struct AvaxFromToInfo {
    pub amount: u64,
    pub address: Vec<String>,
    pub path_prefix: String,
}

impl AvaxFromToInfo {
    pub fn from(amount: u64, address: Vec<String>, path_prefix: String) -> Self {
        AvaxFromToInfo {
            amount,
            address,
            path_prefix,
        }
    }
}
