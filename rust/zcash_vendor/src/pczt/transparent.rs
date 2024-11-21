use alloc::{collections::BTreeMap, string::String, vec::Vec};

use super::merge_optional;

/// PCZT fields that are specific to producing the transaction's transparent bundle (if
/// any).
#[derive(Clone)]
pub(crate) struct Bundle {
    pub(crate) inputs: Vec<Input>,
    pub(crate) outputs: Vec<Output>,
}

#[derive(Clone)]
pub(crate) struct Input {
    //
    // Transparent effecting data.
    //
    // These are required fields that are part of the final transaction, and are filled in
    // by the Constructor when adding an output.
    //
    pub(crate) prevout_txid: [u8; 32],
    pub(crate) prevout_index: u32,
    /// TODO: which role should set this?
    pub(crate) sequence: u32,

    /// A satisfying witness for the `script_pubkey` of the input being spent.
    ///
    /// This is set by the Spend Finalizer.
    pub(crate) script_sig: Option<Vec<u8>>,

    // These are required by the Transaction Extractor, to derive the shielded sighash
    // needed for computing the binding signatures.
    pub(crate) value: u64,
    pub(crate) script_pubkey: Vec<u8>,

    /// A map from a pubkey to a signature created by it.
    ///
    /// - Each entry is set by a Signer.
    /// - These are required by the Spend Finalizer to assemble `script_sig`.
    ///
    /// TODO: Decide on map key type.
    pub(crate) signatures: BTreeMap<Vec<u8>, Vec<u8>>,

    // TODO derivation path

    pub(crate) proprietary: BTreeMap<String, Vec<u8>>,
}

#[derive(Clone)]
pub(crate) struct Output {
    //
    // Transparent effecting data.
    //
    // These are required fields that are part of the final transaction, and are filled in
    // by the Constructor when adding an output.
    //
    pub(crate) value: u64,
    pub(crate) script_pubkey: Vec<u8>,

    // TODO derivation path

    pub(crate) proprietary: BTreeMap<String, Vec<u8>>,
}

impl Bundle {
    /// Merges this bundle with another.
    ///
    /// Returns `None` if the bundles have conflicting data.
    pub(crate) fn merge(mut self, other: Self) -> Option<Self> {
        // Destructure `other` to ensure we handle everything.
        let Self {
            mut inputs,
            mut outputs,
        } = other;

        // If the other bundle has more inputs or outputs than us, move them over; these
        // cannot conflict by construction.
        self.inputs.extend(inputs.drain(self.inputs.len()..));
        self.outputs.extend(outputs.drain(self.outputs.len()..));

        // Leverage the early-exit behaviour of zip to confirm that the remaining data in
        // the other bundle matches this one.
        for (lhs, rhs) in self.inputs.iter_mut().zip(inputs.into_iter()) {
            // Destructure `rhs` to ensure we handle everything.
            let Input {
                prevout_txid,
                prevout_index,
                sequence,
                script_sig,
                value,
                script_pubkey,
                signatures,
                proprietary,
            } = rhs;

            if lhs.prevout_txid != prevout_txid
                || lhs.prevout_index != prevout_index
                || lhs.sequence != sequence
                || lhs.value != value
                || lhs.script_pubkey != script_pubkey
            {
                return None;
            }

            if !merge_optional(&mut lhs.script_sig, script_sig) {
                return None;
            }

            // TODO: Merge signature maps.

            // TODO: Decide how to merge proprietary fields.
        }

        for (lhs, rhs) in self.outputs.iter_mut().zip(outputs.into_iter()) {
            // Destructure `rhs` to ensure we handle everything.
            let Output {
                value,
                script_pubkey,
                proprietary,
            } = rhs;

            if lhs.value != value || lhs.script_pubkey != script_pubkey {
                return None;
            }

            // TODO: Decide how to merge proprietary fields.
        }

        Some(self)
    }
}
