use serde::{Deserialize, Serialize};

#[derive(Debug, Ord, PartialOrd, Eq, PartialEq, Hash, Clone, Copy, Serialize, Deserialize)]
pub struct AccountAddress([u8; AccountAddress::LENGTH]);

impl AccountAddress {
    pub const LENGTH: usize = 32;

    pub const fn new(address: [u8; Self::LENGTH]) -> Self {
        Self(address)
    }
}
