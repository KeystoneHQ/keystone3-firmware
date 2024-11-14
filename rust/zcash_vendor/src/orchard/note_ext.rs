use core::iter;

use bitvec::{array::BitArray, order::Lsb0};
use chacha20poly1305::AeadInPlace;
use chacha20poly1305::{ChaCha20Poly1305, KeyInit};
use ff::{PrimeField, PrimeFieldBits};
use pasta_curves::{arithmetic::CurveExt, group::GroupEncoding, pallas, Ep};
use subtle::ConstantTimeEq;

use crate::sinsemilla;

use super::note::Nullifier;
use super::note_encryption::{check_note_validity, NoteValidity};
use super::{
    commitment::{NoteCommitTrapdoor, NoteCommitment},
    keys::{
        DiversifiedTransmissionKey, EphemeralKeyBytes, EphemeralSecretKey, NullifierDerivingKey,
    },
    note::{Memo, Note, NoteValue, RandomSeed, Recipient, Rho},
    note_encryption::{Domain, OutgoingCipherKey},
    spec::{diversify_hash, extract_p, mod_r_p},
};

pub const NOTE_COMMITMENT_PERSONALIZATION: &str = "z.cash:Orchard-NoteCommit";

pub(crate) const L_ORCHARD_BASE: usize = 255;

pub const COMPACT_NOTE_SIZE: usize = 1 + // version
    11 + // diversifier
    8  + // value
    32; // rseed (or rcm prior to ZIP 212)
/// The size of [`NotePlaintextBytes`].
pub const NOTE_PLAINTEXT_SIZE: usize = COMPACT_NOTE_SIZE + 512;
/// The size of [`OutPlaintextBytes`].
pub const OUT_PLAINTEXT_SIZE: usize = 32 + // pk_d
    32; // esk
const AEAD_TAG_SIZE: usize = 16;
/// The size of an encrypted note plaintext.
pub const ENC_CIPHERTEXT_SIZE: usize = NOTE_PLAINTEXT_SIZE + AEAD_TAG_SIZE;
/// The size of an encrypted outgoing plaintext.
pub const OUT_CIPHERTEXT_SIZE: usize = OUT_PLAINTEXT_SIZE + AEAD_TAG_SIZE;

/// Newtype representing the byte encoding of a note plaintext.
pub struct NotePlaintextBytes(pub [u8; NOTE_PLAINTEXT_SIZE]);
/// Newtype representing the byte encoding of a outgoing plaintext.
pub struct OutPlaintextBytes(pub [u8; OUT_PLAINTEXT_SIZE]);

pub fn calculate_note_commitment(
    recipient: &[u8; 43],
    value: u64,
    rho: &[u8; 32],
    rseed: &[u8; 32],
) -> Ep {
    let (divisifier, pk_d) = recipient.split_at(11);
    //TODO: no unwrap
    let divisifier: [u8; 11] = divisifier.try_into().unwrap();
    let pk_d: [u8; 32] = pk_d.try_into().unwrap();

    let g_d = diversify_hash(&divisifier).to_bytes();

    //TODO: no unwrap
    let rho = Rho::from_bytes(rho).unwrap();

    //TODO: no unwrap
    let rseed = RandomSeed::from_bytes(rseed.clone(), &rho).unwrap();

    let value = NoteValue::from_raw(value);

    NoteCommitment::derive(
        g_d,
        pk_d,
        value,
        rho.clone().into_inner(),
        rseed.psi(&rho),
        rseed.rcm(&rho),
    )
    .unwrap()
    .inner()
}

pub fn calculate_nullifier(
    nk: &NullifierDerivingKey,
    rho: &[u8; 32],
    rseed: &[u8; 32],
    cm: Ep,
) -> [u8; 32] {
    let rho = Rho::from_bytes(rho).unwrap();
    let rseed = RandomSeed::from_bytes(rseed.clone(), &rho).unwrap();
    let psi = rseed.psi(&rho);
    let cm = NoteCommitment(cm);

    Nullifier::derive(nk, rho.into_inner(), psi, cm).to_bytes()
}

pub fn try_output_recovery_with_ovk<D: Domain>(
    domain: &D,
    ovk: &D::OutgoingViewingKey,
    ephemeral_key: &EphemeralKeyBytes,
    cmx: &D::ExtractedCommitmentBytes,
    cv: &D::ValueCommitment,
    enc_ciphertext: &[u8; ENC_CIPHERTEXT_SIZE],
    out_ciphertext: &[u8; OUT_CIPHERTEXT_SIZE],
) -> Option<(D::Note, D::Recipient, D::Memo)> {
    let ock = D::derive_ock(ovk, cv, cmx, ephemeral_key);
    try_output_recovery_with_ock(
        domain,
        &ock,
        cmx,
        ephemeral_key,
        enc_ciphertext,
        out_ciphertext,
    )
}

/// Recovery of the full note plaintext by the sender.
///
/// Attempts to decrypt and validate the given shielded output using the given `ock`.
/// If successful, the corresponding note and memo are returned, along with the address to
/// which the note was sent.
///
/// Implements part of section 4.19.3 of the
/// [Zcash Protocol Specification](https://zips.z.cash/protocol/nu5.pdf#decryptovk).
/// For decryption using a Full Viewing Key see [`try_output_recovery_with_ovk`].
pub fn try_output_recovery_with_ock<D: Domain>(
    domain: &D,
    ock: &OutgoingCipherKey,
    cmx: &D::ExtractedCommitmentBytes,
    ephemeral_key: &EphemeralKeyBytes,
    enc_ciphertext: &[u8; ENC_CIPHERTEXT_SIZE],
    out_ciphertext: &[u8; OUT_CIPHERTEXT_SIZE],
) -> Option<(D::Note, D::Recipient, D::Memo)> {
    let mut op = OutPlaintextBytes([0; OUT_PLAINTEXT_SIZE]);
    op.0.copy_from_slice(&out_ciphertext[..OUT_PLAINTEXT_SIZE]);

    ChaCha20Poly1305::new(ock.as_ref().into())
        .decrypt_in_place_detached(
            [0u8; 12][..].into(),
            &[],
            &mut op.0,
            out_ciphertext[OUT_PLAINTEXT_SIZE..].into(),
        )
        .ok()?;

    let pk_d = D::extract_pk_d(&op)?;
    let esk = D::extract_esk(&op)?;

    let shared_secret = D::ka_agree_enc(&esk, &pk_d);
    // The small-order point check at the point of output parsing rejects
    // non-canonical encodings, so reencoding here for the KDF should
    // be okay.
    let key = D::kdf(shared_secret, &ephemeral_key);

    let mut plaintext = NotePlaintextBytes([0; NOTE_PLAINTEXT_SIZE]);
    plaintext
        .0
        .copy_from_slice(&enc_ciphertext[..NOTE_PLAINTEXT_SIZE]);

    ChaCha20Poly1305::new(key.as_ref().into())
        .decrypt_in_place_detached(
            [0u8; 12][..].into(),
            &[],
            &mut plaintext.0,
            enc_ciphertext[NOTE_PLAINTEXT_SIZE..].into(),
        )
        .ok()?;

    let (note, to) = domain.parse_note_plaintext_without_memo_ovk(&pk_d, &plaintext)?;
    let memo = domain.extract_memo(&plaintext);

    // ZIP 212: Check that the esk provided to this function is consistent with the esk we can
    // derive from the note. This check corresponds to `ToScalar(PRF^{expand}_{rseed}([4]) = esk`
    // in https://zips.z.cash/protocol/protocol.pdf#decryptovk. (`ρ^opt = []` for Sapling.)
    if let Some(derived_esk) = D::derive_esk(&note) {
        if (!derived_esk.ct_eq(&esk)).into() {
            return None;
        }
    }

    if let NoteValidity::Valid = check_note_validity::<D>(&note, &ephemeral_key, cmx) {
        Some((note, to, memo))
    } else {
        None
    }
}

fn extract_memo(plaintext: &NotePlaintextBytes) -> Memo {
    plaintext.0[COMPACT_NOTE_SIZE..NOTE_PLAINTEXT_SIZE]
        .try_into()
        .unwrap()
}

fn extract_pk_d(out_plaintext: &OutPlaintextBytes) -> Option<DiversifiedTransmissionKey> {
    DiversifiedTransmissionKey::from_bytes(out_plaintext.0[0..32].try_into().unwrap()).into()
}

fn extract_esk(out_plaintext: &OutPlaintextBytes) -> Option<EphemeralSecretKey> {
    EphemeralSecretKey::from_bytes(out_plaintext.0[32..OUT_PLAINTEXT_SIZE].try_into().unwrap())
        .into()
}
