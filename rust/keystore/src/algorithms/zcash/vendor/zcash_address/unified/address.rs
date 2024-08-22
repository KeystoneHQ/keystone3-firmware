// use zcash_protocol::PoolType;

use alloc::vec::Vec;

use crate::algorithms::zcash::vendor::zcash_protocol::PoolType;

use super::{private::SealedItem, ParseError, Typecode};

use core::convert::{TryFrom, TryInto};

/// The set of known Receivers for Unified Addresses.
#[derive(Clone, Debug, PartialEq, Eq, Hash)]
pub enum Receiver {
    Orchard([u8; 43]),
    Sapling([u8; 43]),
    P2pkh([u8; 20]),
    P2sh([u8; 20]),
    Unknown { typecode: u32, data: Vec<u8> },
}

impl TryFrom<(u32, &[u8])> for Receiver {
    type Error = ParseError;

    fn try_from((typecode, addr): (u32, &[u8])) -> Result<Self, Self::Error> {
        match typecode.try_into()? {
            Typecode::P2pkh => addr.try_into().map(Receiver::P2pkh),
            Typecode::P2sh => addr.try_into().map(Receiver::P2sh),
            Typecode::Sapling => addr.try_into().map(Receiver::Sapling),
            Typecode::Orchard => addr.try_into().map(Receiver::Orchard),
            Typecode::Unknown(_) => Ok(Receiver::Unknown {
                typecode,
                data: addr.to_vec(),
            }),
        }
        .map_err(|e| {
            ParseError::InvalidEncoding(format!("Invalid address for typecode {}: {}", typecode, e))
        })
    }
}

impl SealedItem for Receiver {
    fn typecode(&self) -> Typecode {
        match self {
            Receiver::P2pkh(_) => Typecode::P2pkh,
            Receiver::P2sh(_) => Typecode::P2sh,
            Receiver::Sapling(_) => Typecode::Sapling,
            Receiver::Orchard(_) => Typecode::Orchard,
            Receiver::Unknown { typecode, .. } => Typecode::Unknown(*typecode),
        }
    }

    fn data(&self) -> &[u8] {
        match self {
            Receiver::P2pkh(data) => data,
            Receiver::P2sh(data) => data,
            Receiver::Sapling(data) => data,
            Receiver::Orchard(data) => data,
            Receiver::Unknown { data, .. } => data,
        }
    }
}

#[derive(Clone, Debug, PartialEq, Eq, Hash)]
pub struct Address(pub(crate) Vec<Receiver>);

impl Address {
    // Returns whether this address has the ability to receive transfers of the given pool type.
    pub fn has_receiver_of_type(&self, pool_type: PoolType) -> bool {
        self.0.iter().any(|r| match r {
            Receiver::Orchard(_) => pool_type == PoolType::ORCHARD,
            Receiver::Sapling(_) => pool_type == PoolType::SAPLING,
            Receiver::P2pkh(_) | Receiver::P2sh(_) => pool_type == PoolType::TRANSPARENT,
            Receiver::Unknown { .. } => false,
        })
    }

    /// Returns whether this address contains the given receiver.
    pub fn contains_receiver(&self, receiver: &Receiver) -> bool {
        self.0.contains(receiver)
    }

    /// Returns whether this address can receive a memo.
    pub fn can_receive_memo(&self) -> bool {
        self.0
            .iter()
            .any(|r| matches!(r, Receiver::Sapling(_) | Receiver::Orchard(_)))
    }
}

impl super::private::SealedContainer for Address {
    /// The HRP for a Bech32m-encoded mainnet Unified Address.
    ///
    /// Defined in [ZIP 316][zip-0316].
    ///
    /// [zip-0316]: https://zips.z.cash/zip-0316
    const MAINNET: &'static str = "u";

    /// The HRP for a Bech32m-encoded testnet Unified Address.
    ///
    /// Defined in [ZIP 316][zip-0316].
    ///
    /// [zip-0316]: https://zips.z.cash/zip-0316
    const TESTNET: &'static str = "utest";

    /// The HRP for a Bech32m-encoded regtest Unified Address.
    const REGTEST: &'static str = "uregtest";

    fn from_inner(receivers: Vec<Self::Item>) -> Self {
        Self(receivers)
    }
}

impl super::Encoding for Address {}
impl super::Container for Address {
    type Item = Receiver;

    fn items_as_parsed(&self) -> &[Receiver] {
        &self.0
    }
}

