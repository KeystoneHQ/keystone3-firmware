use core::convert::{TryFrom, TryInto};

use alloc::{format, vec::Vec};

use super::{
    private::{SealedContainer, SealedItem},
    Container, Encoding, ParseError, Typecode,
};

/// The set of known FVKs for Unified FVKs.
#[derive(Clone, Debug, PartialEq, Eq, Hash)]
pub enum Fvk {
    /// The raw encoding of an Orchard Full Viewing Key.
    ///
    /// `(ak, nk, rivk)` each 32 bytes.
    Orchard([u8; 96]),

    /// Data contained within the Sapling component of a Unified Full Viewing Key
    ///
    /// `(ak, nk, ovk, dk)` each 32 bytes.
    Sapling([u8; 128]),

    /// A pruned version of the extended public key for the BIP 44 account corresponding to the
    /// transparent address subtree from which transparent addresses are derived. This
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

impl TryFrom<(u32, &[u8])> for Fvk {
    type Error = ParseError;

    fn try_from((typecode, data): (u32, &[u8])) -> Result<Self, Self::Error> {
        let data = data.to_vec();
        match typecode.try_into()? {
            Typecode::P2pkh => data.try_into().map(Fvk::P2pkh),
            Typecode::P2sh => Err(data),
            Typecode::Sapling => data.try_into().map(Fvk::Sapling),
            Typecode::Orchard => data.try_into().map(Fvk::Orchard),
            Typecode::Unknown(_) => Ok(Fvk::Unknown { typecode, data }),
        }
        .map_err(|e| {
            ParseError::InvalidEncoding(format!("Invalid fvk for typecode {}: {:?}", typecode, e))
        })
    }
}

impl SealedItem for Fvk {
    fn typecode(&self) -> Typecode {
        match self {
            Fvk::P2pkh(_) => Typecode::P2pkh,
            Fvk::Sapling(_) => Typecode::Sapling,
            Fvk::Orchard(_) => Typecode::Orchard,
            Fvk::Unknown { typecode, .. } => Typecode::Unknown(*typecode),
        }
    }

    fn data(&self) -> &[u8] {
        match self {
            Fvk::P2pkh(data) => data,
            Fvk::Sapling(data) => data,
            Fvk::Orchard(data) => data,
            Fvk::Unknown { data, .. } => data,
        }
    }
}

/// A Unified Full Viewing Key.
///
/// # Examples
///
/// ```
/// # use std::error::Error;
/// use zcash_address::unified::{self, Container, Encoding};
///
/// # fn main() -> Result<(), Box<dyn Error>> {
/// # let ufvk_from_user = || "uview1cgrqnry478ckvpr0f580t6fsahp0a5mj2e9xl7hv2d2jd4ldzy449mwwk2l9yeuts85wjls6hjtghdsy5vhhvmjdw3jxl3cxhrg3vs296a3czazrycrr5cywjhwc5c3ztfyjdhmz0exvzzeyejamyp0cr9z8f9wj0953fzht0m4lenk94t70ruwgjxag2tvp63wn9ftzhtkh20gyre3w5s24f6wlgqxnjh40gd2lxe75sf3z8h5y2x0atpxcyf9t3em4h0evvsftluruqne6w4sm066sw0qe5y8qg423grple5fftxrqyy7xmqmatv7nzd7tcjadu8f7mqz4l83jsyxy4t8pkayytyk7nrp467ds85knekdkvnd7hqkfer8mnqd7pv";
/// let example_ufvk: &str = ufvk_from_user();
///
/// let (network, ufvk) = unified::Ufvk::decode(example_ufvk)?;
///
/// // We can obtain the pool-specific Full Viewing Keys for the UFVK in preference
/// // order (the order in which wallets should prefer to use their corresponding
/// // address receivers):
/// let fvks: Vec<unified::Fvk> = ufvk.items();
///
/// // And we can create the UFVK from a list of FVKs:
/// let new_ufvk = unified::Ufvk::try_from_items(fvks)?;
/// assert_eq!(new_ufvk, ufvk);
/// # Ok(())
/// # }
/// ```
#[derive(Clone, Debug, PartialEq, Eq, Hash)]
pub struct Ufvk(pub Vec<Fvk>);

impl Container for Ufvk {
    type Item = Fvk;

    /// Returns the FVKs contained within this UFVK, in the order they were
    /// parsed from the string encoding.
    ///
    /// This API is for advanced usage; in most cases you should use `Ufvk::receivers`.
    fn items_as_parsed(&self) -> &[Fvk] {
        &self.0
    }
}

impl Encoding for Ufvk {}

impl SealedContainer for Ufvk {
    /// The HRP for a Bech32m-encoded mainnet Unified FVK.
    ///
    /// Defined in [ZIP 316][zip-0316].
    ///
    /// [zip-0316]: https://zips.z.cash/zip-0316
    const MAINNET: &'static str = "uview";

    /// The HRP for a Bech32m-encoded testnet Unified FVK.
    ///
    /// Defined in [ZIP 316][zip-0316].
    ///
    /// [zip-0316]: https://zips.z.cash/zip-0316
    const TESTNET: &'static str = "uviewtest";

    /// The HRP for a Bech32m-encoded regtest Unified FVK.
    const REGTEST: &'static str = "uviewregtest";

    fn from_inner(fvks: Vec<Self::Item>) -> Self {
        Self(fvks)
    }
}
