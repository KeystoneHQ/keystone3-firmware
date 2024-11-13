use alloc::{collections::BTreeMap, string::String, vec::Vec};

use super::{common::Zip32Derivation, merge_map, merge_optional};

const GROTH_PROOF_SIZE: usize = 48 + 96 + 48;

/// PCZT fields that are specific to producing the transaction's Sapling bundle (if any).
#[derive(Clone, Debug)]
pub struct Bundle {
    pub spends: Vec<Spend>,
    pub outputs: Vec<Output>,

    /// The net value of Sapling spends minus outputs.
    ///
    /// This is initialized by the Creator, and updated by the Constructor as spends or
    /// outputs are added to the PCZT. It enables per-spend and per-output values to be
    /// redacted from the PCZT after they are no longer necessary.
    pub value_balance: u64,

    /// The Sapling anchor for this transaction.
    ///
    /// Set by the Creator.
    pub anchor: [u8; 32],

    /// The Sapling binding signature signing key.
    ///
    /// - This is `None` until it is set by the IO Finalizer.
    /// - The Transaction Extractor uses this to produce the binding signature.
    pub bsk: Option<[u8; 32]>,
}

/// Information about a Sapling spend within a transaction.
#[derive(Clone, Debug)]
pub struct Spend {
    //
    // SpendDescription effecting data.
    //
    // These are required fields that are part of the final transaction, and are filled in
    // by the Constructor when adding an output.
    //
    pub cv: [u8; 32],
    pub nullifier: [u8; 32],
    pub rk: [u8; 32],

    /// The Spend proof.
    ///
    /// This is set by the Prover.
    pub zkproof: Option<[u8; GROTH_PROOF_SIZE]>,

    /// The spend authorization signature.
    ///
    /// This is set by the Signer.
    pub spend_auth_sig: Option<[u8; 64]>,

    /// The address that received the note being spent.
    ///
    /// - This is set by the Constructor (or Updater?).
    /// - This is required by the Prover.
    pub recipient: Option<[u8; 43]>,

    /// The value of the input being spent.
    ///
    /// This may be used by Signers to verify that the value matches `cv`, and to confirm
    /// the values and change involved in the transaction.
    ///
    /// This exposes the input value to all participants. For Signers who don't need this
    /// information, or after signatures have been applied, this can be redacted.
    pub value: Option<u64>,

    /// The seed randomness for the note being spent.
    ///
    /// - This is set by the Constructor.
    /// - This is required by the Prover.
    pub rseed: Option<[u8; 32]>,

    /// The value commitment randomness.
    ///
    /// - This is set by the Constructor.
    /// - The IO Finalizer compresses it into the bsk.
    /// - This is required by the Prover.
    /// - This may be used by Signers to verify that the value correctly matches `cv`.
    ///
    /// This opens `cv` for all participants. For Signers who don't need this information,
    /// or after proofs / signatures have been applied, this can be redacted.
    pub rcv: Option<[u8; 32]>,

    /// The proof generation key `(ak, nsk)` corresponding to the recipient that received
    /// the note being spent.
    ///
    /// - This is set by the Updater.
    /// - This is required by the Prover.
    pub proof_generation_key: Option<([u8; 32], [u8; 32])>,

    /// A witness from the note to the bundle's anchor.
    ///
    /// - This is set by the Updater.
    /// - This is required by the Prover.
    pub witness: Option<(u32, [[u8; 32]; 32])>,

    /// The spend authorization randomizer.
    ///
    /// - This is chosen by the Constructor.
    /// - This is required by the Signer for creating `spend_auth_sig`, and may be used to
    ///   validate `rk`.
    /// - After`zkproof` / `spend_auth_sig` has been set, this can be redacted.
    pub alpha: Option<[u8; 32]>,

    /// The ZIP 32 derivation path at which the spending key can be found for the note
    /// being spent.
    pub zip32_derivation: Option<Zip32Derivation>,

    pub proprietary: BTreeMap<String, Vec<u8>>,
}

/// Information about a Sapling output within a transaction.
#[derive(Clone, Debug)]
pub struct Output {
    //
    // OutputDescription effecting data.
    //
    // These are required fields that are part of the final transaction, and are filled in
    // by the Constructor when adding an output.
    //
    pub cv: [u8; 32],
    pub cmu: [u8; 32],
    pub ephemeral_key: [u8; 32],
    /// The encrypted note plaintext for the output.
    ///
    /// Encoded as a `Vec<u8>` because its length depends on the transaction version.
    ///
    /// Once we have memo bundles, we will be able to set memos independently of Outputs.
    /// For now, the Constructor sets both at the same time.
    pub enc_ciphertext: Vec<u8>,
    /// The encrypted note plaintext for the output.
    ///
    /// Encoded as a `Vec<u8>` because its length depends on the transaction version.
    pub out_ciphertext: Vec<u8>,

    /// The Output proof.
    ///
    /// This is set by the Prover.
    pub zkproof: Option<[u8; GROTH_PROOF_SIZE]>,

    /// The address that will receive the output.
    ///
    /// - This is set by the Constructor.
    /// - This is required by the Prover.
    pub recipient: Option<[u8; 43]>,

    /// The value of the output.
    ///
    /// This may be used by Signers to verify that the value matches `cv`, and to confirm
    /// the values and change involved in the transaction.
    ///
    /// This exposes the output value to all participants. For Signers who don't need this
    /// information, or after signatures have been applied, this can be redacted.
    pub value: Option<u64>,

    /// The seed randomness for the output.
    ///
    /// - This is set by the Constructor.
    /// - This is required by the Prover, instead of disclosing `shared_secret` to them.
    pub rseed: Option<[u8; 32]>,

    /// The value commitment randomness.
    ///
    /// - This is set by the Constructor.
    /// - The IO Finalizer compresses it into the bsk.
    /// - This is required by the Prover.
    /// - This may be used by Signers to verify that the value correctly matches `cv`.
    ///
    /// This opens `cv` for all participants. For Signers who don't need this information,
    /// or after proofs / signatures have been applied, this can be redacted.
    pub rcv: Option<[u8; 32]>,

    /// The symmetric shared secret used to encrypt `enc_ciphertext`.
    ///
    /// This enables Signers to verify that `enc_ciphertext` is correctly encrypted (and
    /// contains a note plaintext matching the public commitments), and to confirm the
    /// value of the memo.
    pub shared_secret: Option<[u8; 32]>,

    /// The `ock` value used to encrypt `out_ciphertext`.
    ///
    /// This enables Signers to verify that `out_ciphertext` is correctly encrypted.
    ///
    /// This may be `None` if the Constructor added the output using an OVK policy of
    /// "None", to make the output unrecoverable from the chain by the sender.
    pub ock: Option<[u8; 32]>,

    /// The ZIP 32 derivation path at which the spending key can be found for the output.
    pub zip32_derivation: Option<Zip32Derivation>,

    pub proprietary: BTreeMap<String, Vec<u8>>,
}

impl Bundle {
    /// Merges this bundle with another.
    ///
    /// Returns `None` if the bundles have conflicting data.
    pub fn merge(mut self, other: Self) -> Option<Self> {
        // Destructure `other` to ensure we handle everything.
        let Self {
            mut spends,
            mut outputs,
            value_balance,
            anchor,
            bsk,
        } = other;

        // If `bsk` is set on either bundle, the IO Finalizer has run, which means we
        // cannot have differing numbers of spends or outputs, and the value balances must
        // match.
        match (self.bsk.as_mut(), bsk) {
            (Some(lhs), Some(rhs)) if lhs != &rhs => return None,
            (Some(_), _) | (_, Some(_))
                if self.spends.len() != spends.len()
                    || self.outputs.len() != outputs.len()
                    || self.value_balance != value_balance =>
            {
                return None
            }
            // IO Finalizer has run, and neither bundle has excess spends or outputs.
            (Some(_), _) | (_, Some(_)) => (),
            // IO Finalizer has not run on either bundle. If the other bundle has more
            // spends or outputs than us, move them over; these cannot conflict by
            // construction.
            (None, None) => {
                if spends.len() > self.spends.len() {
                    // TODO: Update `self.value_balance`.
                    self.spends.extend(spends.drain(self.spends.len()..));
                }
                if outputs.len() > self.outputs.len() {
                    // TODO: Update `self.value_balance`.
                    self.outputs.extend(outputs.drain(self.outputs.len()..));
                }
            }
        }

        if self.anchor != anchor {
            return None;
        }

        // Leverage the early-exit behaviour of zip to confirm that the remaining data in
        // the other bundle matches this one.
        for (lhs, rhs) in self.spends.iter_mut().zip(spends.into_iter()) {
            // Destructure `rhs` to ensure we handle everything.
            let Spend {
                cv,
                nullifier,
                rk,
                zkproof,
                spend_auth_sig,
                recipient,
                value,
                rseed,
                rcv,
                proof_generation_key,
                witness,
                alpha,
                zip32_derivation,
                proprietary,
            } = rhs;

            if lhs.cv != cv || lhs.nullifier != nullifier || lhs.rk != rk {
                return None;
            }

            if !(merge_optional(&mut lhs.zkproof, zkproof)
                && merge_optional(&mut lhs.spend_auth_sig, spend_auth_sig)
                && merge_optional(&mut lhs.recipient, recipient)
                && merge_optional(&mut lhs.value, value)
                && merge_optional(&mut lhs.rseed, rseed)
                && merge_optional(&mut lhs.rcv, rcv)
                && merge_optional(&mut lhs.proof_generation_key, proof_generation_key)
                && merge_optional(&mut lhs.witness, witness)
                && merge_optional(&mut lhs.alpha, alpha)
                && merge_optional(&mut lhs.zip32_derivation, zip32_derivation)
                && merge_map(&mut lhs.proprietary, proprietary))
            {
                return None;
            }
        }

        for (lhs, rhs) in self.outputs.iter_mut().zip(outputs.into_iter()) {
            // Destructure `rhs` to ensure we handle everything.
            let Output {
                cv,
                cmu,
                ephemeral_key,
                enc_ciphertext,
                out_ciphertext,
                zkproof,
                recipient,
                value,
                rseed,
                rcv,
                shared_secret,
                ock,
                zip32_derivation,
                proprietary,
            } = rhs;

            if lhs.cv != cv
                || lhs.cmu != cmu
                || lhs.ephemeral_key != ephemeral_key
                || lhs.enc_ciphertext != enc_ciphertext
                || lhs.out_ciphertext != out_ciphertext
            {
                return None;
            }

            if !(merge_optional(&mut lhs.zkproof, zkproof)
                && merge_optional(&mut lhs.recipient, recipient)
                && merge_optional(&mut lhs.value, value)
                && merge_optional(&mut lhs.rseed, rseed)
                && merge_optional(&mut lhs.rcv, rcv)
                && merge_optional(&mut lhs.shared_secret, shared_secret)
                && merge_optional(&mut lhs.ock, ock)
                && merge_optional(&mut lhs.zip32_derivation, zip32_derivation)
                && merge_map(&mut lhs.proprietary, proprietary))
            {
                return None;
            }
        }

        Some(self)
    }
}
