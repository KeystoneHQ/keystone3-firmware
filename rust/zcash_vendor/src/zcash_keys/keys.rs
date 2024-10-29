use crate::zcash_protocol::consensus::NetworkConstants;
use core::{
    error,
    fmt::{self, Display},
};

use super::{
    super::zcash_address::unified::{self, Typecode, Ufvk, Uivk},
    address::UnifiedAddress,
};
use crate::zcash_address::unified::{Container, Encoding};
use crate::zcash_primitives::{self, legacy::keys::IncomingViewingKey};
use crate::zcash_protocol;
use crate::zip32::{AccountId, DiversifierIndex};
use alloc::{
    format,
    string::{String, ToString},
    vec,
    vec::Vec,
};
use bip32;
use zcash_protocol::consensus;

use crate::orchard;

use {
    super::super::zcash_primitives::legacy::keys::{self as legacy, NonHardenedChildIndex},
    core::convert::TryInto,
};

use crate::orchard::keys::Scope;

fn to_transparent_child_index(j: DiversifierIndex) -> Option<NonHardenedChildIndex> {
    let (low_4_bytes, rest) = j.as_bytes().split_at(4);
    let transparent_j = u32::from_le_bytes(low_4_bytes.try_into().unwrap());
    if rest.iter().any(|b| b != &0) {
        None
    } else {
        NonHardenedChildIndex::from_index(transparent_j)
    }
}

#[derive(Debug)]
pub enum DerivationError {
    // #[cfg(feature = "orchard")]
    Orchard(orchard::zip32::Error),
    // #[cfg(feature = "transparent-inputs")]
    Transparent(bip32::Error),
}

impl Display for DerivationError {
    fn fmt(&self, _f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self {
            // #[cfg(feature = "orchard")]
            DerivationError::Orchard(e) => write!(_f, "Orchard error: {}", e),
            // #[cfg(feature = "transparent-inputs")]
            DerivationError::Transparent(e) => write!(_f, "Transparent error: {}", e),
            // #[cfg(not(any(feature = "orchard", feature = "transparent-inputs")))]
            // other => {
            //     unreachable!("Unhandled DerivationError variant {:?}", other)
            // }
        }
    }
}

/// A version identifier for the encoding of unified spending keys.
///
/// Each era corresponds to a range of block heights. During an era, the unified spending key
/// parsed from an encoded form tagged with that era's identifier is expected to provide
/// sufficient spending authority to spend any non-Sprout shielded note created in a transaction
/// within the era's block range.
// #[cfg(feature = "unstable")]
// #[derive(Debug, PartialEq, Eq)]
// pub enum Era {
//     /// The Orchard era begins at Orchard activation, and will end if a new pool that requires a
//     /// change to unified spending keys is introduced.
//     Orchard,
// }

/// A type for errors that can occur when decoding keys from their serialized representations.
#[derive(Debug, PartialEq, Eq)]
pub enum DecodingError {
    // #[cfg(feature = "unstable")]
    // ReadError(&'static str),
    // #[cfg(feature = "unstable")]
    // EraInvalid,
    // #[cfg(feature = "unstable")]
    // EraMismatch(Era),
    // #[cfg(feature = "unstable")]
    // TypecodeInvalid,
    // #[cfg(feature = "unstable")]
    // LengthInvalid,
    // #[cfg(feature = "unstable")]
    // LengthMismatch(Typecode, u32),
    // #[cfg(feature = "unstable")]
    // InsufficientData(Typecode),
    /// The key data could not be decoded from its string representation to a valid key.
    KeyDataInvalid(Typecode),
}

impl core::fmt::Display for DecodingError {
    fn fmt(&self, f: &mut core::fmt::Formatter<'_>) -> core::fmt::Result {
        match self {
            // #[cfg(feature = "unstable")]
            // DecodingError::ReadError(s) => write!(f, "Read error: {}", s),
            // #[cfg(feature = "unstable")]
            // DecodingError::EraInvalid => write!(f, "Invalid era"),
            // #[cfg(feature = "unstable")]
            // DecodingError::EraMismatch(e) => write!(f, "Era mismatch: actual {:?}", e),
            // #[cfg(feature = "unstable")]
            // DecodingError::TypecodeInvalid => write!(f, "Invalid typecode"),
            // #[cfg(feature = "unstable")]
            // DecodingError::LengthInvalid => write!(f, "Invalid length"),
            // #[cfg(feature = "unstable")]
            // DecodingError::LengthMismatch(t, l) => {
            //     write!(
            //         f,
            //         "Length mismatch: received {} bytes for typecode {:?}",
            //         l, t
            //     )
            // }
            // #[cfg(feature = "unstable")]
            // DecodingError::InsufficientData(t) => {
            //     write!(f, "Insufficient data for typecode {:?}", t)
            // }
            DecodingError::KeyDataInvalid(t) => write!(f, "Invalid key data for key type {:?}", t),
        }
    }
}

#[cfg(feature = "unstable")]
impl Era {
    /// Returns the unique identifier for the era.
    fn id(&self) -> u32 {
        // We use the consensus branch id of the network upgrade that introduced a
        // new USK format as the identifier for the era.
        match self {
            Era::Orchard => u32::from(BranchId::Nu5),
        }
    }

    fn try_from_id(id: u32) -> Option<Self> {
        BranchId::try_from(id).ok().and_then(|b| match b {
            BranchId::Nu5 => Some(Era::Orchard),
            _ => None,
        })
    }
}

/// A set of spending keys that are all associated with a single ZIP-0032 account identifier.
#[derive(Clone, Debug)]
pub struct UnifiedSpendingKey {
    transparent: legacy::AccountPrivKey,
    orchard: orchard::keys::SpendingKey,
}

impl UnifiedSpendingKey {
    pub fn from_seed<P: consensus::Parameters>(
        _params: &P,
        seed: &[u8],
        _account: AccountId,
    ) -> Result<UnifiedSpendingKey, DerivationError> {
        if seed.len() < 32 {
            panic!("ZIP 32 seeds MUST be at least 32 bytes");
        }

        UnifiedSpendingKey::from_checked_parts(
            legacy::AccountPrivKey::from_seed(_params, seed, _account)
                .map_err(DerivationError::Transparent)?,
            orchard::keys::SpendingKey::from_zip32_seed(seed, _params.coin_type(), _account)
                .map_err(DerivationError::Orchard)?,
        )
    }

    /// Construct a USK from its constituent parts, after verifying that UIVK derivation can
    /// succeed.
    fn from_checked_parts(
        transparent: legacy::AccountPrivKey,
        orchard: orchard::keys::SpendingKey,
    ) -> Result<UnifiedSpendingKey, DerivationError> {
        // Verify that FVK and IVK derivation succeed; we don't want to construct a USK
        // that can't derive transparent addresses.
        let _ = transparent.to_account_pubkey().derive_external_ivk()?;

        Ok(UnifiedSpendingKey {
            transparent,
            orchard,
        })
    }

    pub fn to_unified_full_viewing_key(&self) -> UnifiedFullViewingKey {
        UnifiedFullViewingKey {
            transparent: Some(self.transparent.to_account_pubkey()),
            orchard: Some((&self.orchard).into()),
            unknown: vec![],
        }
    }

    /// Returns the transparent component of the unified key at the
    /// BIP44 path `m/44'/<coin_type>'/<account>'`.
    pub fn transparent(&self) -> &legacy::AccountPrivKey {
        &self.transparent
    }

    /// Returns the Orchard spending key component of this unified spending key.
    pub fn orchard(&self) -> &orchard::keys::SpendingKey {
        &self.orchard
    }
}

/// Errors that can occur in the generation of unified addresses.
#[derive(Clone, Debug)]
pub enum AddressGenerationError {
    InvalidTransparentChildIndex(DiversifierIndex),
    /// The space of available diversifier indices has been exhausted.
    DiversifierSpaceExhausted,
    /// A requested address typecode was not recognized, so we are unable to generate the address
    /// as requested.
    ReceiverTypeNotSupported(Typecode),
    /// A requested address typecode was recognized, but the unified key being used to generate the
    /// address lacks an item of the requested type.
    KeyNotAvailable(Typecode),
    /// A Unified address cannot be generated without at least one shielded receiver being
    /// included.
    ShieldedReceiverRequired,
}

impl fmt::Display for AddressGenerationError {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        match &self {
            AddressGenerationError::InvalidTransparentChildIndex(i) => {
                write!(
                    f,
                    "Child index {:?} does not generate a valid transparent receiver",
                    i
                )
            }
            AddressGenerationError::DiversifierSpaceExhausted => {
                write!(
                    f,
                    "Exhausted the space of diversifier indices without finding an address."
                )
            }
            AddressGenerationError::ReceiverTypeNotSupported(t) => {
                write!(
                    f,
                    "Unified Address generation does not yet support receivers of type {:?}.",
                    t
                )
            }
            AddressGenerationError::KeyNotAvailable(t) => {
                write!(
                    f,
                    "The Unified Viewing Key does not contain a key for typecode {:?}.",
                    t
                )
            }
            AddressGenerationError::ShieldedReceiverRequired => {
                write!(f, "A Unified Address requires at least one shielded (Sapling or Orchard) receiver.")
            }
        }
    }
}

impl error::Error for AddressGenerationError {}

/// Specification for how a unified address should be generated from a unified viewing key.
#[derive(Clone, Copy, Debug)]
pub struct UnifiedAddressRequest {
    has_orchard: bool,
    has_p2pkh: bool,
}

impl UnifiedAddressRequest {
    /// Construct a new unified address request from its constituent parts.
    ///
    /// Returns `None` if the resulting unified address would not include at least one shielded receiver.
    pub fn new(has_orchard: bool, has_p2pkh: bool) -> Option<Self> {
        let has_shielded_receiver = has_orchard;

        if !has_shielded_receiver {
            None
        } else {
            Some(Self {
                has_orchard,
                has_p2pkh,
            })
        }
    }

    /// Constructs a new unified address request that includes a request for a receiver of each
    /// type that is supported given the active feature flags.
    pub fn all() -> Option<Self> {
        let _has_orchard = true;

        let _has_p2pkh = true;

        Self::new(_has_orchard, _has_p2pkh)
    }

    /// Constructs a new unified address request that includes only the receivers
    /// that appear both in itself and a given other request.
    pub fn intersect(&self, other: &UnifiedAddressRequest) -> Option<UnifiedAddressRequest> {
        Self::new(
            self.has_orchard && other.has_orchard,
            self.has_p2pkh && other.has_p2pkh,
        )
    }

    /// Construct a new unified address request from its constituent parts.
    ///
    /// Panics: at least one of `has_orchard` or `has_sapling` must be `true`.
    pub const fn unsafe_new(has_orchard: bool, has_p2pkh: bool) -> Self {
        if !(has_orchard) {
            panic!("At least one shielded receiver must be requested.")
        }

        Self {
            has_orchard,
            has_p2pkh,
        }
    }
}

impl From<bip32::Error> for DerivationError {
    fn from(e: bip32::Error) -> Self {
        DerivationError::Transparent(e)
    }
}

/// A [ZIP 316](https://zips.z.cash/zip-0316) unified full viewing key.
#[derive(Clone, Debug)]
pub struct UnifiedFullViewingKey {
    transparent: Option<legacy::AccountPubKey>,
    orchard: Option<orchard::keys::FullViewingKey>,
    unknown: Vec<(u32, Vec<u8>)>,
}

impl UnifiedFullViewingKey {
    /// Construct a UFVK from its constituent parts, after verifying that UIVK derivation can
    /// succeed.
    fn from_checked_parts(
        transparent: Option<legacy::AccountPubKey>,
        orchard: Option<orchard::keys::FullViewingKey>,
        unknown: Vec<(u32, Vec<u8>)>,
    ) -> Result<UnifiedFullViewingKey, DerivationError> {
        // Verify that IVK derivation succeeds; we don't want to construct a UFVK
        // that can't derive transparent addresses.
        let _ = transparent
            .as_ref()
            .map(|t| t.derive_external_ivk())
            .transpose()?;

        Ok(UnifiedFullViewingKey {
            transparent,
            orchard,
            unknown,
        })
    }

    /// Parses a `UnifiedFullViewingKey` from its [ZIP 316] string encoding.
    ///
    /// [ZIP 316]: https://zips.z.cash/zip-0316
    pub fn decode<P: consensus::Parameters>(params: &P, encoding: &str) -> Result<Self, String> {
        let (net, ufvk) = unified::Ufvk::decode(encoding).map_err(|e| e.to_string())?;
        let expected_net = params.network_type();
        if net != expected_net {
            return Err(format!(
                "UFVK is for network {:?} but we expected {:?}",
                net, expected_net,
            ));
        }

        Self::parse(&ufvk).map_err(|e| e.to_string())
    }

    /// Parses a `UnifiedFullViewingKey` from its [ZIP 316] string encoding.
    ///
    /// [ZIP 316]: https://zips.z.cash/zip-0316
    pub fn parse(ufvk: &Ufvk) -> Result<Self, DecodingError> {
        let mut orchard = None;
        let mut transparent = None;

        // We can use as-parsed order here for efficiency, because we're breaking out the
        // receivers we support from the unknown receivers.
        let unknown = ufvk
            .items_as_parsed()
            .iter()
            .filter_map(|receiver| match receiver {
                unified::Fvk::Orchard(data) => orchard::keys::FullViewingKey::from_bytes(&data)
                    .ok_or(DecodingError::KeyDataInvalid(Typecode::Orchard))
                    .map(|addr| {
                        orchard = Some(addr);
                        None
                    })
                    .transpose(),
                unified::Fvk::Sapling(data) => Some(Ok::<_, DecodingError>((
                    u32::from(unified::Typecode::Sapling),
                    data.to_vec(),
                ))),
                unified::Fvk::P2pkh(data) => legacy::AccountPubKey::deserialize(&data)
                    .map_err(|_| DecodingError::KeyDataInvalid(Typecode::P2pkh))
                    .map(|tfvk| {
                        transparent = Some(tfvk);
                        None
                    })
                    .transpose(),
                unified::Fvk::Unknown { typecode, data } => Some(Ok((*typecode, data.clone()))),
            })
            .collect::<Result<_, _>>()?;

        Self::from_checked_parts(transparent, orchard, unknown)
            .map_err(|_| DecodingError::KeyDataInvalid(Typecode::P2pkh))
    }

    /// Returns the string encoding of this `UnifiedFullViewingKey` for the given network.
    pub fn encode<P: consensus::Parameters>(&self, params: &P) -> String {
        self.to_ufvk().encode(&params.network_type())
    }

    /// Returns the string encoding of this `UnifiedFullViewingKey` for the given network.
    fn to_ufvk(&self) -> Ufvk {
        let items = core::iter::empty().chain(self.unknown.iter().map(|(typecode, data)| {
            unified::Fvk::Unknown {
                typecode: *typecode,
                data: data.clone(),
            }
        }));
        let items = items.chain(
            self.orchard
                .as_ref()
                .map(|fvk| fvk.to_bytes())
                .map(unified::Fvk::Orchard),
        );
        let items = items.chain(
            self.transparent
                .as_ref()
                .map(|tfvk| tfvk.serialize().try_into().unwrap())
                .map(unified::Fvk::P2pkh),
        );

        unified::Ufvk::try_from_items(items.collect())
            .expect("UnifiedFullViewingKey should only be constructed safely")
    }

    /// Derives a Unified Incoming Viewing Key from this Unified Full Viewing Key.
    pub fn to_unified_incoming_viewing_key(&self) -> UnifiedIncomingViewingKey {
        UnifiedIncomingViewingKey {
            transparent: self.transparent.as_ref().map(|t| {
                t.derive_external_ivk()
                    .expect("Transparent IVK derivation was checked at construction.")
            }),
            orchard: self.orchard.as_ref().map(|o| o.to_ivk(Scope::External)),
            unknown: Vec::new(),
        }
    }

    /// Returns the transparent component of the unified key at the
    /// BIP44 path `m/44'/<coin_type>'/<account>'`.
    pub fn transparent(&self) -> Option<&legacy::AccountPubKey> {
        self.transparent.as_ref()
    }

    /// Returns the Orchard full viewing key component of this unified key.
    pub fn orchard(&self) -> Option<&orchard::keys::FullViewingKey> {
        self.orchard.as_ref()
    }

    /// Attempts to derive the Unified Address for the given diversifier index and
    /// receiver types.
    ///
    /// Returns `None` if the specified index does not produce a valid diversifier.
    pub fn address(
        &self,
        j: DiversifierIndex,
        request: UnifiedAddressRequest,
    ) -> Result<UnifiedAddress, AddressGenerationError> {
        self.to_unified_incoming_viewing_key().address(j, request)
    }

    /// Searches the diversifier space starting at diversifier index `j` for one which will
    /// produce a valid diversifier, and return the Unified Address constructed using that
    /// diversifier along with the index at which the valid diversifier was found.
    ///
    /// Returns an `Err(AddressGenerationError)` if no valid diversifier exists or if the features
    /// required to satisfy the unified address request are not properly enabled.
    #[allow(unused_mut)]
    pub fn find_address(
        &self,
        mut j: DiversifierIndex,
        request: UnifiedAddressRequest,
    ) -> Result<(UnifiedAddress, DiversifierIndex), AddressGenerationError> {
        self.to_unified_incoming_viewing_key()
            .find_address(j, request)
    }

    /// Find the Unified Address corresponding to the smallest valid diversifier index, along with
    /// that index.
    ///
    /// Returns an `Err(AddressGenerationError)` if no valid diversifier exists or if the features
    /// required to satisfy the unified address request are not properly enabled.
    pub fn default_address(
        &self,
        request: UnifiedAddressRequest,
    ) -> Result<(UnifiedAddress, DiversifierIndex), AddressGenerationError> {
        self.find_address(DiversifierIndex::new(), request)
    }
}

/// A [ZIP 316](https://zips.z.cash/zip-0316) unified incoming viewing key.
#[derive(Clone, Debug)]
pub struct UnifiedIncomingViewingKey {
    transparent: Option<zcash_primitives::legacy::keys::ExternalIvk>,
    orchard: Option<orchard::keys::IncomingViewingKey>,
    /// Stores the unrecognized elements of the unified encoding.
    unknown: Vec<(u32, Vec<u8>)>,
}

impl UnifiedIncomingViewingKey {
    /// Parses a `UnifiedFullViewingKey` from its [ZIP 316] string encoding.
    ///
    /// [ZIP 316]: https://zips.z.cash/zip-0316
    pub fn decode<P: consensus::Parameters>(params: &P, encoding: &str) -> Result<Self, String> {
        let (net, ufvk) = unified::Uivk::decode(encoding).map_err(|e| e.to_string())?;
        let expected_net = params.network_type();
        if net != expected_net {
            return Err(format!(
                "UIVK is for network {:?} but we expected {:?}",
                net, expected_net,
            ));
        }

        Self::parse(&ufvk).map_err(|e| e.to_string())
    }

    /// Constructs a unified incoming viewing key from a parsed unified encoding.
    fn parse(uivk: &Uivk) -> Result<Self, DecodingError> {
        let mut orchard = None;
        let mut transparent = None;

        let mut unknown = vec![];

        // We can use as-parsed order here for efficiency, because we're breaking out the
        // receivers we support from the unknown receivers.
        for receiver in uivk.items_as_parsed() {
            match receiver {
                unified::Ivk::Orchard(data) => {
                    orchard = Some(
                        Option::from(orchard::keys::IncomingViewingKey::from_bytes(&data))
                            .ok_or(DecodingError::KeyDataInvalid(Typecode::Orchard))?,
                    );
                }
                unified::Ivk::Sapling(data) => {
                    unknown.push((u32::from(unified::Typecode::Sapling), data.to_vec()));
                }
                unified::Ivk::P2pkh(data) => {
                    transparent = Some(
                        legacy::ExternalIvk::deserialize(&data)
                            .map_err(|_| DecodingError::KeyDataInvalid(Typecode::P2pkh))?,
                    );
                }
                unified::Ivk::Unknown { typecode, data } => {
                    unknown.push((*typecode, data.clone()));
                }
            }
        }

        Ok(Self {
            transparent,
            orchard,
            unknown,
        })
    }

    /// Returns the string encoding of this `UnifiedFullViewingKey` for the given network.
    pub fn encode<P: consensus::Parameters>(&self, params: &P) -> String {
        self.render().encode(&params.network_type())
    }

    /// Converts this unified incoming viewing key to a unified encoding.
    fn render(&self) -> Uivk {
        let items = core::iter::empty().chain(self.unknown.iter().map(|(typecode, data)| {
            unified::Ivk::Unknown {
                typecode: *typecode,
                data: data.clone(),
            }
        }));
        let items = items.chain(
            self.orchard
                .as_ref()
                .map(|ivk| ivk.to_bytes())
                .map(unified::Ivk::Orchard),
        );
        let items = items.chain(
            self.transparent
                .as_ref()
                .map(|tivk| tivk.serialize().try_into().unwrap())
                .map(unified::Ivk::P2pkh),
        );

        unified::Uivk::try_from_items(items.collect())
            .expect("UnifiedIncomingViewingKey should only be constructed safely.")
    }

    /// Returns the Transparent external IVK, if present.
    pub fn transparent(&self) -> &Option<zcash_primitives::legacy::keys::ExternalIvk> {
        &self.transparent
    }

    /// Returns the Orchard IVK, if present.
    pub fn orchard(&self) -> &Option<orchard::keys::IncomingViewingKey> {
        &self.orchard
    }

    /// Attempts to derive the Unified Address for the given diversifier index and
    /// receiver types.
    ///
    /// Returns `None` if the specified index does not produce a valid diversifier.
    pub fn address(
        &self,
        _j: DiversifierIndex,
        request: UnifiedAddressRequest,
    ) -> Result<UnifiedAddress, AddressGenerationError> {
        let mut orchard = None;
        if request.has_orchard {
            if let Some(oivk) = &self.orchard {
                let orchard_j = orchard::keys::DiversifierIndex::from(*_j.as_bytes());
                orchard = Some(oivk.address_at(orchard_j))
            } else {
                return Err(AddressGenerationError::KeyNotAvailable(Typecode::Orchard));
            }
        }

        let mut transparent = None;
        if request.has_p2pkh {
            if let Some(tivk) = self.transparent.as_ref() {
                // If a transparent receiver type is requested, we must be able to construct an
                // address; if we're unable to do so, then no Unified Address exists at this
                // diversifier.
                let transparent_j = to_transparent_child_index(_j)
                    .ok_or(AddressGenerationError::InvalidTransparentChildIndex(_j))?;

                transparent = Some(
                    tivk.derive_address(transparent_j)
                        .map_err(|_| AddressGenerationError::InvalidTransparentChildIndex(_j))?,
                );
            } else {
                return Err(AddressGenerationError::KeyNotAvailable(Typecode::P2pkh));
            }
        }

        UnifiedAddress::from_receivers(orchard, transparent)
            .ok_or(AddressGenerationError::ShieldedReceiverRequired)
    }

    /// Searches the diversifier space starting at diversifier index `j` for one which will
    /// produce a valid diversifier, and return the Unified Address constructed using that
    /// diversifier along with the index at which the valid diversifier was found.
    ///
    /// Returns an `Err(AddressGenerationError)` if no valid diversifier exists or if the features
    /// required to satisfy the unified address request are not properly enabled.
    #[allow(unused_mut)]
    pub fn find_address(
        &self,
        mut j: DiversifierIndex,
        request: UnifiedAddressRequest,
    ) -> Result<(UnifiedAddress, DiversifierIndex), AddressGenerationError> {
        // If we need to generate a transparent receiver, check that the user has not
        // specified an invalid transparent child index, from which we can never search to
        // find a valid index.
        if request.has_p2pkh
            && self.transparent.is_some()
            && to_transparent_child_index(j).is_none()
        {
            return Err(AddressGenerationError::InvalidTransparentChildIndex(j));
        }

        // Find a working diversifier and construct the associated address.
        loop {
            let res = self.address(j, request);
            match res {
                Ok(ua) => {
                    return Ok((ua, j));
                }
                Err(other) => {
                    return Err(other);
                }
            }
        }
    }

    /// Find the Unified Address corresponding to the smallest valid diversifier index, along with
    /// that index.
    ///
    /// Returns an `Err(AddressGenerationError)` if no valid diversifier exists or if the features
    /// required to satisfy the unified address request are not properly enabled.
    pub fn default_address(
        &self,
        request: UnifiedAddressRequest,
    ) -> Result<(UnifiedAddress, DiversifierIndex), AddressGenerationError> {
        self.find_address(DiversifierIndex::new(), request)
    }

    /// Constructs a [`UnifiedAddressRequest`] that includes the components of this UIVK.
    pub fn to_address_request(&self) -> Option<UnifiedAddressRequest> {
        let has_orchard = self.orchard.is_some();
        let has_p2pkh = self.transparent.is_some();

        UnifiedAddressRequest::new(has_orchard, has_p2pkh)
    }
}
