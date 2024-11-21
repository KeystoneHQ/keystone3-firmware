//! A minimal RedPallas implementation for use in Zcash.

use core::cmp::{Ord, Ordering, PartialOrd};

use pasta_curves::pallas;
use rand_chacha::rand_core::{CryptoRng, RngCore};

pub use reddsa::batch;


/// A RedPallas signature type.
pub trait SigType: reddsa::SigType + private::Sealed {}

/// A type variable corresponding to an Orchard spend authorization signature.
pub type SpendAuth = reddsa::orchard::SpendAuth;
impl SigType for SpendAuth {}

/// A type variable corresponding to an Orchard binding signature.
pub type Binding = reddsa::orchard::Binding;
impl SigType for Binding {}

/// A RedPallas signing key.
#[derive(Clone, Debug)]
pub struct SigningKey<T: SigType>(reddsa::SigningKey<T>);

impl<T: SigType> From<SigningKey<T>> for [u8; 32] {
    fn from(sk: SigningKey<T>) -> [u8; 32] {
        sk.0.into()
    }
}

impl<T: SigType> From<&SigningKey<T>> for [u8; 32] {
    fn from(sk: &SigningKey<T>) -> [u8; 32] {
        sk.0.into()
    }
}

impl<T: SigType> TryFrom<[u8; 32]> for SigningKey<T> {
    type Error = reddsa::Error;

    fn try_from(bytes: [u8; 32]) -> Result<Self, Self::Error> {
        bytes.try_into().map(SigningKey)
    }
}

impl SigningKey<SpendAuth> {
    /// Randomizes this signing key with the given `randomizer`.
    ///
    /// Randomization is only supported for `SpendAuth` keys.
    pub fn randomize(&self, randomizer: &pallas::Scalar) -> Self {
        SigningKey(self.0.randomize(randomizer))
    }
}

impl<T: SigType> SigningKey<T> {
    /// Creates a signature of type `T` on `msg` using this `SigningKey`.
    pub fn sign<R: RngCore + CryptoRng>(&self, rng: R, msg: &[u8]) -> Signature<T> {
        Signature(self.0.sign(rng, msg))
    }
}

/// A RedPallas verification key.
#[derive(Clone, Debug)]
pub struct VerificationKey<T: SigType>(reddsa::VerificationKey<T>);

impl<T: SigType> From<VerificationKey<T>> for [u8; 32] {
    fn from(vk: VerificationKey<T>) -> [u8; 32] {
        vk.0.into()
    }
}

impl<T: SigType> From<&VerificationKey<T>> for [u8; 32] {
    fn from(vk: &VerificationKey<T>) -> [u8; 32] {
        vk.0.into()
    }
}

impl<T: SigType> TryFrom<[u8; 32]> for VerificationKey<T> {
    type Error = reddsa::Error;

    fn try_from(bytes: [u8; 32]) -> Result<Self, Self::Error> {
        bytes.try_into().map(VerificationKey)
    }
}

impl<'a, T: SigType> From<&'a SigningKey<T>> for VerificationKey<T> {
    fn from(sk: &'a SigningKey<T>) -> VerificationKey<T> {
        VerificationKey((&sk.0).into())
    }
}

impl<T: SigType> PartialEq for VerificationKey<T> {
    fn eq(&self, other: &Self) -> bool {
        <[u8; 32]>::from(self).eq(&<[u8; 32]>::from(other))
    }
}

impl<T: SigType> Eq for VerificationKey<T> {}

impl<T: SigType> PartialOrd for VerificationKey<T> {
    fn partial_cmp(&self, other: &Self) -> Option<Ordering> {
        <[u8; 32]>::from(self).partial_cmp(&<[u8; 32]>::from(other))
    }
}

impl<T: SigType> Ord for VerificationKey<T> {
    fn cmp(&self, other: &Self) -> Ordering {
        <[u8; 32]>::from(self).cmp(&<[u8; 32]>::from(other))
    }
}

impl VerificationKey<SpendAuth> {
    /// Randomizes this verification key with the given `randomizer`.
    ///
    /// Randomization is only supported for `SpendAuth` keys.
    pub fn randomize(&self, randomizer: &pallas::Scalar) -> Self {
        VerificationKey(self.0.randomize(randomizer))
    }

    /// Creates a batch validation item from a `SpendAuth` signature.
    pub fn create_batch_item<M: AsRef<[u8]>>(
        &self,
        sig: Signature<SpendAuth>,
        msg: &M,
    ) -> batch::Item<SpendAuth, Binding> {
        batch::Item::from_spendauth(self.0.into(), sig.0, msg)
    }
}

impl VerificationKey<Binding> {
    /// Creates a batch validation item from a `Binding` signature.
    pub fn create_batch_item<M: AsRef<[u8]>>(
        &self,
        sig: Signature<Binding>,
        msg: &M,
    ) -> batch::Item<SpendAuth, Binding> {
        batch::Item::from_binding(self.0.into(), sig.0, msg)
    }
}

impl<T: SigType> VerificationKey<T> {
    /// Verifies a purported `signature` over `msg` made by this verification key.
    pub fn verify(&self, msg: &[u8], signature: &Signature<T>) -> Result<(), reddsa::Error> {
        self.0.verify(msg, &signature.0)
    }
}

/// A RedPallas signature.
#[derive(Debug, Clone)]
pub struct Signature<T: SigType>(reddsa::Signature<T>);

impl<T: SigType> From<[u8; 64]> for Signature<T> {
    fn from(bytes: [u8; 64]) -> Self {
        Signature(bytes.into())
    }
}

impl<T: SigType> From<&Signature<T>> for [u8; 64] {
    fn from(sig: &Signature<T>) -> Self {
        sig.0.into()
    }
}

pub(crate) mod private {
    use super::{Binding, SpendAuth};

    pub trait Sealed {}

    impl Sealed for SpendAuth {}

    impl Sealed for Binding {}
}

