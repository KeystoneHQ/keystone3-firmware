// Copyright (c) Mysten Labs, Inc.
// SPDX-License-Identifier: Apache-2.0

use alloc::{str::FromStr, string::String, vec::Vec};
use serde::{Deserialize, Serialize};
use {bcs, hex};

use crate::errors::{Result, SuiError};

/// The version here is to distinguish between signing different versions of the struct
/// or enum. Serialized output between two different versions of the same struct/enum
/// might accidentally (or maliciously on purpose) match.
#[derive(Copy, Clone, PartialEq, Eq, Serialize, Deserialize, Debug, Hash)]
pub enum IntentVersion {
    V0 = 0,
}

impl TryFrom<u8> for IntentVersion {
    fn try_from(value: u8) -> Result<Self> {
        bcs::from_bytes(&[value])
            .map_err(|_| SuiError::InvalidData(String::from("Invalid IntentVersion")))
    }

    type Error = SuiError;
}

/// This enums specifies the application ID. Two intents in two different applications
/// (i.e., Narwhal, Sui, Ethereum etc) should never collide, so that even when a signing
/// key is reused, nobody can take a signature designated for app_1 and present it as a
/// valid signature for an (any) intent in app_2.
#[derive(Copy, Clone, PartialEq, Eq, Serialize, Deserialize, Debug, Hash)]
#[repr(u8)]
pub enum AppId {
    Sui = 0,
    Narwhal = 1,
}

// TODO(joyqvq): Use num_derive
impl TryFrom<u8> for AppId {
    fn try_from(value: u8) -> Result<Self> {
        bcs::from_bytes(&[value]).map_err(|_| SuiError::InvalidData(String::from("Invalid AppId")))
    }

    type Error = SuiError;
}

impl Default for AppId {
    fn default() -> Self {
        Self::Sui
    }
}

/// This enums specifies the intent scope. Two intents for different scope should
/// never collide, so no signature provided for one intent scope can be used for
/// another, even when the serialized data itself may be the same.
#[derive(Copy, Clone, PartialEq, Eq, Serialize, Deserialize, Debug, Hash)]
pub enum IntentScope {
    TransactionData = 0,         // Used for a user signature on a transaction data.
    TransactionEffects = 1,      // Used for an authority signature on transaction effects.
    CheckpointSummary = 2,       // Used for an authority signature on a checkpoint summary.
    PersonalMessage = 3,         // Used for a user signature on a personal message.
    SenderSignedTransaction = 4, // Used for an authority signature on a user signed transaction.
    ProofOfPossession = 5, // Used as a signature representing an authority's proof of possession of its authority protocol key.
    HeaderDigest = 6,      // Used for narwhal authority signature on header digest.
}

impl TryFrom<u8> for IntentScope {
    fn try_from(value: u8) -> Result<Self> {
        bcs::from_bytes(&[value])
            .map_err(|_| SuiError::InvalidData(String::from("Invalid IntentScope")))
    }

    type Error = SuiError;
}

/// An intent is a compact struct serves as the domain separator for a message that a signature commits to.
/// It consists of three parts: [enum IntentScope] (what the type of the message is), [enum IntentVersion], [enum AppId] (what application that the signature refers to).
/// It is used to construct [struct IntentMessage] that what a signature commits to.
///
/// The serialization of an Intent is a 3-byte array where each field is represented by a byte.
#[derive(Debug, PartialEq, Eq, Serialize, Deserialize, Clone, Hash, Copy)]
pub struct Intent {
    pub scope: IntentScope,
    pub version: IntentVersion,
    pub app_id: AppId,
}

impl FromStr for Intent {
    fn from_str(s: &str) -> Result<Self> {
        let s: Vec<u8> =
            hex::decode(s).map_err(|_| SuiError::InvalidData(String::from("Invalid Intent")))?;
        if s.len() != 3 {
            return Err(SuiError::InvalidData(String::from("Invalid Intent")));
        }
        Ok(Self {
            scope: s[0].try_into()?,
            version: s[1].try_into()?,
            app_id: s[2].try_into()?,
        })
    }

    type Err = SuiError;
}

impl Intent {
    pub fn sui_app(scope: IntentScope) -> Self {
        Self {
            version: IntentVersion::V0,
            scope,
            app_id: AppId::Sui,
        }
    }

    pub fn sui_transaction() -> Self {
        Self {
            scope: IntentScope::TransactionData,
            version: IntentVersion::V0,
            app_id: AppId::Sui,
        }
    }

    pub fn narwhal_app(scope: IntentScope) -> Self {
        Self {
            scope,
            version: IntentVersion::V0,
            app_id: AppId::Narwhal,
        }
    }
}

/// Intent Message is a wrapper around a message with its intent. The message can
/// be any type that implements [trait Serialize]. *ALL* signatures in Sui must commits
/// to the intent message, not the message itself. This guarantees any intent
/// message signed in the system cannot collide with another since they are domain
/// separated by intent.
///
/// The serialization of an IntentMessage is compact: it only appends three bytes
/// to the message itself.
#[derive(Debug, PartialEq, Eq, Serialize, Clone, Hash, Deserialize)]
pub struct IntentMessage<T> {
    pub intent: Intent,
    pub value: T,
}

impl<T> IntentMessage<T> {
    pub fn new(intent: Intent, value: T) -> Self {
        Self { intent, value }
    }
}

/// A person message that wraps around a byte array.
#[derive(Debug, PartialEq, Eq, Clone, Serialize, Deserialize)]
pub struct PersonalMessage {
    pub message: Vec<u8>,
}

pub trait SecureIntent: Serialize + private::SealedIntent {}

pub(crate) mod private {
    use super::IntentMessage;

    pub trait SealedIntent {}
    impl<T> SealedIntent for IntentMessage<T> {}
}
