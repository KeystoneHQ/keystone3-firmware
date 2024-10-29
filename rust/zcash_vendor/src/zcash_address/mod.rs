pub mod convert;
pub mod encoding;
pub mod unified;

use alloc::{format, string::String};
pub use convert::{
    ConversionError, ToAddress, TryFromAddress, TryFromRawAddress, UnsupportedAddress,
};
use encoding::ParseError;
use unified::Receiver;

use super::zcash_protocol::{consensus::NetworkType as Network, PoolType};

#[derive(Clone, Debug, PartialEq, Eq, Hash)]
pub struct ZcashAddress {
    net: Network,
    kind: AddressKind,
}

/// Known kinds of Zcash addresses.
#[derive(Clone, Debug, PartialEq, Eq, Hash)]
enum AddressKind {
    Sprout([u8; 64]),
    Sapling([u8; 43]),
    Unified(unified::Address),
    P2pkh([u8; 20]),
    P2sh([u8; 20]),
    Tex([u8; 20]),
}

impl ZcashAddress {
    /// Encodes this Zcash address in its canonical string representation.
    ///
    /// This provides the encoded string representation of the address as defined by the
    /// [Zcash protocol specification](https://zips.z.cash/protocol.pdf) and/or
    /// [ZIP 316](https://zips.z.cash/zip-0316). The [`Display` implementation] can also
    /// be used to produce this encoding using [`address.to_string()`].
    ///
    /// [`Display` implementation]: std::fmt::Display
    /// [`address.to_string()`]: std::string::ToString
    pub fn encode(&self) -> String {
        format!("{}", self)
    }

    /// Attempts to parse the given string as a Zcash address.
    ///
    /// This simply calls [`s.parse()`], leveraging the [`FromStr` implementation].
    ///
    /// [`s.parse()`]: std::primitive::str::parse
    /// [`FromStr` implementation]: ZcashAddress#impl-FromStr
    ///
    /// # Errors
    ///
    /// - If the parser can detect that the string _must_ contain an address encoding used
    ///   by Zcash, [`ParseError::InvalidEncoding`] will be returned if any subsequent
    ///   part of that encoding is invalid.
    ///
    /// - In all other cases, [`ParseError::NotZcash`] will be returned on failure.
    ///
    /// # Examples
    ///
    /// ```
    /// use zcash_address::ZcashAddress;
    ///
    /// let encoded = "zs1z7rejlpsa98s2rrrfkwmaxu53e4ue0ulcrw0h4x5g8jl04tak0d3mm47vdtahatqrlkngh9sly";
    /// let addr = ZcashAddress::try_from_encoded(&encoded);
    /// assert_eq!(encoded.parse(), addr);
    /// ```
    pub fn try_from_encoded(s: &str) -> Result<Self, ParseError> {
        s.parse()
    }

    /// Converts this address into another type.
    ///
    /// `convert` can convert into any type that implements the [`TryFromAddress`] trait.
    /// This enables `ZcashAddress` to be used as a common parsing and serialization
    /// interface for Zcash addresses, while delegating operations on those addresses
    /// (such as constructing transactions) to downstream crates.
    ///
    /// If you want to get the encoded string for this address, use the [`encode`]
    /// method or the [`Display` implementation] via [`address.to_string()`] instead.
    ///
    /// [`encode`]: Self::encode
    /// [`Display` implementation]: std::fmt::Display
    /// [`address.to_string()`]: std::string::ToString
    pub fn convert<T: TryFromAddress>(self) -> Result<T, ConversionError<T::Error>> {
        match self.kind {
            AddressKind::Sprout(data) => T::try_from_sprout(self.net, data),
            AddressKind::Sapling(data) => T::try_from_sapling(self.net, data),
            AddressKind::Unified(data) => T::try_from_unified(self.net, data),
            AddressKind::P2pkh(data) => T::try_from_transparent_p2pkh(self.net, data),
            AddressKind::P2sh(data) => T::try_from_transparent_p2sh(self.net, data),
            AddressKind::Tex(data) => T::try_from_tex(self.net, data),
        }
    }

    /// Converts this address into another type, if it matches the expected network.
    ///
    /// `convert_if_network` can convert into any type that implements the
    /// [`TryFromRawAddress`] trait. This enables `ZcashAddress` to be used as a common
    /// parsing and serialization interface for Zcash addresses, while delegating
    /// operations on those addresses (such as constructing transactions) to downstream
    /// crates.
    ///
    /// If you want to get the encoded string for this address, use the [`encode`]
    /// method or the [`Display` implementation] via [`address.to_string()`] instead.
    ///
    /// [`encode`]: Self::encode
    /// [`Display` implementation]: std::fmt::Display
    /// [`address.to_string()`]: std::string::ToString
    pub fn convert_if_network<T: TryFromRawAddress>(
        self,
        net: Network,
    ) -> Result<T, ConversionError<T::Error>> {
        let network_matches = self.net == net;
        // The Sprout and transparent address encodings use the same prefix for testnet
        // and regtest, so we need to allow parsing testnet addresses as regtest.
        let regtest_exception =
            network_matches || (self.net == Network::Test && net == Network::Regtest);

        match self.kind {
            AddressKind::Sprout(data) if regtest_exception => T::try_from_raw_sprout(data),
            AddressKind::Sapling(data) if network_matches => T::try_from_raw_sapling(data),
            AddressKind::Unified(data) if network_matches => T::try_from_raw_unified(data),
            AddressKind::P2pkh(data) if regtest_exception => {
                T::try_from_raw_transparent_p2pkh(data)
            }
            AddressKind::P2sh(data) if regtest_exception => T::try_from_raw_transparent_p2sh(data),
            AddressKind::Tex(data) if network_matches => T::try_from_raw_tex(data),
            _ => Err(ConversionError::IncorrectNetwork {
                expected: net,
                actual: self.net,
            }),
        }
    }

    /// Returns whether this address has the ability to receive transfers of the given pool type.
    pub fn can_receive_as(&self, pool_type: PoolType) -> bool {
        use AddressKind::*;
        match &self.kind {
            Sprout(_) => false,
            Sapling(_) => pool_type == PoolType::SAPLING,
            Unified(addr) => addr.has_receiver_of_type(pool_type),
            P2pkh(_) | P2sh(_) | Tex(_) => pool_type == PoolType::TRANSPARENT,
        }
    }

    /// Returns whether this address can receive a memo.
    pub fn can_receive_memo(&self) -> bool {
        use AddressKind::*;
        match &self.kind {
            Sprout(_) | Sapling(_) => true,
            Unified(addr) => addr.can_receive_memo(),
            P2pkh(_) | P2sh(_) | Tex(_) => false,
        }
    }

    /// Returns whether or not this address contains or corresponds to the given unified address
    /// receiver.
    pub fn matches_receiver(&self, receiver: &Receiver) -> bool {
        match (&self.kind, receiver) {
            (AddressKind::Unified(ua), r) => ua.contains_receiver(r),
            (AddressKind::Sapling(d), Receiver::Sapling(r)) => r == d,
            (AddressKind::P2pkh(d), Receiver::P2pkh(r)) => r == d,
            (AddressKind::Tex(d), Receiver::P2pkh(r)) => r == d,
            (AddressKind::P2sh(d), Receiver::P2sh(r)) => r == d,
            _ => false,
        }
    }
}
