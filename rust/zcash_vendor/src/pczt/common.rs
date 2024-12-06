use alloc::{
    collections::BTreeMap,
    format,
    string::{String, ToString},
    vec::Vec,
};
use serde::{Deserialize, Serialize};

use super::merge_map;

/// Global fields that are relevant to the transaction as a whole.
#[derive(Clone, Debug, Serialize, Deserialize)]
pub struct Global {
    //
    // Transaction effecting data.
    //
    // These are required fields that are part of the final transaction, and are filled in
    // by the Creator when initializing the PCZT.
    //
    pub(crate) tx_version: u32,
    pub(crate) version_group_id: u32,

    /// The consensus branch ID for the chain in which this transaction will be mined.
    ///
    /// Non-optional because this commits to the set of consensus rules that will apply to
    /// the transaction; differences therein can affect every role.
    pub(crate) consensus_branch_id: u32,

    /// The transaction locktime to use if no inputs specify a required locktime.
    ///
    /// - This is set by the Creator.
    /// - If omitted, the fallback locktime is assumed to be 0.
    pub(crate) fallback_lock_time: Option<u32>,

    pub(crate) expiry_height: u32,

    /// The [SLIP 44] coin type, indicating the network for which this transaction is
    /// being constructed.
    ///
    /// This is technically information that could be determined indirectly from the
    /// `consensus_branch_id` but is included explicitly to enable easy identification.
    /// Note that this field is not included in the transaction and has no consensus
    /// effect (`consensus_branch_id` fills that role).
    ///
    /// - This is set by the Creator.
    /// - Roles that encode network-specific information (for example, derivation paths
    ///   for key identification) should check against this field for correctness.
    ///
    /// [SLIP 44]: https://github.com/satoshilabs/slips/blob/master/slip-0044.md
    pub(crate) coin_type: u32,

    /// A bitfield for various transaction modification flags.
    ///
    /// - Bit 0 is the Inputs Modifiable Flag and indicates whether inputs can be modified.
    ///   - This is set to `true` by the Creator.
    ///   - This is checked by the Constructor before adding inputs, and may be set to
    ///     `false` by the Constructor.
    ///   - This is set to `false` by the IO Finalizer if there are shielded spends or
    ///     outputs.
    ///   - This is set to `false` by a Signer that adds a signature that does not use
    ///     `SIGHASH_ANYONECANPAY`.
    ///   - The Combiner merges this bit towards `false`.
    /// - Bit 1 is the Outputs Modifiable Flag and indicates whether outputs can be
    ///   modified.
    ///   - This is set to `true` by the Creator.
    ///   - This is checked by the Constructor before adding outputs, and may be set to
    ///     `false` by the Constructor.
    ///   - This is set to `false` by the IO Finalizer if there are shielded spends or
    ///     outputs.
    ///   - This is set to `false` by a Signer that adds a signature that does not use
    ///     `SIGHASH_NONE`.
    ///   - The Combiner merges this bit towards `false`.
    /// - Bit 2 is the Has `SIGHASH_SINGLE` flag and indicates whether the transaction has
    ///   a `SIGHASH_SINGLE` signature who's input and output pairing must be preserved.
    ///   - This is set to `false` by the Creator.
    ///   - This is updated by a Constructor.
    ///   - This is set to `true` by a Signer that adds a signature that uses
    ///     `SIGHASH_SINGLE`.
    ///   - This essentially indicates that the Constructor must iterate the transparent
    ///     inputs to determine whether and how to add a transparent input.
    ///   - The Combiner merges this bit towards `true`.
    /// - Bits 3-7 must be 0.
    pub(crate) tx_modifiable: u8,

    /// Proprietary fields related to the overall transaction.
    pub(crate) proprietary: BTreeMap<String, Vec<u8>>,
}

impl Global {
    pub(crate) fn merge(mut self, other: Self) -> Option<Self> {
        let Self {
            tx_version,
            version_group_id,
            consensus_branch_id,
            fallback_lock_time,
            expiry_height,
            coin_type,
            tx_modifiable,
            proprietary,
        } = other;

        if self.tx_version != tx_version
            || self.version_group_id != version_group_id
            || self.consensus_branch_id != consensus_branch_id
            || self.fallback_lock_time != fallback_lock_time
            || self.expiry_height != expiry_height
            || self.coin_type != coin_type
        {
            return None;
        }

        // `tx_modifiable` is explicitly a bitmap; merge it bit-by-bit.
        // - Bit 0 and Bit 1 merge towards `false`.
        if (tx_modifiable & 0b0000_0001) == 0 {
            self.tx_modifiable &= !0b0000_0001;
        }
        if (tx_modifiable & 0b0000_0010) == 0 {
            self.tx_modifiable &= !0b0000_0010;
        }
        // - Bit 2 merges towards `true`.
        if (tx_modifiable & 0b0000_0100) != 0 {
            self.tx_modifiable |= 0b0000_0100;
        }
        // - Bits 3-7 must be 0.
        if (self.tx_modifiable >> 3) != 0 || (tx_modifiable >> 3) != 0 {
            return None;
        }

        if !merge_map(&mut self.proprietary, proprietary) {
            return None;
        }

        Some(self)
    }
}

pub const HARDENED_MASK: u32 = 0x8000_0000;

#[derive(Clone, Debug, PartialEq, Serialize, Deserialize)]
pub struct Zip32Derivation {
    /// The [ZIP 32 seed fingerprint](https://zips.z.cash/zip-0032#seed-fingerprints).
    pub seed_fingerprint: [u8; 32],

    /// The sequence of indices corresponding to the shielded HD path.
    ///
    /// Indices can be hardened or non-hardened (i.e. the hardened flag bit may be set).
    /// When used with a Sapling or Orchard spend, the derivation path will generally be
    /// entirely hardened; when used with a transparent spend, the derivation path will
    /// generally include a non-hardened section matching either the [BIP 44] path, or the
    /// path at which ephemeral addresses are derived for [ZIP 320] transactions.
    ///
    /// [BIP 44]: https://github.com/bitcoin/bips/blob/master/bip-0044.mediawiki
    /// [ZIP 320]: https://zips.z.cash/zip-0320
    pub derivation_path: Vec<u32>,
}

impl ToString for Zip32Derivation {
    fn to_string(&self) -> String {
        let mut path = "m".to_string();
        for i in self.derivation_path.iter() {
            if i & HARDENED_MASK != 0 {
                path.push_str(&format!("/{}'", i - HARDENED_MASK));
            } else {
                path.push_str(&format!("/{}", i));
            }
        }
        path
    }
}

#[cfg(test)]
mod tests {
    use alloc::vec;

    use super::*;

    #[test]
    fn test_zip32_derivation_to_string() {
        let derivation = Zip32Derivation {
            seed_fingerprint: [0; 32],
            derivation_path: vec![
                HARDENED_MASK + 44,
                HARDENED_MASK + 133,
                HARDENED_MASK + 0,
                1,
                0,
            ],
        };
        assert_eq!(derivation.to_string(), "m/44'/133'/0'/1/0");
    }
}
