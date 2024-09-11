use crate::algorithms::zcash::vendor::zip32::{AccountId, DiversifierIndex, Scope};

use super::prf_expand::PrfExpand;
use super::redpallas::SpendAuth;
use super::spec::{commit_ivk, extract_p, to_base, NonZeroPallasBase, NonZeroPallasScalar};
use super::zip32::ExtendedSpendingKey;
use super::{redpallas, zip32::ChildIndex};
use pasta_curves::{
    group::{ff::Field, ff::FromUniformBytes, ff::PrimeField, GroupEncoding},
    pallas,
};
use rand_chacha::rand_core::RngCore;
use subtle::CtOption;

const ZIP32_PURPOSE: u32 = 32;

#[derive(Debug, Copy, Clone)]
pub struct SpendingKey([u8; 32]);

impl SpendingKey {
    /// Generates a random spending key.
    ///
    /// This is only used when generating dummy notes. Real spending keys should be
    /// derived according to [ZIP 32].
    ///
    /// [ZIP 32]: https://zips.z.cash/zip-0032
    pub(crate) fn random(rng: &mut impl RngCore) -> Self {
        loop {
            let mut bytes = [0; 32];
            rng.fill_bytes(&mut bytes);
            let sk = SpendingKey::from_bytes(bytes);
            break sk;
        }
    }

    /// Constructs an Orchard spending key from uniformly-random bytes.
    ///
    /// Returns `None` if the bytes do not correspond to a valid Orchard spending key.
    pub fn from_bytes(sk: [u8; 32]) -> Self {
        let sk = SpendingKey(sk);
        // If ask = 0, discard this key. We call `derive_inner` rather than
        // `SpendAuthorizingKey::from` here because we only need to know
        // whether ask = 0; the adjustment to potentially negate ask is not
        // needed. Also, `from` would panic on ask = 0.
        sk
    }

    /// Returns the raw bytes of the spending key.
    pub fn to_bytes(&self) -> &[u8; 32] {
        &self.0
    }

    pub fn from_zip32_seed(seed: &[u8], coin_type: u32, account: AccountId) -> Self {
        if coin_type >= (1 << 31) {
            panic!()
        }

        // Call zip32 logic
        let path = &[
            ChildIndex::hardened(ZIP32_PURPOSE),
            ChildIndex::hardened(coin_type),
            ChildIndex::hardened(account.into()),
        ];
        match ExtendedSpendingKey::from_path(seed, path).map(|esk| esk.sk()) {
            Ok(sk) => sk,
            Err(e) => panic!(),
        }
    }
}

#[derive(Clone, Debug)]
pub struct SpendAuthorizingKey(redpallas::SigningKey<SpendAuth>);

pub(crate) fn to_scalar(x: [u8; 64]) -> pallas::Scalar {
    pallas::Scalar::from_uniform_bytes(&x)
}

impl SpendAuthorizingKey {
    /// Derives ask from sk. Internal use only, does not enforce all constraints.
    fn derive_inner(sk: &SpendingKey) -> pallas::Scalar {
        to_scalar(PrfExpand::ORCHARD_ASK.with(&sk.0))
    }

    /// Randomizes this spend authorizing key with the given `randomizer`.
    ///
    /// The resulting key can be used to actually sign a spend.
    pub fn randomize(&self, randomizer: &pallas::Scalar) -> redpallas::SigningKey<SpendAuth> {
        self.0.randomize(randomizer)
    }
}

impl From<&SpendingKey> for SpendAuthorizingKey {
    fn from(sk: &SpendingKey) -> Self {
        let ask = Self::derive_inner(sk);
        // SpendingKey cannot be constructed such that this assertion would fail.
        assert!(!bool::from(ask.is_zero()));
        // TODO: Add TryFrom<S::Scalar> for SpendAuthorizingKey.
        let ret = SpendAuthorizingKey(ask.to_repr().try_into().unwrap());
        // If the last bit of repr_P(ak) is 1, negate ask.
        if (<[u8; 32]>::from(SpendValidatingKey::from(&ret).0)[31] >> 7) == 1 {
            SpendAuthorizingKey((-ask).to_repr().try_into().unwrap())
        } else {
            ret
        }
    }
}

#[derive(Debug, Clone, PartialOrd, Ord)]
pub struct SpendValidatingKey(redpallas::VerificationKey<SpendAuth>);

impl From<&SpendAuthorizingKey> for SpendValidatingKey {
    fn from(ask: &SpendAuthorizingKey) -> Self {
        SpendValidatingKey((&ask.0).into())
    }
}

impl From<&SpendValidatingKey> for pallas::Point {
    fn from(spend_validating_key: &SpendValidatingKey) -> pallas::Point {
        pallas::Point::from_bytes(&(&spend_validating_key.0).into()).unwrap()
    }
}

impl PartialEq for SpendValidatingKey {
    fn eq(&self, other: &Self) -> bool {
        <[u8; 32]>::from(&self.0).eq(&<[u8; 32]>::from(&other.0))
    }
}

impl Eq for SpendValidatingKey {}

impl SpendValidatingKey {
    /// Randomizes this spend validating key with the given `randomizer`.
    pub fn randomize(&self, randomizer: &pallas::Scalar) -> redpallas::VerificationKey<SpendAuth> {
        self.0.randomize(randomizer)
    }

    /// Converts this spend validating key to its serialized form,
    /// I2LEOSP_256(ak).
    #[cfg_attr(feature = "unstable-frost", visibility::make(pub))]
    pub(crate) fn to_bytes(&self) -> [u8; 32] {
        // This is correct because the wrapped point must have ỹ = 0, and
        // so the point repr is the same as I2LEOSP of its x-coordinate.
        let b = <[u8; 32]>::from(&self.0);
        assert!(b[31] & 0x80 == 0);
        b
    }

    /// Attempts to parse a byte slice as a spend validating key, `I2LEOSP_256(ak)`.
    ///
    /// Returns `None` if the given slice does not contain a valid spend validating key.
    #[cfg_attr(feature = "unstable-frost", visibility::make(pub))]
    pub(crate) fn from_bytes(bytes: &[u8]) -> Option<Self> {
        <[u8; 32]>::try_from(bytes)
            .ok()
            .and_then(|b| {
                // Structural validity checks for ak_P:
                // - The point must not be the identity
                //   (which for Pallas is canonically encoded as all-zeroes).
                // - The sign of the y-coordinate must be positive.
                if b != [0; 32] && b[31] & 0x80 == 0 {
                    <redpallas::VerificationKey<SpendAuth>>::try_from(b).ok()
                } else {
                    None
                }
            })
            .map(SpendValidatingKey)
    }
}

#[derive(Copy, Debug, Clone, PartialEq, Eq, PartialOrd, Ord)]
pub(crate) struct NullifierDerivingKey(pallas::Base);

impl NullifierDerivingKey {
    pub(crate) fn inner(&self) -> pallas::Base {
        self.0
    }
}

impl From<&SpendingKey> for NullifierDerivingKey {
    fn from(sk: &SpendingKey) -> Self {
        NullifierDerivingKey(to_base(PrfExpand::ORCHARD_NK.with(&sk.0)))
    }
}

/// The randomness for $\mathsf{Commit}^\mathsf{ivk}$.
///
/// $\mashsf{rivk}$ as defined in [Zcash Protocol Spec § 4.2.3: Orchard Key Components][orchardkeycomponents].
///
/// [orchardkeycomponents]: https://zips.z.cash/protocol/nu5.pdf#orchardkeycomponents
#[derive(Copy, Debug, Clone, PartialEq, Eq, PartialOrd, Ord)]
pub(crate) struct CommitIvkRandomness(pallas::Scalar);

impl From<&SpendingKey> for CommitIvkRandomness {
    fn from(sk: &SpendingKey) -> Self {
        CommitIvkRandomness(to_scalar(PrfExpand::ORCHARD_RIVK.with(&sk.0)))
    }
}

#[derive(Debug, Clone, PartialEq, Eq, PartialOrd, Ord)]
pub struct FullViewingKey {
    ak: SpendValidatingKey,
    nk: NullifierDerivingKey,
    rivk: CommitIvkRandomness,
}

impl From<&SpendingKey> for FullViewingKey {
    fn from(sk: &SpendingKey) -> Self {
        FullViewingKey {
            ak: (&SpendAuthorizingKey::from(sk)).into(),
            nk: sk.into(),
            rivk: sk.into(),
        }
    }
}

impl From<&ExtendedSpendingKey> for FullViewingKey {
    fn from(extsk: &ExtendedSpendingKey) -> Self {
        (&extsk.sk()).into()
    }
}

impl From<FullViewingKey> for SpendValidatingKey {
    fn from(fvk: FullViewingKey) -> Self {
        fvk.ak
    }
}

impl FullViewingKey {
    /// Serializes the full viewing key as specified in [Zcash Protocol Spec § 5.6.4.4: Orchard Raw Full Viewing Keys][orchardrawfullviewingkeys]
    ///
    /// [orchardrawfullviewingkeys]: https://zips.z.cash/protocol/protocol.pdf#orchardfullviewingkeyencoding
    pub fn to_bytes(&self) -> [u8; 96] {
        let mut result = [0u8; 96];
        result[0..32].copy_from_slice(&<[u8; 32]>::from(self.ak.0.clone()));
        result[32..64].copy_from_slice(&self.nk.0.to_repr());
        result[64..96].copy_from_slice(&self.rivk.0.to_repr());
        result
    }
}

#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub struct Diversifier([u8; 11]);

impl Diversifier {
    /// Reads a diversifier from a byte array.
    pub fn from_bytes(d: [u8; 11]) -> Self {
        Diversifier(d)
    }

    /// Returns the byte array corresponding to this diversifier.
    pub fn as_array(&self) -> &[u8; 11] {
        &self.0
    }
}

#[derive(Clone, Debug, PartialEq, Eq, PartialOrd, Ord)]
pub struct IncomingViewingKey {
    dk: DiversifierKey,
    ivk: KeyAgreementPrivateKey,
}

impl IncomingViewingKey {
    /// Helper method.
    fn from_fvk(fvk: &FullViewingKey) -> Self {
        IncomingViewingKey {
            dk: fvk.derive_dk_ovk().0,
            ivk: KeyAgreementPrivateKey::from_fvk(fvk),
        }
    }
}

impl IncomingViewingKey {
    /// Serializes an Orchard incoming viewing key to its raw encoding as specified in [Zcash Protocol Spec § 5.6.4.3: Orchard Raw Incoming Viewing Keys][orchardrawinviewingkeys]
    ///
    /// [orchardrawinviewingkeys]: https://zips.z.cash/protocol/protocol.pdf#orchardinviewingkeyencoding
    pub fn to_bytes(&self) -> [u8; 64] {
        let mut result = [0u8; 64];
        result[..32].copy_from_slice(self.dk.to_bytes());
        result[32..].copy_from_slice(&self.ivk.0.to_repr());
        result
    }

    /// Parses an Orchard incoming viewing key from its raw encoding.
    pub fn from_bytes(bytes: &[u8; 64]) -> CtOption<Self> {
        NonZeroPallasBase::from_bytes(bytes[32..].try_into().unwrap()).map(|ivk| {
            IncomingViewingKey {
                dk: DiversifierKey(bytes[..32].try_into().unwrap()),
                ivk: KeyAgreementPrivateKey(ivk.into()),
            }
        })
    }

    /// Checks whether the given address was derived from this incoming viewing
    /// key, and returns the diversifier index used to derive the address if
    /// so. Returns `None` if the address was not derived from this key.
    pub fn diversifier_index(&self, addr: &Address) -> Option<DiversifierIndex> {
        let j = self.dk.diversifier_index(&addr.diversifier());
        if &self.address_at(j) == addr {
            Some(j)
        } else {
            None
        }
    }

    // /// Returns the payment address for this key at the given index.
    // pub fn address_at(&self, j: impl Into<DiversifierIndex>) -> Address {
    //     self.address(self.dk.get(j))
    // }

    // /// Returns the payment address for this key corresponding to the given diversifier.
    // pub fn address(&self, d: Diversifier) -> Address {
    //     self.ivk.address(d)
    // }

    // /// Returns the [`PreparedIncomingViewingKey`] for this [`IncomingViewingKey`].
    // pub fn prepare(&self) -> PreparedIncomingViewingKey {
    //     PreparedIncomingViewingKey::new(self)
    // }
}

#[derive(Clone, Debug, PartialEq, Eq, PartialOrd, Ord)]
struct KeyAgreementPrivateKey(NonZeroPallasScalar);

impl KeyAgreementPrivateKey {
    /// Derives `KeyAgreementPrivateKey` from fvk.
    ///
    /// Defined in [Zcash Protocol Spec § 4.2.3: Orchard Key Components][orchardkeycomponents].
    ///
    /// [orchardkeycomponents]: https://zips.z.cash/protocol/protocol.pdf#orchardkeycomponents
    fn from_fvk(fvk: &FullViewingKey) -> Self {
        // FullViewingKey cannot be constructed such that this unwrap would fail.
        let ivk = KeyAgreementPrivateKey::derive_inner(fvk).unwrap();
        KeyAgreementPrivateKey(ivk.into())
    }
}

impl KeyAgreementPrivateKey {
    /// Derives ivk from fvk. Internal use only, does not enforce all constraints.
    ///
    /// Defined in [Zcash Protocol Spec § 4.2.3: Orchard Key Components][orchardkeycomponents].
    ///
    /// [orchardkeycomponents]: https://zips.z.cash/protocol/protocol.pdf#orchardkeycomponents
    fn derive_inner(fvk: &FullViewingKey) -> CtOption<NonZeroPallasBase> {
        let ak = extract_p(&pallas::Point::from_bytes(&(&fvk.ak.0).into()).unwrap());
        commit_ivk(&ak, &fvk.nk.0, &fvk.rivk.0)
            // sinsemilla::CommitDomain::short_commit returns a value in range
            // [0..q_P] ∪ {⊥}:
            // - sinsemilla::HashDomain::hash_to_point uses incomplete addition and
            //   returns a point in P* ∪ {⊥}.
            // - sinsemilla::CommitDomain::commit applies a final complete addition step
            //   and returns a point in P ∪ {⊥}.
            // - 0 is not a valid x-coordinate for any Pallas point.
            // - sinsemilla::CommitDomain::short_commit calls extract_p_bottom, which
            //   replaces the identity (which has no affine coordinates) with 0.
            //
            // Commit^ivk.Output is specified as [1..q_P] ∪ {⊥}, so we explicitly check
            // for 0 and map it to None. Note that we are collapsing this case (which is
            // rejected by the circuit) with ⊥ (which the circuit explicitly allows for
            // efficiency); this is fine because we don't want users of the `orchard`
            // crate to encounter either case (and it matches the behaviour described in
            // Section 4.2.3 of the protocol spec when generating spending keys).
            .and_then(NonZeroPallasBase::from_base)
    }

    // /// Returns the payment address for this key corresponding to the given diversifier.
    // fn address(&self, d: Diversifier) -> Address {
    //     let prepared_ivk = PreparedIncomingViewingKey::new_inner(self);
    //     let pk_d = DiversifiedTransmissionKey::derive(&prepared_ivk, &d);
    //     Address::from_parts(d, pk_d)
    // }
}