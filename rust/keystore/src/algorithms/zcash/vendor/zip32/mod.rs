use core::mem;

pub mod fingerprint;

/// A type-safe wrapper for account identifiers.
///
/// Accounts are 31-bit unsigned integers, and are always treated as hardened in
/// derivation paths.
#[derive(Debug, Default, Copy, Clone, PartialEq, Eq, PartialOrd, Ord, Hash)]
pub struct AccountId(u32);

impl TryFrom<u32> for AccountId {
    type Error = TryFromIntError;

    fn try_from(id: u32) -> Result<Self, Self::Error> {
        // Account IDs are always hardened in derivation paths, so they are effectively at
        // most 31 bits.
        if id < (1 << 31) {
            Ok(Self(id))
        } else {
            Err(TryFromIntError(()))
        }
    }
}

impl From<AccountId> for u32 {
    fn from(id: AccountId) -> Self {
        id.0
    }
}

impl From<AccountId> for ChildIndex {
    fn from(id: AccountId) -> Self {
        // Account IDs are always hardened in derivation paths.
        ChildIndex::hardened(id.0)
    }
}

impl AccountId {
    /// The ID for account zero (the first account).
    pub const ZERO: Self = Self(0);

    /// Returns the next account ID in sequence, or `None` on overflow.
    pub fn next(&self) -> Option<Self> {
        Self::try_from(self.0 + 1).ok()
    }
}

/// The error type returned when a checked integral type conversion fails.
#[derive(Clone, Copy, Debug)]
pub struct TryFromIntError(());

impl core::fmt::Display for TryFromIntError {
    fn fmt(&self, f: &mut core::fmt::Formatter<'_>) -> core::fmt::Result {
        write!(f, "out of range integral type conversion attempted")
    }
}

// ZIP 32 structures

/// A child index for a derived key.
///
/// Only hardened derivation is supported.
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub struct ChildIndex(u32);

impl ChildIndex {
    /// Parses the given ZIP 32 child index.
    ///
    /// Returns `None` if the hardened bit is not set.
    pub fn from_index(i: u32) -> Option<Self> {
        if i >= (1 << 31) {
            Some(ChildIndex(i))
        } else {
            None
        }
    }

    /// Constructs a hardened `ChildIndex` from the given value.
    ///
    /// # Panics
    ///
    /// Panics if `value >= (1 << 31)`.
    pub const fn hardened(value: u32) -> Self {
        assert!(value < (1 << 31));
        Self(value + (1 << 31))
    }

    /// Returns the index as a 32-bit integer, including the hardened bit.
    pub fn index(&self) -> u32 {
        self.0
    }
}

/// A value that is needed, in addition to a spending key, in order to derive descendant
/// keys and addresses of that key.
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub struct ChainCode([u8; 32]);

impl ChainCode {
    /// Constructs a `ChainCode` from the given array.
    pub fn new(c: [u8; 32]) -> Self {
        Self(c)
    }

    /// Returns the byte representation of the chain code, as required for
    /// [ZIP 32](https://zips.z.cash/zip-0032) encoding.
    pub fn as_bytes(&self) -> &[u8; 32] {
        &self.0
    }
}

/// The index for a particular diversifier.
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub struct DiversifierIndex([u8; 11]);

impl Default for DiversifierIndex {
    fn default() -> Self {
        DiversifierIndex::new()
    }
}

macro_rules! di_from {
    ($n:ident) => {
        impl From<$n> for DiversifierIndex {
            fn from(j: $n) -> Self {
                let mut j_bytes = [0; 11];
                j_bytes[..mem::size_of::<$n>()].copy_from_slice(&j.to_le_bytes());
                DiversifierIndex(j_bytes)
            }
        }
    };
}
di_from!(u32);
di_from!(u64);
di_from!(usize);

impl From<[u8; 11]> for DiversifierIndex {
    fn from(j_bytes: [u8; 11]) -> Self {
        DiversifierIndex(j_bytes)
    }
}

impl TryFrom<u128> for DiversifierIndex {
    type Error = TryFromIntError;

    fn try_from(value: u128) -> Result<Self, Self::Error> {
        if (value >> 88) == 0 {
            Ok(Self(value.to_le_bytes()[..11].try_into().unwrap()))
        } else {
            Err(TryFromIntError(()))
        }
    }
}

macro_rules! di_try_into {
    ($n:ident) => {
        impl TryFrom<DiversifierIndex> for $n {
            type Error = core::num::TryFromIntError;

            fn try_from(di: DiversifierIndex) -> Result<Self, Self::Error> {
                u128::from(di).try_into()
            }
        }
    };
}
di_try_into!(u32);
di_try_into!(u64);
di_try_into!(usize);

impl From<DiversifierIndex> for u128 {
    fn from(di: DiversifierIndex) -> Self {
        let mut u128_bytes = [0u8; 16];
        u128_bytes[0..11].copy_from_slice(&di.0[..]);
        u128::from_le_bytes(u128_bytes)
    }
}

impl DiversifierIndex {
    /// Constructs the zero index.
    pub fn new() -> Self {
        DiversifierIndex([0; 11])
    }

    /// Returns the raw bytes of the diversifier index.
    pub fn as_bytes(&self) -> &[u8; 11] {
        &self.0
    }

    /// Increments this index, failing on overflow.
    pub fn increment(&mut self) -> Result<(), DiversifierIndexOverflowError> {
        for k in 0..11 {
            self.0[k] = self.0[k].wrapping_add(1);
            if self.0[k] != 0 {
                // No overflow
                return Ok(());
            }
        }
        // Overflow
        Err(DiversifierIndexOverflowError)
    }
}

/// The error type returned when a [`DiversifierIndex`] increment fails.
#[derive(Clone, Copy, Debug)]
pub struct DiversifierIndexOverflowError;

impl core::fmt::Display for DiversifierIndexOverflowError {
    fn fmt(&self, f: &mut core::fmt::Formatter<'_>) -> core::fmt::Result {
        write!(f, "DiversifierIndex increment overflowed")
    }
}

/// The scope of a viewing key or address.
///
/// A "scope" narrows the visibility or usage to a level below "full".
///
/// Consistent usage of `Scope` enables the user to provide consistent views over a wallet
/// to other people. For example, a user can give an external incoming viewing key to a
/// merchant terminal, enabling it to only detect "real" transactions from customers and
/// not internal transactions from the wallet.
#[derive(Clone, Copy, Debug, PartialEq, Eq, Hash)]
pub enum Scope {
    /// A scope used for wallet-external operations, namely deriving addresses to give to
    /// other users in order to receive funds.
    External,
    /// A scope used for wallet-internal operations, such as creating change notes,
    /// auto-shielding, and note management.
    Internal,
}
