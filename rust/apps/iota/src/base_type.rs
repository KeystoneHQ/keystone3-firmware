use serde::{Deserialize, Serialize};
use super::account::AccountAddress;


#[derive(
    Eq,
    PartialEq,
    Ord,
    PartialOrd,
    Copy,
    Clone,
    Hash,
    Default,
    Debug,
    Serialize,
    Deserialize,
)]pub struct SequenceNumber(u64);
#[derive(Clone, Copy, PartialEq, Eq, PartialOrd, Ord, Hash, Serialize, Deserialize)]
pub struct ObjectDigest(Digest);

pub type ObjectRef = (ObjectID, SequenceNumber, ObjectDigest);

#[derive(
    Clone, Copy, Default, PartialEq, Eq, PartialOrd, Ord, Hash, Serialize, Deserialize
)]
pub struct Digest(
    [u8; 32],
);

#[derive(Debug, Eq, PartialEq, Clone, Copy, PartialOrd, Ord, Hash, Serialize, Deserialize)]
pub struct ObjectID(
    AccountAddress,
);

impl ObjectID {
    /// The number of bytes in an address.
    pub const LENGTH: usize = AccountAddress::LENGTH;
}