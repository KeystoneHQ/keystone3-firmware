use crate::cosmos_sdk_proto as proto;
use crate::cosmos_sdk_proto::traits::Message;
use crate::proto_wrapper::mode_info::ModeInfo;
use crate::proto_wrapper::msg::base::Any;
use crate::proto_wrapper::serde_helper::base64_format;
use crate::CosmosError;
use alloc::string::{String, ToString};
use alloc::vec::Vec;
use serde::Serialize;

#[derive(Debug, Serialize)]
pub struct SignerInfo {
    /// public_key is the public key of the signer. It is optional for accounts
    /// that already exist in state. If unset, the verifier can use the required \
    /// signer address for this position and lookup the public key.
    pub public_key: Option<SignerPublicKey>,
    /// mode_info describes the signing mode of the signer and is a nested
    /// structure to support nested multisig pubkey's
    pub mode_info: Option<ModeInfo>,
    /// sequence is the sequence of the account, which describes the
    /// number of committed transactions signed by a given address. It is used to
    /// prevent replay attacks.
    pub sequence: u64,
}

#[derive(Debug, Serialize)]
pub enum SignerPublicKey {
    /// Single singer.
    Single(PublicKey),

    /// Other key types beyond the ones provided above.
    Any(Any),
}

impl From<PublicKey> for SignerPublicKey {
    fn from(pk: PublicKey) -> SignerPublicKey {
        Self::Single(pk)
    }
}

impl TryFrom<Any> for SignerPublicKey {
    type Error = CosmosError;

    fn try_from(any: Any) -> Result<SignerPublicKey, CosmosError> {
        match any.type_url.as_str() {
            PublicKey::ED25519_TYPE_URL | PublicKey::SECP256K1_TYPE_URL => {
                PublicKey::try_from(&any).map(Into::into)
            }
            _ => Ok(Self::Any(any)),
        }
    }
}

#[derive(Debug, Serialize)]
pub struct PublicKey {
    pub type_url: String,

    #[serde(with = "base64_format")]
    pub key: Vec<u8>,
}

impl PublicKey {
    /// Protobuf [`Any`] type URL for Ed25519 public keys
    pub const ED25519_TYPE_URL: &'static str = "/cosmos.crypto.ed25519.PubKey";

    /// Protobuf [`Any`] type URL for secp256k1 public keys
    pub const SECP256K1_TYPE_URL: &'static str = "/cosmos.crypto.secp256k1.PubKey";
}

impl TryFrom<&Any> for PublicKey {
    type Error = CosmosError;

    fn try_from(any: &Any) -> Result<PublicKey, CosmosError> {
        match any.type_url.as_str() {
            Self::ED25519_TYPE_URL => {
                let pub_key: proto::cosmos::crypto::ed25519::PubKey = Message::decode(&*any.value)
                    .map_err(|err| {
                        CosmosError::ParseTxError(format!(
                            "proto ed25519::PubKey deserialize failed {}",
                            err.to_string()
                        ))
                    })?;
                Ok(PublicKey {
                    type_url: Self::ED25519_TYPE_URL.to_string(),
                    key: pub_key.key,
                })
            }
            Self::SECP256K1_TYPE_URL => {
                let pub_key: proto::cosmos::crypto::secp256k1::PubKey =
                    Message::decode(&*any.value).map_err(|err| {
                        CosmosError::ParseTxError(format!(
                            "proto secp256k1::PubKey deserialize failed {}",
                            err.to_string()
                        ))
                    })?;
                Ok(PublicKey {
                    type_url: Self::SECP256K1_TYPE_URL.to_string(),
                    key: pub_key.key,
                })
            }
            other => Err(CosmosError::ParseTxError(format!(
                "{} is not supported!!!",
                other.to_string()
            ))),
        }
    }
}

impl TryFrom<&proto::cosmos::tx::v1beta1::SignerInfo> for SignerInfo {
    type Error = CosmosError;

    fn try_from(proto: &proto::cosmos::tx::v1beta1::SignerInfo) -> Result<SignerInfo, CosmosError> {
        let public_key = match &proto.public_key {
            Some(value) => {
                let any = Any {
                    type_url: value.type_url.clone(),
                    value: value.value.clone(),
                };
                Some(any.try_into()?)
            }
            None => None,
        };
        let mode_info = match &proto.mode_info {
            Some(value) => Some(value.try_into()?),
            None => None,
        };

        Ok(SignerInfo {
            public_key,
            mode_info,
            sequence: proto.sequence,
        })
    }
}
