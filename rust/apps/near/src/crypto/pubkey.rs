use alloc::str::FromStr;
use alloc::string::{String, ToString};
use borsh::maybestd::io::{Error, ErrorKind, Write};
use borsh::{BorshDeserialize, BorshSerialize};
use core::cmp::Ordering;
use core::cmp::PartialOrd;
use core::convert::AsRef;
use core::fmt::{Debug, Display, Formatter};
use serde::{Deserialize, Serialize};
use third_party::base58 as bs58;

#[derive(Debug, Copy, Clone, Serialize, Deserialize)]
pub enum KeyType {
    ED25519 = 0,
    SECP256K1 = 1,
}

impl Display for KeyType {
    fn fmt(&self, f: &mut Formatter<'_>) -> Result<(), core::fmt::Error> {
        write!(
            f,
            "{}",
            match self {
                KeyType::ED25519 => "ed25519",
                KeyType::SECP256K1 => "secp256k1",
            },
        )
    }
}

impl FromStr for KeyType {
    type Err = crate::crypto::errors::ParseKeyTypeError;

    fn from_str(value: &str) -> Result<Self, Self::Err> {
        let lowercase_key_type = value.to_ascii_lowercase();
        match lowercase_key_type.as_str() {
            "ed25519" => Ok(KeyType::ED25519),
            "secp256k1" => Ok(KeyType::SECP256K1),
            _ => Err(Self::Err::UnknownKeyType {
                unknown_key_type: lowercase_key_type,
            }),
        }
    }
}

impl TryFrom<u8> for KeyType {
    type Error = crate::crypto::errors::ParseKeyTypeError;

    fn try_from(value: u8) -> Result<Self, Self::Error> {
        match value {
            0 => Ok(KeyType::ED25519),
            1 => Ok(KeyType::SECP256K1),
            unknown_key_type => Err(Self::Error::UnknownKeyType {
                unknown_key_type: unknown_key_type.to_string(),
            }),
        }
    }
}

fn split_key_type_data(
    value: &str,
) -> Result<(KeyType, &str), crate::crypto::errors::ParseKeyTypeError> {
    if let Some(idx) = value.find(':') {
        let (prefix, key_data) = value.split_at(idx);
        Ok((KeyType::from_str(prefix)?, &key_data[1..]))
    } else {
        // If there is no prefix then we Default to ED25519.
        Ok((KeyType::ED25519, value))
    }
}

#[derive(Clone)]
pub struct Secp256K1PublicKey([u8; 64]);

impl From<[u8; 64]> for Secp256K1PublicKey {
    fn from(data: [u8; 64]) -> Self {
        Self(data)
    }
}

impl TryFrom<&[u8]> for Secp256K1PublicKey {
    type Error = crate::crypto::errors::ParseKeyError;

    fn try_from(data: &[u8]) -> Result<Self, Self::Error> {
        // It is suboptimal, but optimized implementation in Rust standard
        // library only implements TryFrom for arrays up to 32 elements at
        // the moment. Once https://github.com/rust-lang/rust/pull/74254
        // lands, we can use the following impl:
        //
        // Ok(Self(data.try_into().map_err(|_| TryFromSliceError(()))?))
        if data.len() != 64 {
            return Err(Self::Error::InvalidLength {
                expected_length: 64,
                received_length: data.len(),
            });
        }
        let mut public_key = Self([0; 64]);
        public_key.0.copy_from_slice(data);
        Ok(public_key)
    }
}

impl AsRef<[u8]> for Secp256K1PublicKey {
    fn as_ref(&self) -> &[u8] {
        &self.0
    }
}

impl Debug for Secp256K1PublicKey {
    fn fmt(&self, f: &mut Formatter<'_>) -> Result<(), core::fmt::Error> {
        write!(f, "{}", bs58::encode(&self.0.to_vec()))
    }
}

impl From<Secp256K1PublicKey> for [u8; 64] {
    fn from(pubkey: Secp256K1PublicKey) -> Self {
        pubkey.0
    }
}

impl PartialEq for Secp256K1PublicKey {
    fn eq(&self, other: &Self) -> bool {
        self.0[..] == other.0[..]
    }
}

impl Eq for Secp256K1PublicKey {}

#[derive(Clone)]
pub struct ED25519PublicKey(pub [u8; ED25519_PUBLIC_KEY_LENGTH]);

impl From<[u8; ED25519_PUBLIC_KEY_LENGTH]> for ED25519PublicKey {
    fn from(data: [u8; ED25519_PUBLIC_KEY_LENGTH]) -> Self {
        Self(data)
    }
}

impl TryFrom<&[u8]> for ED25519PublicKey {
    type Error = crate::crypto::errors::ParseKeyError;

    fn try_from(data: &[u8]) -> Result<Self, Self::Error> {
        Ok(Self(data.try_into().map_err(|_| {
            crate::crypto::errors::ParseKeyError::InvalidLength {
                expected_length: ED25519_PUBLIC_KEY_LENGTH,
                received_length: data.len(),
            }
        })?))
    }
}

impl Debug for ED25519PublicKey {
    fn fmt(&self, f: &mut Formatter<'_>) -> Result<(), core::fmt::Error> {
        write!(f, "{}", bs58::encode(&self.0.to_vec()))
    }
}

impl PartialEq for ED25519PublicKey {
    fn eq(&self, other: &Self) -> bool {
        self.0[..] == other.0[..]
    }
}

impl PartialOrd for ED25519PublicKey {
    fn partial_cmp(&self, other: &Self) -> Option<Ordering> {
        self.0[..].partial_cmp(&other.0[..])
    }
}

impl Eq for ED25519PublicKey {}

impl Ord for ED25519PublicKey {
    fn cmp(&self, other: &Self) -> Ordering {
        self.0[..].cmp(&other.0[..])
    }
}

/// Public key container supporting different curves.
#[derive(Clone, PartialEq, Eq)]
pub enum PublicKey {
    ED25519(ED25519PublicKey),
    /// 512 bit elliptic curve based public-key used in Bitcoin's public-key cryptography.
    SECP256K1(Secp256K1PublicKey),
}

impl PublicKey {
    pub fn len(&self) -> usize {
        match self {
            Self::ED25519(_) => ED25519_PUBLIC_KEY_LENGTH + 1,
            Self::SECP256K1(_) => 65,
        }
    }

    pub fn empty(key_type: KeyType) -> Self {
        match key_type {
            KeyType::ED25519 => {
                PublicKey::ED25519(ED25519PublicKey([0u8; ED25519_PUBLIC_KEY_LENGTH]))
            }
            KeyType::SECP256K1 => PublicKey::SECP256K1(Secp256K1PublicKey([0u8; 64])),
        }
    }

    pub fn key_type(&self) -> KeyType {
        match self {
            Self::ED25519(_) => KeyType::ED25519,
            Self::SECP256K1(_) => KeyType::SECP256K1,
        }
    }

    pub fn key_data(&self) -> &[u8] {
        match self {
            Self::ED25519(key) => key.0.as_slice(),
            Self::SECP256K1(key) => key.0.as_slice(),
        }
    }

    pub fn unwrap_as_ed25519(&self) -> &ED25519PublicKey {
        match self {
            Self::ED25519(key) => key,
            Self::SECP256K1(_) => panic!(),
        }
    }
}

impl Display for PublicKey {
    fn fmt(&self, f: &mut Formatter<'_>) -> Result<(), core::fmt::Error> {
        write!(f, "{}", String::from(self))
    }
}

impl Debug for PublicKey {
    fn fmt(&self, f: &mut Formatter<'_>) -> Result<(), core::fmt::Error> {
        write!(f, "{}", String::from(self))
    }
}

impl BorshSerialize for PublicKey {
    fn serialize<W: Write>(&self, writer: &mut W) -> Result<(), Error> {
        match self {
            PublicKey::ED25519(public_key) => {
                BorshSerialize::serialize(&0u8, writer)?;
                writer.write_all(&public_key.0)?;
            }
            PublicKey::SECP256K1(public_key) => {
                BorshSerialize::serialize(&1u8, writer)?;
                writer.write_all(&public_key.0)?;
            }
        }
        Ok(())
    }
}

impl BorshDeserialize for PublicKey {
    fn deserialize(buf: &mut &[u8]) -> Result<Self, Error> {
        let key_type = KeyType::try_from(<u8 as BorshDeserialize>::deserialize(buf)?)
            .map_err(|err| Error::new(ErrorKind::InvalidData, err.to_string()))?;
        match key_type {
            KeyType::ED25519 => Ok(PublicKey::ED25519(ED25519PublicKey(
                BorshDeserialize::deserialize(buf)?,
            ))),
            KeyType::SECP256K1 => Ok(PublicKey::SECP256K1(Secp256K1PublicKey(
                BorshDeserialize::deserialize(buf)?,
            ))),
        }
    }
}

impl serde::Serialize for PublicKey {
    fn serialize<S>(
        &self,
        serializer: S,
    ) -> Result<<S as serde::Serializer>::Ok, <S as serde::Serializer>::Error>
    where
        S: serde::Serializer,
    {
        serializer.serialize_str(&String::from(self))
    }
}

impl<'de> serde::Deserialize<'de> for PublicKey {
    fn deserialize<D>(deserializer: D) -> Result<Self, <D as serde::Deserializer<'de>>::Error>
    where
        D: serde::Deserializer<'de>,
    {
        let s = <String as serde::Deserialize>::deserialize(deserializer)?;
        s.parse()
            .map_err(|err: crate::crypto::errors::ParseKeyError| {
                serde::de::Error::custom(err.to_string())
            })
    }
}

impl From<&PublicKey> for String {
    fn from(public_key: &PublicKey) -> Self {
        match public_key {
            PublicKey::ED25519(public_key) => {
                format!("{}:{}", KeyType::ED25519, bs58::encode(&public_key.0))
            }
            PublicKey::SECP256K1(public_key) => format!(
                "{}:{}",
                KeyType::SECP256K1,
                bs58::encode(&public_key.0.to_vec())
            ),
        }
    }
}

pub const ED25519_PUBLIC_KEY_LENGTH: usize = 32;

impl FromStr for PublicKey {
    type Err = crate::crypto::errors::ParseKeyError;

    fn from_str(value: &str) -> Result<Self, Self::Err> {
        let (key_type, key_data) = split_key_type_data(value)?;
        let key_data = bs58::decode(key_data).map_err(|err| Self::Err::InvalidData {
            error_message: err.to_string(),
        })?;
        match key_type {
            KeyType::ED25519 => {
                if key_data.len() != ED25519_PUBLIC_KEY_LENGTH {
                    return Err(Self::Err::InvalidLength {
                        expected_length: ED25519_PUBLIC_KEY_LENGTH,
                        received_length: key_data.len(),
                    });
                }
                Ok(PublicKey::ED25519(ED25519PublicKey::try_from(
                    key_data.as_slice(),
                )?))
            }
            KeyType::SECP256K1 => {
                if key_data.len() != 64 {
                    return Err(Self::Err::InvalidLength {
                        expected_length: 64,
                        received_length: key_data.len(),
                    });
                }
                Ok(PublicKey::SECP256K1(Secp256K1PublicKey::try_from(
                    key_data.as_slice(),
                )?))
            }
        }
    }
}

impl From<ED25519PublicKey> for PublicKey {
    fn from(ed25519: ED25519PublicKey) -> Self {
        Self::ED25519(ed25519)
    }
}

impl From<Secp256K1PublicKey> for PublicKey {
    fn from(secp256k1: Secp256K1PublicKey) -> Self {
        Self::SECP256K1(secp256k1)
    }
}
