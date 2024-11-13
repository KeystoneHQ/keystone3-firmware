use alloc::{collections::BTreeMap, string::String, vec::Vec};

use super::{common::Zip32Derivation, merge_map, merge_optional};

/// PCZT fields that are specific to producing the transaction's Orchard bundle (if any).
#[derive(Clone, Debug)]
pub struct Bundle {
    /// The Orchard actions in this bundle.
    pub actions: Vec<Action>,

    /// The flags for the Orchard bundle.
    pub flags: u8,

    /// The net value of Orchard spends minus outputs.
    pub value_balance: u64,

    /// The Orchard anchor for this transaction.
    pub anchor: [u8; 32],

    /// The Orchard bundle proof.
    pub zkproof: Option<Vec<u8>>,

    /// The Orchard binding signature signing key.
    pub bsk: Option<[u8; 32]>,
}

#[derive(Clone, Debug)]
pub struct Action {
    pub cv: [u8; 32],
    pub spend: Spend,
    pub output: Output,
    pub rcv: Option<[u8; 32]>,
}

/// Information about a Sapling spend within a transaction.
#[derive(Clone, Debug)]
pub struct Spend {
    pub nullifier: [u8; 32],
    pub rk: [u8; 32],
    pub spend_auth_sig: Option<[u8; 64]>,
    pub recipient: Option<[u8; 43]>,
    pub value: Option<u64>,
    pub rho: Option<[u8; 32]>,
    pub rseed: Option<[u8; 32]>,
    pub fvk: Option<[u8; 96]>,
    pub witness: Option<(u32, [[u8; 32]; 32])>,
    pub alpha: Option<[u8; 32]>,
    pub zip32_derivation: Option<Zip32Derivation>,
    pub proprietary: BTreeMap<String, Vec<u8>>,
}

/// Information about an Orchard output within a transaction.
#[derive(Clone, Debug)]
pub struct Output {
    pub cmx: [u8; 32],
    pub ephemeral_key: [u8; 32],
    pub enc_ciphertext: Vec<u8>,
    pub out_ciphertext: Vec<u8>,
    pub recipient: Option<[u8; 43]>,
    pub value: Option<u64>,
    pub rseed: Option<[u8; 32]>,
    pub shared_secret: Option<[u8; 32]>,
    pub ock: Option<[u8; 32]>,
    pub zip32_derivation: Option<Zip32Derivation>,
    pub proprietary: BTreeMap<String, Vec<u8>>,
}

impl Bundle {
    /// Merges this bundle with another.
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

        if self.anchor != anchor {
            return None;
        }

        if !merge_optional(&mut self.zkproof, zkproof) {
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
                        zip32_derivation: spend_zip32_derivation,
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
                        zip32_derivation: output_zip32_derivation,
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
                && merge_optional(&mut lhs.spend.zip32_derivation, spend_zip32_derivation)
                && merge_map(&mut lhs.spend.proprietary, spend_proprietary)
                && merge_optional(&mut lhs.output.recipient, output_recipient)
                && merge_optional(&mut lhs.output.value, output_value)
                && merge_optional(&mut lhs.output.rseed, output_rseed)
                && merge_optional(&mut lhs.output.shared_secret, shared_secret)
                && merge_optional(&mut lhs.output.ock, ock)
                && merge_optional(&mut lhs.output.zip32_derivation, output_zip32_derivation)
                && merge_map(&mut lhs.output.proprietary, output_proprietary)
                && merge_optional(&mut lhs.rcv, rcv))
            {
                return None;
            }
        }

        Some(self)
    }
}
