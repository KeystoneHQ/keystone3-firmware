use crate::pczt::Pczt;
use crate::zcash_encoding::WriteBytesExt;
use alloc::vec;
use alloc::vec::Vec;
use blake2b_simd::{Hash, Params, State};
use byteorder::LittleEndian;

use super::transparent::{Input, Output};

/// TxId tree root personalization
const ZCASH_TX_PERSONALIZATION_PREFIX: &[u8; 12] = b"ZcashTxHash_";

// TxId level 1 node personalization
const ZCASH_HEADERS_HASH_PERSONALIZATION: &[u8; 16] = b"ZTxIdHeadersHash";
pub(crate) const ZCASH_TRANSPARENT_HASH_PERSONALIZATION: &[u8; 16] = b"ZTxIdTranspaHash";
const ZCASH_SAPLING_HASH_PERSONALIZATION: &[u8; 16] = b"ZTxIdSaplingHash";
#[cfg(zcash_unstable = "zfuture")]
const ZCASH_TZE_HASH_PERSONALIZATION: &[u8; 16] = b"ZTxIdTZE____Hash";

// TxId transparent level 2 node personalization
const ZCASH_PREVOUTS_HASH_PERSONALIZATION: &[u8; 16] = b"ZTxIdPrevoutHash";
const ZCASH_SEQUENCE_HASH_PERSONALIZATION: &[u8; 16] = b"ZTxIdSequencHash";
const ZCASH_OUTPUTS_HASH_PERSONALIZATION: &[u8; 16] = b"ZTxIdOutputsHash";

// TxId tze level 2 node personalization
#[cfg(zcash_unstable = "zfuture")]
const ZCASH_TZE_INPUTS_HASH_PERSONALIZATION: &[u8; 16] = b"ZTxIdTZEIns_Hash";
#[cfg(zcash_unstable = "zfuture")]
const ZCASH_TZE_OUTPUTS_HASH_PERSONALIZATION: &[u8; 16] = b"ZTxIdTZEOutsHash";

// TxId sapling level 2 node personalization
const ZCASH_SAPLING_SPENDS_HASH_PERSONALIZATION: &[u8; 16] = b"ZTxIdSSpendsHash";
const ZCASH_SAPLING_SPENDS_COMPACT_HASH_PERSONALIZATION: &[u8; 16] = b"ZTxIdSSpendCHash";
const ZCASH_SAPLING_SPENDS_NONCOMPACT_HASH_PERSONALIZATION: &[u8; 16] = b"ZTxIdSSpendNHash";

const ZCASH_SAPLING_OUTPUTS_HASH_PERSONALIZATION: &[u8; 16] = b"ZTxIdSOutputHash";
const ZCASH_SAPLING_OUTPUTS_COMPACT_HASH_PERSONALIZATION: &[u8; 16] = b"ZTxIdSOutC__Hash";
const ZCASH_SAPLING_OUTPUTS_MEMOS_HASH_PERSONALIZATION: &[u8; 16] = b"ZTxIdSOutM__Hash";
const ZCASH_SAPLING_OUTPUTS_NONCOMPACT_HASH_PERSONALIZATION: &[u8; 16] = b"ZTxIdSOutN__Hash";

const ZCASH_AUTH_PERSONALIZATION_PREFIX: &[u8; 12] = b"ZTxAuthHash_";
const ZCASH_AUTH_SCRIPTS_HASH_PERSONALIZATION: &[u8; 16] = b"ZTxAuthTransHash";
const ZCASH_SAPLING_SIGS_HASH_PERSONALIZATION: &[u8; 16] = b"ZTxAuthSapliHash";
#[cfg(zcash_unstable = "zfuture")]
const ZCASH_TZE_WITNESSES_HASH_PERSONALIZATION: &[u8; 16] = b"ZTxAuthTZE__Hash";

const ZCASH_ORCHARD_HASH_PERSONALIZATION: &[u8; 16] = b"ZTxIdOrchardHash";
const ZCASH_ORCHARD_ACTIONS_COMPACT_HASH_PERSONALIZATION: &[u8; 16] = b"ZTxIdOrcActCHash";
const ZCASH_ORCHARD_ACTIONS_MEMOS_HASH_PERSONALIZATION: &[u8; 16] = b"ZTxIdOrcActMHash";
const ZCASH_ORCHARD_ACTIONS_NONCOMPACT_HASH_PERSONALIZATION: &[u8; 16] = b"ZTxIdOrcActNHash";
const ZCASH_ORCHARD_SIGS_HASH_PERSONALIZATION: &[u8; 16] = b"ZTxAuthOrchaHash";

const ZCASH_TRANSPARENT_INPUT_HASH_PERSONALIZATION: &[u8; 16] = b"Zcash___TxInHash";
const ZCASH_TRANSPARENT_AMOUNTS_HASH_PERSONALIZATION: &[u8; 16] = b"ZTxTrAmountsHash";
const ZCASH_TRANSPARENT_SCRIPTS_HASH_PERSONALIZATION: &[u8; 16] = b"ZTxTrScriptsHash";

pub const SIGHASH_ALL: u8 = 0x01;
pub const SIGHASH_NONE: u8 = 0x02;
pub const SIGHASH_SINGLE: u8 = 0x03;
pub const SIGHASH_MASK: u8 = 0x1f;
pub const SIGHASH_ANYONECANPAY: u8 = 0x80;

fn hasher(personal: &[u8; 16]) -> State {
    Params::new().hash_length(32).personal(personal).to_state()
}

struct TransparentDigests {
    prevouts_digest: Hash,
    sequence_digest: Hash,
    outputs_digest: Hash,
}

impl Pczt {
    fn has_transparent(&self) -> bool {
        !self.transparent.inputs.is_empty() && !self.transparent.outputs.is_empty()
    }

    fn is_transparent_coinbase(&self) -> bool {
        self.transparent.inputs.len() == 1
            && self.transparent.inputs[0].prevout_index == u32::MAX
            && self.transparent.inputs[0].prevout_txid.as_ref() == &[0u8; 32]
    }

    fn has_sapling(&self) -> bool {
        self.sapling.anchor.is_some()
    }

    fn has_orchard(&self) -> bool {
        self.orchard.anchor.is_some()
    }

    fn digest_header(&self) -> Hash {
        let version = self.global.tx_version;
        let version_group_id = self.global.version_group_id;
        let consensus_branch_id = self.global.consensus_branch_id;
        let lock_time = self.global.lock_time;
        let expiry_height = self.global.expiry_height;

        let mut h = hasher(ZCASH_HEADERS_HASH_PERSONALIZATION);

        h.update(&version.to_le_bytes());
        h.update(&version_group_id.to_le_bytes());
        h.update(&consensus_branch_id.to_le_bytes());
        h.update(&lock_time.to_le_bytes());
        h.update(&expiry_height.to_le_bytes());

        h.finalize()
    }
    fn digest_transparent_prevouts(inputs: &[Input]) -> Hash {
        let mut h = hasher(ZCASH_PREVOUTS_HASH_PERSONALIZATION);

        for input in inputs {
            h.update(&input.prevout_txid);
            h.update(&input.prevout_index.to_le_bytes());
        }
        h.finalize()
    }
    fn digest_transparent_sequence(inputs: &[Input]) -> Hash {
        let mut h = hasher(ZCASH_SEQUENCE_HASH_PERSONALIZATION);
        for input in inputs {
            h.update(&input.sequence.to_le_bytes());
        }
        h.finalize()
    }
    fn digest_transparent_outputs(outputs: &[Output]) -> Hash {
        let mut h = hasher(ZCASH_OUTPUTS_HASH_PERSONALIZATION);
        for output in outputs {
            h.update(&output.value.to_le_bytes());
            h.update(&output.script_pubkey);
        }
        h.finalize()
    }
    fn transparent_digest(&self) -> TransparentDigests {
        TransparentDigests {
            prevouts_digest: Self::digest_transparent_prevouts(&self.transparent.inputs),
            sequence_digest: Self::digest_transparent_sequence(&self.transparent.inputs),
            outputs_digest: Self::digest_transparent_outputs(&self.transparent.outputs),
        }
    }
    fn hash_transparent_tx_id(&self, t_digests: Option<TransparentDigests>) -> Hash {
        let mut h = hasher(ZCASH_TRANSPARENT_HASH_PERSONALIZATION);
        if let Some(d) = t_digests {
            h.update(d.prevouts_digest.as_bytes());
            h.update(d.sequence_digest.as_bytes());
            h.update(d.outputs_digest.as_bytes());
        }
        h.finalize()
    }

    fn digest_orchard(&self) -> Hash {
        let mut h = hasher(ZCASH_ORCHARD_HASH_PERSONALIZATION);
        let mut ch = hasher(ZCASH_ORCHARD_ACTIONS_COMPACT_HASH_PERSONALIZATION);
        let mut mh = hasher(ZCASH_ORCHARD_ACTIONS_MEMOS_HASH_PERSONALIZATION);
        let mut nh = hasher(ZCASH_ORCHARD_ACTIONS_NONCOMPACT_HASH_PERSONALIZATION);

        for action in self.orchard.actions.iter() {
            // println!("{:?}", &action);
            ch.update(&action.spend.nullifier);
            ch.update(&action.output.cmx);
            ch.update(&action.output.ephemeral_key);
            ch.update(&action.output.enc_ciphertext[..52]);

            mh.update(&action.output.enc_ciphertext[52..564]);

            nh.update(&action.cv);
            nh.update(&action.spend.rk);
            nh.update(&action.output.enc_ciphertext[564..]);
            nh.update(&action.output.out_ciphertext);
        }

        h.update(ch.finalize().as_bytes());
        h.update(mh.finalize().as_bytes());
        h.update(nh.finalize().as_bytes());
        h.update(&[self.orchard.flags]);
        h.update(&self.orchard.value_balance.to_le_bytes());
        h.update(&self.orchard.anchor.unwrap());
        h.finalize()
    }

    fn hash_sapling_spends(&self) -> Hash {
        let mut h = hasher(ZCASH_SAPLING_SPENDS_HASH_PERSONALIZATION);
        if !self.sapling.spends.is_empty() {
            let mut ch = hasher(ZCASH_SAPLING_SPENDS_COMPACT_HASH_PERSONALIZATION);
            let mut nh = hasher(ZCASH_SAPLING_SPENDS_NONCOMPACT_HASH_PERSONALIZATION);
            for s_spend in &self.sapling.spends {
                // we build the hash of nullifiers separately for compact blocks.
                ch.update(&s_spend.nullifier);

                nh.update(&s_spend.cv);
                nh.update(&self.sapling.anchor.unwrap());
                nh.update(&s_spend.rk);
            }

            let compact_digest = ch.finalize();
            h.update(compact_digest.as_bytes());
            let noncompact_digest = nh.finalize();
            h.update(noncompact_digest.as_bytes());
        }
        h.finalize()
    }

    fn hash_sapling_outputs(&self) -> Hash {
        let mut h = hasher(ZCASH_SAPLING_OUTPUTS_HASH_PERSONALIZATION);
        if !self.sapling.outputs.is_empty() {
            let mut ch = hasher(ZCASH_SAPLING_OUTPUTS_COMPACT_HASH_PERSONALIZATION);
            let mut mh = hasher(ZCASH_SAPLING_OUTPUTS_MEMOS_HASH_PERSONALIZATION);
            let mut nh = hasher(ZCASH_SAPLING_OUTPUTS_NONCOMPACT_HASH_PERSONALIZATION);
            for s_out in &self.sapling.outputs {
                ch.update(&s_out.cmu);
                ch.update(&s_out.ephemeral_key);
                ch.update(&s_out.enc_ciphertext[..52]);

                mh.update(&s_out.enc_ciphertext[52..564]);

                nh.update(&s_out.cv);
                nh.update(&s_out.enc_ciphertext[564..]);
                nh.update(&s_out.out_ciphertext);
            }

            h.update(ch.finalize().as_bytes());
            h.update(mh.finalize().as_bytes());
            h.update(nh.finalize().as_bytes());
        }
        h.finalize()
    }

    fn digest_sapling(&self) -> Hash {
        let mut h = hasher(ZCASH_SAPLING_HASH_PERSONALIZATION);
        if !(self.sapling.spends.is_empty() && self.sapling.outputs.is_empty()) {
            h.update(self.hash_sapling_spends().as_bytes());
            h.update(self.hash_sapling_outputs().as_bytes());

            h.update(&self.sapling.value_balance.to_le_bytes());
        }
        h.finalize()
    }

    fn hash_sapling_txid_empty() -> Hash {
        hasher(ZCASH_SAPLING_HASH_PERSONALIZATION).finalize()
    }

    fn hash_orchard_txid_empty() -> Hash {
        hasher(ZCASH_ORCHARD_HASH_PERSONALIZATION).finalize()
    }

    fn sheilded_sig_commitment(&self) -> Hash {
        let mut personal = [0; 16];
        personal[..12].copy_from_slice(ZCASH_TX_PERSONALIZATION_PREFIX);
        (&mut personal[12..])
            .write_u32::<LittleEndian>(self.global.consensus_branch_id.into())
            .unwrap();

        let mut h = hasher(&personal);
        h.update(self.digest_header().as_bytes());
        h.update(self.transparent_sig_digest(None).as_bytes());
        h.update(
            self.has_sapling()
                .then(|| self.digest_sapling())
                .unwrap_or_else(Self::hash_sapling_txid_empty)
                .as_bytes(),
        );
        h.update(
            self.has_orchard()
                .then(|| self.digest_orchard())
                .unwrap_or_else(Self::hash_orchard_txid_empty)
                .as_bytes(),
        );
        h.finalize()
    }

    fn transparent_sig_digest(&self, input_info: Option<(&Input, u32)>) -> Hash {
        if !self.has_transparent() {
            self.hash_transparent_tx_id(None)
        } else {
            if self.is_transparent_coinbase() || self.transparent.inputs.is_empty() {
                self.hash_transparent_tx_id(Some(self.transparent_digest()))
            } else {
                //SIGHASH_ALL
                let prevouts_digest = Self::digest_transparent_prevouts(&self.transparent.inputs);

                let amounts_digest = {
                    let mut h = hasher(ZCASH_TRANSPARENT_AMOUNTS_HASH_PERSONALIZATION);
                    self.transparent.inputs.iter().for_each(|input| {
                        h.update(&input.value.to_le_bytes());
                    });
                    h.finalize()
                };

                let scripts_digest = {
                    let mut h = hasher(ZCASH_TRANSPARENT_SCRIPTS_HASH_PERSONALIZATION);
                    self.transparent.inputs.iter().for_each(|input| {
                        h.update(&input.script_pubkey);
                    });
                    h.finalize()
                };

                let sequence_digest = Self::digest_transparent_sequence(&self.transparent.inputs);

                let outputs_digest = Self::digest_transparent_outputs(&self.transparent.outputs);

                //S.2g.i:   prevout      (field encoding)
                //S.2g.ii:  value        (8-byte signed little-endian)
                //S.2g.iii: scriptPubKey (field encoding)
                //S.2g.iv:  nSequence    (4-byte unsigned little-endian)
                let mut ch = hasher(ZCASH_TRANSPARENT_INPUT_HASH_PERSONALIZATION);
                if let Some((input, index)) = input_info {
                    ch.update(&input.prevout_txid);
                    ch.update(&input.prevout_index.to_le_bytes());
                    ch.update(&(input.value as i64).to_le_bytes());
                    ch.update(&input.script_pubkey);
                    ch.update(&input.sequence.to_le_bytes());
                }
                let txin_sig_digest = ch.finalize();

                let mut h = hasher(ZCASH_TRANSPARENT_HASH_PERSONALIZATION);
                h.update(&[SIGHASH_ALL]);
                h.update(prevouts_digest.as_bytes());
                h.update(amounts_digest.as_bytes());
                h.update(scripts_digest.as_bytes());
                h.update(sequence_digest.as_bytes());
                h.update(outputs_digest.as_bytes());
                h.update(txin_sig_digest.as_bytes());
                h.finalize()
            }
        }
    }
}
