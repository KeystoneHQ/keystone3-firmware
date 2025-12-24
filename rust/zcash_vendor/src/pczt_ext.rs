// This file is an extension of the current PCZT implementation.
// It adds a PCZTSigner trait that can be used to sign PCZTs.
// All other variables and functions are taken from zcash implementation.

use crate::pczt::Pczt;

use alloc::vec::Vec;
use blake2b_simd::{Hash, Params, State};

use pczt::{
    common::determine_lock_time,
    roles::low_level_signer::Signer,
    transparent::{Input, Output},
};
use transparent::sighash::SignableInput;
use zcash_protocol::value::ZatBalance;

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

#[allow(unused)]
const ZCASH_AUTH_PERSONALIZATION_PREFIX: &[u8; 12] = b"ZTxAuthHash_";
#[allow(unused)]
const ZCASH_AUTH_SCRIPTS_HASH_PERSONALIZATION: &[u8; 16] = b"ZTxAuthTransHash";
#[allow(unused)]
const ZCASH_SAPLING_SIGS_HASH_PERSONALIZATION: &[u8; 16] = b"ZTxAuthSapliHash";
#[cfg(zcash_unstable = "zfuture")]
const ZCASH_TZE_WITNESSES_HASH_PERSONALIZATION: &[u8; 16] = b"ZTxAuthTZE__Hash";

const ZCASH_ORCHARD_HASH_PERSONALIZATION: &[u8; 16] = b"ZTxIdOrchardHash";
const ZCASH_ORCHARD_ACTIONS_COMPACT_HASH_PERSONALIZATION: &[u8; 16] = b"ZTxIdOrcActCHash";
const ZCASH_ORCHARD_ACTIONS_MEMOS_HASH_PERSONALIZATION: &[u8; 16] = b"ZTxIdOrcActMHash";
const ZCASH_ORCHARD_ACTIONS_NONCOMPACT_HASH_PERSONALIZATION: &[u8; 16] = b"ZTxIdOrcActNHash";
#[allow(unused)]
const ZCASH_ORCHARD_SIGS_HASH_PERSONALIZATION: &[u8; 16] = b"ZTxAuthOrchaHash";

const ZCASH_TRANSPARENT_INPUT_HASH_PERSONALIZATION: &[u8; 16] = b"Zcash___TxInHash";
const ZCASH_TRANSPARENT_AMOUNTS_HASH_PERSONALIZATION: &[u8; 16] = b"ZTxTrAmountsHash";
const ZCASH_TRANSPARENT_SCRIPTS_HASH_PERSONALIZATION: &[u8; 16] = b"ZTxTrScriptsHash";

pub const SIGHASH_ALL: u8 = 0x01;
pub const SIGHASH_NONE: u8 = 0x02;
pub const SIGHASH_SINGLE: u8 = 0x03;
pub const SIGHASH_MASK: u8 = 0x1f;
pub const SIGHASH_ANYONECANPAY: u8 = 0x80;

pub(crate) const FLAG_TRANSPARENT_INPUTS_MODIFIABLE: u8 = 0b0000_0001;
pub(crate) const FLAG_TRANSPARENT_OUTPUTS_MODIFIABLE: u8 = 0b0000_0010;
pub(crate) const FLAG_HAS_SIGHASH_SINGLE: u8 = 0b0000_0100;
pub(crate) const FLAG_SHIELDED_MODIFIABLE: u8 = 0b1000_0000;

fn hasher(personal: &[u8; 16]) -> State {
    Params::new().hash_length(32).personal(personal).to_state()
}

struct TransparentDigests {
    prevouts_digest: Hash,
    sequence_digest: Hash,
    outputs_digest: Hash,
}

pub type ZcashSignature = [u8; 64];
pub type TransparentSignatureDER = Vec<u8>;

pub trait PcztSigner {
    type Error;
    #[cfg(feature = "transparent")]
    fn sign_transparent<F>(
        &self,
        index: usize,
        input: &mut transparent::pczt::Input,
        hash: F,
    ) -> Result<(), Self::Error>
    where
        F: FnOnce(SignableInput) -> [u8; 32];

    #[cfg(feature = "orchard")]
    fn sign_orchard(
        &self,
        action: &mut orchard::pczt::Action,
        hash: Hash,
    ) -> Result<(), Self::Error>;
}

fn has_transparent(pczt: &Pczt) -> bool {
    !pczt.transparent().inputs().is_empty() || !pczt.transparent().outputs().is_empty()
}

fn is_transparent_coinbase(pczt: &Pczt) -> bool {
    pczt.transparent().inputs().len() == 1
        && pczt.transparent().inputs()[0].prevout_index() == &u32::MAX
        && pczt.transparent().inputs()[0].prevout_txid() == &[0u8; 32]
}

fn has_sapling(pczt: &Pczt) -> bool {
    !pczt.sapling().spends().is_empty() || !pczt.sapling().outputs().is_empty()
}

fn has_orchard(pczt: &Pczt) -> bool {
    !pczt.orchard().actions().is_empty()
}

fn digest_header(pczt: &Pczt, lock_time: u32) -> Hash {
    let version = pczt.global().tx_version();
    let version_group_id = pczt.global().version_group_id();
    let consensus_branch_id = pczt.global().consensus_branch_id();
    let expiry_height = pczt.global().expiry_height();

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
        h.update(input.prevout_txid());
        h.update(&input.prevout_index().to_le_bytes());
    }
    h.finalize()
}
fn digest_transparent_sequence(inputs: &[Input]) -> Hash {
    let mut h = hasher(ZCASH_SEQUENCE_HASH_PERSONALIZATION);
    for input in inputs {
        h.update(&input.sequence().unwrap_or(0xffffffff).to_le_bytes());
    }
    h.finalize()
}
fn digest_transparent_outputs(outputs: &[Output]) -> Hash {
    let mut h = hasher(ZCASH_OUTPUTS_HASH_PERSONALIZATION);
    for output in outputs {
        let value = *output.value() as i64;
        h.update(&value.to_le_bytes());
        let len = output.script_pubkey().len();
        h.update(&[len as u8]);
        h.update(output.script_pubkey());
    }
    h.finalize()
}
fn transparent_digest(pczt: &Pczt) -> TransparentDigests {
    TransparentDigests {
        prevouts_digest: digest_transparent_prevouts(pczt.transparent().inputs()),
        sequence_digest: digest_transparent_sequence(pczt.transparent().inputs()),
        outputs_digest: digest_transparent_outputs(pczt.transparent().outputs()),
    }
}
fn hash_transparent_tx_id(t_digests: Option<TransparentDigests>) -> Hash {
    let mut h = hasher(ZCASH_TRANSPARENT_HASH_PERSONALIZATION);
    if let Some(d) = t_digests {
        h.update(d.prevouts_digest.as_bytes());
        h.update(d.sequence_digest.as_bytes());
        h.update(d.outputs_digest.as_bytes());
    }
    h.finalize()
}

fn digest_orchard(pczt: &Pczt) -> Hash {
    let mut h = hasher(ZCASH_ORCHARD_HASH_PERSONALIZATION);
    let mut ch = hasher(ZCASH_ORCHARD_ACTIONS_COMPACT_HASH_PERSONALIZATION);
    let mut mh = hasher(ZCASH_ORCHARD_ACTIONS_MEMOS_HASH_PERSONALIZATION);
    let mut nh = hasher(ZCASH_ORCHARD_ACTIONS_NONCOMPACT_HASH_PERSONALIZATION);

    for action in pczt.orchard().actions().iter() {
        ch.update(action.spend().nullifier());
        ch.update(action.output().cmx());
        ch.update(action.output().ephemeral_key());
        ch.update(&action.output().enc_ciphertext()[..52]);

        mh.update(&action.output().enc_ciphertext()[52..564]);

        nh.update(action.cv_net());
        nh.update(action.spend().rk());
        nh.update(&action.output().enc_ciphertext()[564..]);
        nh.update(action.output().out_ciphertext());
    }

    h.update(ch.finalize().as_bytes());
    h.update(mh.finalize().as_bytes());
    h.update(nh.finalize().as_bytes());
    h.update(&[*pczt.orchard().flags()]);
    let (magnitude, sign) = pczt.orchard().value_sum();
    let value_balance = if *sign {
        -(*magnitude as i64)
    } else {
        *magnitude as i64
    };
    h.update(&value_balance.to_le_bytes());

    h.update(pczt.orchard().anchor());
    h.finalize()
}

fn hash_sapling_spends(pczt: &Pczt) -> Hash {
    let mut h = hasher(ZCASH_SAPLING_SPENDS_HASH_PERSONALIZATION);
    if !pczt.sapling().spends().is_empty() {
        let mut ch = hasher(ZCASH_SAPLING_SPENDS_COMPACT_HASH_PERSONALIZATION);
        let mut nh = hasher(ZCASH_SAPLING_SPENDS_NONCOMPACT_HASH_PERSONALIZATION);
        for s_spend in pczt.sapling().spends() {
            // we build the hash of nullifiers separately for compact blocks.
            ch.update(s_spend.nullifier());

            nh.update(s_spend.cv());
            nh.update(pczt.sapling().anchor());
            nh.update(s_spend.rk());
        }

        let compact_digest = ch.finalize();
        h.update(compact_digest.as_bytes());
        let noncompact_digest = nh.finalize();
        h.update(noncompact_digest.as_bytes());
    }
    h.finalize()
}

fn hash_sapling_outputs(pczt: &Pczt) -> Hash {
    let mut h = hasher(ZCASH_SAPLING_OUTPUTS_HASH_PERSONALIZATION);
    if !pczt.sapling().outputs().is_empty() {
        let mut ch = hasher(ZCASH_SAPLING_OUTPUTS_COMPACT_HASH_PERSONALIZATION);
        let mut mh = hasher(ZCASH_SAPLING_OUTPUTS_MEMOS_HASH_PERSONALIZATION);
        let mut nh = hasher(ZCASH_SAPLING_OUTPUTS_NONCOMPACT_HASH_PERSONALIZATION);
        for s_out in pczt.sapling().outputs() {
            ch.update(s_out.cmu());
            ch.update(s_out.ephemeral_key());
            ch.update(&s_out.enc_ciphertext()[..52]);

            mh.update(&s_out.enc_ciphertext()[52..564]);

            nh.update(s_out.cv());
            nh.update(&s_out.enc_ciphertext()[564..]);
            nh.update(s_out.out_ciphertext());
        }

        h.update(ch.finalize().as_bytes());
        h.update(mh.finalize().as_bytes());
        h.update(nh.finalize().as_bytes());
    }
    h.finalize()
}

fn digest_sapling(pczt: &Pczt) -> Hash {
    let mut h = hasher(ZCASH_SAPLING_HASH_PERSONALIZATION);
    if !(pczt.sapling().spends().is_empty() && pczt.sapling().outputs().is_empty()) {
        h.update(hash_sapling_spends(pczt).as_bytes());
        h.update(hash_sapling_outputs(pczt).as_bytes());
        let value_balance = (*pczt.sapling().value_sum())
            .try_into()
            .ok()
            .and_then(|v| ZatBalance::from_i64(v).ok())
            .expect("should be validated before here");
        h.update(&value_balance.to_i64_le_bytes());
    }
    h.finalize()
}

fn hash_sapling_txid_empty() -> Hash {
    hasher(ZCASH_SAPLING_HASH_PERSONALIZATION).finalize()
}

fn hash_orchard_txid_empty() -> Hash {
    hasher(ZCASH_ORCHARD_HASH_PERSONALIZATION).finalize()
}

fn shielded_sig_commitment(pczt: &Pczt, lock_time: u32, input_info: Option<SignableInput>) -> Hash {
    let mut personal = [0; 16];
    personal[..12].copy_from_slice(ZCASH_TX_PERSONALIZATION_PREFIX);
    personal[12..].copy_from_slice(&pczt.global().consensus_branch_id().to_le_bytes());

    let mut h = hasher(&personal);
    h.update(digest_header(pczt, lock_time).as_bytes());
    let sig_digest = transparent_sig_digest(pczt, input_info);
    h.update(sig_digest.as_bytes());
    h.update(
        if has_sapling(pczt) {
            digest_sapling(pczt)
        } else {
            hash_sapling_txid_empty()
        }
        .as_bytes(),
    );
    h.update(
        if has_orchard(pczt) {
            digest_orchard(pczt)
        } else {
            hash_orchard_txid_empty()
        }
        .as_bytes(),
    );
    h.finalize()
}

fn transparent_sig_digest(pczt: &Pczt, input_info: Option<SignableInput>) -> Hash {
    if !has_transparent(pczt) {
        hash_transparent_tx_id(None)
    } else if is_transparent_coinbase(pczt) || pczt.transparent().inputs().is_empty() {
        hash_transparent_tx_id(Some(transparent_digest(pczt)))
    } else {
        if let Some(input) = &input_info {
            // this should have been checked earlier
            assert_eq!(input.hash_type().encode(), SIGHASH_ALL);
        }
        //SIGHASH_ALL
        let prevouts_digest = digest_transparent_prevouts(pczt.transparent().inputs());

        let amounts_digest = {
            let mut h = hasher(ZCASH_TRANSPARENT_AMOUNTS_HASH_PERSONALIZATION);
            pczt.transparent().inputs().iter().for_each(|input| {
                h.update(&input.value().to_le_bytes());
            });
            h.finalize()
        };

        let scripts_digest = {
            let mut h = hasher(ZCASH_TRANSPARENT_SCRIPTS_HASH_PERSONALIZATION);
            pczt.transparent().inputs().iter().for_each(|input| {
                //len should be a compact size
                let len = input.script_pubkey().len();
                h.update(&[len as u8]);
                h.update(input.script_pubkey());
            });
            h.finalize()
        };
        let sequence_digest = digest_transparent_sequence(pczt.transparent().inputs());

        let outputs_digest = digest_transparent_outputs(pczt.transparent().outputs());

        //S.2g.i:   prevout      (field encoding)
        //S.2g.ii:  value        (8-byte signed little-endian)
        //S.2g.iii: scriptPubKey (field encoding)
        //S.2g.iv:  nSequence    (4-byte unsigned little-endian)
        let mut ch = hasher(ZCASH_TRANSPARENT_INPUT_HASH_PERSONALIZATION);
        if let Some(signable_input) = input_info {
            let input = pczt
                .transparent()
                .inputs()
                .get(*signable_input.index())
                .expect("valid by construction");
            ch.update(input.prevout_txid());
            ch.update(&input.prevout_index().to_le_bytes());
            ch.update(&signable_input.value().to_i64_le_bytes());
            let len = signable_input.script_pubkey().0.len();
            ch.update(&[len as u8]);
            ch.update(&signable_input.script_pubkey().0);
            ch.update(&input.sequence().unwrap_or(0xffffffff).to_le_bytes());
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

#[cfg(feature = "transparent")]
pub fn sign_transparent<T>(llsigner: Signer, signer: &T) -> Result<Signer, T::Error>
where
    T: PcztSigner,
    T::Error: From<transparent::pczt::ParseError>,
{
    llsigner.sign_transparent_with::<T::Error, _>(|pczt, signable, tx_modifiable| {
        let lock_time = determine_lock_time(pczt.global(), pczt.transparent().inputs())
            .ok_or(transparent::pczt::ParseError::InvalidRequiredHeightLocktime)?;
        signable
            .inputs_mut()
            .iter_mut()
            .enumerate()
            .try_for_each(|(i, input)| {
                signer.sign_transparent(i, input, |signable_input| {
                    shielded_sig_commitment(pczt, lock_time, Some(signable_input))
                        .as_bytes()
                        .try_into()
                        .expect("correct length")
                })?;

                if input.sighash_type().encode() & SIGHASH_ANYONECANPAY == 0 {
                    *tx_modifiable &= !FLAG_TRANSPARENT_INPUTS_MODIFIABLE;
                }

                if (input.sighash_type().encode() & !SIGHASH_ANYONECANPAY) != SIGHASH_NONE {
                    *tx_modifiable &= !FLAG_TRANSPARENT_OUTPUTS_MODIFIABLE;
                }

                if (input.sighash_type().encode() & !SIGHASH_ANYONECANPAY) == SIGHASH_SINGLE {
                    *tx_modifiable |= FLAG_HAS_SIGHASH_SINGLE;
                }

                *tx_modifiable &= !FLAG_SHIELDED_MODIFIABLE;
                Ok(())
            })
    })
}

#[cfg(feature = "orchard")]
pub fn sign_orchard<T>(llsigner: Signer, signer: &T) -> Result<Signer, T::Error>
where
    T: PcztSigner,
    T::Error: From<orchard::pczt::ParseError>,
    T::Error: From<transparent::pczt::ParseError>,
{
    llsigner.sign_orchard_with::<T::Error, _>(|pczt, signable, tx_modifiable| {
        let lock_time = determine_lock_time(pczt.global(), pczt.transparent().inputs())
            .ok_or(transparent::pczt::ParseError::InvalidRequiredHeightLocktime)?;
        signable.actions_mut().iter_mut().try_for_each(|action| {
            match action.spend().value().map(|v| v.inner()) {
                //dummy spend maybe
                Some(0) | None => {
                    return Ok(());
                }
                Some(_) => {
                    signer.sign_orchard(action, shielded_sig_commitment(pczt, lock_time, None))?;
                    *tx_modifiable &= !(FLAG_TRANSPARENT_INPUTS_MODIFIABLE
                        | FLAG_TRANSPARENT_OUTPUTS_MODIFIABLE
                        | FLAG_SHIELDED_MODIFIABLE);
                }
            }
            Ok(())
        })
    })
}

#[cfg(feature = "cypherpunk")]
#[cfg(test)]
mod tests {
    use pczt::Pczt;
    use transparent::{address::Script, sighash::SighashType};
    use zcash_protocol::value::Zatoshis;

    use super::*;

    #[test]
    fn test_basic_functions_orchard2orchard() {
        //orchard to orchard
        let pczt_hex = "50435a5401000000058ace9cb502d5a09cc70c010082efad01850100000000000000fbc2f4300c01f0b7820d00e3347c8da4ee614674376cbc45359daa54f9b5493e010000000000000000000000000000000000000000000000000000000000000000027c44511582af6b5e43e3ab36c3ab1e0c2997d8e6b7809ccccdbc212a6a359a39aef8e1290671b922d3a8468de66fc9281dfdf30f5518fd69bdb9652ccaf9280593ba8cb150ee5473d6a98a61581791bacbef4b23d3ae8b60a808f4321b0ccdbf00017390fedb30aa721eb96ca33f1a7da10f9900e21ef8f4ab7e15d744edc60dcdc1a4c9debe9fd9a8b2cf56b90180ade20401e959b1bef8243433317a2c69fc1167f12528cdae5d9c2a19f925a5a3114c7f2801d2d4de02e32e57569d8a25bc3254545c58d8f41e22b1e0f75128e92601ce09a60183ceb48bb24a4debb403c3cc02043f5157ec4f084b8fb359a58f53ab4cd3741b339461f1504081a0765b12b87cac9c4a3baa55b155192d3cfd98a7517b8a59019744b22b89ba0daaa32de9d883a727be7e2ca09446f87efe0e5b29ce98c65f19000192ca4017b375e36aa0a58aa94a644e44964245190a20e40a479b229535df0c0c01af5d9c247e91d0cbf603f6f6f49cfc218eabf9a2c1d886ce94fc167d6da7e66403a080808008858180800880808080080000ea1fb4757517b5726c2968b11fb287644589749f5aab3bcfaf380cae003b1a0cf9b3db1aeee6eba391326b324470bccb0f155a9a9a2750188f9e29ea263bb9bcc4046a98d226a6a2ea117f7b9eb85d8456843f02edc96507cb22213aeeaf7038f014a3b8197e649b886bad2c9a2c7cba19b17f6800825c63dc1740959d25f1affdda6d7e3657da22cdfa9ddf5c6cb45a4d744c1e150697db5b6766daf235d92c52ef3088eddb73c5020f612796cebd02b404a498332a875f6f3c3c0875139708c9e0a430573cf040304d3060049fe464b212a520c79c2bce30635b3c258d4ff00368bfddeafe1c77204db3c3eaf7f037383f43faba6325c3d22635372d62baf3cef53d9729067826652f26f9731d6338887fc902155ec57e85d16d001535aeb3dfd34375cf8109d2c7340e291ed3d0a6e6e4905dfdeb5c6638954b28587ae7e88c6aec3fe71b83ef973d9083c1eaedf5c782919ed9ac5c1d6485bd1acef2cdc8af172d289b27f0f0c32a4196b2dd9ef4e5f40176f5baf432d22dcf7eb7d506c3501cd40b7dd352263207525e03607fbf4b19d0a7db480e0f4aa26ac7c1a7d8335c3de0927a20aabd267872ed19fd52730a5b3f52f92957bce0b97354b7b1b39216163bdac02467f054446e72afafea3dffe85ef524c4092fe74732fcf4dc47bb084fc7c9407ba2ff0eab28af33b12c0b4ef1b4743f518521a8301f2c900c6bc88f631a591f953ea4cbfe7e8131aed76eb0292b12db2ae879d3407030dbaa650749b6fe92ab0790b52362dc9be3899729e10282f8cdf37fd026cba7ef5c6668a5a5c346fd57f96991f874a738ff7437f3c2da90117e99f0bde461784e3fc22ed4d3af23953485722436d4c51d3bd474c457dfbaa8b8dbb65dcec841a85ce9726f5a56eca822be50564ed3670e7259460b54cf9179387bc50b43f45cc75869b510c331444a199f3caeb7d1674d023880cc3b287afb2914bc5038e2db4b08efe0752d54f0edd31d44d3020fc1d81e9f36b4414bb871c0476c01d288df138de809fa058e8b7b240650cb1aae966ea3614fc54db414e8294066e8936b02d96b006cbfffc52701c0843d010d8a10e45671953357c419835d685db7462c67678b9d2c618e237aad4ddd0948000001d501753163637868356e7274366132656d6838687473793461673263397638666c6d786e61733537727468656c6d6377676e76397534766864747575703571646e3570676e6537766a736c35373673337a7333646b63386771377638756c68346573646a746a64727a6c65377138633237723934773672707061726a3737726b6a7633677076703964646a79687838307276756e7767677a646d716a65366e3973723466676e6d777a75656c646c793568707339746c6672796b656179356a6a6d343272773379777966367264716538327734326a737300019f9736e85c4df8db84a6c16f7ab547290055182038e33e1ecef672a0aabe5d0075ac53737630399b01fbaae346e017c15d2ab2af798e268f1f03ceaf6fca6f02643f09ad24e0595215a8ee3e2b51a79ed8550c1b9ee1280082aae3268a0b161abe47b6f2e823adc61207d2f0ca7831e6f2c62af9c3ab8c3d7f5110c39799e88101ef86dd3a4ef054afd8728204cef346bc8118847985075fee7cf3420b1c6589383bab1a24973a0a633a7260c3d9f455391a760814924d569d853f257ed70d0c2f017a3779a010062bb8936ec2ee5dd49d45803bda6b23debad15e557cc56b4682e2338d757001c9cd76a74e2501000178b41d35aaafb6cfe0ebca3394908dd1a7d470a8be71eabeda361b5002c1a23e010af9f077b558d4e71f09ba56c91562adcc550feff26d9b53e1c6600ae0ec7b6e01118945457852e7765e9c7ffa8f154e3b7bd9e485ca81419d7d662628609e5c1eaf1095a229485256020ff1dfc5f00bdf3d43a8150f958542dfba8a792d935b2a48440dd87abcceb2f73bd3d5104ce0cebd86b9ea5f7b69ff6f67f9bc37206a03000151c4407a8eb1f5fedfda4f3ad969e057da950af38b4f1fdcc17574258bc0e33e00000064bca5dfa19fcfeaa4c0f32547b945c53d7492002ea01d5cafc38b5b29afa21b378f4b30c10f2561ff28ece62d5d53bd0f65af67312836c0c1da2bff357af690c404a214d4b126ffbb847fd825cd69e467342e77c8be8ed5ec578f2db0bee6753bd134f819e1053dab1fcb1d58375477ff15234cabb528d174f66476c3dc3fedbafd16c63f62f30176a9c7c0d85516150379d77bf0d9510726f6dd5d3e1dd20a77b32124fd6e7012958d7dfef9d73781ae15a89e5f3d45b8b2e322e5da87721c05135307aa0ab047f7ee71031de0bf9f6d5192e2e1a0b370e8d16daacf9565460332e7e6758de3b59b5fbf585f3ef231da5d8cd3574b5c8eab4ea0368111fd3e39f642cfbd020d10959d60d1ceb21247efba84040a0c3e6b722ec5524a2928418cc321c44a5f74e28cdbe3b484459230c668cf876bd13f7db32d571b756d976ef844023a4c0146108ed1a4e8d9318dbc2cde5de055c6b69f7cb01b869f76b41ac3ebd59a07cae039571c3a67307de946ce5e28652aea2835e934c94fd231889fe9e0cbb5920a1930867e229e04f255ef0cb11e8397aa9fcd4a5b032fe6aea6e920a97f20e3dccf8563f1ffbaee4dc607ef977f95704d855ada4b6bec066ce92bac969bb7f615b8ded28a326b65bbd63be1d7a69d4a5b9903b51b351b563b17d0c469dceff53dfb775886bd1e8c47d8deb95ab2d0478d14ab2311334af47dc33c42061ecff5b476aaef385dfab49fe2444e842a521f62472284d95f0573c6d5d812fb27d83bf0a6f113e78c8de378a7b71143f766a3d303085d7af51dba4ac80937e9e1256246a755e664ff24babb099122c6aa31f4b340b6a4b7a0e9601f244cb030a8624b2bf46942fba2bf1ac15d21263bafa8d648ce1586beb43f0d9f2225addfed115c0a501fc5f856412572e53e4cb3fc6fd9b35761a3d34306b535f8eb86634ebd4274a6d9dc8c6328955722ec2f48a838046cc22f4622a79213ea88e73471a1c75dddfad71e8bae58ee08bbc74ea61a754f17c30138369b5a129c5592507890680300364a6193ec81f4d59f5c7418728f9416731f5ef0f085368010a525a7a001b0daa40401aeeb72da7e5b6b08d443d0d705cbf7e173e02be1d9b1b30f5e9c8cdd4228f6d300000000017d0607eac901c56a1b00d9aa4e1841c0811830c30d3d69b48e362c72e4a9ba0703904e006702c684eca9a8b72d603d306af1f699408e76798fc2bd69c86a67e08253fe1200011c9e3dd2264fbd46a0a69a1ac9cd88e9816d48e34520a8d25c2d9f128f681808";
        let pczt = Pczt::parse(&hex::decode(pczt_hex).unwrap()).unwrap();
        assert!(!has_transparent(&pczt));
        assert!(!is_transparent_coinbase(&pczt));
        assert!(!has_sapling(&pczt));
        assert!(has_orchard(&pczt));
        assert_eq!(
            hex::encode(digest_header(&pczt, 0).as_bytes()),
            "3f85a5b3ff138bde71704243213f0cdd8d7483832dc4c2007c0f15fc2e3d17eb"
        );
        assert_eq!(
            hex::encode(digest_transparent_prevouts(pczt.transparent().inputs()).as_bytes()),
            "a04b16834dc939ccf632d15dddaa6bcaed253d12068dca169fbd28bc403cf3ba"
        );
        assert_eq!(
            hex::encode(digest_transparent_sequence(pczt.transparent().inputs()).as_bytes()),
            "3a0033336603235001742438238d713c09c1c55bf67bd20b80805aea0b46eba5"
        );
        assert_eq!(
            hex::encode(digest_transparent_outputs(pczt.transparent().outputs()).as_bytes()),
            "25f311cc149ecccef0e8ca8c9facd897ef88806008bc15818069470db9f84a37"
        );
        assert_eq!(
            hex::encode(digest_sapling(&pczt).as_bytes()),
            "6f2fc8f98feafd94e74a0df4bed74391ee0b5a69945e4ced8ca8a095206f00ae"
        );
        assert_eq!(
            hex::encode(digest_orchard(&pczt).as_bytes()),
            "0ee1912a92e13f43e2511d9c0a12ab26c165391eefc7311e382d752806e6cb8a"
        );
        assert_eq!(
            hex::encode(shielded_sig_commitment(&pczt, 0, None).as_bytes()),
            "bd0488e0117fe59e2b58fe9897ce803200ad72f74a9a94594217a6a79050f66f"
        );
    }

    #[test]
    fn test_basic_functions_transparent2orchard() {
        let pczt_hex = "50435a5401000000058ace9cb502d5a09cc70c0100ebf0ad0185010000025acf12ca226205024695507ed5da494d7900957792d312cd4fe4fecaa04d1ee10000000000c08db7011976a914840418dfb50329ec38a21f869e63ccb4d5d9ef2d88ac00000101035fd07149bf45a6bf1edd52817e57e637228afbe79732fbfb4a293d58aad45cecaf5d9c247e91d0cbf603f6f6f49cfc218eabf9a2c1d886ce94fc167d6da7e66405ac8080800885818080088080808008000000000000009572f7a4fd81fc04ddaff4deb1073cf96db3834fa34afc7cc9719e21d47c9f570000000000c0a8a5041976a914840418dfb50329ec38a21f869e63ccb4d5d9ef2d88ac00000101035fd07149bf45a6bf1edd52817e57e637228afbe79732fbfb4a293d58aad45cecaf5d9c247e91d0cbf603f6f6f49cfc218eabf9a2c1d886ce94fc167d6da7e66405ac80808008858180800880808080080000000000000000000000fbc2f4300c01f0b7820d00e3347c8da4ee614674376cbc45359daa54f9b5493e01000000000000000000000000000000000000000000000000000000000000000002e45d04f3d7a562c6a1794e5e89cf8f36163ab6883f3d8b353e2083bcb1b2249e35a6c9941ef00d0ae9ee84a0e1b3f0856f589f89c67ff6d2d3d0123242373305626c5fd324a19134789116eb2682afc7a03226e308fbc5c2f96e826868b6651a01d4ba7b75d4266bb789a2aadf544d1052b02f34a3fde7d1feb4e22f2fc579598d14d5f12d5867be9b938d18c4424d3e92a89a8581cae7056a42f144ac42f94a060106467650ee9f96ec2ea6f71580e743b645a5b6c9d19c25cb816cc6e7b7747958b3a4576929698dc873d6a3010001ea94578a626ca4e5621fc4407b206fdff0af992ac501ede7c120cf976bc78a1f0169d74e5472fdd307c5eaf279fe330dc7edfba8f111cd870cd92ab08413b1ff8a01c767e69d2d93ff9d6e88ebacd5b91181271470782bdd4db149b6b0695ec76b0c49e471efcf295a933584261e6ff9d979a7d87b748ace5040ee39cd65765afa0adb5325b76e95d75482224edaa7f12e7f908361a0db283647585efe4607839e280001ae6ae6ba850b89ff2c8cc83799472a90ae3b45d119d43a667f24454c93932f2d00000082bfa85adfe80b71cc73aec7412401f7649582cde529cd8029c1359a75dd831d730b61d4e33a37bd22501d591b170bbb80c26e0abb07e477f0953fc2e04110adc40489487a0e18b14486ab2189631d565186c3918d7dfbfbfb6484579850ffa1576b74c5f6206a97f06ab413596e97b2f2b520973bf4517daadae99268d927d27d2457f6be435b6809abbf237955cd07746a7ddf6c0f630ca7ffcabcf05848c111480cbb57ac013985a1f83ac56440ef3f85b1ad72db48c287650758723d56c6881e0cc45d9022ef6d8b3102d65f3231401a7ff7307bc7ddba41f8a1bf440e5c67363b7e28078102b3e04dab3e74933fb0abbda260de62cc480cc76945478d151536b7e26d121873db53dee1a6b89a755605d4c608da0af5844c6dab152b4716093cd4294b1110d4083254fbd7d3f04395ebb52a634f37118b8bf04762bd9179871649c30a723d3eeb7d03949ccaebff60fb385d8d29690ed2472b26c7486462033bf37d4d1ab691b7373a33d6ad043bbfbf9e7a3e01554de639f2c920de8a0af1a12ef2114b5f7545afffa7d85d03ba0421ab5a90734b49c5de39dd7581735bfaa9e5e7685c75c2c5891824ea6b7c36e298a09c56c68de639928cef69850ac8c648bf59edd29caa45b4bef19e93b5c63d71edc97b211480e504af05ac7ea654cef2e13ea3908639f91980f79d148f2c24df46b48632196fa4f760c2e460a6a0a29d6cb0d9a3e2458f0c674089085e0dd9f675377a055132e422dc588f11a7d7c45e11708b4bfb6ab81c134d4ba37cb4a5da09baebbdcf64aaf27f9e6197405494c348c1fe5343f3a5e7957cc6ea9002d90baac76efd6c06988241772b01c512c3b7d5390f3a284833f74effd56ac3723e05b8fa8340da1d6d4a71c4ecaa5762550bdd9ab94550040ed0209b2649245ece137b5f709565f2396d3c79626e640013e16c8112f9a8d046f172464f5be6f76cff56c5c0ebae70d5188b78509b254956b08d76278ac0a93d3338cecb8b233b67a76da168ab010138369b5a129c5592507890680300364a6193ec81f4d59f5c7418728f9416731f5ef0f085368010a525a7a001e099db05016e798c3a068d860dbd0fbf8493f6a495b930b0910dfc9c80da042ee324af8658000000000135049ad6155472f47f4f6c71922caeeec0394f3938a06b2b7fbd27429d78750fd7243a84ff4c759b869f132f939d063d6067f435161bf70bc1c59fea8b38043d45af9e07e68fc4ea58021d1aa8d30b6162fe67c07e60801c33fdc2f14d2c8806e3804c6b1dd4e056185fc25a52820fa5be87656184e00ae49a295d3f362e2baf01f6897096c00f6f4d1f26adc5779903cd07f546a7355499c9a3522724e749aa9edd5ee16e6218b875f79f7ad1c9681177089469549418b4e4f360da5d60a2fb1c015ac82c56f5f22c5cb06d376cd31a257af081ddf79e1448cc7e4654060269cdfd0ac38a2b6c0510bf9e951a01000144725db55f1eef840698c8393450e035c68ede9e914e88453081e045cef34705015cbaa9fb333b10a11d6b98a8ed6f315e36b8f38bfde610806859820a0e87b9c901833fe595747cc0971b28efdb3457c5e2b3ded473485b6f2cc62514e444ea1d0f352e72e22d112bf0407f39572e889ec95cbbdc03e60b14c50f11bf541d457a0745b4b2ede6365538357f51f0fa7f3e9c001389aaf09eedf2824072c50ffdff0e0001506ab89ec073252918bdbefe6298cfee59496c0a52218b8360784de1dc15b43c000000c60cff359faf5712e4894a98862943502a899561525e0f97c0b899c30feafa2604cb447292efc54926c2f9677df030c13808d20bcaed3ad7c727f6764bbb9e86c404feda6d3f41d8c82d02376c7e3437a4ebb9084a985e8a0d085aa470e339d2f8c05988cf53c44cf24e34e738332364aee390cdaa25f478602ca7d83e9964bc31a66ab50683c40bce9f81bd51846f13bfebce2821f4142b319406c4d90eab508bc123e52b30c2d308421e71860eed0a59e43a1aca227ce7e54d9423920709b096d628ca56f8c09c846720808d453cedc856e9cbf804cef1d68abbd614759f8f31cd1784b753baaa70cae727d4f82bc590386bc1bea84e0906367f99d45cfeb68bc8e30b195c601affca838aad5fa7f481612ab4d731162af7db44c096c2313720d52f79a2c617e0916f859ce288c1b055bb590046be19627515e268bfe01a23953dbf55c03e3ae8705d638ce9fb08f53eba34937ef4a2bb35ca6c2f428a469e4db5e97dec974ab583451eb5067d031642e27e655155e77d34a2ad8393737df5ba88d2d4ce7c5db62c762b93eda05a431d2b2cf9a4cd30a4f14579d8f8f2948dd15193706455b5a42b790578bcc8296d2e80a87a50add4950985f2dcc3c9e70221c5141d14a23ce9e6a20d6463326ca51d703f3a81c79545080b99e1f048f6ad5b40da8c57e088c3a6e77ea1811f68d2e9187e188075ec39e4469d67e0a45920e4bc45d29d8544b3bf4b9bf914df374cd449e75732c9f2543026fc220fc3365e6bf5c18f087d15083326184c55bc53fa18c8a51af0131070c1de84d837f00cca36929596fda18560d026170c66d69611fbb18b2f1b1443b68abad8de29c160588e3e2fd549687ee5825ed52119030f59679a5078855532f90b2cff7e00db6616118ca6fda1e3504057a7e67b05025c1f1cf7151fbcd3c860946485f2c936283dea193f3e0aab4c9c9280661212be0e5e7e0085be33e234b21b94eac0fa1db03e563227422f67670747aa2a6bc57226f1ca6c4535926e29011d1274b41a693712d5f50baea17a14534be815ef2261b84847e49e6636ccbaf1f48d4b5efd6612824208a7010001e1450753fdabb3d1e6cb965985bda36d6237bfd06ac24f287fe154f8dad5effb0000000001b95a1022e76e6138181e5eb33770ad25c198fdb874ca1ad2310dd05244e56b1803e099db0501ae2935f1dfd8a24aed7c70df7de3a668eb7a49b1319880dde2bbd9031ae5d82f0001ee5eaaf8fcc2d32c986dca24ca9c5b1482d24cf2ac6a86fdb0caf794e15de127";
        let pczt = Pczt::parse(&hex::decode(pczt_hex).unwrap()).unwrap();
        assert!(has_transparent(&pczt));
        assert!(!is_transparent_coinbase(&pczt));
        assert!(!has_sapling(&pczt));
        assert!(has_orchard(&pczt));
        assert_eq!(
            hex::encode(digest_header(&pczt, 0).as_bytes()),
            "b17f07724f36f3cfe54140d9225d853a9246f4e4cfea722b583fdb151f005f76"
        );
        assert_eq!(
            hex::encode(digest_transparent_prevouts(pczt.transparent().inputs()).as_bytes()),
            "8c34a460a39541d94e062f481e45495c90f3420b39ea0b4ecc45d55485c28c8b"
        );
        assert_eq!(
            hex::encode(digest_transparent_sequence(pczt.transparent().inputs()).as_bytes()),
            "7b0e9ba5bfc487e7471c657ef3cb743f90c7548c3fdcc355dc267559394c1bc1"
        );
        assert_eq!(
            hex::encode(digest_transparent_outputs(pczt.transparent().outputs()).as_bytes()),
            "25f311cc149ecccef0e8ca8c9facd897ef88806008bc15818069470db9f84a37"
        );
        assert_eq!(
            hex::encode(digest_sapling(&pczt).as_bytes()),
            "6f2fc8f98feafd94e74a0df4bed74391ee0b5a69945e4ced8ca8a095206f00ae"
        );
        assert_eq!(
            hex::encode(digest_orchard(&pczt).as_bytes()),
            "56dd210c9d1813eda55d7e2abea55091759b53deec092a4eda9e6f1b536b527a"
        );
        assert_eq!(
            hex::encode(shielded_sig_commitment(&pczt, 0, None).as_bytes()),
            "fea284c0b63a4de21c2f660587b2e04461f7089d6c9f8c2e60a3caed77c037ae"
        );

        let script_code = Script(pczt.transparent().inputs()[0].script_pubkey().clone());

        let signable_input = SignableInput::from_parts(
            SighashType::parse(SIGHASH_ALL).unwrap(),
            0,
            &script_code,
            &script_code,
            Zatoshis::from_u64(*pczt.transparent().inputs()[0].value()).unwrap(),
        );
        assert_eq!(
            hex::encode(shielded_sig_commitment(&pczt, 0, Some(signable_input)).as_bytes()),
            "a2865e1c7f3de700eee25fe233da6bbdab267d524bc788998485359441ad3140"
        );
        let script_code = Script(pczt.transparent().inputs()[1].script_pubkey().clone());
        let signable_input2 = SignableInput::from_parts(
            SighashType::parse(SIGHASH_ALL).unwrap(),
            1,
            &script_code,
            &script_code,
            Zatoshis::from_u64(*pczt.transparent().inputs()[1].value()).unwrap(),
        );
        assert_eq!(
            hex::encode(shielded_sig_commitment(&pczt, 0, Some(signable_input2)).as_bytes()),
            "9c10678495dfdb1f29beb6583d652bc66cb4e3d27d24d75fb6922f230e9953e8"
        );
    }

    #[test]
    fn test_basic_functions_orchard2transparent() {
        let pczt_hex = "50435a5401000000058ace9cb502d5a09cc70c0100a6f0ad01850100000001c0a8a5041976a914840418dfb50329ec38a21f869e63ccb4d5d9ef2d88ac000001237431567565374c47704756654b71635a77346657517472487250654d705a70767a554d00000000fbc2f4300c01f0b7820d00e3347c8da4ee614674376cbc45359daa54f9b5493e0100000000000000000000000000000000000000000000000000000000000000000228f7d939331e2b57f59159b23257b15ff8b62eedcc199204202a504e35039b18aef8e1290671b922d3a8468de66fc9281dfdf30f5518fd69bdb9652ccaf928054984ddfa67257d5ce17f00e4f617bc7473633d591a06b908b2ebdbe7c991173100017390fedb30aa721eb96ca33f1a7da10f9900e21ef8f4ab7e15d744edc60dcdc1a4c9debe9fd9a8b2cf56b90180ade20401e959b1bef8243433317a2c69fc1167f12528cdae5d9c2a19f925a5a3114c7f2801d2d4de02e32e57569d8a25bc3254545c58d8f41e22b1e0f75128e92601ce09a60183ceb48bb24a4debb403c3cc02043f5157ec4f084b8fb359a58f53ab4cd3741b339461f1504081a0765b12b87cac9c4a3baa55b155192d3cfd98a7517b8a59019744b22b89ba0daaa32de9d883a727be7e2ca09446f87efe0e5b29ce98c65f1900015b5ea077873b8e99f4d4d5764814aedc404dfac1bd1595038f416c242627d10901af5d9c247e91d0cbf603f6f6f49cfc218eabf9a2c1d886ce94fc167d6da7e66403a080808008858180800880808080080000263260db365ce227e49b6fc82f3421d43f6e5015140108b6dfbfa8922b453820127bb102f72a0fa8952c1f48309f3555e5dc11596afcf833575cab33c45b07bcc404dac1fce71b7237ae028b033dccfcb48daa80fcfe7fac5f654b6f71fc504f106e5da90e587458cc8085c30aa589f7b8f3463eef7fbef615ed7d299fa510660dc6273a75e5834a3526aeeea392c3ade636780322369ca451603aaa09cf96c6da65b4d97d8b21a780fbba10f33f2b813042c1f98218164df6c605a51a7b2600b879efe3b46ddee584ce036e8d3acad4da581d7a5d0e85286b3958564f8f57b24ca256c7d22243047c817823751bab9f8dd2d927982234a68cba68b8333fc2fb910dff308460d320c1a00f6c81f1c8aebee041e09a5aaac6961d50694def0f550b471a993d739fe55ab951886d2a66cd68b980ac03fe26bbb47ceabdc1e1964f44dc24766dcaa601ea6ed936ea9b18f97e964e042c97a25e5a396b5b51cec8aaf0d317bbdcf618939e6f5abf8afa233ab4e93c31ab9279a85404d78f8a246cde211f3cf3f28a617943c226e2176893b2df222ff868c8b3562b1bd2486df4eb5d2efec1dc1637225d76a9fa871ecadb0d11c243899a5cf4ecc3a9e35579f1356a3ccf11a13b3f9cb5dff8da3a1d860671ace6bfe0f1517d78f076785dc4dc6da5dd90a031af65ab7fc6ceb752e38211976785f99df62fe3870f92b14de895bfef4c4a83e047186ae048b1d63c37b3d359ab1f6c43f99f2e9d1aee5cd23e681219d1eaca638a9a3e21041898f69d135c0a764b7651dcd42518346ce982f4c2db58a1db132d154865fe5f519c6cf3a5b313b59e5707f1c44155f8e7ff4d0032e0974dbef293fb30b358e78c3d84afe5975ca3eae931d845791c5fd6f2934bc2d688eb8435071f1750f0d443b7525b522cba196af504f942867549d86f7be547762e0e2ffb15a980b0a45442a5e77024ecc7d3bc4b2b2f816807821e060c78a24b618e44fd72554d562ed1593f613085f38436728f3790c09d0138369b5a129c5592507890680300364a6193ec81f4d59f5c7418728f9416731f5ef0f085368010a525a7a001a88f3c01fffe88188356ab88a1b657a35a61486f5fb98eec1049b7f04c5f51bbd8cf8b86000000000131a63c58f332274cc0c4e2cec543ed76f0e910eed117c891908ae6448cdb243d391626d82709a53b72bcc726e410d5f2715c086e533b030b991d74f1ebfbbc8b5bbaaba0dc8d344af826e55ef1e4ba402c5ff1f2a07f9ba7aa8da838a1ce902757e5927cb50ee58e55e38e395e72bc567344a9f6e0721507d8f938aec1042fbd0173e74b82eea12d8febce6317d925d0c39c814e6122981e3e665b7c012873a0ae18332aea7e3714b22a18ef3cf27c569ba177e7d18b8c5b934b004c20beeac00401cea03ba224c78cebbcc29aab7e0d9443a24bcacb6a52f96b25d8f4af6308cf3e4c7990fc442a31a348663c0100019d9eae30a3a59d4fe08ad61836b1cfaaa23a76053d9b26179093737f30adf43301d225bd908d85e143de5cbaf98423bc0c3735c77b46225aff10d0e2b6471a2a5201af9626d26a9f6d86cd208c9753d8462b4b7929699484ffe54bfa629f85a9e0283e64b6ac0f6ad307b99006c8fdba71450ad99487fb7e6a00a3ae9b6c85e51b229797292c3f8e260b67b25d5906f614926828fd1a974ac262cfa0d6b011187f1f0001d6e40e658731055551ef8dbb1427b25606a082affe87a279b163cc54bb6c51200000009a88fbf9a9ef37158278bb506ca50d2c845f95dbf9e1e2e43bb0e2201693e8123206e79f6c75a113e56cc2a86a90f22f5f06c641279c0bb9d80ecd9aaa40e080c4046025419e1b75c3a10ae3be56b52abec98a45ff3281114b6d3e4d04f0914b2ff59cd810678aa8375e23565293b44551d81b3243d7241e6063acbc7ce9d251dac468b53ad7d88b50648e05629b3311c568a7a6b45ba0f4f7eadeb5b51daba640743fa7ee182d70f2c21d8ac75e649a8f1144bfcaa1faf1ded13644f450e2a95ac9b7dabb6705c8b77d5f7fe5d805909f711e9a834c2325fa26fc2c0946f60ada5ddae592f9e87e516398856453d8443013b6034ea6edd721b819e6fe32d69568e558ff5aba5693bcd61fa7b46e7724d8e009463385575e5fb4467b47db28669bd5af9db5fad9e2e3dcbe16499d28914613a1174ea72b482a75766226a82879add4ee4dda2523634017341f30cb88e742af5cbdefebd17658522181b7a94fe9be3a42e57c60871c710e8d360aafd256d87e0671b912a8e3712ac18ca8f7c5957229440fcae6a329a6965e3a5079384e0489c22786eb445b4115cc8ed4f1a6720ffd34b1eaec705a996d9c52eae9c64693b5b41e777b5f9229dd0b668428daa439bcf57bfd643362023c47fbf3d458262460a4f4d36cccb01e2c434f2e72ce3eb500975a43222e853866b22cd737610bdcb2d17c30a836f3b56a6c5839d4bccaf3dba45645de8a6e05e05bec4d131162855d0587df0e524c10df282dd3bab6e91e6942d50915a007b6c79fed726a1228415f83c5ff9e77adb46196b578a19a0da9d9f5f0a780ebbe977d342edfdd62cf0687f323f3cbf7bbc0ee4af643df2355dd216c7bc7d6ba80f3a73f129062a057f459692e1f09dee80df372338c7417c0cad51bf004f050ce1ca59bc07ec1dccda5a610016696df431b51a5f34b6c090ecc097398c30d5cf425b56abc309d6d08ddaa7127ffa838d67ff51b3bcb635564877fe51ed3b8ec2dbb4541cdef4183c227c941cc849de201b8e1f32d6700073b93cad2702f9ada4931c2db2f6c019bc8f3fa86afbcbe5aa9ee25230c508362d30074ba0100010834300f4c8af9038f6d32993ada26f6261ac0d97a1d439c96462a116954c5fe00000000012c63452816f36831e4e8efa30514d3114824d896e626102f937ba2bb9782632b03d89da604004b0821b178104998cf5676cdf00f1b6cffb86056518d55a2ff066f8ab992631a00015c098280e83a49f1c6043e69cfbe7966380ee984b83ed8c023068900245e8828";
        let pczt = Pczt::parse(&hex::decode(pczt_hex).unwrap()).unwrap();
        assert!(has_transparent(&pczt));
        assert!(!is_transparent_coinbase(&pczt));
        assert!(!has_sapling(&pczt));
        assert!(has_orchard(&pczt));
        assert_eq!(
            hex::encode(digest_header(&pczt, 0).as_bytes()),
            "96c60ee6c88b3b44c751be6793669a38154552ddb490b58d5b732c80255c5be5"
        );
        assert_eq!(
            hex::encode(digest_transparent_prevouts(pczt.transparent().inputs()).as_bytes()),
            "a04b16834dc939ccf632d15dddaa6bcaed253d12068dca169fbd28bc403cf3ba"
        );
        assert_eq!(
            hex::encode(digest_transparent_sequence(pczt.transparent().inputs()).as_bytes()),
            "3a0033336603235001742438238d713c09c1c55bf67bd20b80805aea0b46eba5"
        );
        assert_eq!(
            hex::encode(digest_transparent_outputs(pczt.transparent().outputs()).as_bytes()),
            "f214119a412438e233a369039397ed5e247da4511b49af8304f25392292de53c"
        );
        assert_eq!(
            hex::encode(digest_sapling(&pczt).as_bytes()),
            "6f2fc8f98feafd94e74a0df4bed74391ee0b5a69945e4ced8ca8a095206f00ae"
        );
        assert_eq!(
            hex::encode(digest_orchard(&pczt).as_bytes()),
            "90dc4739bdcae8f81c162213e9742d988b6abd29beecf1203e1a59a794e8cb4b"
        );
        assert_eq!(
            hex::encode(shielded_sig_commitment(&pczt, 0, None).as_bytes()),
            "d9b80aac7a7e0f9cd525572877656bd923ff0c557be9a1ff16ff6e8e389ccc81"
        );
    }
}
