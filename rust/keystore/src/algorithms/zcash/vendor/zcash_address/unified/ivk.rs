use core::convert::{TryFrom, TryInto};

use alloc::vec::Vec;

use super::{
    private::{SealedContainer, SealedItem},
    Container, Encoding, ParseError, Typecode,
};

/// The set of known IVKs for Unified IVKs.
#[derive(Clone, Debug, PartialEq, Eq, Hash)]
pub enum Ivk {
    /// The raw encoding of an Orchard Incoming Viewing Key.
    ///
    /// `(dk, ivk)` each 32 bytes.
    Orchard([u8; 64]),

    /// Data contained within the Sapling component of a Unified Incoming Viewing Key.
    ///
    /// In order to ensure that Unified Addresses can always be derived from UIVKs, we
    /// store more data here than was specified to be part of a Sapling IVK. Specifically,
    /// we store the same data here as we do for Orchard.
    ///
    /// `(dk, ivk)` each 32 bytes.
    Sapling([u8; 64]),

    /// A pruned version of the extended public key for the BIP 44 account corresponding to the
    /// transparent address subtree from which transparent addresses are derived,
    /// at the external `change` BIP 44 path, i.e. `m/44'/133'/<account_id>'/0`. This
    /// includes just the chain code (32 bytes) and the compressed public key (33 bytes), and excludes
    /// the depth of in the derivation tree, the parent key fingerprint, and the child key
    /// number (which would reveal the wallet account number for which this UFVK was generated).
    ///
    /// Transparent addresses don't have "viewing keys" - the addresses themselves serve
    /// that purpose. However, we want the ability to derive diversified Unified Addresses
    /// from Unified Viewing Keys, and to not break the unlinkability property when they
    /// include transparent receivers. To achieve this, we treat the last hardened node in
    /// the BIP 44 derivation path as the "transparent viewing key"; all addresses derived
    /// from this node use non-hardened derivation, and can thus be derived just from this
    /// pruned extended public key.
    P2pkh([u8; 65]),

    Unknown {
        typecode: u32,
        data: Vec<u8>,
    },
}

impl TryFrom<(u32, &[u8])> for Ivk {
    type Error = ParseError;

    fn try_from((typecode, data): (u32, &[u8])) -> Result<Self, Self::Error> {
        let data = data.to_vec();
        match typecode.try_into()? {
            Typecode::P2pkh => data.try_into().map(Ivk::P2pkh),
            Typecode::P2sh => Err(data),
            Typecode::Sapling => data.try_into().map(Ivk::Sapling),
            Typecode::Orchard => data.try_into().map(Ivk::Orchard),
            Typecode::Unknown(_) => Ok(Ivk::Unknown { typecode, data }),
        }
        .map_err(|e| {
            ParseError::InvalidEncoding(format!("Invalid ivk for typecode {}: {:?}", typecode, e))
        })
    }
}

impl SealedItem for Ivk {
    fn typecode(&self) -> Typecode {
        match self {
            Ivk::P2pkh(_) => Typecode::P2pkh,
            Ivk::Sapling(_) => Typecode::Sapling,
            Ivk::Orchard(_) => Typecode::Orchard,
            Ivk::Unknown { typecode, .. } => Typecode::Unknown(*typecode),
        }
    }

    fn data(&self) -> &[u8] {
        match self {
            Ivk::P2pkh(data) => data,
            Ivk::Sapling(data) => data,
            Ivk::Orchard(data) => data,
            Ivk::Unknown { data, .. } => data,
        }
    }
}

/// A Unified Incoming Viewing Key.
///
/// # Examples
///
/// ```
/// # use std::error::Error;
/// use zcash_address::unified::{self, Container, Encoding};
///
/// # fn main() -> Result<(), Box<dyn Error>> {
/// # let uivk_from_user = || "uivk1djetqg3fws7y7qu5tekynvcdhz69gsyq07ewvppmzxdqhpfzdgmx8urnkqzv7ylz78ez43ux266pqjhecd59fzhn7wpe6zarnzh804hjtkyad25ryqla5pnc8p5wdl3phj9fczhz64zprun3ux7y9jc08567xryumuz59rjmg4uuflpjqwnq0j0tzce0x74t4tv3gfjq7nczkawxy6y7hse733ae3vw7qfjd0ss0pytvezxp42p6rrpzeh6t2zrz7zpjk0xhngcm6gwdppxs58jkx56gsfflugehf5vjlmu7vj3393gj6u37wenavtqyhdvcdeaj86s6jczl4zq";
/// let example_uivk: &str = uivk_from_user();
///
/// let (network, uivk) = unified::Uivk::decode(example_uivk)?;
///
/// // We can obtain the pool-specific Incoming Viewing Keys for the UIVK in
/// // preference order (the order in which wallets should prefer to use their
/// // corresponding address receivers):
/// let ivks: Vec<unified::Ivk> = uivk.items();
///
/// // And we can create the UIVK from a list of IVKs:
/// let new_uivk = unified::Uivk::try_from_items(ivks)?;
/// assert_eq!(new_uivk, uivk);
/// # Ok(())
/// # }
/// ```
#[derive(Clone, Debug, PartialEq, Eq, Hash)]
pub struct Uivk(pub(crate) Vec<Ivk>);

impl Container for Uivk {
    type Item = Ivk;

    /// Returns the IVKs contained within this UIVK, in the order they were
    /// parsed from the string encoding.
    ///
    /// This API is for advanced usage; in most cases you should use `Uivk::items`.
    fn items_as_parsed(&self) -> &[Ivk] {
        &self.0
    }
}

impl Encoding for Uivk {}

impl SealedContainer for Uivk {
    /// The HRP for a Bech32m-encoded mainnet Unified IVK.
    ///
    /// Defined in [ZIP 316][zip-0316].
    ///
    /// [zip-0316]: https://zips.z.cash/zip-0316
    const MAINNET: &'static str = "uivk";

    /// The HRP for a Bech32m-encoded testnet Unified IVK.
    ///
    /// Defined in [ZIP 316][zip-0316].
    ///
    /// [zip-0316]: https://zips.z.cash/zip-0316
    const TESTNET: &'static str = "uivktest";

    /// The HRP for a Bech32m-encoded regtest Unified IVK.
    const REGTEST: &'static str = "uivkregtest";

    fn from_inner(ivks: Vec<Self::Item>) -> Self {
        Self(ivks)
    }
}
