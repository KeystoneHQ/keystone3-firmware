use crate::pczt::Pczt;
use crate::zcash_encoding::WriteBytesExt;
use alloc::collections::btree_map::BTreeMap;
use blake2b_simd::{Hash, Params, State};
use byteorder::LittleEndian;

use super::common::Zip32Derivation;
use super::merge_map;
use super::transparent::{Input, Output};

/// TxId tree root personalization
const ZCASH_TX_PERSONALIZATION_PREFIX: &[u8; 12] = b"ZcashTxHash_";

// TxId level 1 node personalization
const ZCASH_HEADERS_HASH_PERSONALIZATION: &[u8; 16] = b"ZTxIdHeadersHash";
pub const ZCASH_TRANSPARENT_HASH_PERSONALIZATION: &[u8; 16] = b"ZTxIdTranspaHash";
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

pub type ZcashSignature = [u8; 64];

pub trait PcztSigner {
    type Error;
    fn sign_transparent(
        &self,
        hash: &[u8],
        key_path: BTreeMap<[u8; 33], Zip32Derivation>,
    ) -> Result<BTreeMap<[u8; 33], ZcashSignature>, Self::Error>;
    fn sign_sapling(
        &self,
        hash: Option<Hash>,
        alpha: [u8; 32],
        path: Zip32Derivation,
    ) -> Result<Option<ZcashSignature>, Self::Error>;
    fn sign_orchard(
        &self,
        hash: Option<Hash>,
        alpha: [u8; 32],
        path: Zip32Derivation,
    ) -> Result<Option<ZcashSignature>, Self::Error>;
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
        !self.sapling.spends.is_empty() && !self.sapling.outputs.is_empty()
    }

    fn has_orchard(&self) -> bool {
        !self.orchard.actions.is_empty()
    }

    fn determine_lock_time(&self) -> Result<u32, ()> {
        // The nLockTime field of a transaction is determined by inspecting the
        // `Global.fallback_lock_time` and each input's `required_time_lock_time` and
        // `required_height_lock_time` fields.

        // If one or more inputs have a `required_time_lock_time` or `required_height_lock_time`,
        let have_required_lock_time = self.transparent.inputs.iter().any(|input| {
            input.required_time_lock_time.is_some() || input.required_height_lock_time.is_some()
        });
        // then the field chosen is the one which is supported by all of the inputs. This can
        // be determined by looking at all of the inputs which specify a locktime in either of
        // those fields, and choosing the field which is present in all of those inputs.
        // Inputs not specifying a lock time field can take both types of lock times, as can
        // those that specify both.
        let time_lock_time_unsupported = self.transparent.inputs
            .iter()
            .any(|input| input.required_height_lock_time.is_some());
        let height_lock_time_unsupported = self.transparent.inputs
            .iter()
            .any(|input| input.required_time_lock_time.is_some());

        // The lock time chosen is then the maximum value of the chosen type of lock time.
        match (
            have_required_lock_time,
            time_lock_time_unsupported,
            height_lock_time_unsupported,
        ) {
            (true, true, true) => Err(()),
            (true, false, true) => Ok(self.transparent.inputs
                .iter()
                .filter_map(|input| input.required_time_lock_time)
                .max()
                .expect("iterator is non-empty because have_required_lock_time is true")),
            // If a PSBT has both types of locktimes possible because one or more inputs
            // specify both `required_time_lock_time` and `required_height_lock_time`, then
            // locktime determined by looking at the `required_height_lock_time` fields of the
            // inputs must be chosen.
            (true, _, false) => Ok(self.transparent.inputs
                .iter()
                .filter_map(|input| input.required_height_lock_time)
                .max()
                .expect("iterator is non-empty because have_required_lock_time is true")),
            // If none of the inputs have a `required_time_lock_time` and
            // `required_height_lock_time`, then `Global.fallback_lock_time` must be used. If
            // `Global.fallback_lock_time` is not provided, then it is assumed to be 0.
            (false, _, _) => Ok(self.global.fallback_lock_time.unwrap_or(0)),
        }
    }

    fn digest_header(&self) -> Result<Hash, ()> {
        let version = self.global.tx_version;
        let version_group_id = self.global.version_group_id;
        let consensus_branch_id = self.global.consensus_branch_id;
        let lock_time = self.determine_lock_time()?;
        let expiry_height = self.global.expiry_height;

        let mut h = hasher(ZCASH_HEADERS_HASH_PERSONALIZATION);

        h.update(&((1 << 31) | version).to_le_bytes());
        h.update(&version_group_id.to_le_bytes());
        h.update(&consensus_branch_id.to_le_bytes());
        h.update(&lock_time.to_le_bytes());
        h.update(&expiry_height.to_le_bytes());

        Ok(h.finalize())
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
            h.update(&input.sequence.unwrap_or(0xffffffff).to_le_bytes());
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
            ch.update(&action.spend.nullifier);
            ch.update(&action.output.cmx);
            ch.update(&action.output.ephemeral_key);
            ch.update(&action.output.enc_ciphertext[..52]);

            mh.update(&action.output.enc_ciphertext[52..564]);

            nh.update(&action.cv_net);
            nh.update(&action.spend.rk);
            nh.update(&action.output.enc_ciphertext[564..]);
            nh.update(&action.output.out_ciphertext);
        }

        h.update(ch.finalize().as_bytes());
        h.update(mh.finalize().as_bytes());
        h.update(nh.finalize().as_bytes());
        h.update(&[self.orchard.flags]);
        let value_balance = match self.orchard.value_sum {
            (magnitude, sign) => {
                if sign {
                    magnitude as i128
                } else {
                    -(magnitude as i128)
                }
            }
        };
        h.update(&value_balance.to_le_bytes());
        h.update(&self.orchard.anchor);
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
                nh.update(&self.sapling.anchor);
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
            let value_balance = self.sapling.value_sum;
            h.update(&self.sapling.value_sum.to_le_bytes());
        }
        h.finalize()
    }

    fn hash_sapling_txid_empty() -> Hash {
        hasher(ZCASH_SAPLING_HASH_PERSONALIZATION).finalize()
    }

    fn hash_orchard_txid_empty() -> Hash {
        hasher(ZCASH_ORCHARD_HASH_PERSONALIZATION).finalize()
    }

    fn sheilded_sig_commitment(&self) -> Result<Hash, ()> {
        let mut personal = [0; 16];
        personal[..12].copy_from_slice(ZCASH_TX_PERSONALIZATION_PREFIX);
        (&mut personal[12..])
            .write_u32::<LittleEndian>(self.global.consensus_branch_id.into())
            .unwrap();

        let mut h = hasher(&personal);
        h.update(self.digest_header()?.as_bytes());
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
        Ok(h.finalize())
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
                    ch.update(&input.sequence.unwrap_or(0xffffffff).to_le_bytes());
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

    pub fn sign<T: PcztSigner>(&self, signer: &T) -> Result<Self, T::Error> {
        let mut pczt = self.clone();
        pczt.transparent
            .inputs
            .iter_mut()
            .enumerate()
            .try_for_each(|(i, input)| {
                let signatures = signer.sign_transparent(
                    self.transparent_sig_digest(Some((input, i as u32)))
                        .as_bytes(),
                    input.bip32_derivation.clone(),
                )?;
                merge_map(
                    &mut input.partial_signatures,
                    signatures
                        .iter()
                        .map(|(pubkey, signature)| (pubkey.clone(), signature.to_vec()))
                        .collect(),
                );
                Ok(())
            })?;
        pczt.sapling.spends.iter_mut().try_for_each(|spend| {
            if let Some(ref d) = spend.zip32_derivation {
                let signature = signer.sign_sapling(
                    self.sheilded_sig_commitment().ok(),
                    pczt.sapling.anchor,
                    d.clone(),
                )?;
                spend.spend_auth_sig = signature;
            }
            Ok(())
        })?;
        pczt.orchard.actions.iter_mut().try_for_each(|action| {
            if let Some(ref d) = action.spend.zip32_derivation {
                let signature = signer.sign_orchard(
                    self.sheilded_sig_commitment().ok(),
                    action.spend.alpha.unwrap(),
                    d.clone(),
                )?;
                action.spend.spend_auth_sig = signature;
            }
            Ok(())
        })?;
        Ok(pczt)
    }
}

#[cfg(test)]
mod tests {
    extern crate std;
    use alloc::vec;
    use alloc::{collections::btree_map::BTreeMap, vec::Vec};
    use secp256k1::Message;
    use std::println;

    use crate::pczt::common::Zip32Derivation;
    use crate::pczt::{
        self,
        common::Global,
        orchard::{self, Action},
        sapling, transparent, V5_TX_VERSION, V5_VERSION_GROUP_ID,
    };

    const HARDENED_MASK: u32 = 0x8000_0000;

    use super::*;

    #[test]
    fn test_pczt_hash() {
        
    }
}
