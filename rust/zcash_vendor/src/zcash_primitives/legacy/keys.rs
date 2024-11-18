//! Transparent key components.

use alloc::string::ToString;
use alloc::vec::Vec;
use bip32::{
    self, ChildNumber, ExtendedKey, ExtendedKeyAttrs, ExtendedPrivateKey, ExtendedPublicKey, Prefix,
};
use sha2::Sha256;
use sha2::Digest;
use subtle::{Choice, ConstantTimeEq};
use secp256k1::{self, PublicKey};

use crate::orchard::prf_expand::PrfExpand;
use crate::zcash_protocol::consensus::{self, NetworkConstants};
use crate::zip32::{self, AccountId};

use super::TransparentAddress;

/// The scope of a transparent key.
///
/// This type can represent [`zip32`] internal and external scopes, as well as custom scopes that
/// may be used in non-hardened derivation at the `change` level of the BIP 44 key path.
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub struct TransparentKeyScope(u32);

impl TransparentKeyScope {
    /// Returns an arbitrary custom `TransparentKeyScope`.
    ///
    /// This should be used with care: funds associated with keys derived under a custom
    /// scope may not be recoverable if the wallet seed is restored in another wallet. It
    /// is usually preferable to use standardized key scopes.
    pub const fn custom(i: u32) -> Option<Self> {
        if i < (1 << 31) {
            Some(TransparentKeyScope(i))
        } else {
            None
        }
    }

    /// The scope used to derive keys for external transparent addresses,
    /// intended to be used to send funds to this wallet.
    pub const EXTERNAL: Self = TransparentKeyScope(0);

    /// The scope used to derive keys for internal wallet operations, e.g.
    /// change or UTXO management.
    pub const INTERNAL: Self = TransparentKeyScope(1);

    /// The scope used to derive keys for ephemeral transparent addresses.
    pub const EPHEMERAL: Self = TransparentKeyScope(2);
}

impl From<zip32::Scope> for TransparentKeyScope {
    fn from(value: zip32::Scope) -> Self {
        match value {
            zip32::Scope::External => TransparentKeyScope::EXTERNAL,
            zip32::Scope::Internal => TransparentKeyScope::INTERNAL,
        }
    }
}

impl From<TransparentKeyScope> for ChildNumber {
    fn from(value: TransparentKeyScope) -> Self {
        ChildNumber::new(value.0, false).expect("TransparentKeyScope is correct by construction")
    }
}

/// A child index for a derived transparent address.
///
/// Only NON-hardened derivation is supported.
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub struct NonHardenedChildIndex(u32);

impl ConstantTimeEq for NonHardenedChildIndex {
    fn ct_eq(&self, other: &Self) -> Choice {
        self.0.ct_eq(&other.0)
    }
}

impl NonHardenedChildIndex {
    pub const ZERO: NonHardenedChildIndex = NonHardenedChildIndex(0);

    /// Parses the given ZIP 32 child index.
    ///
    /// Returns `None` if the hardened bit is set.
    pub fn from_index(i: u32) -> Option<Self> {
        if i < (1 << 31) {
            Some(NonHardenedChildIndex(i))
        } else {
            None
        }
    }

    /// Returns the index as a 32-bit integer.
    pub fn index(&self) -> u32 {
        self.0
    }

    pub fn next(&self) -> Option<Self> {
        // overflow cannot happen because self.0 is 31 bits, and the next index is at most 32 bits
        // which in that case would lead from_index to return None.
        Self::from_index(self.0 + 1)
    }
}

impl TryFrom<ChildNumber> for NonHardenedChildIndex {
    type Error = ();

    fn try_from(value: ChildNumber) -> Result<Self, Self::Error> {
        if value.is_hardened() {
            Err(())
        } else {
            NonHardenedChildIndex::from_index(value.index()).ok_or(())
        }
    }
}

impl From<NonHardenedChildIndex> for ChildNumber {
    fn from(value: NonHardenedChildIndex) -> Self {
        Self::new(value.index(), false).expect("NonHardenedChildIndex is correct by construction")
    }
}

/// A [BIP44] private key at the account path level `m/44'/<coin_type>'/<account>'`.
///
/// [BIP44]: https://github.com/bitcoin/bips/blob/master/bip-0044.mediawiki
#[derive(Clone, Debug)]
pub struct AccountPrivKey(ExtendedPrivateKey<secp256k1::SecretKey>);

impl AccountPrivKey {
    /// Performs derivation of the extended private key for the BIP44 path:
    /// `m/44'/<coin_type>'/<account>'`.
    ///
    /// This produces the root of the derivation tree for transparent
    /// viewing keys and addresses for the provided account.
    pub fn from_seed<P: consensus::Parameters>(
        params: &P,
        seed: &[u8],
        account: AccountId,
    ) -> Result<AccountPrivKey, bip32::Error> {
        ExtendedPrivateKey::new(seed)?
            .derive_child(ChildNumber::new(44, true)?)?
            .derive_child(ChildNumber::new(params.coin_type(), true)?)?
            .derive_child(ChildNumber::new(account.into(), true)?)
            .map(AccountPrivKey)
    }

    pub fn from_extended_privkey(extprivkey: ExtendedPrivateKey<secp256k1::SecretKey>) -> Self {
        AccountPrivKey(extprivkey)
    }

    pub fn to_account_pubkey(&self) -> AccountPubKey {
        AccountPubKey(ExtendedPublicKey::from(&self.0))
    }

    /// Derives the BIP44 private spending key for the child path
    /// `m/44'/<coin_type>'/<account>'/<scope>/<address_index>`.
    pub fn derive_secret_key(
        &self,
        scope: TransparentKeyScope,
        address_index: NonHardenedChildIndex,
    ) -> Result<secp256k1::SecretKey, bip32::Error> {
        self.0
            .derive_child(scope.into())?
            .derive_child(address_index.into())
            .map(|k| *k.private_key())
    }

    /// Derives the BIP44 private spending key for the external (incoming payment) child path
    /// `m/44'/<coin_type>'/<account>'/0/<address_index>`.
    pub fn derive_external_secret_key(
        &self,
        address_index: NonHardenedChildIndex,
    ) -> Result<secp256k1::SecretKey, bip32::Error> {
        self.derive_secret_key(zip32::Scope::External.into(), address_index)
    }

    /// Derives the BIP44 private spending key for the internal (change) child path
    /// `m/44'/<coin_type>'/<account>'/1/<address_index>`.
    pub fn derive_internal_secret_key(
        &self,
        address_index: NonHardenedChildIndex,
    ) -> Result<secp256k1::SecretKey, bip32::Error> {
        self.derive_secret_key(zip32::Scope::Internal.into(), address_index)
    }

    /// Returns the `AccountPrivKey` serialized using the encoding for a
    /// [BIP 32](https://en.bitcoin.it/wiki/BIP_0032) ExtendedPrivateKey, excluding the
    /// 4 prefix bytes.
    pub fn to_bytes(&self) -> Vec<u8> {
        // Convert to `xprv` encoding.
        let xprv_encoded = self.0.to_extended_key(Prefix::XPRV).to_string();

        // Now decode it and return the bytes we want.
        bs58::decode(xprv_encoded)
            .with_check(None)
            .into_vec()
            .expect("correct")
            .split_off(Prefix::LENGTH)
    }

    /// Decodes the `AccountPrivKey` from the encoding specified for a
    /// [BIP 32](https://en.bitcoin.it/wiki/BIP_0032) ExtendedPrivateKey, excluding the
    /// 4 prefix bytes.
    pub fn from_bytes(b: &[u8]) -> Option<Self> {
        // Convert to `xprv` encoding.
        let mut bytes = Prefix::XPRV.to_bytes().to_vec();
        bytes.extend_from_slice(b);
        let xprv_encoded = bs58::encode(bytes).with_check().into_string();

        // Now we can parse it.
        xprv_encoded
            .parse::<ExtendedKey>()
            .ok()
            .and_then(|k| ExtendedPrivateKey::try_from(k).ok())
            .map(AccountPrivKey::from_extended_privkey)
    }
}

/// A [BIP44] public key at the account path level `m/44'/<coin_type>'/<account>'`.
///
/// This provides the necessary derivation capability for the transparent component of a unified
/// full viewing key.
///
/// [BIP44]: https://github.com/bitcoin/bips/blob/master/bip-0044.mediawiki
#[derive(Clone, Debug)]
pub struct AccountPubKey(ExtendedPublicKey<PublicKey>);

impl AccountPubKey {
    /// Derives the BIP44 public key at the external "change level" path
    /// `m/44'/<coin_type>'/<account>'/0`.
    pub fn derive_external_ivk(&self) -> Result<ExternalIvk, bip32::Error> {
        self.0
            .derive_child(ChildNumber::new(0, false)?)
            .map(ExternalIvk)
    }

    pub fn to_inner(&self) -> ExtendedPublicKey<PublicKey> {
        self.0.clone()
    }

    /// Derives the BIP44 public key at the internal "change level" path
    /// `m/44'/<coin_type>'/<account>'/1`.
    pub fn derive_internal_ivk(&self) -> Result<InternalIvk, bip32::Error> {
        self.0
            .derive_child(ChildNumber::new(1, false)?)
            .map(InternalIvk)
    }

    /// Derives the public key at the "ephemeral" path
    /// `m/44'/<coin_type>'/<account>'/2`.
    pub fn derive_ephemeral_ivk(&self) -> Result<EphemeralIvk, bip32::Error> {
        self.0
            .derive_child(ChildNumber::new(2, false)?)
            .map(EphemeralIvk)
    }

    /// Derives the internal ovk and external ovk corresponding to this
    /// transparent fvk. As specified in [ZIP 316][transparent-ovk].
    ///
    /// [transparent-ovk]: https://zips.z.cash/zip-0316#deriving-internal-keys
    pub fn ovks_for_shielding(&self) -> (InternalOvk, ExternalOvk) {
        let i_ovk = PrfExpand::TRANSPARENT_ZIP316_OVK
            .with(&self.0.attrs().chain_code, &self.0.public_key().serialize());
        let ovk_external = ExternalOvk(i_ovk[..32].try_into().unwrap());
        let ovk_internal = InternalOvk(i_ovk[32..].try_into().unwrap());

        (ovk_internal, ovk_external)
    }

    /// Derives the internal ovk corresponding to this transparent fvk.
    pub fn internal_ovk(&self) -> InternalOvk {
        self.ovks_for_shielding().0
    }

    /// Derives the external ovk corresponding to this transparent fvk.
    pub fn external_ovk(&self) -> ExternalOvk {
        self.ovks_for_shielding().1
    }

    pub fn serialize(&self) -> Vec<u8> {
        let mut buf = self.0.attrs().chain_code.to_vec();
        buf.extend_from_slice(&self.0.public_key().serialize());
        buf
    }

    pub fn deserialize(data: &[u8; 65]) -> Result<Self, bip32::Error> {
        let chain_code = data[..32].try_into().expect("correct length");
        let public_key = PublicKey::from_slice(&data[32..])?;
        Ok(AccountPubKey(ExtendedPublicKey::new(
            public_key,
            ExtendedKeyAttrs {
                depth: 3,
                // We do not expose the inner `ExtendedPublicKey`, so we can use dummy
                // values for the fields that are not encoded in an `AccountPubKey`.
                parent_fingerprint: [0xff, 0xff, 0xff, 0xff],
                child_number: ChildNumber::new(0, true).expect("correct"),
                chain_code,
            },
        )))
    }
}

/// Derives the P2PKH transparent address corresponding to the given pubkey.
#[deprecated(note = "This function will be removed from the public API in an upcoming refactor.")]
pub fn pubkey_to_address(pubkey: &secp256k1::PublicKey) -> TransparentAddress {
    TransparentAddress::PublicKeyHash(
        *ripemd::Ripemd160::digest(Sha256::digest(pubkey.serialize())).as_ref(),
    )
}

pub mod private {
    use super::TransparentKeyScope;
    use bip32::ExtendedPublicKey;
    use secp256k1::PublicKey;
    pub trait SealedChangeLevelKey {
        const SCOPE: TransparentKeyScope;
        fn extended_pubkey(&self) -> &ExtendedPublicKey<PublicKey>;
        fn from_extended_pubkey(key: ExtendedPublicKey<PublicKey>) -> Self;
    }
}

/// Trait representing a transparent "incoming viewing key".
///
/// Unlike the Sapling and Orchard shielded protocols (which have viewing keys built into
/// their key trees and bound to specific spending keys), the transparent protocol has no
/// "viewing key" concept. Transparent viewing keys are instead emulated by making two
/// observations:
///
/// - [BIP32] hierarchical derivation is structured as a tree.
/// - The [BIP44] key paths use non-hardened derivation below the account level.
///
/// A transparent viewing key for an account is thus defined as the root of a specific
/// non-hardened subtree underneath the account's path.
///
/// [BIP32]: https://github.com/bitcoin/bips/blob/master/bip-0032.mediawiki
/// [BIP44]: https://github.com/bitcoin/bips/blob/master/bip-0044.mediawiki
pub trait IncomingViewingKey: private::SealedChangeLevelKey + core::marker::Sized {
    /// Derives a transparent address at the provided child index.
    #[allow(deprecated)]
    fn derive_address(
        &self,
        address_index: NonHardenedChildIndex,
    ) -> Result<TransparentAddress, bip32::Error> {
        let child_key = self.extended_pubkey().derive_child(address_index.into())?;
        Ok(pubkey_to_address(child_key.public_key()))
    }

    /// Searches the space of child indexes for an index that will
    /// generate a valid transparent address, and returns the resulting
    /// address and the index at which it was generated.
    fn default_address(&self) -> (TransparentAddress, NonHardenedChildIndex) {
        let mut address_index = NonHardenedChildIndex::ZERO;
        loop {
            match self.derive_address(address_index) {
                Ok(addr) => {
                    return (addr, address_index);
                }
                Err(_) => {
                    address_index = address_index.next().unwrap_or_else(|| {
                        panic!("Exhausted child index space attempting to find a default address.");
                    });
                }
            }
        }
    }

    fn serialize(&self) -> Vec<u8> {
        let extpubkey = self.extended_pubkey();
        let mut buf = extpubkey.attrs().chain_code.to_vec();
        buf.extend_from_slice(&extpubkey.public_key().serialize());
        buf
    }

    fn deserialize(data: &[u8; 65]) -> Result<Self, bip32::Error> {
        let chain_code = data[..32].try_into().expect("correct length");
        let public_key = PublicKey::from_slice(&data[32..])?;
        Ok(Self::from_extended_pubkey(ExtendedPublicKey::new(
            public_key,
            ExtendedKeyAttrs {
                depth: 4,
                // We do not expose the inner `ExtendedPublicKey`, so we can use a dummy
                // value for the `parent_fingerprint` that is not encoded in an
                // `IncomingViewingKey`.
                parent_fingerprint: [0xff, 0xff, 0xff, 0xff],
                child_number: Self::SCOPE.into(),
                chain_code,
            },
        )))
    }
}

/// An incoming viewing key at the [BIP44] "external" path
/// `m/44'/<coin_type>'/<account>'/0`.
///
/// This allows derivation of child addresses that may be provided to external parties.
///
/// [BIP44]: https://github.com/bitcoin/bips/blob/master/bip-0044.mediawiki
#[derive(Clone, Debug)]
pub struct ExternalIvk(ExtendedPublicKey<PublicKey>);

impl private::SealedChangeLevelKey for ExternalIvk {
    const SCOPE: TransparentKeyScope = TransparentKeyScope(0);

    fn extended_pubkey(&self) -> &ExtendedPublicKey<PublicKey> {
        &self.0
    }

    fn from_extended_pubkey(key: ExtendedPublicKey<PublicKey>) -> Self {
        ExternalIvk(key)
    }
}

impl IncomingViewingKey for ExternalIvk {}

/// An incoming viewing key at the [BIP44] "internal" path
/// `m/44'/<coin_type>'/<account>'/1`.
///
/// This allows derivation of change addresses for use within the wallet, but which should
/// not be shared with external parties.
///
/// [BIP44]: https://github.com/bitcoin/bips/blob/master/bip-0044.mediawiki
#[derive(Clone, Debug)]
pub struct InternalIvk(ExtendedPublicKey<PublicKey>);

impl private::SealedChangeLevelKey for InternalIvk {
    const SCOPE: TransparentKeyScope = TransparentKeyScope(1);

    fn extended_pubkey(&self) -> &ExtendedPublicKey<PublicKey> {
        &self.0
    }

    fn from_extended_pubkey(key: ExtendedPublicKey<PublicKey>) -> Self {
        InternalIvk(key)
    }
}

impl IncomingViewingKey for InternalIvk {}

/// An incoming viewing key at the "ephemeral" path
/// `m/44'/<coin_type>'/<account>'/2`.
///
/// This allows derivation of ephemeral addresses for use within the wallet.
#[derive(Clone, Debug)]
pub struct EphemeralIvk(ExtendedPublicKey<PublicKey>);

#[cfg(feature = "transparent-inputs")]
impl EphemeralIvk {
    /// Derives a transparent address at the provided child index.
    pub fn derive_ephemeral_address(
        &self,
        address_index: NonHardenedChildIndex,
    ) -> Result<TransparentAddress, bip32::Error> {
        let child_key = self.0.derive_child(address_index.into())?;
        #[allow(deprecated)]
        Ok(pubkey_to_address(child_key.public_key()))
    }
}

/// Internal outgoing viewing key used for autoshielding.
pub struct InternalOvk([u8; 32]);

impl InternalOvk {
    pub fn as_bytes(&self) -> [u8; 32] {
        self.0
    }
}

/// External outgoing viewing key used by `zcashd` for transparent-to-shielded spends to
/// external receivers.
pub struct ExternalOvk([u8; 32]);

impl ExternalOvk {
    pub fn as_bytes(&self) -> [u8; 32] {
        self.0
    }
}
