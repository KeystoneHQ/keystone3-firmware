#![allow(dead_code)]
use alloc::boxed::Box;
use alloc::fmt;
use alloc::string::{String, ToString};
use alloc::vec::Vec;

use crate::primitives_core::serialize::{from_base, to_base};
use borsh::maybestd::io;
use cryptoxide::digest::Digest;
use cryptoxide::sha2;
use serde::{Deserialize, Deserializer, Serialize, Serializer};

#[derive(Copy, Clone, PartialEq, Eq, PartialOrd, Ord)]
pub struct CryptoHash(pub [u8; 32]);

impl CryptoHash {
    pub fn hash_bytes(bytes: &[u8]) -> CryptoHash {
        let mut hasher = sha2::Sha256::new();
        hasher.input(bytes);
        let mut output = [0u8; 32];
        hasher.result(&mut output);
        CryptoHash(output)
    }
}

impl Default for CryptoHash {
    fn default() -> Self {
        CryptoHash(Default::default())
    }
}

impl borsh::BorshSerialize for CryptoHash {
    fn serialize<W: io::Write>(&self, writer: &mut W) -> Result<(), io::Error> {
        writer.write_all(&self.0)?;
        Ok(())
    }
}

impl borsh::BorshDeserialize for CryptoHash {
    fn deserialize(buf: &mut &[u8]) -> Result<Self, io::Error> {
        Ok(CryptoHash(borsh::BorshDeserialize::deserialize(buf)?))
    }
}

impl Serialize for CryptoHash {
    fn serialize<S>(&self, serializer: S) -> Result<<S as Serializer>::Ok, <S as Serializer>::Error>
    where
        S: Serializer,
    {
        serializer.serialize_str(&to_base(&self.0.to_vec()))
    }
}

impl<'de> Deserialize<'de> for CryptoHash {
    fn deserialize<D>(deserializer: D) -> Result<Self, <D as Deserializer<'de>>::Error>
    where
        D: Deserializer<'de>,
    {
        let s = String::deserialize(deserializer)?;
        // base58-encoded string is at most 1.4 longer than the binary sequence, but factor of 2 is
        // good enough to prevent DoS.
        if s.len() > core::mem::size_of::<CryptoHash>() * 2 {
            return Err(serde::de::Error::custom("incorrect length for hash"));
        }
        let base = from_base(&s).map_err(|err| serde::de::Error::custom(err.to_string()))?;
        CryptoHash::try_from(base.as_slice())
            .map_err(|err| serde::de::Error::custom(err.to_string()))
    }
}

impl alloc::str::FromStr for CryptoHash {
    type Err = Box<dyn core::error::Error + Send + Sync>;

    fn from_str(s: &str) -> Result<Self, Self::Err> {
        let bytes = from_base(s).map_err::<Self::Err, _>(|e| e.to_string().into())?;
        Self::try_from(bytes.as_slice())
    }
}

impl TryFrom<&[u8]> for CryptoHash {
    type Error = Box<dyn core::error::Error + Send + Sync>;

    fn try_from(bytes: &[u8]) -> Result<Self, Self::Error> {
        Ok(CryptoHash(bytes.try_into()?))
    }
}

impl From<CryptoHash> for Vec<u8> {
    fn from(hash: CryptoHash) -> Vec<u8> {
        hash.0.to_vec()
    }
}

impl From<&CryptoHash> for Vec<u8> {
    fn from(hash: &CryptoHash) -> Vec<u8> {
        hash.0.to_vec()
    }
}

impl From<CryptoHash> for [u8; 32] {
    fn from(hash: CryptoHash) -> [u8; 32] {
        hash.0
    }
}

impl fmt::Debug for CryptoHash {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "{}", &self.to_string())
    }
}

impl fmt::Display for CryptoHash {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        fmt::Display::fmt(&to_base(&self.0.to_vec()), f)
    }
}

pub fn hash(data: &[u8]) -> CryptoHash {
    CryptoHash::hash_bytes(data)
}

#[cfg(test)]
mod tests {
    use super::*;
    use serde_json;

    #[derive(Deserialize, Serialize)]
    struct Struct {
        hash: CryptoHash,
    }

    #[test]
    fn test_serialize_success() {
        let hash = hash(&[0, 1, 2]);
        let s = Struct { hash };
        let encoded = serde_json::to_string(&s).unwrap();
        assert_eq!(
            encoded,
            "{\"hash\":\"CjNSmWXTWhC3EhRVtqLhRmWMTkRbU96wUACqxMtV1uGf\"}"
        );
    }

    #[test]
    fn test_serialize_default() {
        let s = Struct {
            hash: CryptoHash::default(),
        };
        let encoded = serde_json::to_string(&s).unwrap();
        assert_eq!(encoded, "{\"hash\":\"11111111111111111111111111111111\"}");
    }

    #[test]
    fn test_deserialize_default() {
        let encoded = "{\"hash\":\"11111111111111111111111111111111\"}";
        let decoded: Struct = serde_json::from_str(encoded).unwrap();
        assert_eq!(decoded.hash, CryptoHash::default());
    }

    #[test]
    fn test_deserialize_success() {
        let encoded = "{\"hash\":\"CjNSmWXTWhC3EhRVtqLhRmWMTkRbU96wUACqxMtV1uGf\"}";
        let decoded: Struct = serde_json::from_str(encoded).unwrap();
        assert_eq!(decoded.hash, hash(&[0, 1, 2]));
    }

    #[test]
    fn test_deserialize_not_base58() {
        let encoded = "\"---\"";
        match serde_json::from_str(encoded) {
            Ok(CryptoHash(_)) => assert!(false, "should have failed"),
            Err(_) => (),
        }
    }

    #[test]
    fn test_deserialize_not_crypto_hash() {
        for encoded in &[
            "\"CjNSmWXTWhC3ELhRmWMTkRbU96wUACqxMtV1uGf\"".to_string(),
            "\"\"".to_string(),
            format!("\"{}\"", "1".repeat(31)),
            format!("\"{}\"", "1".repeat(33)),
            format!("\"{}\"", "1".repeat(1000)),
        ] {
            match serde_json::from_str::<CryptoHash>(encoded) {
                Err(e) => if e.to_string() == "could not convert slice to array" {},
                res => assert!(
                    false,
                    "should have failed with incorrect length error: {:?}",
                    res
                ),
            };
        }
    }
}
