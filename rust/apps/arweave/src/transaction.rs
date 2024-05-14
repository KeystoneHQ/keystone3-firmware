use crate::deep_hash::deep_hash;
use crate::errors::ArweaveError;

use alloc::string::{String, ToString};
use alloc::vec::Vec;
use core::fmt;
use core::str::FromStr;
use serde::{de, Deserialize, Deserializer, Serialize, Serializer};

use third_party::base64;

/// Transaction data structure per [Arweave transaction spec](https://docs.arweave.org/developers/server/http-api#transaction-format).
#[derive(Serialize, Deserialize, Debug, Default, PartialEq)]
pub struct Transaction {
    pub format: u8,
    #[serde(skip)]
    pub id: Base64,
    pub owner: Base64,
    #[serde(with = "stringify")]
    pub reward: u64,
    pub target: Base64,
    pub last_tx: Base64,
    #[serde(with = "stringify")]
    pub quantity: u64,
    pub tags: Vec<Tag<Base64>>,
    #[serde(skip)]
    pub data: Base64,
    #[serde(with = "stringify")]
    pub data_size: u64,
    pub data_root: Base64,
    #[serde(skip)]
    pub signature: Base64,
    #[serde(skip)]
    pub signature_data: Vec<u8>,
}
impl Transaction {
    pub fn deep_hash(self) -> Result<[u8; 48], ArweaveError> {
        let deep_hash_item: DeepHashItem = self.to_deep_hash_item()?;
        deep_hash(deep_hash_item)
    }
}

pub mod stringify {
    use alloc::string::String;
    use core::fmt;
    use core::str::FromStr;
    use serde::{de::Error as _, Deserialize, Deserializer, Serialize, Serializer};

    pub fn deserialize<'de, D, T>(deserializer: D) -> Result<T, D::Error>
    where
        D: Deserializer<'de>,
        T: FromStr,
        <T as FromStr>::Err: fmt::Display,
    {
        String::deserialize(deserializer)?
            .parse::<T>()
            .map_err(|e| D::Error::custom(format!("{}", e)))
    }

    pub fn serialize<S, T>(value: &T, serializer: S) -> Result<S::Ok, S::Error>
    where
        S: Serializer,
        T: fmt::Display,
    {
        format!("{}", value).serialize(serializer)
    }
}

pub trait ToItems<'a, T> {
    fn to_deep_hash_item(&'a self) -> Result<DeepHashItem, ArweaveError>;
}

impl<'a> ToItems<'a, Transaction> for Transaction {
    fn to_deep_hash_item(&'a self) -> Result<DeepHashItem, ArweaveError> {
        match &self.format {
            2 => {
                let mut children: Vec<DeepHashItem> = vec![
                    self.format.to_string().as_bytes(),
                    &self.owner.0,
                    &self.target.0,
                    self.quantity.to_string().as_bytes(),
                    self.reward.to_string().as_bytes(),
                    &self.last_tx.0,
                ]
                .into_iter()
                .map(DeepHashItem::from_item)
                .collect();
                children.push(self.tags.to_deep_hash_item()?);
                children.push(DeepHashItem::from_item(
                    self.data_size.to_string().as_bytes(),
                ));
                children.push(DeepHashItem::from_item(&self.data_root.0));

                Ok(DeepHashItem::from_children(children))
            }
            _ => unreachable!(),
        }
    }
}

#[derive(Serialize, Deserialize, Debug, Clone, PartialEq)]
pub struct Tag<T> {
    pub name: T,
    pub value: T,
}

/// Implemented to create [`Tag`]s from utf-8 strings.
pub trait FromUtf8Strs<T> {
    fn from_utf8_strs(name: &str, value: &str) -> Result<T, ArweaveError>;
}

impl FromUtf8Strs<Tag<Base64>> for Tag<Base64> {
    fn from_utf8_strs(name: &str, value: &str) -> Result<Self, ArweaveError> {
        let b64_name = Base64::from_utf8_str(name)?;
        let b64_value = Base64::from_utf8_str(value)?;

        Ok(Self {
            name: b64_name,
            value: b64_value,
        })
    }
}

impl FromUtf8Strs<Tag<String>> for Tag<String> {
    fn from_utf8_strs(name: &str, value: &str) -> Result<Self, ArweaveError> {
        let name = String::from(name);
        let value = String::from(value);

        Ok(Self { name, value })
    }
}

impl<'a> ToItems<'a, Vec<Tag<Base64>>> for Vec<Tag<Base64>> {
    fn to_deep_hash_item(&'a self) -> Result<DeepHashItem, ArweaveError> {
        if self.len() > 0 {
            Ok(DeepHashItem::List(
                self.iter()
                    .map(|t| t.to_deep_hash_item().unwrap())
                    .collect(),
            ))
        } else {
            Ok(DeepHashItem::List(vec![]))
        }
    }
}

impl<'a> ToItems<'a, Tag<Base64>> for Tag<Base64> {
    fn to_deep_hash_item(&'a self) -> Result<DeepHashItem, ArweaveError> {
        Ok(DeepHashItem::List(vec![
            DeepHashItem::Blob(self.name.0.to_vec()),
            DeepHashItem::Blob(self.value.0.to_vec()),
        ]))
    }
}

/// A struct of [`Vec<u8>`] used for all data and address fields.
#[derive(Debug, Clone, PartialEq)]
pub struct Base64(pub Vec<u8>);

impl Default for Base64 {
    fn default() -> Self {
        Base64(vec![])
    }
}

impl fmt::Display for Base64 {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        let string = &base64::display::Base64Display::with_config(&self.0, base64::URL_SAFE_NO_PAD);
        write!(f, "{}", string)
    }
}

/// Converts a base64url encoded string to a Base64 struct.
impl FromStr for Base64 {
    type Err = base64::DecodeError;
    fn from_str(str: &str) -> Result<Self, Self::Err> {
        let result = base64::decode_config(str, base64::URL_SAFE_NO_PAD)?;
        Ok(Self(result))
    }
}

impl Base64 {
    pub fn from_utf8_str(str: &str) -> Result<Self, ArweaveError> {
        Ok(Self(str.as_bytes().to_vec()))
    }
}

impl Serialize for Base64 {
    fn serialize<S: Serializer>(&self, serializer: S) -> Result<S::Ok, S::Error> {
        serializer.collect_str(&format!("{}", &self))
    }
}

impl<'de> Deserialize<'de> for Base64 {
    fn deserialize<D: Deserializer<'de>>(deserializer: D) -> Result<Self, D::Error> {
        struct Vis;
        impl serde::de::Visitor<'_> for Vis {
            type Value = Base64;

            fn expecting(&self, formatter: &mut fmt::Formatter) -> fmt::Result {
                formatter.write_str("a base64 string")
            }

            fn visit_str<E: de::Error>(self, v: &str) -> Result<Self::Value, E> {
                base64::decode_config(v, base64::URL_SAFE_NO_PAD)
                    .map(Base64)
                    .map_err(|_| de::Error::custom("failed to decode base64 string"))
            }
        }
        deserializer.deserialize_str(Vis)
    }
}

/// Recursive data structure that facilitates [`crate::crypto::Provider::deep_hash`] accepting nested
/// arrays of arbitrary depth as an argument with a single type.
#[derive(Debug, Clone, PartialEq, Serialize, Deserialize)]
pub enum DeepHashItem {
    Blob(Vec<u8>),
    List(Vec<DeepHashItem>),
}

impl DeepHashItem {
    pub fn from_item(item: &[u8]) -> DeepHashItem {
        Self::Blob(item.to_vec())
    }
    pub fn from_children(children: Vec<DeepHashItem>) -> DeepHashItem {
        Self::List(children)
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use third_party::serde_json;

    fn base64_encode(data: &[u8]) -> String {
        base64::encode_config(data, base64::URL_SAFE_NO_PAD)
    }

    #[test]
    fn test_transaction_serialization() {
        let transaction = Transaction {
            format: 2,
            id: Base64(vec![1, 2, 3]),
            owner: Base64(vec![4, 5, 6]),
            reward: 5000,
            target: Base64(vec![7, 8, 9]),
            last_tx: Base64(vec![10, 11, 12]),
            quantity: 1000,
            tags: vec![Tag {
                name: Base64(vec![13, 14, 15]),
                value: Base64(vec![16, 17, 18]),
            }],
            data: Base64(vec![19, 20, 21]),
            data_size: 3,
            data_root: Base64(vec![22, 23, 24]),
            signature: Base64(vec![25, 26, 27]),
            signature_data: vec![28, 29, 30],
        };

        let json = serde_json::to_string(&transaction).unwrap();
        let expected_json = format!(
            r#"{{"format":2,"owner":"{}","reward":"5000","target":"{}","last_tx":"{}","quantity":"1000","tags":[{{"name":"{}","value":"{}"}}],"data_size":"3","data_root":"{}"}}"#,
            base64_encode(&[4, 5, 6]),
            base64_encode(&[7, 8, 9]),
            base64_encode(&[10, 11, 12]),
            base64_encode(&[13, 14, 15]),
            base64_encode(&[16, 17, 18]),
            base64_encode(&[22, 23, 24])
        );

        assert_eq!(json, expected_json);
    }

    #[test]
    fn test_base64_from_str() {
        let data = "SGVsbG8gd29ybGQ";
        let base64 = Base64::from_str(data).unwrap();
        assert_eq!(base64, Base64("Hello world".as_bytes().to_vec()));
    }

    #[test]
    fn test_base64_to_string() {
        let base64 = Base64("Hello world".as_bytes().to_vec());
        assert_eq!(base64.to_string(), "SGVsbG8gd29ybGQ");
    }
}
