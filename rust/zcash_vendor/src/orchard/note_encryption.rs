use blake2b_simd::{Hash, Params};
use ff::PrimeField;
use subtle::ConstantTimeEq;

use crate::orchard::note::{NoteValue, RandomSeed};

use super::{
    commitment::ExtractedNoteCommitment,
    keys::{
        DiversifiedTransmissionKey, Diversifier, EphemeralKeyBytes, EphemeralPublicKey,
        EphemeralSecretKey, OutgoingViewingKey, PreparedEphemeralPublicKey,
        PreparedIncomingViewingKey, SharedSecret,
    },
    note::{Note, Rho},
    note_ext::{
        NotePlaintextBytes, OutPlaintextBytes, COMPACT_NOTE_SIZE, NOTE_PLAINTEXT_SIZE,
        OUT_PLAINTEXT_SIZE,
    },
    value::ValueCommitment,
    Address,
};

#[derive(Copy, Clone, PartialEq, Eq)]
pub enum NoteValidity {
    Valid,
    Invalid,
}

pub struct OutgoingCipherKey(pub [u8; 32]);

impl From<[u8; 32]> for OutgoingCipherKey {
    fn from(ock: [u8; 32]) -> Self {
        OutgoingCipherKey(ock)
    }
}

impl AsRef<[u8]> for OutgoingCipherKey {
    fn as_ref(&self) -> &[u8] {
        &self.0
    }
}

const PRF_OCK_ORCHARD_PERSONALIZATION: &[u8; 16] = b"Zcash_Orchardock";

pub(crate) fn prf_ock_orchard(
    ovk: &OutgoingViewingKey,
    cv: &ValueCommitment,
    cmx_bytes: &[u8; 32],
    ephemeral_key: &EphemeralKeyBytes,
) -> OutgoingCipherKey {
    OutgoingCipherKey(
        Params::new()
            .hash_length(32)
            .personal(PRF_OCK_ORCHARD_PERSONALIZATION)
            .to_state()
            .update(ovk.as_ref())
            .update(&cv.to_bytes())
            .update(cmx_bytes)
            .update(ephemeral_key.as_ref())
            .finalize()
            .as_bytes()
            .try_into()
            .unwrap(),
    )
}

/// Trait that encapsulates protocol-specific note encryption types and logic.
///
/// This trait enables most of the note encryption logic to be shared between Sapling and
/// Orchard, as well as between different implementations of those protocols.
pub trait Domain {
    type EphemeralSecretKey: ConstantTimeEq;
    type EphemeralPublicKey;
    type PreparedEphemeralPublicKey;
    type SharedSecret;
    type SymmetricKey: AsRef<[u8]>;
    type Note;
    type Recipient;
    type DiversifiedTransmissionKey;
    type IncomingViewingKey;
    type OutgoingViewingKey;
    type ValueCommitment;
    type ExtractedCommitment;
    type ExtractedCommitmentBytes: Eq + for<'a> From<&'a Self::ExtractedCommitment>;
    type Memo;

    /// Derives the `EphemeralSecretKey` corresponding to this note.
    ///
    /// Returns `None` if the note was created prior to [ZIP 212], and doesn't have a
    /// deterministic `EphemeralSecretKey`.
    ///
    /// [ZIP 212]: https://zips.z.cash/zip-0212
    fn derive_esk(note: &Self::Note) -> Option<Self::EphemeralSecretKey>;

    /// Extracts the `DiversifiedTransmissionKey` from the note.
    fn get_pk_d(note: &Self::Note) -> Self::DiversifiedTransmissionKey;

    /// Prepare an ephemeral public key for more efficient scalar multiplication.
    fn prepare_epk(epk: Self::EphemeralPublicKey) -> Self::PreparedEphemeralPublicKey;

    /// Derives `EphemeralPublicKey` from `esk` and the note's diversifier.
    fn ka_derive_public(
        note: &Self::Note,
        esk: &Self::EphemeralSecretKey,
    ) -> Self::EphemeralPublicKey;

    /// Derives the `SharedSecret` from the sender's information during note encryption.
    fn ka_agree_enc(
        esk: &Self::EphemeralSecretKey,
        pk_d: &Self::DiversifiedTransmissionKey,
    ) -> Self::SharedSecret;

    /// Derives the `SharedSecret` from the recipient's information during note trial
    /// decryption.
    fn ka_agree_dec(
        ivk: &Self::IncomingViewingKey,
        epk: &Self::PreparedEphemeralPublicKey,
    ) -> Self::SharedSecret;

    /// Derives the `SymmetricKey` used to encrypt the note plaintext.
    ///
    /// `secret` is the `SharedSecret` obtained from [`Self::ka_agree_enc`] or
    /// [`Self::ka_agree_dec`].
    ///
    /// `ephemeral_key` is the byte encoding of the [`EphemeralPublicKey`] used to derive
    /// `secret`. During encryption it is derived via [`Self::epk_bytes`]; during trial
    /// decryption it is obtained from [`ShieldedOutput::ephemeral_key`].
    ///
    /// [`EphemeralPublicKey`]: Self::EphemeralPublicKey
    /// [`EphemeralSecretKey`]: Self::EphemeralSecretKey
    fn kdf(secret: Self::SharedSecret, ephemeral_key: &EphemeralKeyBytes) -> Self::SymmetricKey;

    /// Encodes the given `Note` and `Memo` as a note plaintext.
    fn note_plaintext_bytes(note: &Self::Note, memo: &Self::Memo) -> NotePlaintextBytes;

    /// Derives the [`OutgoingCipherKey`] for an encrypted note, given the note-specific
    /// public data and an `OutgoingViewingKey`.
    fn derive_ock(
        ovk: &Self::OutgoingViewingKey,
        cv: &Self::ValueCommitment,
        cmstar_bytes: &Self::ExtractedCommitmentBytes,
        ephemeral_key: &EphemeralKeyBytes,
    ) -> OutgoingCipherKey;

    /// Encodes the outgoing plaintext for the given note.
    fn outgoing_plaintext_bytes(
        note: &Self::Note,
        esk: &Self::EphemeralSecretKey,
    ) -> OutPlaintextBytes;

    /// Returns the byte encoding of the given `EphemeralPublicKey`.
    fn epk_bytes(epk: &Self::EphemeralPublicKey) -> EphemeralKeyBytes;

    /// Attempts to parse `ephemeral_key` as an `EphemeralPublicKey`.
    ///
    /// Returns `None` if `ephemeral_key` is not a valid byte encoding of an
    /// `EphemeralPublicKey`.
    fn epk(ephemeral_key: &EphemeralKeyBytes) -> Option<Self::EphemeralPublicKey>;

    /// Derives the `ExtractedCommitment` for this note.
    fn cmstar(note: &Self::Note) -> Self::ExtractedCommitment;

    /// Parses the given note plaintext from the recipient's perspective.
    ///
    /// The implementation of this method must check that:
    /// - The note plaintext version is valid (for the given decryption domain's context,
    ///   which may be passed via `self`).
    /// - The note plaintext contains valid encodings of its various fields.
    /// - Any domain-specific requirements are satisfied.
    ///
    /// `&self` is passed here to enable the implementation to enforce contextual checks,
    /// such as rules like [ZIP 212] that become active at a specific block height.
    ///
    /// [ZIP 212]: https://zips.z.cash/zip-0212
    ///
    /// # Panics
    ///
    /// Panics if `plaintext` is shorter than [`COMPACT_NOTE_SIZE`].
    fn parse_note_plaintext_without_memo_ivk(
        &self,
        ivk: &Self::IncomingViewingKey,
        plaintext: &[u8],
    ) -> Option<(Self::Note, Self::Recipient)>;

    /// Parses the given note plaintext from the sender's perspective.
    ///
    /// The implementation of this method must check that:
    /// - The note plaintext version is valid (for the given decryption domain's context,
    ///   which may be passed via `self`).
    /// - The note plaintext contains valid encodings of its various fields.
    /// - Any domain-specific requirements are satisfied.
    ///
    /// `&self` is passed here to enable the implementation to enforce contextual checks,
    /// such as rules like [ZIP 212] that become active at a specific block height.
    ///
    /// [ZIP 212]: https://zips.z.cash/zip-0212
    fn parse_note_plaintext_without_memo_ovk(
        &self,
        pk_d: &Self::DiversifiedTransmissionKey,
        plaintext: &NotePlaintextBytes,
    ) -> Option<(Self::Note, Self::Recipient)>;

    /// Extracts the memo field from the given note plaintext.
    ///
    /// # Compatibility
    ///
    /// `&self` is passed here in anticipation of future changes to memo handling, where
    /// the memos may no longer be part of the note plaintext.
    fn extract_memo(&self, plaintext: &NotePlaintextBytes) -> Self::Memo;

    /// Parses the `DiversifiedTransmissionKey` field of the outgoing plaintext.
    ///
    /// Returns `None` if `out_plaintext` does not contain a valid byte encoding of a
    /// `DiversifiedTransmissionKey`.
    fn extract_pk_d(out_plaintext: &OutPlaintextBytes) -> Option<Self::DiversifiedTransmissionKey>;

    /// Parses the `EphemeralSecretKey` field of the outgoing plaintext.
    ///
    /// Returns `None` if `out_plaintext` does not contain a valid byte encoding of an
    /// `EphemeralSecretKey`.
    fn extract_esk(out_plaintext: &OutPlaintextBytes) -> Option<Self::EphemeralSecretKey>;
}

/// Orchard-specific note encryption logic.
#[derive(Debug)]
pub struct OrchardDomain {
    rho: Rho,
}

impl OrchardDomain {
    pub fn new(rho: Rho) -> Self {
        OrchardDomain { rho }
    }
}

impl Domain for OrchardDomain {
    type EphemeralSecretKey = EphemeralSecretKey;
    type EphemeralPublicKey = EphemeralPublicKey;
    type PreparedEphemeralPublicKey = PreparedEphemeralPublicKey;
    type SharedSecret = SharedSecret;
    type SymmetricKey = Hash;
    type Note = Note;
    type Recipient = Address;
    type DiversifiedTransmissionKey = DiversifiedTransmissionKey;
    type IncomingViewingKey = PreparedIncomingViewingKey;
    type OutgoingViewingKey = OutgoingViewingKey;
    type ValueCommitment = ValueCommitment;
    type ExtractedCommitment = ExtractedNoteCommitment;
    type ExtractedCommitmentBytes = [u8; 32];
    type Memo = [u8; 512]; // TODO use a more interesting type

    fn derive_esk(note: &Self::Note) -> Option<Self::EphemeralSecretKey> {
        Some(note.esk())
    }

    fn get_pk_d(note: &Self::Note) -> Self::DiversifiedTransmissionKey {
        *note.recipient().pk_d()
    }

    fn prepare_epk(epk: Self::EphemeralPublicKey) -> Self::PreparedEphemeralPublicKey {
        PreparedEphemeralPublicKey::new(epk)
    }

    fn ka_derive_public(
        note: &Self::Note,
        esk: &Self::EphemeralSecretKey,
    ) -> Self::EphemeralPublicKey {
        esk.derive_public(note.recipient().g_d())
    }

    fn ka_agree_enc(
        esk: &Self::EphemeralSecretKey,
        pk_d: &Self::DiversifiedTransmissionKey,
    ) -> Self::SharedSecret {
        esk.agree(pk_d)
    }

    fn ka_agree_dec(
        ivk: &Self::IncomingViewingKey,
        epk: &Self::PreparedEphemeralPublicKey,
    ) -> Self::SharedSecret {
        epk.agree(ivk)
    }

    fn kdf(secret: Self::SharedSecret, ephemeral_key: &EphemeralKeyBytes) -> Self::SymmetricKey {
        secret.kdf_orchard(ephemeral_key)
    }

    fn note_plaintext_bytes(note: &Self::Note, memo: &Self::Memo) -> NotePlaintextBytes {
        let mut np = [0; NOTE_PLAINTEXT_SIZE];
        np[0] = 0x02;
        np[1..12].copy_from_slice(note.recipient().diversifier().as_array());
        np[12..20].copy_from_slice(&note.value().to_bytes());
        np[20..52].copy_from_slice(note.rseed().as_bytes());
        np[52..].copy_from_slice(memo);
        NotePlaintextBytes(np)
    }

    fn derive_ock(
        ovk: &Self::OutgoingViewingKey,
        cv: &Self::ValueCommitment,
        cmstar_bytes: &Self::ExtractedCommitmentBytes,
        ephemeral_key: &EphemeralKeyBytes,
    ) -> OutgoingCipherKey {
        prf_ock_orchard(ovk, cv, cmstar_bytes, ephemeral_key)
    }

    fn outgoing_plaintext_bytes(
        note: &Self::Note,
        esk: &Self::EphemeralSecretKey,
    ) -> OutPlaintextBytes {
        let mut op = [0; OUT_PLAINTEXT_SIZE];
        op[..32].copy_from_slice(&note.recipient().pk_d().to_bytes());
        op[32..].copy_from_slice(&esk.0.to_repr());
        OutPlaintextBytes(op)
    }

    fn epk_bytes(epk: &Self::EphemeralPublicKey) -> EphemeralKeyBytes {
        epk.to_bytes()
    }

    fn epk(ephemeral_key: &EphemeralKeyBytes) -> Option<Self::EphemeralPublicKey> {
        EphemeralPublicKey::from_bytes(&ephemeral_key.0).into()
    }

    fn cmstar(note: &Self::Note) -> Self::ExtractedCommitment {
        note.commitment().into()
    }

    fn parse_note_plaintext_without_memo_ivk(
        &self,
        ivk: &Self::IncomingViewingKey,
        plaintext: &[u8],
    ) -> Option<(Self::Note, Self::Recipient)> {
        orchard_parse_note_plaintext_without_memo(self, plaintext, |diversifier| {
            DiversifiedTransmissionKey::derive(ivk, diversifier)
        })
    }

    fn parse_note_plaintext_without_memo_ovk(
        &self,
        pk_d: &Self::DiversifiedTransmissionKey,
        plaintext: &NotePlaintextBytes,
    ) -> Option<(Self::Note, Self::Recipient)> {
        orchard_parse_note_plaintext_without_memo(self, &plaintext.0, |_| *pk_d)
    }

    fn extract_memo(&self, plaintext: &NotePlaintextBytes) -> Self::Memo {
        plaintext.0[COMPACT_NOTE_SIZE..NOTE_PLAINTEXT_SIZE]
            .try_into()
            .unwrap()
    }

    fn extract_pk_d(out_plaintext: &OutPlaintextBytes) -> Option<Self::DiversifiedTransmissionKey> {
        DiversifiedTransmissionKey::from_bytes(out_plaintext.0[0..32].try_into().unwrap()).into()
    }

    fn extract_esk(out_plaintext: &OutPlaintextBytes) -> Option<Self::EphemeralSecretKey> {
        EphemeralSecretKey::from_bytes(out_plaintext.0[32..OUT_PLAINTEXT_SIZE].try_into().unwrap())
            .into()
    }
}

pub fn check_note_validity<D: Domain>(
    note: &D::Note,
    ephemeral_key: &EphemeralKeyBytes,
    cmstar_bytes: &D::ExtractedCommitmentBytes,
) -> NoteValidity {
    if &D::ExtractedCommitmentBytes::from(&D::cmstar(note)) == cmstar_bytes {
        // In the case corresponding to specification section 4.19.3, we check that `esk` is equal
        // to `D::derive_esk(note)` prior to calling this method.
        if let Some(derived_esk) = D::derive_esk(note) {
            if D::epk_bytes(&D::ka_derive_public(note, &derived_esk))
                .ct_eq(ephemeral_key)
                .into()
            {
                NoteValidity::Valid
            } else {
                NoteValidity::Invalid
            }
        } else {
            // Before ZIP 212
            NoteValidity::Valid
        }
    } else {
        // Published commitment doesn't match calculated commitment
        NoteValidity::Invalid
    }
}

fn orchard_parse_note_plaintext_without_memo<F>(
    domain: &OrchardDomain,
    plaintext: &[u8],
    get_pk_d: F,
) -> Option<(Note, Address)>
where
    F: FnOnce(&Diversifier) -> DiversifiedTransmissionKey,
{
    assert!(plaintext.len() >= COMPACT_NOTE_SIZE);

    // Check note plaintext version
    if plaintext[0] != 0x02 {
        return None;
    }

    // The unwraps below are guaranteed to succeed by the assertion above
    let diversifier = Diversifier::from_bytes(plaintext[1..12].try_into().unwrap());
    let value = NoteValue::from_bytes(plaintext[12..20].try_into().unwrap());
    let rseed = Option::from(RandomSeed::from_bytes(
        plaintext[20..COMPACT_NOTE_SIZE].try_into().unwrap(),
        &domain.rho,
    ))?;

    let pk_d = get_pk_d(&diversifier);

    let recipient = Address::from_parts(diversifier, pk_d);
    let note = Option::from(Note::from_parts(recipient, value, domain.rho, rseed))?;
    Some((note, recipient))
}
