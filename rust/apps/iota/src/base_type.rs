use super::account::AccountAddress;
use serde::{Deserialize, Serialize};

#[derive(
    Eq, PartialEq, Ord, PartialOrd, Copy, Clone, Hash, Default, Debug, Serialize, Deserialize,
)]
pub struct SequenceNumber(pub u64);

#[derive(Debug, Clone, Copy, PartialEq, Eq, PartialOrd, Ord, Hash, Serialize, Deserialize)]

pub struct ObjectDigest(pub Digest);

pub type ObjectRef = (ObjectID, SequenceNumber, ObjectDigest);

#[derive(
    Clone, Copy, Default, PartialEq, Eq, PartialOrd, Ord, Hash, Serialize, Deserialize, Debug,
)]
pub struct Digest(pub [u8; 32]);

#[derive(Debug, Eq, PartialEq, Clone, Copy, PartialOrd, Ord, Hash, Serialize, Deserialize)]
pub struct ObjectID(AccountAddress);

impl ObjectID {
    /// The number of bytes in an address.
    pub const LENGTH: usize = AccountAddress::LENGTH;

    pub const fn new(address: [u8; Self::LENGTH]) -> Self {
        Self(AccountAddress::new(address))
    }
}
