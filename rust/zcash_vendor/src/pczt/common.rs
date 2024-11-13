use alloc::{
    collections::BTreeMap,
    format,
    string::{String, ToString},
    vec::Vec,
};

/// Global fields that are relevant to the transaction as a whole.
#[derive(Clone, Debug)]
pub struct Global {
    //
    // Transaction effecting data.
    //
    // These are required fields that are part of the final transaction, and are filled in
    // by the Creator when initializing the PCZT.
    //
    pub tx_version: u32,
    pub version_group_id: u32,
    /// The consensus branch ID for the chain in which this transaction will be mined.
    ///
    /// Non-optional because this commits to the set of consensus rules that will apply to
    /// the transaction; differences therein can affect every role.
    pub consensus_branch_id: u32,
    /// TODO: In PSBT this is `fallback_lock_time`; decide whether this should have the
    /// same semantics.
    pub lock_time: u32,
    pub expiry_height: u32,

    pub proprietary: BTreeMap<String, Vec<u8>>,
}

impl Global {
    pub fn merge(self, other: Self) -> Option<Self> {
        let Self {
            tx_version,
            version_group_id,
            consensus_branch_id,
            lock_time,
            expiry_height,
            proprietary,
        } = other;

        if self.tx_version != tx_version
            || self.version_group_id != version_group_id
            || self.consensus_branch_id != consensus_branch_id
            || self.lock_time != lock_time
            || self.expiry_height != expiry_height
        {
            return None;
        }

        // TODO: Decide how to merge proprietary fields.

        Some(self)
    }
}

pub const HARDENED_MASK: u32 = 0x8000_0000;

#[derive(Clone, PartialEq, Debug)]
pub struct Zip32Derivation {
    /// The [ZIP 32 seed fingerprint](https://zips.z.cash/zip-0032#seed-fingerprints).
    pub seed_fingerprint: [u8; 32],

    /// The sequence of indices corresponding to the shielded HD path.
    ///
    /// Indices can be hardened or non-hardened (i.e. the hardened flag bit may be set).
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
