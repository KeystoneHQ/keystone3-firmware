use crate::Result as CosmosResult;
use alloc::string::ToString;
use core::fmt::Debug;
use serde::ser::Error;
use serde::{Serialize, Serializer};
use serde_json::Value;

pub trait SerializeJson {
    fn to_json(&self) -> CosmosResult<Value>;
}

pub trait Msg: SerializeJson + Debug {}

impl Serialize for dyn Msg {
    fn serialize<S>(&self, serializer: S) -> Result<<S as Serializer>::Ok, <S as Serializer>::Error>
    where
        S: Serializer,
    {
        let json_value = self
            .to_json()
            .map_err(|err| Error::custom(err.to_string()))?;
        json_value.serialize(serializer)
    }
}
