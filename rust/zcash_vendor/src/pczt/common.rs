use alloc::{collections::BTreeMap, string::String, vec::Vec};

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
