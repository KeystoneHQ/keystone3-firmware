//! Helper functions defined in the Zcash Protocol Specification.

use core::iter;
use core::ops::Deref;

use super::{super::sinsemilla, constants::{COMMIT_IVK_PERSONALIZATION, KEY_DIVERSIFICATION_PERSONALIZATION, L_ORCHARD_BASE}};
use ff::{Field, FromUniformBytes, PrimeField, PrimeFieldBits};
use group::{Curve, Group, GroupEncoding, WnafBase, WnafScalar};
use pasta_curves::{arithmetic::{CurveAffine, CurveExt}, pallas};
use subtle::{ConditionallySelectable, CtOption};

/// A Pallas point that is guaranteed to not be the identity.
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub struct NonIdentityPallasPoint(pallas::Point);

impl Default for NonIdentityPallasPoint {
    fn default() -> Self {
        NonIdentityPallasPoint(pallas::Point::generator())
    }
}

impl ConditionallySelectable for NonIdentityPallasPoint {
    fn conditional_select(a: &Self, b: &Self, choice: subtle::Choice) -> Self {
        NonIdentityPallasPoint(pallas::Point::conditional_select(&a.0, &b.0, choice))
    }
}

impl NonIdentityPallasPoint {
    pub fn from_bytes(bytes: &[u8; 32]) -> CtOption<Self> {
        pallas::Point::from_bytes(bytes)
            .and_then(|p| CtOption::new(NonIdentityPallasPoint(p), !p.is_identity()))
    }
}

impl Deref for NonIdentityPallasPoint {
    type Target = pallas::Point;

    fn deref(&self) -> &pallas::Point {
        &self.0
    }
}

/// An integer in [1..q_P].
#[derive(Clone, Copy, Debug)]
pub struct NonZeroPallasBase(pallas::Base);

impl Default for NonZeroPallasBase {
    fn default() -> Self {
        NonZeroPallasBase(pallas::Base::one())
    }
}

impl ConditionallySelectable for NonZeroPallasBase {
    fn conditional_select(a: &Self, b: &Self, choice: subtle::Choice) -> Self {
        NonZeroPallasBase(pallas::Base::conditional_select(&a.0, &b.0, choice))
    }
}

impl NonZeroPallasBase {
    pub fn from_bytes(bytes: &[u8; 32]) -> CtOption<Self> {
        pallas::Base::from_repr(*bytes).and_then(NonZeroPallasBase::from_base)
    }

    pub fn to_bytes(self) -> [u8; 32] {
        self.0.to_repr()
    }

    pub fn from_base(b: pallas::Base) -> CtOption<Self> {
        CtOption::new(NonZeroPallasBase(b), !b.is_zero())
    }

    /// Constructs a wrapper for a base field element that is guaranteed to be non-zero.
    ///
    /// # Panics
    ///
    /// Panics if `s.is_zero()`.
    fn guaranteed(s: pallas::Base) -> Self {
        assert!(!bool::from(s.is_zero()));
        NonZeroPallasBase(s)
    }
}

/// An integer in [1..r_P].
#[derive(Clone, Copy, Debug, PartialEq, Eq, PartialOrd, Ord)]
pub struct NonZeroPallasScalar(pallas::Scalar);

impl Default for NonZeroPallasScalar {
    fn default() -> Self {
        NonZeroPallasScalar(pallas::Scalar::one())
    }
}

impl From<NonZeroPallasBase> for NonZeroPallasScalar {
    fn from(s: NonZeroPallasBase) -> Self {
        NonZeroPallasScalar::guaranteed(mod_r_p(s.0))
    }
}

impl ConditionallySelectable for NonZeroPallasScalar {
    fn conditional_select(a: &Self, b: &Self, choice: subtle::Choice) -> Self {
        NonZeroPallasScalar(pallas::Scalar::conditional_select(&a.0, &b.0, choice))
    }
}

impl NonZeroPallasScalar {
    pub fn from_bytes(bytes: &[u8; 32]) -> CtOption<Self> {
        pallas::Scalar::from_repr(*bytes).and_then(NonZeroPallasScalar::from_scalar)
    }

    pub fn from_scalar(s: pallas::Scalar) -> CtOption<Self> {
        CtOption::new(NonZeroPallasScalar(s), !s.is_zero())
    }

    /// Constructs a wrapper for a scalar field element that is guaranteed to be non-zero.
    ///
    /// # Panics
    ///
    /// Panics if `s.is_zero()`.
    fn guaranteed(s: pallas::Scalar) -> Self {
        assert!(!bool::from(s.is_zero()));
        NonZeroPallasScalar(s)
    }
}

impl Deref for NonZeroPallasScalar {
    type Target = pallas::Scalar;

    fn deref(&self) -> &pallas::Scalar {
        &self.0
    }
}

const PREPARED_WINDOW_SIZE: usize = 4;

#[derive(Clone, Debug)]
pub struct PreparedNonIdentityBase(WnafBase<pallas::Point, PREPARED_WINDOW_SIZE>);

impl PreparedNonIdentityBase {
    pub fn new(base: NonIdentityPallasPoint) -> Self {
        PreparedNonIdentityBase(WnafBase::new(base.0))
    }
}

#[derive(Clone, Debug)]
pub struct PreparedNonZeroScalar(WnafScalar<pallas::Scalar, PREPARED_WINDOW_SIZE>);

impl PreparedNonZeroScalar {
    pub fn new(scalar: &NonZeroPallasScalar) -> Self {
        PreparedNonZeroScalar(WnafScalar::new(scalar))
    }
}

/// $\mathsf{ToBase}^\mathsf{Orchard}(x) := LEOS2IP_{\ell_\mathsf{PRFexpand}}(x) (mod q_P)$
///
/// Defined in [Zcash Protocol Spec § 4.2.3: Orchard Key Components][orchardkeycomponents].
///
/// [orchardkeycomponents]: https://zips.z.cash/protocol/nu5.pdf#orchardkeycomponents
pub fn to_base(x: [u8; 64]) -> pallas::Base {
    pallas::Base::from_uniform_bytes(&x)
}

/// $\mathsf{ToScalar}^\mathsf{Orchard}(x) := LEOS2IP_{\ell_\mathsf{PRFexpand}}(x) (mod r_P)$
///
/// Defined in [Zcash Protocol Spec § 4.2.3: Orchard Key Components][orchardkeycomponents].
///
/// [orchardkeycomponents]: https://zips.z.cash/protocol/nu5.pdf#orchardkeycomponents
pub fn to_scalar(x: [u8; 64]) -> pallas::Scalar {
    pallas::Scalar::from_uniform_bytes(&x)
}

/// Converts from pallas::Base to pallas::Scalar (aka $x \pmod{r_\mathbb{P}}$).
///
/// This requires no modular reduction because Pallas' base field is smaller than its
/// scalar field.
pub fn mod_r_p(x: pallas::Base) -> pallas::Scalar {
    pallas::Scalar::from_repr(x.to_repr()).unwrap()
}

/// Defined in [Zcash Protocol Spec § 5.4.8.4: Sinsemilla commitments][concretesinsemillacommit].
///
/// [concretesinsemillacommit]: https://zips.z.cash/protocol/protocol.pdf#concretesinsemillacommit
pub fn commit_ivk(
    ak: &pallas::Base,
    nk: &pallas::Base,
    rivk: &pallas::Scalar,
) -> CtOption<pallas::Base> {
    // We rely on the API contract that to_le_bits() returns at least PrimeField::NUM_BITS
    // bits, which is equal to L_ORCHARD_BASE.
    let domain = sinsemilla::CommitDomain::new(COMMIT_IVK_PERSONALIZATION);
    domain.short_commit(
        iter::empty()
            .chain(ak.to_le_bits().iter().by_vals().take(L_ORCHARD_BASE))
            .chain(nk.to_le_bits().iter().by_vals().take(L_ORCHARD_BASE)),
        rivk,
    )
}

/// Defined in [Zcash Protocol Spec § 5.4.1.6: DiversifyHash^Sapling and DiversifyHash^Orchard Hash Functions][concretediversifyhash].
///
/// [concretediversifyhash]: https://zips.z.cash/protocol/nu5.pdf#concretediversifyhash
pub fn diversify_hash(d: &[u8; 11]) -> NonIdentityPallasPoint {
    let hasher = pallas::Point::hash_to_curve(KEY_DIVERSIFICATION_PERSONALIZATION);
    let g_d = hasher(d);
    // If the identity occurs, we replace it with a different fixed point.
    // TODO: Replace the unwrap_or_else with a cached fixed point.
    NonIdentityPallasPoint(CtOption::new(g_d, !g_d.is_identity()).unwrap_or_else(|| hasher(&[])))
}

/// Defined in [Zcash Protocol Spec § 5.4.5.5: Orchard Key Agreement][concreteorchardkeyagreement].
///
/// [concreteorchardkeyagreement]: https://zips.z.cash/protocol/nu5.pdf#concreteorchardkeyagreement
pub fn ka_orchard(
    sk: &NonZeroPallasScalar,
    b: &NonIdentityPallasPoint,
) -> NonIdentityPallasPoint {
    ka_orchard_prepared(
        &PreparedNonZeroScalar::new(sk),
        &PreparedNonIdentityBase::new(*b),
    )
}

/// Defined in [Zcash Protocol Spec § 5.4.5.5: Orchard Key Agreement][concreteorchardkeyagreement].
///
/// [concreteorchardkeyagreement]: https://zips.z.cash/protocol/nu5.pdf#concreteorchardkeyagreement
pub fn ka_orchard_prepared(
    sk: &PreparedNonZeroScalar,
    b: &PreparedNonIdentityBase,
) -> NonIdentityPallasPoint {
    NonIdentityPallasPoint(&b.0 * &sk.0)
}

/// Coordinate extractor for Pallas.
///
/// Defined in [Zcash Protocol Spec § 5.4.9.7: Coordinate Extractor for Pallas][concreteextractorpallas].
///
/// [concreteextractorpallas]: https://zips.z.cash/protocol/nu5.pdf#concreteextractorpallas
pub fn extract_p(point: &pallas::Point) -> pallas::Base {
    point
        .to_affine()
        .coordinates()
        .map(|c| *c.x())
        .unwrap_or_else(pallas::Base::zero)
}

/// Coordinate extractor for Pallas.
///
/// Defined in [Zcash Protocol Spec § 5.4.9.7: Coordinate Extractor for Pallas][concreteextractorpallas].
///
/// [concreteextractorpallas]: https://zips.z.cash/protocol/nu5.pdf#concreteextractorpallas
pub fn extract_p_bottom(point: CtOption<pallas::Point>) -> CtOption<pallas::Base> {
    point.map(|p| extract_p(&p))
}