use alloc::{
    format,
    string::{String, ToString},
    vec::Vec,
};

use bitcoin::secp256k1;
use keystore::algorithms::{
    secp256k1::{get_private_key_by_seed, get_public_key_by_seed},
    zcash::calculate_seed_fingerprint,
};
use zcash_vendor::{
    pczt::{
        roles::{
            low_level_signer,
            redactor::{orchard::OrchardRedactor, Redactor},
            updater::Updater,
        },
        Pczt,
    },
    transparent,
};

#[cfg(feature = "cypherpunk")]
use zcash_vendor::orchard;

#[cfg(any(feature = "cypherpunk", feature = "multi_coins"))]
use zcash_vendor::{
    pczt_ext::{self, PcztSigner},
    transparent::sighash::SignableInput,
};

#[cfg(feature = "cypherpunk")]
use {
    blake2b_simd::Hash,
    core::{
        cell::{Cell, RefCell},
        mem::MaybeUninit,
    },
    rand_core::OsRng,
    zeroize::Zeroize,
};

use crate::{errors::ZcashError, version::KEYSTONE_FW_VERSION};

/// `global.proprietary` key stamped into every signed PCZT response.
/// Value is 3 bytes `[major, minor, build]`. Wallets read this to check
/// whether the device meets their minimum version requirements.
const PROP_KEY_FW_VERSION: &str = "keystone:fw_version";

#[cfg(all(feature = "multi_coins", not(feature = "cypherpunk")))]
struct SeedSigner<'a> {
    seed: &'a [u8],
    coin_type: u32,
}

#[cfg(all(feature = "multi_coins", not(feature = "cypherpunk")))]
impl PcztSigner for SeedSigner<'_> {
    type Error = ZcashError;

    fn sign_transparent<F>(
        &self,
        index: usize,
        input: &mut transparent::pczt::Input,
        hash: F,
    ) -> Result<(), Self::Error>
    where
        F: FnOnce(SignableInput) -> [u8; 32],
    {
        if let Some(path) = transparent_key_path_for_input(self.seed, input, self.coin_type)? {
            let sk = get_private_key_by_seed(self.seed, &path).map_err(|e| {
                ZcashError::SigningError(format!("failed to get private key: {e:?}"))
            })?;
            let secp = secp256k1::Secp256k1::new();
            input
                .sign(index, hash, &sk, &secp)
                .map_err(|e| ZcashError::SigningError(format!("failed to sign input: {e:?}")))?;
        }

        Ok(())
    }
}

/// Signs the PCZT's transparent inputs with keys derived for `network`, which
/// the caller must decode from the same PCZT's bytes (see
/// [`super::PcztNetwork::from_pczt_bytes`]).
#[cfg(not(feature = "cypherpunk"))]
pub fn sign_pczt(pczt: Pczt, seed: &[u8], network: super::PcztNetwork) -> crate::Result<Vec<u8>> {
    super::validate_supported_pczt(&pczt)?;
    reject_unsupported_pczt(&pczt)?;

    let signer = low_level_signer::Signer::new(pczt);

    #[cfg(feature = "multi_coins")]
    let signer = pczt_ext::sign_transparent(
        signer,
        &SeedSigner {
            seed,
            coin_type: network.coin_type(),
        },
    )
    .map_err(|e| ZcashError::SigningError(e.to_string()))?;

    #[cfg(not(feature = "multi_coins"))]
    let _ = network;

    stamp_and_redact(signer.finish())
        .serialize()
        .map_err(|e| ZcashError::SigningError(format!("serialize signed PCZT: {e:?}")))
}

#[cfg(not(feature = "cypherpunk"))]
fn reject_unsupported_pczt(pczt: &Pczt) -> Result<(), ZcashError> {
    {
        // This path only signs transparent data. Reject shielded content and unknown
        // transaction formats; the shared sighash helper handles transparent-only v6.
        if super::pczt_is_unsupported_by_transparent_only(pczt) {
            return Err(ZcashError::SigningError(
                "PCZT is not supported by transparent-only signing".to_string(),
            ));
        }
    }
    Ok(())
}

/// One cached spend authorizing key slot, scrubbed on replacement and drop.
///
/// `SpendAuthorizingKey` bottoms out in reddsa's `Copy` `SigningKey`, which
/// holds the secret ask scalar and has no `Zeroize` or `Drop` of its own, so a
/// plainly-dropped cache would leave the scalar bytes behind in memory.
#[cfg(feature = "cypherpunk")]
struct AskCacheSlot {
    identity: Option<(u32, zcash_vendor::zip32::AccountId)>,
    // `MaybeUninit` makes every post-scrub bit pattern valid without depending
    // on private Orchard/reddsa layout invariants. The slot invariant is that
    // this field is initialized exactly when `identity` is `Some`.
    ask: MaybeUninit<orchard::keys::SpendAuthorizingKey>,
}

// `MaybeUninit` deliberately suppresses the wrapped value's destructor. Fail
// the build if a dependency bump gives either secret type drop glue; any change
// to an indirect or owning representation requires a fresh zeroization audit
// rather than silently retaining this type-specific strategy.
#[cfg(feature = "cypherpunk")]
const _: () = assert!(!core::mem::needs_drop::<orchard::keys::SpendAuthorizingKey>());
#[cfg(feature = "cypherpunk")]
const _: () = assert!(!core::mem::needs_drop::<orchard::keys::SpendingKey>());

#[cfg(feature = "cypherpunk")]
impl AskCacheSlot {
    fn empty() -> Self {
        Self {
            identity: None,
            ask: MaybeUninit::uninit(),
        }
    }

    fn get(
        &self,
        coin_type: u32,
        account: zcash_vendor::zip32::AccountId,
    ) -> Option<&orchard::keys::SpendAuthorizingKey> {
        if self.identity == Some((coin_type, account)) {
            // SAFETY: `identity` is only set to `Some` after `ask` is written.
            Some(unsafe { self.ask.assume_init_ref() })
        } else {
            None
        }
    }

    fn replace(
        &mut self,
        coin_type: u32,
        account: zcash_vendor::zip32::AccountId,
        ask: orchard::keys::SpendAuthorizingKey,
    ) -> &orchard::keys::SpendAuthorizingKey {
        self.identity = None;
        self.ask.zeroize();
        self.ask.write(ask);
        self.identity = Some((coin_type, account));

        // SAFETY: the value was written immediately above, before `identity`
        // was set to `Some`.
        unsafe { self.ask.assume_init_ref() }
    }
}

#[cfg(feature = "cypherpunk")]
impl Drop for AskCacheSlot {
    fn drop(&mut self) {
        self.identity = None;
        self.ask.zeroize();
    }
}

/// One inline, scrubbed spend authorizing key slot shared by every PCZT and
/// pool pass in a signing request.
///
/// Create one cache per request, pass it to each PCZT signed with the same seed,
/// never reuse it with another seed, and let it drop before the seed leaves
/// request scope. A hit reuses the cached key without another ZIP 32 derivation.
/// Changing network or account scrubs the old key before replacement; dropping
/// the cache scrubs the final key.
#[cfg(feature = "cypherpunk")]
pub struct SpendAuthCache(RefCell<AskCacheSlot>);

#[cfg(feature = "cypherpunk")]
impl SpendAuthCache {
    /// Creates an empty request-scoped cache.
    pub fn new() -> Self {
        Self(RefCell::new(AskCacheSlot::empty()))
    }
}

#[cfg(feature = "cypherpunk")]
impl Default for SpendAuthCache {
    fn default() -> Self {
        Self::new()
    }
}

/// Lean signer for the cypherpunk path. Drives the shallow `low_level_signer` and
/// derives keys / signs each action in place, instead of materializing a full
/// `RoleSigner` (which reconstructs the whole transaction to compute the sighash and
/// blows the task stack). The sighash itself is the byte-level
/// `pczt_ext::shielded_sig_commitment`, proven bit-exact against `RoleSigner` by the
/// `test_lean_sighash_*` oracle tests.
#[cfg(feature = "cypherpunk")]
struct SeedSigner<'a> {
    seed: &'a [u8],
    seed_fingerprint: [u8; 32],
    coin_type: u32,
    /// Restricts checked production signing to the account the user reviewed.
    /// The raw `sign_pczt` helper passes `None` to preserve its unscoped API.
    selected_account: Option<zcash_vendor::zip32::AccountId>,
    pool: ShieldedPool,
    /// Borrowed so every PCZT and both pool passes can share one scrubbed
    /// slot. See [`SpendAuthCache`] for the request-scoping contract.
    ask_cache: &'a SpendAuthCache,
    /// Number of authorizations produced, so `sign_pczt` can distinguish "nothing of
    /// ours to sign" (`PcztNoMyInputs`) from a successful signing.
    signed: Cell<usize>,
}

#[cfg(feature = "cypherpunk")]
impl<'a> SeedSigner<'a> {
    fn new(
        seed: &'a [u8],
        seed_fingerprint: [u8; 32],
        coin_type: u32,
        selected_account: Option<zcash_vendor::zip32::AccountId>,
        pool: ShieldedPool,
        ask_cache: &'a SpendAuthCache,
    ) -> Self {
        Self {
            seed,
            seed_fingerprint,
            coin_type,
            selected_account,
            pool,
            ask_cache,
            signed: Cell::new(0),
        }
    }

    /// Derives the spend authorizing key for `account_index` from the seed,
    /// scrubbing the intermediate spending key before returning.
    fn derive_spend_authorizing_key(
        &self,
        account_index: zcash_vendor::zip32::AccountId,
    ) -> Result<orchard::keys::SpendAuthorizingKey, ZcashError> {
        let mut osk = MaybeUninit::new(
            orchard::keys::SpendingKey::from_zip32_seed(self.seed, self.coin_type, account_index)
                .map_err(|e| {
                ZcashError::SigningError(format!(
                    "failed to derive {} spending key: {e:?}",
                    self.pool
                ))
            })?,
        );
        // SAFETY: `osk` was initialized immediately above and is only scrubbed
        // after this borrow ends.
        let ask = orchard::keys::SpendAuthorizingKey::from(unsafe { osk.assume_init_ref() });
        osk.zeroize();
        Ok(ask)
    }

    /// Looks up (or derives and caches) the spend authorizing key for
    /// `account_index` and runs `f` against it in place, so no unscrubbed
    /// copies of the key are handed out. On a network or account change, the
    /// slot scrubs the old key in place before storing the replacement.
    fn with_spend_authorizing_key<R>(
        &self,
        account_index: zcash_vendor::zip32::AccountId,
        f: impl FnOnce(&orchard::keys::SpendAuthorizingKey) -> Result<R, ZcashError>,
    ) -> Result<R, ZcashError> {
        // Mutably borrowed across `f`, which is fine: `f` only signs and never
        // re-enters the cache.
        let mut cache = self.ask_cache.0.borrow_mut();
        if cache.get(self.coin_type, account_index).is_none() {
            let ask = self.derive_spend_authorizing_key(account_index)?;
            cache.replace(self.coin_type, account_index, ask);
        }
        f(cache
            .get(self.coin_type, account_index)
            .expect("cache contains the requested account"))
    }
}

#[cfg(feature = "cypherpunk")]
impl PcztSigner for SeedSigner<'_> {
    type Error = ZcashError;

    fn sign_transparent<F>(
        &self,
        index: usize,
        input: &mut transparent::pczt::Input,
        hash: F,
    ) -> Result<(), Self::Error>
    where
        F: FnOnce(SignableInput) -> [u8; 32],
    {
        if let Some(path) = transparent_key_path_for_input(self.seed, input, self.coin_type)? {
            let sk = get_private_key_by_seed(self.seed, &path).map_err(|e| {
                ZcashError::SigningError(format!("failed to get private key: {e:?}"))
            })?;
            let secp = secp256k1::Secp256k1::new();
            input
                .sign(index, hash, &sk, &secp)
                .map_err(|e| ZcashError::SigningError(format!("failed to sign input: {e:?}")))?;
            self.signed.set(self.signed.get() + 1);
        }
        Ok(())
    }

    fn sign_orchard(
        &self,
        action: &mut orchard::pczt::Action,
        hash: Hash,
    ) -> Result<(), Self::Error> {
        // Strict per-action validation, ported verbatim from the previous
        // collect_orchard_bundle_signing_keys so the lean signer keeps identical
        // skip/reject semantics to the RoleSigner path.
        if action.spend().spend_auth_sig().is_some() {
            return Ok(());
        }
        if action.spend().dummy_sk().is_some() {
            match action.spend().value().map(|value| value.inner()) {
                Some(0) | None => return Ok(()),
                Some(_) => {
                    return Err(ZcashError::InvalidPczt(format!(
                        "{} spend dummy_sk is only valid for dummy spends",
                        self.pool
                    )));
                }
            }
        }
        // Batch transport clears `dummy_sk`, the finalized dummy signature, and
        // `alpha`; the wallet retains that signature for extraction. A zero-value
        // spend with no `alpha` cannot be signed here and authorizes no value, so
        // skip it. Wallet-controlled zero-value spends retain `alpha` and sign.
        if matches!(action.spend().value(), Some(value) if value.inner() == 0)
            && action.spend().alpha().is_none()
        {
            return Ok(());
        }
        if action.spend().value().is_none() {
            return Ok(());
        }
        let Some(account_index) = super::matching_seed_supported_orchard_account(
            &self.seed_fingerprint,
            action.spend().zip32_derivation().as_ref(),
            self.coin_type,
            self.pool,
        )?
        else {
            // Not derivable from this seed; not ours to sign.
            return Ok(());
        };
        if self
            .selected_account
            .is_some_and(|selected_account| selected_account != account_index)
        {
            return Err(ZcashError::PcztNoMyInputs);
        }

        self.with_spend_authorizing_key(account_index, |ask| {
            action
                .sign(
                    hash.as_bytes().try_into().expect("sighash is 32 bytes"),
                    ask,
                    OsRng,
                )
                .map_err(|e| {
                    ZcashError::SigningError(format!("failed to sign {} action: {e:?}", self.pool))
                })
        })?;
        self.signed.set(self.signed.get() + 1);
        Ok(())
    }
}

/// Signs `pczt` and serializes the stamped, redacted response.
///
/// Thin wrapper over `sign_and_redact_pczt`; see it for the full contract.
#[cfg(feature = "cypherpunk")]
pub fn sign_pczt(pczt: Pczt, seed: &[u8], network: super::PcztNetwork) -> crate::Result<Vec<u8>> {
    sign_and_redact_pczt(pczt, seed, network)?
        .serialize()
        .map_err(|e| ZcashError::SigningError(format!("serialize signed PCZT: {e:?}")))
}

/// `sign_pczt`, but returns the stamped, redacted PCZT without serializing it,
/// so callers that still need the parsed value (in-memory post-sign
/// verification) avoid a byte round trip. `network` selects the key-derivation
/// coin type and must be decoded from the same PCZT's bytes (see
/// [`super::PcztNetwork::from_pczt_bytes`]). Derives keys into a fresh
/// [`SpendAuthCache`]; the batch signing path instead reuses a request-scoped
/// cache so PCZTs for the selected account share one derivation.
#[cfg(feature = "cypherpunk")]
pub fn sign_and_redact_pczt(
    pczt: Pczt,
    seed: &[u8],
    network: super::PcztNetwork,
) -> crate::Result<Pczt> {
    sign_and_redact_pczt_with_cache(pczt, seed, network, None, &SpendAuthCache::new())
}

/// [`sign_and_redact_pczt`] with a caller-provided [`SpendAuthCache`]. When
/// `selected_account` is `Some`, every same-seed shielded authorization is
/// restricted to that reviewed account. `None` is reserved for the raw,
/// unscoped [`sign_pczt`] path. The normal batch path derives its selected
/// account key once and reuses it across PCZTs and pools. An account change
/// scrubs and replaces the slot. The cache must not be reused with another seed.
#[cfg(feature = "cypherpunk")]
pub(crate) fn sign_and_redact_pczt_with_cache(
    pczt: Pczt,
    seed: &[u8],
    network: super::PcztNetwork,
    selected_account: Option<zcash_vendor::zip32::AccountId>,
    ask_cache: &SpendAuthCache,
) -> crate::Result<Pczt> {
    super::validate_supported_pczt(&pczt)?;

    let seed_fingerprint =
        calculate_seed_fingerprint(seed).map_err(|e| ZcashError::SigningError(e.to_string()))?;

    let process_ironwood = super::pczt_should_process_ironwood(&pczt);

    // The orchard signer handles both the transparent inputs and the Orchard bundle
    // (the pool only changes error labels for shielded actions). Ironwood gets its own.
    let orchard_signer = SeedSigner::new(
        seed,
        seed_fingerprint,
        network.coin_type(),
        selected_account,
        ShieldedPool::Orchard,
        ask_cache,
    );

    // Propagate signer errors directly so strict path validation remains
    // `InvalidPczt`.
    let signer = low_level_signer::Signer::new(pczt);
    let signer = pczt_ext::sign_transparent(signer, &orchard_signer)?;
    let signer = pczt_ext::sign_orchard(signer, &orchard_signer)?;

    let ironwood_signer = SeedSigner::new(
        seed,
        seed_fingerprint,
        network.coin_type(),
        selected_account,
        ShieldedPool::Ironwood,
        ask_cache,
    );
    let signer = if process_ironwood {
        pczt_ext::sign_ironwood(signer, &ironwood_signer)?
    } else {
        signer
    };

    if orchard_signer.signed.get() + ironwood_signer.signed.get() == 0 {
        return Err(ZcashError::PcztNoMyInputs);
    }

    Ok(stamp_and_redact(signer.finish()))
}

fn stamp_and_redact(pczt: Pczt) -> Pczt {
    // Stamp the firmware version into `global.proprietary` so the wallet can
    // tell exactly which version of Keystone firmware produced this signature.
    // The Redactor below intentionally does not touch `global`, so this value
    // survives the redaction pass into the returned bytes.
    let stamped_pczt = Updater::new(pczt)
        .update_global_with(|mut g| {
            g.set_proprietary(
                PROP_KEY_FW_VERSION.into(),
                KEYSTONE_FW_VERSION.encode().to_vec(),
            );
        })
        .finish();

    // Now that we've created the signature, remove optional fields that the
    // signing response does not need. This keeps the QR round trip small while
    // preserving signatures and global proprietary fields for the wallet.
    let redactor = Redactor::new(stamped_pczt).redact_orchard_with(redact_orchard_bundle);
    let redactor = redactor.redact_ironwood_with(redact_orchard_bundle);

    redactor
        .redact_sapling_with(|mut r| {
            r.redact_spends(|mut sr| {
                sr.clear_zkproof();
                sr.clear_recipient();
                sr.clear_value();
                sr.clear_rcm();
                sr.clear_rseed();
                sr.clear_rcv();
                sr.clear_proof_generation_key();
                sr.clear_witness();
                sr.clear_alpha();
                sr.clear_zip32_derivation();
                sr.clear_dummy_ask();
            });
            r.redact_outputs(|mut or| {
                or.clear_zkproof();
                or.clear_recipient();
                or.clear_value();
                or.clear_rseed();
                or.clear_rcv();
                or.clear_ock();
                or.clear_zip32_derivation();
                or.clear_user_address();
            });
            r.clear_bsk();
        })
        .redact_transparent_with(|mut r| {
            r.redact_inputs(|mut ir| {
                ir.clear_redeem_script();
                ir.clear_bip32_derivation();
                ir.clear_ripemd160_preimages();
                ir.clear_sha256_preimages();
                ir.clear_hash160_preimages();
                ir.clear_hash256_preimages();
            });
            r.redact_outputs(|mut or| {
                or.clear_redeem_script();
                or.clear_bip32_derivation();
                or.clear_user_address();
            });
        })
        .finish()
}

fn redact_orchard_bundle(mut r: OrchardRedactor<'_>) {
    r.redact_actions(|mut ar| {
        ar.clear_spend_recipient();
        ar.clear_spend_value();
        ar.clear_spend_rho();
        ar.clear_spend_rseed();
        ar.clear_spend_fvk();
        ar.clear_spend_witness();
        ar.clear_spend_alpha();
        ar.clear_spend_zip32_derivation();
        ar.clear_spend_dummy_sk();
        ar.clear_output_recipient();
        ar.clear_output_value();
        ar.clear_output_rseed();
        ar.clear_output_ock();
        ar.clear_output_zip32_derivation();
        ar.clear_output_user_address();
        ar.clear_rcv();
    });
    r.clear_zkproof();
    r.clear_bsk();
}

#[cfg(any(
    feature = "cypherpunk",
    all(feature = "multi_coins", not(feature = "cypherpunk"))
))]
fn transparent_key_path_for_input(
    seed: &[u8],
    input: &transparent::pczt::Input,
    expected_coin_type: u32,
) -> Result<Option<String>, ZcashError> {
    let fingerprint =
        calculate_seed_fingerprint(seed).map_err(|e| ZcashError::SigningError(e.to_string()))?;

    for (pubkey, path) in input.bip32_derivation().iter() {
        let path_fingerprint = *path.seed_fingerprint();
        if fingerprint != path_fingerprint {
            continue;
        }

        let names_expected_network = path.derivation_path().get(1).is_some_and(|path_coin_type| {
            path_coin_type.is_hardened() && path_coin_type.index() == expected_coin_type
        });
        if !names_expected_network {
            return Err(ZcashError::NetworkMismatch(
                "transaction network does not match its transparent input key path".to_string(),
            ));
        }

        let path = {
            let mut ret = "m".to_string();
            for i in path.derivation_path().iter() {
                if i.is_hardened() {
                    ret.push_str(&format!("/{}'", i.index()));
                } else {
                    ret.push_str(&format!("/{}", i.index()));
                }
            }
            ret
        };

        match get_public_key_by_seed(seed, &path) {
            Ok(my_pubkey) if my_pubkey.serialize().to_vec().eq(pubkey) => return Ok(Some(path)),
            Err(e) => return Err(ZcashError::SigningError(e.to_string())),
            _ => {}
        }
    }

    Ok(None)
}

#[cfg(feature = "cypherpunk")]
use super::ShieldedPool;

#[cfg(all(test, feature = "cypherpunk"))]
mod tests {
    use super::*;
    // RoleSigner is the upstream reference signer; tests use its sighash as the
    // bit-exact oracle for the lean pczt_ext::shielded_sig_commitment.
    use zcash_vendor::pczt::roles::redactor::Redactor;
    use zcash_vendor::pczt::roles::signer::Signer as RoleSigner;

    fn assert_invalid_pczt_message<T: core::fmt::Debug>(result: crate::Result<T>, expected: &str) {
        match result {
            Err(ZcashError::InvalidPczt(message)) if message == expected => {}
            other => panic!("unexpected InvalidPczt result: {other:?}"),
        }
    }

    use zcash_vendor::{
        pasta_curves::{group::ff::Field, Fq},
        zip32,
    };

    /// Extracts the ask scalar bytes via public API only: randomizing by zero
    /// returns `rsk = ask + 0 = ask`.
    fn ask_scalar_bytes(ask: &orchard::keys::SpendAuthorizingKey) -> [u8; 32] {
        (&ask.randomize(&Fq::ZERO)).into()
    }

    #[test]
    fn test_ask_cache_slot_zeroizes_on_drop() {
        let seed = [7u8; 32];
        let osk = orchard::keys::SpendingKey::from_zip32_seed(&seed, 133, zip32::AccountId::ZERO)
            .unwrap();
        let mut storage = MaybeUninit::<AskCacheSlot>::uninit();
        let slot = storage.write(AskCacheSlot::empty());
        slot.replace(
            133,
            zip32::AccountId::ZERO,
            orchard::keys::SpendAuthorizingKey::from(&osk),
        );
        assert_ne!(
            ask_scalar_bytes(slot.get(133, zip32::AccountId::ZERO).unwrap()),
            [0u8; 32],
            "a real ask must not start out zero"
        );

        // The outer `MaybeUninit` keeps the backing storage alive after the
        // slot itself is dropped.
        unsafe { core::ptr::drop_in_place(storage.as_mut_ptr()) };

        // SAFETY: `Drop` initialized every byte of the still-allocated
        // `MaybeUninit<SpendAuthorizingKey>` storage to zero. `addr_of!`
        // projects the dropped slot without reading it.
        let bytes = unsafe {
            let ask_storage = core::ptr::addr_of!((*storage.as_ptr()).ask);
            core::slice::from_raw_parts(
                ask_storage.cast::<u8>(),
                core::mem::size_of::<MaybeUninit<orchard::keys::SpendAuthorizingKey>>(),
            )
        };
        assert!(
            bytes.iter().all(|b| *b == 0),
            "cached ask storage must be fully scrubbed"
        );
    }

    #[test]
    fn test_ask_cache_shared_single_slot_consistent() {
        let seed = [7u8; 32];
        let fingerprint = calculate_seed_fingerprint(&seed).unwrap();
        let cache = SpendAuthCache::new();
        let account = |i: u32| zip32::AccountId::try_from(i).unwrap();
        let fresh = |coin_type: u32, i: u32| {
            let osk =
                orchard::keys::SpendingKey::from_zip32_seed(&seed, coin_type, account(i)).unwrap();
            ask_scalar_bytes(&orchard::keys::SpendAuthorizingKey::from(&osk))
        };

        // Separate signers model PCZT and pool passes sharing one request-scoped
        // slot. Every lookup must return the requested account's key, including
        // after replacing the cached account.
        for (pool, coin_type, i) in [
            (ShieldedPool::Orchard, 133, 0u32),
            (ShieldedPool::Ironwood, 133, 0),
            (ShieldedPool::Ironwood, 133, 1),
            (ShieldedPool::Orchard, 1, 1),
            (ShieldedPool::Orchard, 1, 0),
            (ShieldedPool::Orchard, 133, 0),
        ] {
            let signer = SeedSigner::new(&seed, fingerprint, coin_type, None, pool, &cache);
            let bytes = signer
                .with_spend_authorizing_key(account(i), |ask| Ok(ask_scalar_bytes(ask)))
                .unwrap();
            assert_eq!(
                bytes,
                fresh(coin_type, i),
                "coin type {coin_type}, account {i} must match a fresh derivation"
            );
        }

        // The slot identity includes both network and account, and holds the
        // most recently used pair.
        assert_eq!(cache.0.borrow().identity, Some((133, account(0))));
    }

    #[test]
    fn test_ask_cache_can_be_reused_across_pczt_calls() {
        let sample = signable_sample_pczt();
        let cache = SpendAuthCache::new();

        for _ in 0..2 {
            sign_and_redact_pczt_with_cache(
                Pczt::parse(&sample.bytes).unwrap(),
                &sample.seed,
                crate::pczt::PcztNetwork::Mainnet,
                None,
                &cache,
            )
            .expect("shared-cache PCZT should sign");
        }

        assert_eq!(
            cache.0.borrow().identity,
            Some((133, zip32::AccountId::ZERO))
        );
    }

    #[test]
    fn test_scoped_signer_only_signs_selected_account() {
        let sample = crate::pczt::test_support::sample_migration_pczt_from_account(1);
        let account_one = zip32::AccountId::try_from(1).unwrap();
        let pczt = Pczt::parse(&sample.bytes).unwrap();
        let sighash = RoleSigner::new(pczt.clone())
            .expect("account-1 PCZT signer should initialize")
            .shielded_sighash();
        let signed = sign_and_redact_pczt_with_cache(
            pczt,
            &sample.seed,
            crate::pczt::PcztNetwork::Mainnet,
            Some(account_one),
            &SpendAuthCache::new(),
        )
        .expect("account-1 spend should sign when account 1 is selected");
        let action = signed
            .orchard()
            .actions()
            .iter()
            .find(|action| action.spend().spend_auth_sig().is_some())
            .expect("account-1 spend must be signed");
        let sig: orchard::primitives::redpallas::Signature<
            orchard::primitives::redpallas::SpendAuth,
        > = action.spend().spend_auth_sig().unwrap().into();
        let rk = orchard::primitives::redpallas::VerificationKey::<
            orchard::primitives::redpallas::SpendAuth,
        >::try_from(*action.spend().rk())
        .expect("randomized validating key must parse");
        rk.verify(&sighash, &sig)
            .expect("account-1 signature must match its randomized key");

        let result = sign_and_redact_pczt_with_cache(
            Pczt::parse(&sample.bytes).unwrap(),
            &sample.seed,
            crate::pczt::PcztNetwork::Mainnet,
            Some(zip32::AccountId::ZERO),
            &SpendAuthCache::new(),
        );

        assert!(matches!(result, Err(ZcashError::PcztNoMyInputs)));
    }

    #[test]
    fn test_scoped_ironwood_signer_rejects_unselected_account() {
        let sample = crate::pczt::test_support::sample_ironwood_pczt();
        let result = sign_and_redact_pczt_with_cache(
            Pczt::parse(&sample.bytes).unwrap(),
            &sample.seed,
            crate::pczt::PcztNetwork::Mainnet,
            Some(zip32::AccountId::try_from(1).unwrap()),
            &SpendAuthCache::new(),
        );

        assert!(matches!(result, Err(ZcashError::PcztNoMyInputs)));
    }

    fn signable_sample_pczt() -> crate::pczt::test_support::SamplePczt {
        crate::pczt::test_support::sample_ironwood_pczt()
    }

    #[test]
    fn test_sign_pczt_invalid_seed_fingerprint() {
        let sample = signable_sample_pczt();
        let pczt = Pczt::parse(&sample.bytes).unwrap();
        let mismatched_seed = [9u8; 32];

        let result = sign_pczt(pczt, &mismatched_seed, crate::pczt::PcztNetwork::Mainnet);
        assert!(matches!(result, Err(ZcashError::PcztNoMyInputs)));
    }

    // Consensus guard: the lean byte-level sighash (pczt_ext::shielded_sig_commitment) MUST
    // equal the upstream RoleSigner sighash for every shielded shape we sign. These assert it
    // bit-exact for an Orchard-only tx, a dual-pool Orchard->Ironwood migration, and an
    // Ironwood spend, so any upstream sighash change turns CI red instead of silently
    // producing wrong signatures on-device.
    #[test]
    fn test_lean_sighash_transparent_only_v6() {
        struct SighashCapture(Cell<Option<[u8; 32]>>);

        impl PcztSigner for SighashCapture {
            type Error = ZcashError;

            fn sign_transparent<F>(
                &self,
                index: usize,
                input: &mut transparent::pczt::Input,
                hash: F,
            ) -> Result<(), Self::Error>
            where
                F: FnOnce(SignableInput) -> [u8; 32],
            {
                self.0.set(Some(input.with_signable_input(index, hash)));
                Ok(())
            }

            fn sign_orchard(
                &self,
                _action: &mut orchard::pczt::Action,
                _hash: Hash,
            ) -> Result<(), Self::Error> {
                Ok(())
            }
        }

        let sample = crate::pczt::legacy_test_support::legacy_transparent_v6_sample();
        let pczt = Pczt::parse(&sample.bytes).unwrap();
        let oracle = RoleSigner::new(pczt.clone())
            .unwrap()
            .transparent_sighash(0)
            .unwrap();
        let capture = SighashCapture(Cell::new(None));

        pczt_ext::sign_transparent(low_level_signer::Signer::new(pczt), &capture).unwrap();

        assert_eq!(capture.0.get(), Some(oracle));
    }

    #[test]
    fn test_lean_sighash_control_orchard_only() {
        let sample = crate::pczt::test_support::sample_orchard_change_pczt();
        let pczt = Pczt::parse(&sample.bytes).unwrap();
        let oracle = RoleSigner::new(pczt.clone()).unwrap().shielded_sighash();
        let lean: [u8; 32] = zcash_vendor::pczt_ext::shielded_sig_commitment(&pczt, 0, None)
            .as_bytes()
            .try_into()
            .unwrap();
        assert_eq!(
            lean, oracle,
            "orchard-only: lean 4-node sighash should already equal RoleSigner"
        );
    }

    #[test]
    fn test_lean_sighash_migration_dualpool() {
        let sample = crate::pczt::test_support::sample_migration_pczt();
        let pczt = Pczt::parse(&sample.bytes).unwrap();
        let oracle = RoleSigner::new(pczt.clone()).unwrap().shielded_sighash();
        let lean: [u8; 32] = zcash_vendor::pczt_ext::shielded_sig_commitment(&pczt, 0, None)
            .as_bytes()
            .try_into()
            .unwrap();
        assert_eq!(
            lean, oracle,
            "migration: lean sighash must match RoleSigner v6 (Ironwood) sighash"
        );
    }

    #[test]
    fn test_lean_sighash_ironwood_spend() {
        // Exercises a populated Ironwood bundle with a real spend action.
        let sample = crate::pczt::test_support::sample_ironwood_pczt();
        let pczt = Pczt::parse(&sample.bytes).unwrap();
        let oracle = RoleSigner::new(pczt.clone()).unwrap().shielded_sighash();
        let lean: [u8; 32] = zcash_vendor::pczt_ext::shielded_sig_commitment(&pczt, 0, None)
            .as_bytes()
            .try_into()
            .unwrap();
        assert_eq!(
            lean, oracle,
            "ironwood-spend: lean sighash must match RoleSigner v6 sighash"
        );
    }

    // End-to-end: an Orchard->Ironwood migration signs the Orchard spend and leaves the
    // output-only Ironwood bundle unsigned.
    #[test]
    fn test_sign_pczt_migration_signs_orchard_only() {
        let sample = crate::pczt::test_support::sample_migration_pczt();
        let signed = sign_pczt(
            Pczt::parse(&sample.bytes).unwrap(),
            &sample.seed,
            crate::pczt::PcztNetwork::Mainnet,
        )
        .expect("migration PCZT should sign");
        let parsed = Pczt::parse(&signed).expect("signed migration PCZT must parse");
        assert!(
            parsed
                .orchard()
                .actions()
                .iter()
                .any(|a| a.spend().spend_auth_sig().is_some()),
            "migration Orchard spend must be authorized",
        );
        assert!(
            parsed
                .ironwood()
                .actions()
                .iter()
                .all(|a| a.spend().spend_auth_sig().is_none()),
            "output-only Ironwood bundle must not be authorized",
        );
    }

    #[test]
    fn test_sign_pczt_ironwood_spend() {
        let sample = crate::pczt::test_support::sample_ironwood_pczt();
        let pczt = Pczt::parse(&sample.bytes).unwrap();

        let base_sighash = RoleSigner::new(pczt.clone())
            .expect("Ironwood PCZT signer should initialize")
            .shielded_sighash();
        // Mirror the wallet's batch redaction: clearing the anchor rebuilds the
        // anchor-elided request the wallet sends for batch PCZTs, with the
        // full-anchor PCZT as its own oracle. The v6 Ironwood sighash does not
        // commit the anchor, so the elided form must leave the shielded sighash
        // unchanged; a client-provided anchor may equally stay on the wire.
        let cleared_anchor_pczt = Redactor::new(pczt.clone())
            .redact_ironwood_with(|mut r| r.clear_anchor())
            .finish();
        assert_ne!(
            pczt.ironwood().anchor(),
            cleared_anchor_pczt.ironwood().anchor()
        );
        assert_eq!(
            base_sighash,
            RoleSigner::new(cleared_anchor_pczt)
                .expect("anchor-cleared Ironwood PCZT signer should initialize")
                .shielded_sighash(),
            "v6 Ironwood spend signatures must not commit to the anchor"
        );

        let signed_pczt_bytes = sign_pczt(pczt, &sample.seed, crate::pczt::PcztNetwork::Mainnet)
            .expect("Ironwood PCZT should sign");
        let parsed = Pczt::parse(&signed_pczt_bytes).expect("signed PCZT must parse");

        let stamp = parsed
            .global()
            .proprietary()
            .get(PROP_KEY_FW_VERSION)
            .expect("firmware version stamp must be present");
        assert_eq!(stamp, &KEYSTONE_FW_VERSION.encode().to_vec());
        assert!(
            parsed
                .ironwood()
                .actions()
                .iter()
                .any(|action| action.spend().spend_auth_sig().is_some()),
            "Ironwood spend authorization signature must be present",
        );
        for action in parsed.ironwood().actions().iter() {
            if let Some(sig) = action.spend().spend_auth_sig() {
                let rk = orchard::primitives::redpallas::VerificationKey::<
                    orchard::primitives::redpallas::SpendAuth,
                >::try_from(*action.spend().rk())
                .expect("Ironwood randomized validating key must parse");
                let sig: orchard::primitives::redpallas::Signature<
                    orchard::primitives::redpallas::SpendAuth,
                > = (*sig).into();

                rk.verify(&base_sighash, &sig)
                    .expect("Ironwood spend authorization signature must match v6 sighash");
            }
        }
        assert!(
            signed_pczt_bytes.len() < sample.bytes.len(),
            "signed response should be redacted",
        );
    }

    #[test]
    fn test_sign_pczt_ironwood_spend_rejects_unsupported_zip32_path() {
        let sample = crate::pczt::test_support::sample_ironwood_pczt();
        for path in crate::pczt::test_support::unsupported_orchard_spend_paths() {
            let pczt = crate::pczt::test_support::ironwood_pczt_with_spend_derivation(
                &sample.bytes,
                sample.seed_fingerprint,
                path,
            );

            assert_invalid_pczt_message(
                sign_pczt(
                    Pczt::parse(&pczt).unwrap(),
                    &sample.seed,
                    crate::pczt::PcztNetwork::Mainnet,
                ),
                "unsupported Ironwood spend ZIP 32 derivation path",
            );
        }
    }

    #[test]
    fn test_sign_pczt_ironwood_spend_ignores_dummy_zip32_metadata() {
        let sample = crate::pczt::test_support::sample_ironwood_pczt();
        let pczt = crate::pczt::test_support::ironwood_pczt_with_dummy_spend_derivation(
            &sample.bytes,
            sample.seed_fingerprint,
            crate::pczt::test_support::orchard_spend_path_for_account(1),
        );

        let signed = sign_pczt(
            Pczt::parse(&pczt).unwrap(),
            &sample.seed,
            crate::pczt::PcztNetwork::Mainnet,
        )
        .expect("dummy spend ZIP 32 metadata must not block signing real spends");
        let parsed = Pczt::parse(&signed).expect("signed PCZT must parse");
        assert!(
            parsed
                .ironwood()
                .actions()
                .iter()
                .any(|action| action.spend().spend_auth_sig().is_some()),
            "Ironwood spend authorization signature must be present",
        );
    }

    #[test]
    fn test_sign_pczt_skips_finalized_redacted_dummy_spend() {
        let sample = crate::pczt::test_support::sample_ironwood_pczt();
        let pczt = crate::pczt::test_support::ironwood_pczt_with_dummy_spend_derivation(
            &sample.bytes,
            sample.seed_fingerprint,
            crate::pczt::test_support::orchard_spend_path_for_account(0),
        );
        let pczt = Pczt::parse(&pczt).unwrap();
        let mut dummy_indices = Vec::new();
        let pczt = zcash_vendor::pczt::roles::verifier::Verifier::new(pczt)
            .with_ironwood::<(), _>(|bundle| {
                dummy_indices.extend(bundle.actions().iter().enumerate().filter_map(
                    |(index, action)| {
                        matches!(action.spend().value().map(|value| value.inner()), Some(0))
                            .then_some(index)
                    },
                ));
                Ok(())
            })
            .unwrap()
            .finish();
        assert!(!dummy_indices.is_empty());
        let pczt = zcash_vendor::pczt::roles::io_finalizer::IoFinalizer::new(pczt)
            .finalize_io()
            .unwrap();
        let pczt = zcash_vendor::pczt::roles::verifier::Verifier::new(pczt)
            .with_ironwood::<(), _>(|bundle| {
                for index in &dummy_indices {
                    let spend = bundle.actions()[*index].spend();
                    assert!(spend.dummy_sk().is_none());
                    assert!(spend.spend_auth_sig().is_some());
                    assert!(spend.alpha().is_some());
                    assert!(spend.zip32_derivation().is_some());
                }
                Ok(())
            })
            .unwrap()
            .finish();

        // Match Vizor's batch transport after IO finalization: the finalizer
        // consumed dummy_sk, then redaction removed the dummy signature and
        // alpha while retaining the output action's spend derivation.
        let pczt = Redactor::new(pczt)
            .redact_ironwood_with(|mut bundle| {
                bundle.redact_actions(|mut action| {
                    action.clear_spend_fvk();
                    action.clear_spend_auth_sig();
                });
                for index in dummy_indices {
                    bundle.redact_action(index, |mut action| {
                        action.clear_spend_alpha();
                    });
                }
            })
            .finish();

        let signed = sign_pczt(pczt, &sample.seed, crate::pczt::PcztNetwork::Mainnet)
            .expect("finalized redacted dummy spend must not block the real signature");
        let signed_count = Pczt::parse(&signed)
            .unwrap()
            .ironwood()
            .actions()
            .iter()
            .filter(|action| action.spend().spend_auth_sig().is_some())
            .count();
        assert_eq!(signed_count, 1, "only the real spend should be signed");
    }

    #[test]
    fn test_sign_pczt_orchard_change_output_spend() {
        let sample = crate::pczt::test_support::sample_orchard_change_pczt();
        let pczt = Pczt::parse(&sample.bytes).unwrap();

        let signed = sign_pczt(pczt, &sample.seed, crate::pczt::PcztNetwork::Mainnet)
            .expect("Orchard change output spend should sign");
        let parsed = Pczt::parse(&signed).expect("signed PCZT must parse");
        let signed_actions = parsed
            .orchard()
            .actions()
            .iter()
            .filter(|action| action.spend().spend_auth_sig().is_some())
            .count();
        assert_eq!(
            signed_actions, 2,
            "real spend and wallet controlled zero value spend must be signed",
        );
    }

    fn pczt_with_min_version(min_version: &[u8]) -> Pczt {
        let sample = signable_sample_pczt();
        let base = Pczt::parse(&sample.bytes).unwrap();
        let min_version = min_version.to_vec();
        Updater::new(base)
            .update_global_with(|mut g| {
                g.set_proprietary("test:min_fw_version".to_string(), min_version);
            })
            .finish()
    }

    fn test_seed() -> Vec<u8> {
        [7u8; 32].to_vec()
    }

    #[test]
    fn firmware_equal_version_stamps_response() {
        let pczt = pczt_with_min_version(&KEYSTONE_FW_VERSION.encode());
        let signed = sign_pczt(pczt, &test_seed(), crate::pczt::PcztNetwork::Mainnet)
            .expect("equal-version PCZT should sign");
        let parsed = Pczt::parse(&signed).expect("signed PCZT must parse");

        let stamp = parsed
            .global()
            .proprietary()
            .get(PROP_KEY_FW_VERSION)
            .expect("firmware version stamp must be present");
        assert_eq!(stamp, &KEYSTONE_FW_VERSION.encode().to_vec());

        let request_min = parsed
            .global()
            .proprietary()
            .get("test:min_fw_version")
            .expect("request min-version should round-trip");
        assert_eq!(request_min, &KEYSTONE_FW_VERSION.encode().to_vec());
    }

    #[test]
    fn firmware_older_min_version_still_stamps_response() {
        let pczt = pczt_with_min_version(&[1, 0, 0]);
        let signed = sign_pczt(pczt, &test_seed(), crate::pczt::PcztNetwork::Mainnet)
            .expect("older-min PCZT should sign");
        let parsed = Pczt::parse(&signed).expect("signed PCZT must parse");

        let stamp = parsed
            .global()
            .proprietary()
            .get(PROP_KEY_FW_VERSION)
            .expect("firmware version stamp must be present");
        assert_eq!(stamp, &KEYSTONE_FW_VERSION.encode().to_vec());
    }

    #[test]
    fn malformed_min_version_round_trips_and_stamps() {
        let pczt = pczt_with_min_version(&[1, 2]);
        let signed = sign_pczt(pczt, &test_seed(), crate::pczt::PcztNetwork::Mainnet)
            .expect("malformed min bytes must not block signing");
        let parsed = Pczt::parse(&signed).expect("signed PCZT must parse");

        let stamp = parsed
            .global()
            .proprietary()
            .get(PROP_KEY_FW_VERSION)
            .expect("firmware version stamp must be present");
        assert_eq!(stamp, &KEYSTONE_FW_VERSION.encode().to_vec());

        let request_min = parsed
            .global()
            .proprietary()
            .get("test:min_fw_version")
            .expect("wallet-set min key must survive round trip");
        assert_eq!(request_min.as_slice(), &[1u8, 2][..]);
    }
}

#[cfg(all(test, feature = "multi_coins", not(feature = "cypherpunk")))]
mod legacy_tests {
    use super::*;

    #[test]
    fn legacy_signing_accepts_transparent_only_v6_pczt() {
        let sample = super::super::legacy_test_support::legacy_transparent_v6_sample();
        let signed = sign_pczt(
            Pczt::parse(&sample.bytes).unwrap(),
            &sample.seed,
            crate::pczt::PcztNetwork::Mainnet,
        )
        .expect("transparent-only v6 PCZT should sign");
        let signed = Pczt::parse(&signed).unwrap();

        assert!(super::super::pczt_is_v6(&signed));
        assert_eq!(
            signed.global().proprietary().get(PROP_KEY_FW_VERSION),
            Some(&KEYSTONE_FW_VERSION.encode().to_vec())
        );
    }
}
