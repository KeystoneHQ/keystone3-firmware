use alloc::{collections::BTreeMap, string::String, vec::Vec};
use getset::{Getters, MutGetters};
use serde::{Deserialize, Serialize};
use serde_with::serde_as;

use super::{common::Zip32Derivation, merge_map, merge_optional, ParseError};

/// PCZT fields that are specific to producing the transaction's transparent bundle (if
/// any).
#[derive(Clone, Debug, Serialize, Deserialize, Getters, MutGetters)]
pub struct Bundle {
    #[getset(get = "pub")]
    pub(crate) inputs: Vec<Input>,
    #[getset(get = "pub")]
    pub(crate) outputs: Vec<Output>,
}

impl Bundle {
    pub fn inputs_mut(&mut self) -> &mut [Input] {
        &mut self.inputs
    }

    pub fn outputs_mut(&mut self) -> &mut [Output] {
        &mut self.outputs
    }
}

#[serde_as]
#[derive(Clone, Debug, Serialize, Deserialize, Getters, MutGetters)]
#[getset(get = "pub", get_mut = "pub")]
pub struct Input {
    //
    // Transparent effecting data.
    //
    // These are required fields that are part of the final transaction, and are filled in
    // by the Constructor when adding an output.
    //
    pub(crate) prevout_txid: [u8; 32],
    pub(crate) prevout_index: u32,

    /// The sequence number of this input.
    ///
    /// - This is set by the Constructor.
    /// - If omitted, the sequence number is assumed to be the final sequence number
    ///   (`0xffffffff`).
    pub(crate) sequence: Option<u32>,

    /// The minimum Unix timstamp that this input requires to be set as the transaction's
    /// lock time.
    ///
    /// - This is set by the Constructor.
    /// - This must be greater than or equal to 500000000.
    pub(crate) required_time_lock_time: Option<u32>,

    /// The minimum block height that this input requires to be set as the transaction's
    /// lock time.
    ///
    /// - This is set by the Constructor.
    /// - This must be greater than 0 and less than 500000000.
    pub(crate) required_height_lock_time: Option<u32>,

    /// A satisfying witness for the `script_pubkey` of the input being spent.
    ///
    /// This is set by the Spend Finalizer.
    pub(crate) script_sig: Option<Vec<u8>>,

    // These are required by the Transaction Extractor, to derive the shielded sighash
    // needed for computing the binding signatures.
    pub(crate) value: u64,
    pub(crate) script_pubkey: Vec<u8>,

    /// The script required to spend this output, if it is P2SH.
    ///
    /// Set to `None` if this is a P2PKH output.
    pub(crate) redeem_script: Option<Vec<u8>>,

    /// A map from a pubkey to a signature created by it.
    ///
    /// - Each pubkey should appear in `script_pubkey` or `redeem_script`.
    /// - Each entry is set by a Signer, and should contain an ECDSA signature that is
    ///   valid under the corresponding pubkey.
    /// - These are required by the Spend Finalizer to assemble `script_sig`.
    #[serde_as(as = "BTreeMap<[_; 33], _>")]
    pub(crate) partial_signatures: BTreeMap<[u8; 33], Vec<u8>>,

    /// The sighash type to be used for this input.
    ///
    /// - Signers must use this sighash type to produce their signatures. Signers that
    ///   cannot produce signatures for this sighash type must not provide a signature.
    /// - Spend Finalizers must fail to finalize inputs which have signatures that do not
    ///   match this sighash type.
    pub(crate) sighash_type: u8,

    /// A map from a pubkey to the BIP 32 derivation path at which its corresponding
    /// spending key can be found.
    ///
    /// - The pubkeys should appear in `script_pubkey` or `redeem_script`.
    /// - Each entry is set by an Updater.
    /// - Individual entries may be required by a Signer.
    #[serde_as(as = "BTreeMap<[_; 33], _>")]
    pub(crate) bip32_derivation: BTreeMap<[u8; 33], Zip32Derivation>,

    /// Mappings of the form `key = RIPEMD160(value)`.
    ///
    /// - These may be used by the Signer to inspect parts of `script_pubkey` or
    ///   `redeem_script`.
    pub(crate) ripemd160_preimages: BTreeMap<[u8; 20], Vec<u8>>,

    /// Mappings of the form `key = SHA256(value)`.
    ///
    /// - These may be used by the Signer to inspect parts of `script_pubkey` or
    ///   `redeem_script`.
    pub(crate) sha256_preimages: BTreeMap<[u8; 32], Vec<u8>>,

    /// Mappings of the form `key = RIPEMD160(SHA256(value))`.
    ///
    /// - These may be used by the Signer to inspect parts of `script_pubkey` or
    ///   `redeem_script`.
    pub(crate) hash160_preimages: BTreeMap<[u8; 20], Vec<u8>>,

    /// Mappings of the form `key = SHA256(SHA256(value))`.
    ///
    /// - These may be used by the Signer to inspect parts of `script_pubkey` or
    ///   `redeem_script`.
    pub(crate) hash256_preimages: BTreeMap<[u8; 32], Vec<u8>>,

    /// Proprietary fields related to the note being spent.
    #[getset(get = "pub", get_mut = "pub")]
    pub(crate) proprietary: BTreeMap<String, Vec<u8>>,
}

#[serde_as]
#[derive(Clone, Debug, Serialize, Deserialize, Getters, MutGetters)]
#[getset(get = "pub")]
pub struct Output {
    //
    // Transparent effecting data.
    //
    // These are required fields that are part of the final transaction, and are filled in
    // by the Constructor when adding an output.
    //
    pub(crate) value: u64,
    pub(crate) script_pubkey: Vec<u8>,

    /// The script required to spend this output, if it is P2SH.
    ///
    /// Set to `None` if this is a P2PKH output.
    pub(crate) redeem_script: Option<Vec<u8>>,

    /// A map from a pubkey to the BIP 32 derivation path at which its corresponding
    /// spending key can be found.
    ///
    /// - The pubkeys should appear in `script_pubkey` or `redeem_script`.
    /// - Each entry is set by an Updater.
    /// - Individual entries may be required by a Signer.
    #[serde_as(as = "BTreeMap<[_; 33], _>")]
    pub bip32_derivation: BTreeMap<[u8; 33], Zip32Derivation>,

    /// Proprietary fields related to the note being spent.
    #[getset(get = "pub", get_mut = "pub")]
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
                required_time_lock_time,
                required_height_lock_time,
                script_sig,
                value,
                script_pubkey,
                redeem_script,
                partial_signatures,
                sighash_type,
                bip32_derivation,
                ripemd160_preimages,
                sha256_preimages,
                hash160_preimages,
                hash256_preimages,
                proprietary,
            } = rhs;

            if lhs.prevout_txid != prevout_txid
                || lhs.prevout_index != prevout_index
                || lhs.value != value
                || lhs.script_pubkey != script_pubkey
                || lhs.sighash_type != sighash_type
            {
                return None;
            }

            if !(merge_optional(&mut lhs.sequence, sequence)
                && merge_optional(&mut lhs.required_time_lock_time, required_time_lock_time)
                && merge_optional(
                    &mut lhs.required_height_lock_time,
                    required_height_lock_time,
                )
                && merge_optional(&mut lhs.script_sig, script_sig)
                && merge_optional(&mut lhs.redeem_script, redeem_script)
                && merge_map(&mut lhs.partial_signatures, partial_signatures)
                && merge_map(&mut lhs.bip32_derivation, bip32_derivation)
                && merge_map(&mut lhs.ripemd160_preimages, ripemd160_preimages)
                && merge_map(&mut lhs.sha256_preimages, sha256_preimages)
                && merge_map(&mut lhs.hash160_preimages, hash160_preimages)
                && merge_map(&mut lhs.hash256_preimages, hash256_preimages)
                && merge_map(&mut lhs.proprietary, proprietary))
            {
                return None;
            }
        }

        for (lhs, rhs) in self.outputs.iter_mut().zip(outputs.into_iter()) {
            // Destructure `rhs` to ensure we handle everything.
            let Output {
                value,
                script_pubkey,
                redeem_script,
                bip32_derivation,
                proprietary,
            } = rhs;

            if lhs.value != value || lhs.script_pubkey != script_pubkey {
                return None;
            }

            if !(merge_optional(&mut lhs.redeem_script, redeem_script)
                && merge_map(&mut lhs.bip32_derivation, bip32_derivation)
                && merge_map(&mut lhs.proprietary, proprietary))
            {
                return None;
            }
        }

        Some(self)
    }
}
