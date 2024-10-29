//! Structs for handling supported address types.

use crate::{
    orchard,
    zcash_address::{self, ToAddress},
    zcash_primitives, zcash_protocol,
};

use alloc::{
    string::{String, ToString},
    vec,
    vec::Vec,
};
use zcash_address::{
    unified::{self, Container, Encoding, Typecode},
    ConversionError, TryFromRawAddress, ZcashAddress,
};
use zcash_primitives::legacy::TransparentAddress;
use zcash_protocol::consensus::{self, NetworkType};

use zcash_protocol::{PoolType, ShieldedProtocol};

/// A Unified Address.
#[derive(Clone, Debug, PartialEq, Eq)]
pub struct UnifiedAddress {
    orchard: Option<orchard::address::Address>,
    transparent: Option<TransparentAddress>,
    unknown: Vec<(u32, Vec<u8>)>,
}

impl TryFrom<unified::Address> for UnifiedAddress {
    type Error = &'static str;

    fn try_from(ua: unified::Address) -> Result<Self, Self::Error> {
        let mut orchard = None;
        let mut transparent = None;

        let mut unknown: Vec<(u32, Vec<u8>)> = vec![];

        // We can use as-parsed order here for efficiency, because we're breaking out the
        // receivers we support from the unknown receivers.
        for item in ua.items_as_parsed() {
            match item {
                unified::Receiver::Orchard(data) => {
                    orchard = Some(
                        Option::from(orchard::Address::from_raw_address_bytes(data))
                            .ok_or("Invalid Orchard receiver in Unified Address")?,
                    );
                }

                unified::Receiver::Sapling(data) => {
                    #[cfg(not(feature = "sapling"))]
                    {
                        unknown.push((unified::Typecode::Sapling.into(), data.to_vec()));
                    }
                }

                unified::Receiver::P2pkh(data) => {
                    transparent = Some(TransparentAddress::PublicKeyHash(*data));
                }

                unified::Receiver::P2sh(data) => {
                    transparent = Some(TransparentAddress::ScriptHash(*data));
                }

                unified::Receiver::Unknown { typecode, data } => {
                    unknown.push((*typecode, data.clone()));
                }
            }
        }

        Ok(Self {
            orchard,
            transparent,
            unknown,
        })
    }
}

impl UnifiedAddress {
    /// Constructs a Unified Address from a given set of receivers.
    ///
    /// Returns `None` if the receivers would produce an invalid Unified Address (namely,
    /// if no shielded receiver is provided).
    pub fn from_receivers(
        orchard: Option<orchard::address::Address>,
        transparent: Option<TransparentAddress>,
        // TODO: Add handling for address metadata items.
    ) -> Option<Self> {
        let has_orchard = orchard.is_some();

        let has_sapling = false;

        if has_orchard || has_sapling {
            Some(Self {
                orchard,
                transparent,
                unknown: vec![],
            })
        } else {
            // UAs require at least one shielded receiver.
            None
        }
    }

    /// Returns whether this address has an Orchard receiver.
    ///
    /// This method is available irrespective of whether the `orchard` feature flag is enabled.
    pub fn has_orchard(&self) -> bool {
        return self.orchard.is_some();
    }

    /// Returns the Orchard receiver within this Unified Address, if any.
    pub fn orchard(&self) -> Option<&orchard::address::Address> {
        self.orchard.as_ref()
    }

    /// Returns whether this address has a Sapling receiver.
    pub fn has_sapling(&self) -> bool {
        return false;
    }

    /// Returns whether this address has a Transparent receiver.
    pub fn has_transparent(&self) -> bool {
        self.transparent.is_some()
    }

    /// Returns the transparent receiver within this Unified Address, if any.
    pub fn transparent(&self) -> Option<&TransparentAddress> {
        self.transparent.as_ref()
    }

    /// Returns the set of unknown receivers of the unified address.
    pub fn unknown(&self) -> &[(u32, Vec<u8>)] {
        &self.unknown
    }

    fn to_address(&self, net: NetworkType) -> ZcashAddress {
        let items = self
            .unknown
            .iter()
            .map(|(typecode, data)| unified::Receiver::Unknown {
                typecode: *typecode,
                data: data.clone(),
            });

        let items = items.chain(
            self.orchard
                .as_ref()
                .map(|addr| addr.to_raw_address_bytes())
                .map(unified::Receiver::Orchard),
        );

        // #[cfg(feature = "sapling")]
        // let items = items.chain(
        //     self.sapling
        //         .as_ref()
        //         .map(|pa| pa.to_bytes())
        //         .map(unified::Receiver::Sapling),
        // );

        let items = items.chain(self.transparent.as_ref().map(|taddr| match taddr {
            TransparentAddress::PublicKeyHash(data) => unified::Receiver::P2pkh(*data),
            TransparentAddress::ScriptHash(data) => unified::Receiver::P2sh(*data),
        }));

        let ua = unified::Address::try_from_items(items.collect())
            .expect("UnifiedAddress should only be constructed safely");
        ZcashAddress::from_unified(net, ua)
    }

    /// Returns the string encoding of this `UnifiedAddress` for the given network.
    pub fn encode<P: consensus::Parameters>(&self, params: &P) -> String {
        self.to_address(params.network_type()).to_string()
    }

    /// Returns the set of receiver typecodes.
    pub fn receiver_types(&self) -> Vec<Typecode> {
        let result = core::iter::empty();
        let result = result.chain(self.orchard.map(|_| Typecode::Orchard));
        let result = result.chain(self.transparent.map(|taddr| match taddr {
            TransparentAddress::PublicKeyHash(_) => Typecode::P2pkh,
            TransparentAddress::ScriptHash(_) => Typecode::P2sh,
        }));
        let result = result.chain(
            self.unknown()
                .iter()
                .map(|(typecode, _)| Typecode::Unknown(*typecode)),
        );
        result.collect()
    }
}

/// An enumeration of protocol-level receiver types.
///
/// While these correspond to unified address receiver types, this is a distinct type because it is
/// used to represent the protocol-level recipient of a transfer, instead of a part of an encoded
/// address.
pub enum Receiver {
    Orchard(orchard::address::Address),
    Transparent(TransparentAddress),
}

impl Receiver {
    /// Converts this receiver to a [`ZcashAddress`] for the given network.
    ///
    /// This conversion function selects the least-capable address format possible; this means that
    /// Orchard receivers will be rendered as Unified addresses, Sapling receivers will be rendered
    /// as bare Sapling addresses, and Transparent receivers will be rendered as taddrs.
    pub fn to_zcash_address(&self, net: NetworkType) -> ZcashAddress {
        match self {
            Receiver::Orchard(addr) => {
                let receiver = unified::Receiver::Orchard(addr.to_raw_address_bytes());
                let ua = unified::Address::try_from_items(vec![receiver])
                    .expect("A unified address may contain a single Orchard receiver.");
                ZcashAddress::from_unified(net, ua)
            }
            Receiver::Transparent(TransparentAddress::PublicKeyHash(data)) => {
                ZcashAddress::from_transparent_p2pkh(net, *data)
            }
            Receiver::Transparent(TransparentAddress::ScriptHash(data)) => {
                ZcashAddress::from_transparent_p2sh(net, *data)
            }
        }
    }

    /// Returns whether or not this receiver corresponds to `addr`, or is contained
    /// in `addr` when the latter is a Unified Address.
    pub fn corresponds(&self, addr: &ZcashAddress) -> bool {
        addr.matches_receiver(&match self {
            Receiver::Orchard(addr) => unified::Receiver::Orchard(addr.to_raw_address_bytes()),
            Receiver::Transparent(TransparentAddress::PublicKeyHash(data)) => {
                unified::Receiver::P2pkh(*data)
            }
            Receiver::Transparent(TransparentAddress::ScriptHash(data)) => {
                unified::Receiver::P2sh(*data)
            }
        })
    }
}

/// An address that funds can be sent to.
#[derive(Debug, PartialEq, Eq, Clone)]
pub enum Address {
    /// A Sapling payment address.
    #[cfg(feature = "sapling")]
    Sapling(PaymentAddress),

    /// A transparent address corresponding to either a public key hash or a script hash.
    Transparent(TransparentAddress),

    /// A [ZIP 316] Unified Address.
    ///
    /// [ZIP 316]: https://zips.z.cash/zip-0316
    Unified(UnifiedAddress),

    /// A [ZIP 320] transparent-source-only P2PKH address, or "TEX address".
    ///
    /// [ZIP 320]: https://zips.z.cash/zip-0320
    Tex([u8; 20]),
}

#[cfg(feature = "sapling")]
impl From<PaymentAddress> for Address {
    fn from(addr: PaymentAddress) -> Self {
        Address::Sapling(addr)
    }
}

impl From<TransparentAddress> for Address {
    fn from(addr: TransparentAddress) -> Self {
        Address::Transparent(addr)
    }
}

impl From<UnifiedAddress> for Address {
    fn from(addr: UnifiedAddress) -> Self {
        Address::Unified(addr)
    }
}

impl TryFromRawAddress for Address {
    type Error = &'static str;

    #[cfg(feature = "sapling")]
    fn try_from_raw_sapling(data: [u8; 43]) -> Result<Self, ConversionError<Self::Error>> {
        let pa = PaymentAddress::from_bytes(&data).ok_or("Invalid Sapling payment address")?;
        Ok(pa.into())
    }

    fn try_from_raw_unified(
        ua: zcash_address::unified::Address,
    ) -> Result<Self, ConversionError<Self::Error>> {
        UnifiedAddress::try_from(ua)
            .map_err(ConversionError::User)
            .map(Address::from)
    }

    fn try_from_raw_transparent_p2pkh(
        data: [u8; 20],
    ) -> Result<Self, ConversionError<Self::Error>> {
        Ok(TransparentAddress::PublicKeyHash(data).into())
    }

    fn try_from_raw_transparent_p2sh(data: [u8; 20]) -> Result<Self, ConversionError<Self::Error>> {
        Ok(TransparentAddress::ScriptHash(data).into())
    }

    fn try_from_raw_tex(data: [u8; 20]) -> Result<Self, ConversionError<Self::Error>> {
        Ok(Address::Tex(data))
    }
}

impl Address {
    /// Attempts to decode an [`Address`] value from its [`ZcashAddress`] encoded representation.
    ///
    /// Returns `None` if any error is encountered in decoding. Use
    /// [`Self::try_from_zcash_address(s.parse()?)?`] if you need detailed error information.
    pub fn decode<P: consensus::Parameters>(params: &P, s: &str) -> Option<Self> {
        Self::try_from_zcash_address(params, s.parse::<ZcashAddress>().ok()?).ok()
    }

    /// Attempts to decode an [`Address`] value from its [`ZcashAddress`] encoded representation.
    pub fn try_from_zcash_address<P: consensus::Parameters>(
        params: &P,
        zaddr: ZcashAddress,
    ) -> Result<Self, ConversionError<&'static str>> {
        zaddr.convert_if_network(params.network_type())
    }

    /// Converts this [`Address`] to its encoded [`ZcashAddress`] representation.
    pub fn to_zcash_address<P: consensus::Parameters>(&self, params: &P) -> ZcashAddress {
        let net = params.network_type();

        match self {
            Address::Transparent(addr) => match addr {
                TransparentAddress::PublicKeyHash(data) => {
                    ZcashAddress::from_transparent_p2pkh(net, *data)
                }
                TransparentAddress::ScriptHash(data) => {
                    ZcashAddress::from_transparent_p2sh(net, *data)
                }
            },
            Address::Unified(ua) => ua.to_address(net),
            Address::Tex(data) => ZcashAddress::from_tex(net, *data),
        }
    }

    /// Converts this [`Address`] to its encoded string representation.
    pub fn encode<P: consensus::Parameters>(&self, params: &P) -> String {
        self.to_zcash_address(params).to_string()
    }

    /// Returns whether or not this [`Address`] can receive funds in the specified pool.
    pub fn can_receive_as(&self, pool_type: PoolType) -> bool {
        match self {
            Address::Transparent(_) | Address::Tex(_) => {
                matches!(pool_type, PoolType::Transparent)
            }
            Address::Unified(ua) => match pool_type {
                PoolType::Transparent => ua.transparent().is_some(),
                PoolType::Shielded(ShieldedProtocol::Sapling) => {
                    return false;
                }
                PoolType::Shielded(ShieldedProtocol::Orchard) => {
                    return ua.orchard().is_some();
                }
            },
        }
    }
}
