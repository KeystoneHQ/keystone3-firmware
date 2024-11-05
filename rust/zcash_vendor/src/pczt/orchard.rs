use alloc::{collections::BTreeMap, string::String, vec::Vec};

use super::merge_optional;

/// PCZT fields that are specific to producing the transaction's Orchard bundle (if any).
#[derive(Clone, Debug)]
pub struct Bundle {
    /// The Orchard actions in this bundle.
    ///
    /// Entries are added by the Constructor, and modified by an Updater, IO Finalizer,
    /// Signer, Combiner, or Spend Finalizer.
    pub actions: Vec<Action>,

    /// The flags for the Orchard bundle.
    ///
    /// Contains:
    /// - `enableSpendsOrchard` flag (bit 0)
    /// - `enableOutputsOrchard` flag (bit 1)
    /// - Reserved, zeros (bits 2..=7)
    ///
    /// This is set by the Creator. The Constructor MUST only add spends and outputs that
    /// are consistent with these flags (i.e. are dummies as appropriate).
    pub flags: u8,

    /// The net value of Orchard spends minus outputs.
    ///
    /// This is initialized by the Creator, and updated by the Constructor as spends or
    /// outputs are added to the PCZT. It enables per-spend and per-output values to be
    /// redacted from the PCZT after they are no longer necessary.
    pub value_balance: i64,

    /// The Orchard anchor for this transaction.
    ///
    /// TODO: Should this be non-optional and set by the Creator (which would be simpler)?
    /// Or do we need a separate role that picks the anchor, which runs before the
    /// Constructor adds spends?
    pub anchor: Option<[u8; 32]>,

    /// The Orchard bundle proof.
    ///
    /// This is `None` until it is set by the Prover.
    pub zkproof: Option<Vec<u8>>,

    /// The Orchard binding signature signing key.
    ///
    /// - This is `None` until it is set by the IO Finalizer.
    /// - The Transaction Extractor uses this to produce the binding signature.
    pub bsk: Option<[u8; 32]>,
}

#[derive(Clone, Debug)]
pub struct Action {
    //
    // Action effecting data.
    //
    // These are required fields that are part of the final transaction, and are filled in
    // by the Constructor when adding an output.
    //
    pub cv: [u8; 32],
    pub spend: Spend,
    pub output: Output,

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
}

/// Information about a Sapling spend within a transaction.
#[derive(Clone, Debug)]
pub struct Spend {
    //
    // Spend-specific Action effecting data.
    //
    // These are required fields that are part of the final transaction, and are filled in
    // by the Constructor when adding an output.
    //
    pub nullifier: [u8; 32],
    pub rk: [u8; 32],

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
    /// - This is required by the Prover.
    /// - This may be used by Signers to verify that the value matches `cv`, and to
    ///   confirm the values and change involved in the transaction.
    ///
    /// This exposes the input value to all participants. For Signers who don't need this
    /// information, or after signatures have been applied, this can be redacted.
    pub value: Option<u64>,

    /// The rho value for the note being spent.
    ///
    /// - This is set by the Constructor.
    /// - This is required by the Prover.
    ///
    /// TODO: This could be merged with `rseed` into a tuple. `recipient` and `value` are
    /// separate because they might need to be independently redacted. (For which role?)
    pub rho: Option<[u8; 32]>,

    /// The seed randomness for the note being spent.
    ///
    /// - This is set by the Constructor.
    /// - This is required by the Prover.
    pub rseed: Option<[u8; 32]>,

    /// The full viewing key that received the note being spent.
    ///
    /// - This is set by the Updater.
    /// - This is required by the Prover.
    pub fvk: Option<[u8; 96]>,

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

    // TODO derivation path

    // TODO FROST

    pub proprietary: BTreeMap<String, Vec<u8>>,
}

/// Information about an Orchard output within a transaction.
#[derive(Clone, Debug)]
pub struct Output {
    //
    // Output-specific Action effecting data.
    //
    // These are required fields that are part of the final transaction, and are filled in
    // by the Constructor when adding an output.
    //
    pub cmx: [u8; 32],
    pub ephemeral_key: [u8; 32],
    /// TODO: Should it be possible to choose the memo _value_ after defining an Output?
    pub enc_ciphertext: [u8; 580],
    pub out_ciphertext: [u8; 80],

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
    /// This exposes the value to all participants. For Signers who don't need this
    /// information, we can drop the values and compress the rcvs into the bsk global.
    pub value: Option<u64>,

    /// The seed randomness for the output.
    ///
    /// - This is set by the Constructor.
    /// - This is required by the Prover.
    ///
    /// TODO: This could instead be decrypted from `enc_ciphertext` if `shared_secret`
    /// were required by the Prover. Likewise for `recipient` and `value`; is there ever a
    /// need for these to be independently redacted though?
    pub rseed: Option<[u8; 32]>,

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

    // TODO derivation path

    pub proprietary: BTreeMap<String, Vec<u8>>,
}

impl Bundle {
    /// Merges this bundle with another.
    ///
    /// Returns `None` if the bundles have conflicting data.
    pub fn merge(mut self, other: Self) -> Option<Self> {
        // Destructure `other` to ensure we handle everything.
        let Self {
            mut actions,
            flags,
            value_balance,
            anchor,
            zkproof,
            bsk,
        } = other;

        if self.flags != flags {
            return None;
        }

        // If `bsk` is set on either bundle, the IO Finalizer has run, which means we
        // cannot have differing numbers of actions, and the value balances must match.
        match (self.bsk.as_mut(), bsk) {
            (Some(lhs), Some(rhs)) if lhs != &rhs => return None,
            (Some(_), _) | (_, Some(_))
                if self.actions.len() != actions.len() || self.value_balance != value_balance =>
            {
                return None
            }
            // IO Finalizer has run, and neither bundle has excess spends or outputs.
            (Some(_), _) | (_, Some(_)) => (),
            // IO Finalizer has not run on either bundle. If the other bundle has more
            // spends or outputs than us, move them over; these cannot conflict by
            // construction.
            (None, None) => {
                if actions.len() > self.actions.len() {
                    self.actions.extend(actions.drain(self.actions.len()..));

                    // We check below that the overlapping actions match. Assuming here
                    // that they will, we can take the other bundle's value balance.
                    self.value_balance = value_balance;
                }
            }
        }

        if !(merge_optional(&mut self.anchor, anchor) && merge_optional(&mut self.zkproof, zkproof))
        {
            return None;
        }

        // Leverage the early-exit behaviour of zip to confirm that the remaining data in
        // the other bundle matches this one.
        for (lhs, rhs) in self.actions.iter_mut().zip(actions.into_iter()) {
            // Destructure `rhs` to ensure we handle everything.
            let Action {
                cv,
                spend:
                    Spend {
                        nullifier,
                        rk,
                        spend_auth_sig,
                        recipient,
                        value,
                        rho,
                        rseed,
                        fvk,
                        witness,
                        alpha,
                        proprietary: spend_proprietary,
                    },
                output:
                    Output {
                        cmx,
                        ephemeral_key,
                        enc_ciphertext,
                        out_ciphertext,
                        recipient: output_recipient,
                        value: output_value,
                        rseed: output_rseed,
                        shared_secret,
                        ock,
                        proprietary: output_proprietary,
                    },
                rcv,
            } = rhs;

            if lhs.cv != cv
                || lhs.spend.nullifier != nullifier
                || lhs.spend.rk != rk
                || lhs.output.cmx != cmx
                || lhs.output.ephemeral_key != ephemeral_key
                || lhs.output.enc_ciphertext != enc_ciphertext
                || lhs.output.out_ciphertext != out_ciphertext
            {
                return None;
            }

            if !(merge_optional(&mut lhs.spend.spend_auth_sig, spend_auth_sig)
                && merge_optional(&mut lhs.spend.recipient, recipient)
                && merge_optional(&mut lhs.spend.value, value)
                && merge_optional(&mut lhs.spend.rho, rho)
                && merge_optional(&mut lhs.spend.rseed, rseed)
                && merge_optional(&mut lhs.spend.fvk, fvk)
                && merge_optional(&mut lhs.spend.witness, witness)
                && merge_optional(&mut lhs.spend.alpha, alpha)
                && merge_optional(&mut lhs.output.recipient, output_recipient)
                && merge_optional(&mut lhs.output.value, output_value)
                && merge_optional(&mut lhs.output.rseed, output_rseed)
                && merge_optional(&mut lhs.output.shared_secret, shared_secret)
                && merge_optional(&mut lhs.output.ock, ock)
                && merge_optional(&mut lhs.rcv, rcv))
            {
                return None;
            }

            // TODO: Decide how to merge proprietary fields.
        }

        Some(self)
    }
}
