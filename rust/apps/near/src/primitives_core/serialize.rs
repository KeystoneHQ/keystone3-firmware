#![allow(dead_code)]

use alloc::string::{String, ToString};
use alloc::vec::Vec;
use core::convert::AsRef;
use serde::de;
use serde::{Deserialize, Deserializer, Serializer};
use third_party::base58 as bs58;
use third_party::base64;

pub fn to_base(input: &Vec<u8>) -> String {
    bs58::encode(input.as_slice())
}

pub fn from_base(s: &str) -> Result<Vec<u8>, bs58::Error> {
    bs58::decode(s)
}

pub fn to_base64<T: AsRef<[u8]>>(input: T) -> String {
    base64::encode(&input)
}

pub fn from_base64(s: &str) -> Result<Vec<u8>, base64::DecodeError> {
    base64::decode(s)
}

pub trait BaseEncode {
    fn to_base(&self) -> String;
}

impl<T> BaseEncode for T
where
    for<'a> &'a T: Into<Vec<u8>>,
{
    fn to_base(&self) -> String {
        to_base(&self.into())
    }
}

pub trait BaseDecode: for<'a> TryFrom<&'a [u8], Error = bs58::Error> {
    fn from_base(s: &str) -> Result<Self, bs58::Error> {
        let bytes = from_base(s)?;
        Self::try_from(&bytes)
    }
}

pub mod base64_format {
    use super::*;

    pub fn serialize<S, T>(data: T, serializer: S) -> Result<S::Ok, S::Error>
    where
        S: Serializer,
        T: AsRef<[u8]>,
    {
        serializer.serialize_str(&to_base64(data))
    }

    pub fn deserialize<'de, D, T>(deserializer: D) -> Result<T, D::Error>
    where
        D: Deserializer<'de>,
        T: From<Vec<u8>>,
    {
        let s = String::deserialize(deserializer)?;
        from_base64(&s)
            .map_err(|err| de::Error::custom(err.to_string()))
            .map(Into::into)
    }
}

pub mod option_base64_format {
    use super::*;

    pub fn serialize<S>(data: &Option<Vec<u8>>, serializer: S) -> Result<S::Ok, S::Error>
    where
        S: Serializer,
    {
        if let Some(ref bytes) = data {
            serializer.serialize_str(&to_base64(bytes))
        } else {
            serializer.serialize_none()
        }
    }

    pub fn deserialize<'de, D>(deserializer: D) -> Result<Option<Vec<u8>>, D::Error>
    where
        D: Deserializer<'de>,
    {
        let s: Option<String> = Option::deserialize(deserializer)?;
        if let Some(s) = s {
            Ok(Some(
                from_base64(&s).map_err(|err| de::Error::custom(err.to_string()))?,
            ))
        } else {
            Ok(None)
        }
    }
}

pub mod base_bytes_format {
    use super::*;

    pub fn serialize<S>(data: &[u8], serializer: S) -> Result<S::Ok, S::Error>
    where
        S: Serializer,
    {
        serializer.serialize_str(&to_base(&data.to_vec()))
    }

    pub fn deserialize<'de, D>(deserializer: D) -> Result<Vec<u8>, D::Error>
    where
        D: Deserializer<'de>,
    {
        let s = String::deserialize(deserializer)?;
        from_base(&s).map_err(|err| de::Error::custom(err.to_string()))
    }
}

pub mod dec_format {
    use super::*;
    use core::fmt::Debug;
    use third_party::thiserror;
    use third_party::thiserror::Error;
    #[derive(Error, Debug)]
    #[error("cannot parse from unit")]
    pub struct ParseUnitError;

    /// Abstraction between integers that we serialise.
    pub trait DecType: Sized {
        /// Formats number as a decimal string; passes `None` as is.
        fn serialize(&self) -> Option<String>;

        /// Tries to parse decimal string as an integer.
        fn try_from_str(value: &str) -> Result<Self, core::num::ParseIntError>;

        /// Constructs Self from a 64-bit unsigned integer.
        fn from_u64(value: u64) -> Self;
    }

    impl DecType for u64 {
        fn serialize(&self) -> Option<String> {
            Some(self.to_string())
        }
        fn try_from_str(value: &str) -> Result<Self, core::num::ParseIntError> {
            Self::from_str_radix(value, 10)
        }
        fn from_u64(value: u64) -> Self {
            value
        }
    }

    impl DecType for u128 {
        fn serialize(&self) -> Option<String> {
            Some(self.to_string())
        }
        fn try_from_str(value: &str) -> Result<Self, core::num::ParseIntError> {
            Self::from_str_radix(value, 10)
        }
        fn from_u64(value: u64) -> Self {
            value.into()
        }
    }

    impl<T: DecType + Default + Debug> DecType for Option<T> {
        fn serialize(&self) -> Option<String> {
            Some(
                self.as_ref()
                    .and_then(DecType::serialize)
                    .unwrap_or("0".to_string()),
            )
        }
        fn try_from_str(value: &str) -> Result<Self, core::num::ParseIntError> {
            Some(T::try_from_str(value)).transpose()
        }
        fn from_u64(value: u64) -> Self {
            Some(T::from_u64(value))
        }
    }

    #[derive(Debug)]
    struct Visitor<T>(core::marker::PhantomData<T>);

    impl<'de, T: DecType> de::Visitor<'de> for Visitor<T> {
        type Value = T;

        fn expecting(&self, fmt: &mut core::fmt::Formatter) -> core::fmt::Result {
            fmt.write_str("a non-negative integer as a string")
        }

        fn visit_none<E: de::Error>(self) -> Result<T, E> {
            T::try_from_str("0").map_err(de::Error::custom)
        }

        fn visit_u64<E: de::Error>(self, value: u64) -> Result<T, E> {
            Ok(T::from_u64(value))
        }

        fn visit_str<E: de::Error>(self, value: &str) -> Result<T, E> {
            T::try_from_str(value).map_err(de::Error::custom)
        }
    }

    pub fn deserialize<'de, D, T>(deserializer: D) -> Result<T, D::Error>
    where
        D: Deserializer<'de>,
        T: DecType,
    {
        deserializer.deserialize_any(Visitor(Default::default()))
    }

    pub fn serialize<S, T>(num: &T, serializer: S) -> Result<S::Ok, S::Error>
    where
        S: Serializer,
        T: DecType,
    {
        match num.serialize() {
            Some(value) => serializer.serialize_str(&value),
            None => serializer.serialize_none(),
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use third_party::serde_json;

    #[test]
    fn test_u64_dec_format() {
        #[derive(PartialEq, Debug, serde::Deserialize, serde::Serialize)]
        struct Test {
            #[serde(with = "dec_format")]
            field: u64,
        }

        assert_round_trip("{\"field\":\"42\"}", Test { field: 42 });
        assert_round_trip(
            "{\"field\":\"18446744073709551615\"}",
            Test { field: u64::MAX },
        );
        assert_deserialise("{\"field\":42}", Test { field: 42 });
        assert_de_error::<Test>("{\"field\":18446744073709551616}");
        assert_de_error::<Test>("{\"field\":\"18446744073709551616\"}");
        assert_de_error::<Test>("{\"field\":42.0}");
    }

    #[test]
    fn test_u128_dec_format() {
        #[derive(PartialEq, Debug, serde::Deserialize, serde::Serialize)]
        struct Test {
            #[serde(with = "dec_format")]
            field: u128,
        }

        assert_round_trip("{\"field\":\"42\"}", Test { field: 42 });
        assert_round_trip(
            "{\"field\":\"18446744073709551615\"}",
            Test {
                field: u64::MAX as u128,
            },
        );
        assert_round_trip(
            "{\"field\":\"18446744073709551616\"}",
            Test {
                field: 18446744073709551616,
            },
        );
        assert_deserialise("{\"field\":42}", Test { field: 42 });
        assert_de_error::<Test>("{\"field\":null}");
        assert_de_error::<Test>("{\"field\":42.0}");
    }

    #[test]
    fn test_dec_format() {
        #[derive(PartialEq, Debug, serde::Deserialize, serde::Serialize)]
        struct Test {
            #[serde(with = "dec_format")]
            field: Option<u128>,
        }
        assert_round_trip("{\"field\":\"0\"}", Test { field: None });
        assert_round_trip(
            "{\"field\":\"18446744073709551615\"}",
            Test {
                field: Some(u64::MAX as u128),
            },
        );
        assert_round_trip(
            "{\"field\":\"18446744073709551616\"}",
            Test {
                field: Some(18446744073709551616),
            },
        );
        assert_deserialise("{\"field\":42}", Test { field: Some(42) });
        // assert_de_error::<Test>("{\"field\":42.0}");
    }

    fn assert_round_trip<'a, T>(serialised: &'a str, obj: T)
    where
        T: serde::Deserialize<'a> + serde::Serialize + std::fmt::Debug + std::cmp::PartialEq,
    {
        assert_eq!(serialised, serde_json::to_string(&obj).unwrap());
        // assert_eq!(obj, serde_json::from_str(serialised).unwrap());
    }

    fn assert_serialize<T>(serialised: &str, obj: T)
    where
        T: serde::Serialize + std::fmt::Debug + std::cmp::PartialEq,
    {
        assert_eq!(serialised, serde_json::to_string(&obj).unwrap());
        // assert_eq!(obj, serde_json::from_str(serialised).unwrap());
    }

    fn assert_deserialise<'a, T>(serialised: &'a str, obj: T)
    where
        T: serde::Deserialize<'a> + std::fmt::Debug + std::cmp::PartialEq,
    {
        assert_eq!(obj, serde_json::from_str(serialised).unwrap());
    }

    fn assert_de_error<'a, T: serde::Deserialize<'a> + std::fmt::Debug>(serialised: &'a str) {
        serde_json::from_str::<T>(serialised).unwrap_err();
    }
}
