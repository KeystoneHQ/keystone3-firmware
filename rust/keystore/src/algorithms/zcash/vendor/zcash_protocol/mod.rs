pub mod consensus;
pub mod constants;
use core::fmt;

/// A Zcash shielded transfer protocol.
#[derive(Debug, Copy, Clone, PartialEq, Eq, PartialOrd, Ord)]
pub enum ShieldedProtocol {
    /// The Sapling protocol
    Sapling,
    /// The Orchard protocol
    Orchard,
}
/// A value pool in the Zcash protocol.
#[derive(Debug, Copy, Clone, PartialEq, Eq, PartialOrd, Ord)]
pub enum PoolType {
    /// The transparent value pool
    Transparent,
    /// A shielded value pool.
    Shielded(ShieldedProtocol),
}

impl PoolType {
    pub const TRANSPARENT: PoolType = PoolType::Transparent;
    pub const SAPLING: PoolType = PoolType::Shielded(ShieldedProtocol::Sapling);
    pub const ORCHARD: PoolType = PoolType::Shielded(ShieldedProtocol::Orchard);
}

impl fmt::Display for PoolType {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self {
            PoolType::Transparent => f.write_str("Transparent"),
            PoolType::Shielded(ShieldedProtocol::Sapling) => f.write_str("Sapling"),
            PoolType::Shielded(ShieldedProtocol::Orchard) => f.write_str("Orchard"),
        }
    }
}
