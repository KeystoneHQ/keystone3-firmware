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
        if let Some(path) = transparent_key_path_for_input(self.seed, input)? {
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

#[cfg(not(feature = "cypherpunk"))]
pub fn sign_pczt(pczt: Pczt, seed: &[u8]) -> crate::Result<Vec<u8>> {
    super::validate_supported_pczt(&pczt)?;
    reject_legacy_unsupported_pczt(&pczt)?;

    let signer = low_level_signer::Signer::new(pczt);

    #[cfg(feature = "multi_coins")]
    let signer = pczt_ext::sign_transparent(signer, &SeedSigner { seed })
        .map_err(|e| ZcashError::SigningError(e.to_string()))?;

    stamp_and_redact(signer.finish())
        .serialize()
        .map_err(|e| ZcashError::SigningError(format!("serialize signed PCZT: {e:?}")))
}

#[cfg(not(feature = "cypherpunk"))]
fn reject_legacy_unsupported_pczt(pczt: &Pczt) -> Result<(), ZcashError> {
    {
        // The legacy helper below carries the pre-NU6.3 transparent sighash implementation.
        // It must not be used for shielded (Sapling/Orchard/Ironwood) or V6 PCZTs.
        if super::pczt_requires_cypherpunk_support(pczt) {
            return Err(ZcashError::SigningError(
                "Shielded or V6 PCZTs require cypherpunk signing support".to_string(),
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
    account: Option<zcash_vendor::zip32::AccountId>,
    // `MaybeUninit` makes every post-scrub bit pattern valid without depending
    // on private Orchard/reddsa layout invariants. The slot invariant is that
    // this field is initialized exactly when `account` is `Some`.
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
            account: None,
            ask: MaybeUninit::uninit(),
        }
    }

    fn get(
        &self,
        account: zcash_vendor::zip32::AccountId,
    ) -> Option<&orchard::keys::SpendAuthorizingKey> {
        if self.account == Some(account) {
            // SAFETY: `account` is only set to `Some` after `ask` is written.
            Some(unsafe { self.ask.assume_init_ref() })
        } else {
            None
        }
    }

    fn replace(
        &mut self,
        account: zcash_vendor::zip32::AccountId,
        ask: orchard::keys::SpendAuthorizingKey,
    ) -> &orchard::keys::SpendAuthorizingKey {
        self.account = None;
        self.ask.zeroize();
        self.ask.write(ask);
        self.account = Some(account);

        // SAFETY: the value was written immediately above, before `account`
        // was set to `Some`.
        unsafe { self.ask.assume_init_ref() }
    }
}

#[cfg(feature = "cypherpunk")]
impl Drop for AskCacheSlot {
    fn drop(&mut self) {
        self.account = None;
        self.ask.zeroize();
    }
}

/// One scrubbed spend authorizing key slot shared by the pool signing passes.
#[cfg(feature = "cypherpunk")]
struct SpendAuthCache(RefCell<AskCacheSlot>);

#[cfg(feature = "cypherpunk")]
impl SpendAuthCache {
    fn new() -> Self {
        Self(RefCell::new(AskCacheSlot::empty()))
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
    pool: ShieldedPool,
    /// Single-slot cache for the spend authorizing key. The key depends only on
    /// (seed, account), not on the action or pool, so consecutive actions for one
    /// account derive once and the Orchard and Ironwood passes can share it. A
    /// change of account replaces and scrubs the old key. Interior mutability is
    /// needed because `PcztSigner` signs through `&self`; the slot lives inline
    /// in the signing call and never touches the heap.
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
        pool: ShieldedPool,
        ask_cache: &'a SpendAuthCache,
    ) -> Self {
        Self {
            seed,
            seed_fingerprint,
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
            orchard::keys::SpendingKey::from_zip32_seed(self.seed, 133, account_index).map_err(
                |e| {
                    ZcashError::SigningError(format!(
                        "failed to derive {} spending key: {e:?}",
                        self.pool.label()
                    ))
                },
            )?,
        );
        // SAFETY: `osk` was initialized immediately above and is only scrubbed
        // after this borrow ends.
        let ask = orchard::keys::SpendAuthorizingKey::from(unsafe { osk.assume_init_ref() });
        osk.zeroize();
        Ok(ask)
    }

    /// Looks up (or derives and caches) the spend authorizing key for
    /// `account_index` and runs `f` against it in place, so no unscrubbed
    /// copies of the key are handed out. On an account change, the slot scrubs
    /// the old key in place before storing the replacement.
    fn with_spend_authorizing_key<R>(
        &self,
        account_index: zcash_vendor::zip32::AccountId,
        f: impl FnOnce(&orchard::keys::SpendAuthorizingKey) -> Result<R, ZcashError>,
    ) -> Result<R, ZcashError> {
        // Mutably borrowed across `f`, which is fine: `f` only signs and never
        // re-enters the cache.
        let mut cache = self.ask_cache.0.borrow_mut();
        if cache.get(account_index).is_none() {
            let ask = self.derive_spend_authorizing_key(account_index)?;
            cache.replace(account_index, ask);
        }
        f(cache
            .get(account_index)
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
        if let Some(path) = transparent_key_path_for_input(self.seed, input)? {
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
        let pool_label = self.pool.label();
        if action.spend().spend_auth_sig().is_some() {
            return Ok(());
        }
        if action.spend().dummy_sk().is_some() {
            match action.spend().value().map(|value| value.inner()) {
                Some(0) | None => return Ok(()),
                Some(_) => {
                    return Err(ZcashError::InvalidPczt(format!(
                        "{pool_label} spend dummy_sk is only valid for dummy spends"
                    )));
                }
            }
        }
        if action.spend().value().is_none() {
            return Ok(());
        }
        let Some(account_index) = super::matching_seed_supported_orchard_account(
            &self.seed_fingerprint,
            action.spend().zip32_derivation().as_ref(),
            133,
            self.pool,
        )?
        else {
            // Not derivable from this seed; not ours to sign.
            return Ok(());
        };

        self.with_spend_authorizing_key(account_index, |ask| {
            action
                .sign(
                    hash.as_bytes().try_into().expect("sighash is 32 bytes"),
                    ask,
                    OsRng,
                )
                .map_err(|e| {
                    ZcashError::SigningError(format!("failed to sign {pool_label} action: {e:?}"))
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
pub fn sign_pczt(pczt: Pczt, seed: &[u8]) -> crate::Result<Vec<u8>> {
    sign_and_redact_pczt(pczt, seed)?
        .serialize()
        .map_err(|e| ZcashError::SigningError(format!("serialize signed PCZT: {e:?}")))
}

/// `sign_pczt`, but returns the stamped, redacted PCZT without serializing it,
/// so callers that still need the parsed value (in-memory post-sign
/// verification) avoid a byte round trip.
#[cfg(feature = "cypherpunk")]
pub fn sign_and_redact_pczt(pczt: Pczt, seed: &[u8]) -> crate::Result<Pczt> {
    super::validate_supported_pczt(&pczt)?;

    let seed_fingerprint =
        calculate_seed_fingerprint(seed).map_err(|e| ZcashError::SigningError(e.to_string()))?;

    let process_ironwood = super::pczt_should_process_ironwood(&pczt);

    // Keep one scrubbed key slot and one lean signer for both pool passes.
    let ask_cache = SpendAuthCache::new();
    let mut seed_signer =
        SeedSigner::new(seed, seed_fingerprint, ShieldedPool::Orchard, &ask_cache);

    // The Orchard pass also handles transparent inputs; the pool only changes
    // error labels and ZIP 32 matching for shielded actions. Propagate signer
    // errors directly so strict path validation remains `InvalidPczt`.
    let signer = low_level_signer::Signer::new(pczt);
    let signer = pczt_ext::sign_transparent(signer, &seed_signer)?;
    let signer = pczt_ext::sign_orchard(signer, &seed_signer)?;

    let signer = if process_ironwood {
        seed_signer.pool = ShieldedPool::Ironwood;
        pczt_ext::sign_ironwood(signer, &seed_signer)?
    } else {
        signer
    };

    if seed_signer.signed.get() == 0 {
        return Err(ZcashError::PcztNoMyInputs);
    }

    // The low-level signer does not borrow the cache, so scrub the cached key
    // before finishing and redacting the response.
    drop(seed_signer);
    drop(ask_cache);
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
) -> Result<Option<String>, ZcashError> {
    let fingerprint =
        calculate_seed_fingerprint(seed).map_err(|e| ZcashError::SigningError(e.to_string()))?;

    for (pubkey, path) in input.bip32_derivation().iter() {
        let path_fingerprint = *path.seed_fingerprint();
        if fingerprint != path_fingerprint {
            continue;
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
            zip32::AccountId::ZERO,
            orchard::keys::SpendAuthorizingKey::from(&osk),
        );
        assert_ne!(
            ask_scalar_bytes(slot.get(zip32::AccountId::ZERO).unwrap()),
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
        let mut signer = SeedSigner::new(&seed, fingerprint, ShieldedPool::Orchard, &cache);
        let account = |i: u32| zip32::AccountId::try_from(i).unwrap();
        let fresh = |i: u32| {
            let osk = orchard::keys::SpendingKey::from_zip32_seed(&seed, 133, account(i)).unwrap();
            ask_scalar_bytes(&orchard::keys::SpendAuthorizingKey::from(&osk))
        };

        // Empty-slot miss, hit, replace-on-miss, hit-after-replace, and
        // cross-pool reuse all hand out exactly the key a fresh derivation
        // produces. The same signer and slot are reused across pool passes.
        for (pool, i) in [
            (ShieldedPool::Orchard, 0u32),
            (ShieldedPool::Orchard, 0),
            (ShieldedPool::Ironwood, 1),
            (ShieldedPool::Ironwood, 1),
            (ShieldedPool::Orchard, 0),
            (ShieldedPool::Ironwood, 2),
        ] {
            signer.pool = pool;
            let bytes = signer
                .with_spend_authorizing_key(account(i), |ask| Ok(ask_scalar_bytes(ask)))
                .unwrap();
            assert_eq!(bytes, fresh(i), "account {i} must match a fresh derivation");
        }

        // The slot holds exactly the most recently used account's key.
        assert_eq!(cache.0.borrow().account, Some(account(2)));
    }

    fn signable_sample_pczt() -> crate::pczt::test_support::SamplePczt {
        crate::pczt::test_support::sample_ironwood_pczt()
    }

    #[test]
    fn test_sign_pczt_invalid_seed_fingerprint() {
        let sample = signable_sample_pczt();
        let pczt = Pczt::parse(&sample.bytes).unwrap();
        let mismatched_seed = [9u8; 32];

        let result = sign_pczt(pczt, &mismatched_seed);
        assert!(matches!(result, Err(ZcashError::PcztNoMyInputs)));
    }

    // Consensus guard: the lean byte-level sighash (pczt_ext::shielded_sig_commitment) MUST
    // equal the upstream RoleSigner sighash for every shielded shape we sign. These assert it
    // bit-exact for an Orchard-only tx, a dual-pool Orchard->Ironwood migration, and an
    // Ironwood spend, so any upstream sighash change turns CI red instead of silently
    // producing wrong signatures on-device.
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
        let signed = sign_pczt(Pczt::parse(&sample.bytes).unwrap(), &sample.seed)
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
        // anchor-elided request the wallet sends for batch children, with the
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

        let signed_pczt_bytes = sign_pczt(pczt, &sample.seed).expect("Ironwood PCZT should sign");
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
                sign_pczt(Pczt::parse(&pczt).unwrap(), &sample.seed),
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

        let signed = sign_pczt(Pczt::parse(&pczt).unwrap(), &sample.seed)
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
    fn test_sign_pczt_orchard_change_output_spend() {
        let sample = crate::pczt::test_support::sample_orchard_change_pczt();
        let pczt = Pczt::parse(&sample.bytes).unwrap();

        let signed =
            sign_pczt(pczt, &sample.seed).expect("Orchard change output spend should sign");
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
        let signed = sign_pczt(pczt, &test_seed()).expect("equal-version PCZT should sign");
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
        let signed = sign_pczt(pczt, &test_seed()).expect("older-min PCZT should sign");
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
        let signed =
            sign_pczt(pczt, &test_seed()).expect("malformed min bytes must not block signing");
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
    use zcash_vendor::{
        pczt::roles::creator::Creator,
        zcash_protocol::consensus::{BranchId, MainNetwork, NetworkConstants},
    };

    #[test]
    fn legacy_signing_rejects_v6_pczt() {
        let pczt = Creator::new(
            BranchId::Nu6_3.into(),
            10,
            MainNetwork.coin_type(),
            [0; 32],
            [0; 32],
        )
        .unwrap()
        .build();

        let result = sign_pczt(pczt, &[7u8; 32]);

        assert!(matches!(
            result,
            Err(ZcashError::SigningError(msg))
                if msg == "Shielded or V6 PCZTs require cypherpunk signing support"
        ));
    }
}
