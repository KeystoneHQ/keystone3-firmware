use crate::pczt::Pczt;
use crate::zcash_encoding::WriteBytesExt;
use alloc::collections::btree_map::BTreeMap;
use alloc::string::String;
use alloc::string::ToString;
use alloc::vec::Vec;
use blake2b_simd::{Hash, Params, State};
use byteorder::LittleEndian;
use pasta_curves::Fq;

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
        key_path: BTreeMap<Vec<u8>, Zip32Derivation>,
    ) -> Result<BTreeMap<Vec<u8>, ZcashSignature>, Self::Error>;
    fn sign_sapling(
        &self,
        hash: &[u8],
        alpha: [u8; 32],
        path: Zip32Derivation,
    ) -> Result<Option<ZcashSignature>, Self::Error>;
    fn sign_orchard(
        &self,
        hash: &[u8],
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

    fn digest_header(&self) -> Hash {
        let version = self.global.tx_version;
        let version_group_id = self.global.version_group_id;
        let consensus_branch_id = self.global.consensus_branch_id;
        let lock_time = self.global.lock_time;
        let expiry_height = self.global.expiry_height;

        let mut h = hasher(ZCASH_HEADERS_HASH_PERSONALIZATION);

        h.update(&((1 << 31) | version).to_le_bytes());
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
                    self.sheilded_sig_commitment().as_bytes(),
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
                    self.sheilded_sig_commitment().as_bytes(),
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
        sapling, transparent, Version, V5_TX_VERSION, V5_VERSION_GROUP_ID,
    };

    const HARDENED_MASK: u32 = 0x8000_0000;

    use super::*;

    #[test]
    fn test_pczt_hash() {
        let pczt = Pczt {
            version: Version::V0,
            global: Global {
                tx_version: V5_TX_VERSION,
                version_group_id: V5_VERSION_GROUP_ID,
                consensus_branch_id: 0xc2d6_d0b4,
                lock_time: 0,
                expiry_height: 2705363,
                proprietary: BTreeMap::new(),
            },
            transparent: transparent::Bundle {
                inputs: vec![],
                outputs: vec![],
            },
            sapling: sapling::Bundle {
                anchor: [0; 32],
                spends: vec![],
                outputs: vec![],
                value_balance: 0,
                bsk: None,
            },
            orchard: orchard::Bundle {
                anchor: hex::decode("ed3e3e7dd1c81ac9cc31cd69c213939b2e21067758d4bd7dc9c2fed1eaf95829").unwrap().try_into().unwrap(),
                actions: vec![
                    Action {
                        cv: hex::decode("2262e5f410e151d1f373224cfa45f6287ab7cad2fef81e2926c1c8e052388e07").unwrap().try_into().unwrap(),
                        spend: orchard::Spend {
                            value: None,
                            witness: None,
                            alpha: Some(hex::decode("1af2a18b8647aa197a70a2779b8272d56cfdb8e0e2c6e50bc837a97716cb2cb7").unwrap().try_into().unwrap()),
                            fvk: None,
                            proprietary: BTreeMap::new(),
                            recipient: None,
                            rho: None,
                            rseed: None,
                            nullifier: hex::decode("f35440b9ef04865f982a9e74a46a66864df9999070d1611a4fae263cb1cf5211").unwrap().try_into().unwrap(),
                            rk: hex::decode("9e196d6d045d1d43a00100bca908a609e3411cdf5fef2fd89e23f2e60c43540a").unwrap().try_into().unwrap(),
                            spend_auth_sig: None,
                            zip32_derivation: Some(Zip32Derivation {
                                seed_fingerprint: [0; 32],
                                derivation_path: vec![HARDENED_MASK + 44, HARDENED_MASK + 133, HARDENED_MASK + 0],
                            }),
                        },
                        output: orchard::Output {
                            cmx: hex::decode("0b4ca8a1c5c626285ef039069d7147370d512dd0ef94df8430b703701a978d06").unwrap().try_into().unwrap(),
                            ephemeral_key: hex::decode("d6187bb2b5623400639196b1f7ef73a77a8ceaf3f71c4971ff90922eea642eaa").unwrap().try_into().unwrap(),
                            enc_ciphertext: hex::decode("9f1739b05e37faaaebf4c25ea1158693c338e8f9e30faeb06716930796ee3a29a40bc74c1e2c2029bd4672ea9e27dbc9055f057ff2d74f39ea04bbc414f0aa7a53386348183d9ddbea1499244fe1f3b000e328cd301f2b25bfa5406a9a02bd80bc07fc1e9f5d0205c0bac50b2cca4c5f82d6e6939366ac4a2bbe4afb1c43cbcf2db08a279d13b997a07d3c4b60b9c57ce9ee1861061d05f5a5c8dd86e416c2d9be8c320f9bcea93340fedb1d6cbbfe9dd6810612f321012174fde466ccf9ec1833ea89eb84f0d8b51bc10388555e23eee06cc8f8f10ae1993fadb1c5a4208ce0b033cbd3cec70f6d1738cd9a88dd6ae173f2185789f231f8a861002a5ecc798989032fe570509187e68eade9b461baa69abec787adfad546e98e6cf55667ba57d11701df67e65f1b551eeb9f402a0b5699a019858537ac49c89231c95932012ec690d0678f0f378ac764a300de1cfafdf2ab96d4dde78f8965da5796a5a4cea3e9fab98c699e74cc0662b084ce86c16fb79a40d07e54975920805b114f1a1db55c4e8d811e12a4245125d4e2a03fd2cedd32ae35fa4a6cbdac8893c3bc0f3c8a7d7b225db6bfc853d7ac5bf438c25d5b8cadbb5c1a5d5b6d9e9e152f02fabf2f02effd685e9d919a4faaa9cc87d64bd9162642c01a467a439bb66181dd5ef647fa1da3433263bef38aefe2a73d6d1eb04ce6b64dd56f4b3ca1665b600f9ec122bce8689a862f53a8d886cf642061db116fde176a6f8d1a9dcb53e91551d1742223e05be553e06a4266e562a06b5ae6a9ceb4fb2b6d338c5118e3c0fb04e5866da51563c8").unwrap().try_into().unwrap(),
                            out_ciphertext: hex::decode("1d7a687847d1fbafb6c051b952a67361dd66f8bf31ff20ae342dcfc00533b60f9edabe1dc68bc7182e80e89d8274ceedf03c309d676f8b0d0a9e9540adef6f85e808aec8790ceab00173cce2007f71b1").unwrap().try_into().unwrap(),
                            ock: None,
                            proprietary: BTreeMap::new(),
                            recipient: None,
                            rseed: None,
                            shared_secret: None,
                            value: None,
                            zip32_derivation: Some(Zip32Derivation {
                                seed_fingerprint: [0; 32],
                                derivation_path: vec![HARDENED_MASK + 44, HARDENED_MASK + 133, HARDENED_MASK + 0],
                            }),
                        },
                        rcv: None,
                    },
                    Action {
                        cv: hex::decode("3675ed5f6142e0e407dff2d850754ae13a084e46344d6408eafad993ba509822").unwrap().try_into().unwrap(),
                        spend: orchard::Spend {
                            value: None,
                            witness: None,
                            alpha: Some(hex::decode("1b1e87277818a289b9af5faccdbeede8d9fb1aa240c4cbd0017bb963119b83cb").unwrap().try_into().unwrap()),
                            fvk: None,
                            proprietary: BTreeMap::new(),
                            recipient: None,
                            rho: None,
                            rseed: None,
                            nullifier: hex::decode("dbf349555524523f0edbc811adb445ed3e79d8d5a94fe29c3a682381c571c123").unwrap().try_into().unwrap(),
                            rk: hex::decode("9d566b785aee161d20342e7b805facf2e9c103ab36ce3453ccf2161bc0da9d8c").unwrap().try_into().unwrap(),
                            spend_auth_sig: None,
                            zip32_derivation: Some(Zip32Derivation {
                                seed_fingerprint: [0; 32],
                                derivation_path: vec![HARDENED_MASK + 44, HARDENED_MASK + 133, HARDENED_MASK + 0],
                            })
                        },
                        output: orchard::Output {
                            cmx: hex::decode("40ce12b40aa59c0170f9440e36152509f9191a5b21c0378c6eb02e5ee530a935").unwrap().try_into().unwrap(),
                            ephemeral_key: hex::decode("70aa37601528cef93f619478d1ccd0a5431735dce8daf870ee3ebfb6b4169ca9").unwrap().try_into().unwrap(),
                            enc_ciphertext: hex::decode("4074376701408cbe4e06941da0354bc7dbb902776d375b3f355cd9cd82d69203abb8da5dc5ee7fd89db500153d5e024d1f7c29236387e37f53b90776795729295b7a538056b2889952df1652fd31682e629dfc0bfd5a73d9379deff0797081b257fdfad73fba3244813c021315b44fcf2e56a517b12cc8c9d7cb2ab5d8ee1467263c7e0cc9e0aabffc5a54c4f5d3bc4c25bbfaf956be2d64fdcdb27f444a435356e3aa135bf23dca861860d7b793450180b1b6eb063e87275d1bf48c0f39aea51b64d9de58ae8b0f5b5a95892ab4d3037215a7ca1b3ab87dd031412736dad973fa58f0598da9693a1c05b36a2fe580c46b9f02b18335326c4ecb153869e170e719ee80aee3dd22b3e7dedc47a7722a39638db3d97873b9ca1c4b30d1c803d2712fd628eff6edd7ed85890862fa3b59266a1cf3b130b6b044f9c5cbfce7fb2ed760e431bf11b1923bdd44510d0deadb55a427f79ebc3eb85a061b33ec779bb16785adef8a839e86e09f5f05da6ca309835dcde5c061ffa242b6504b712526f73a17ca28bdbc2cfa5cb0fc14c754c1ba5d643d10023049061e97251aa3f55335bbc9d963d6d4da0cbe304da0c76e518fa36a9043fd1030ba64e04ed65ec321de345d522dd1f471ee335f917fca98f516ba09a31aaad8d4f473cfbcb5f9704911bb88f39a6787675f9588afdb8196521484b034987b8df53c6ad54b79e8c3f484e3eb0cdb6182dcf48656165495527cd067a3a7b829be4c971e6714f2c4e2baa2b08de7a7399c3a12bc1a23df7a035d7f54ecd5542268eac06a67b5641e15ed9b423a352eb2").unwrap().try_into().unwrap(),
                            out_ciphertext: hex::decode("07ac9a6b96fcb208db821504a31b6af0509fff70c46bd2a6643711f1645816935135fabca8ae43c86897135c7653444b3361de0d75a3b886d35832bb6c89ad3b339e4109b3c40b3d3c165b11bffd58f9").unwrap().try_into().unwrap(),
                            ock: None,
                            proprietary: BTreeMap::new(),
                            recipient: None,
                            rseed: None,
                            shared_secret: None,
                            value: None,
                            zip32_derivation: Some(Zip32Derivation {
                                seed_fingerprint: [0; 32],
                                derivation_path: vec![HARDENED_MASK + 44, HARDENED_MASK + 133, HARDENED_MASK + 0],
                            }),
                        },
                        rcv: None,
                    }
                ],
                flags: 3,
                value_balance: 10000,
                zkproof: None,
                bsk: None,
            },
        };

        let hash = pczt.sheilded_sig_commitment();
        assert_eq!(
            "3840e39aef20acc050a509658397bbaa9500370967e37fe30d18e5fba05aba81",
            hex::encode(hash.as_bytes())
        );
    }
}
