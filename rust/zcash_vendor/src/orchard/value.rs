use group::Group;
use pasta_curves::pallas;
use group::GroupEncoding;
use subtle::CtOption;

/// A commitment to a [`ValueSum`].
#[derive(Clone, Debug)]
pub struct ValueCommitment(pallas::Point);

impl ValueCommitment {
    /// Derives a `ValueCommitment` by $\mathsf{ValueCommit^{Orchard}}$.
    ///
    /// Defined in [Zcash Protocol Spec § 5.4.8.3: Homomorphic Pedersen commitments (Sapling and Orchard)][concretehomomorphiccommit].
    ///
    /// [concretehomomorphiccommit]: https://zips.z.cash/protocol/nu5.pdf#concretehomomorphiccommit
    // #[allow(non_snake_case)]
    // pub fn derive(value: ValueSum, rcv: ValueCommitTrapdoor) -> Self {
    //     let hasher = pallas::Point::hash_to_curve(VALUE_COMMITMENT_PERSONALIZATION);
    //     let V = hasher(&VALUE_COMMITMENT_V_BYTES);
    //     let R = hasher(&VALUE_COMMITMENT_R_BYTES);
    //     let abs_value = u64::try_from(value.0.abs()).expect("value must be in valid range");

    //     let value = if value.0.is_negative() {
    //         -pallas::Scalar::from(abs_value)
    //     } else {
    //         pallas::Scalar::from(abs_value)
    //     };

    //     ValueCommitment(V * value + R * rcv.0)
    // }

    // pub(crate) fn into_bvk(self) -> redpallas::VerificationKey<Binding> {
    //     // TODO: impl From<pallas::Point> for redpallas::VerificationKey.
    //     self.0.to_bytes().try_into().unwrap()
    // }

    /// Deserialize a value commitment from its byte representation
    pub fn from_bytes(bytes: &[u8; 32]) -> CtOption<ValueCommitment> {
        pallas::Point::from_bytes(bytes).map(ValueCommitment)
    }

    /// Serialize this value commitment to its canonical byte representation.
    pub fn to_bytes(&self) -> [u8; 32] {
        self.0.to_bytes()
    }

    // /// x-coordinate of this value commitment.
    // pub(crate) fn x(&self) -> pallas::Base {
    //     if self.0 == pallas::Point::identity() {
    //         pallas::Base::zero()
    //     } else {
    //         *self.0.to_affine().coordinates().unwrap().x()
    //     }
    // }

    // /// y-coordinate of this value commitment.
    // pub(crate) fn y(&self) -> pallas::Base {
    //     if self.0 == pallas::Point::identity() {
    //         pallas::Base::zero()
    //     } else {
    //         *self.0.to_affine().coordinates().unwrap().y()
    //     }
    // }
}