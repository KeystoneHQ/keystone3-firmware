pub mod check;
pub mod parse;
pub mod sign;
pub mod structs;

use alloc::{format, string::ToString};
use zcash_vendor::{
    pczt::Pczt,
    transparent,
    zcash_protocol::{constants, value::ZatBalance},
    zip32,
};

use crate::errors::ZcashError;

pub(crate) fn parse_pczt(bytes: &[u8]) -> Result<Pczt, ZcashError> {
    Pczt::parse(bytes).map_err(|_| ZcashError::InvalidPczt("invalid pczt data".to_string()))
}

pub(crate) fn validate_supported_pczt(pczt: &Pczt) -> Result<(), ZcashError> {
    validate_sapling_bundle_consistency(pczt)?;

    {
        if pczt_has_ironwood_actions(pczt) && !pczt_is_v6(pczt) {
            return Err(ZcashError::InvalidPczt(
                "Ironwood actions require a v6 PCZT".to_string(),
            ));
        }

        // The lean v6 shielded sighash (`pczt_ext::shielded_sig_commitment`) implements
        // the Orchard and Ironwood v6 commitment domains, but NOT the v6 Sapling-spend
        // domain (which uses a distinct noncompact personalization and omits the
        // per-spend anchor; ZIP-244). Keystone's cypherpunk signing never produces
        // Sapling spend authorizations, so reject any v6 PCZT carrying a Sapling spend
        // rather than committing our Orchard/Ironwood signature to a v5-rules (wrong)
        // Sapling digest. Sapling OUTPUTS are version-independent in ZIP-244 and remain
        // supported; v5 transactions are unaffected (the lean v5 Sapling digest matches).
        if pczt_is_v6(pczt) && !pczt.sapling().spends().is_empty() {
            return Err(ZcashError::InvalidPczt(
                "Sapling spends are not supported in v6 transactions".to_string(),
            ));
        }
    }

    #[cfg(feature = "cypherpunk")]
    validate_distinct_orchard_protocol_rks(pczt)?;

    Ok(())
}

/// Ensures every Orchard protocol action has a distinct randomized validating key.
///
/// Orchard and Ironwood spend authorization signatures cover the same transaction-wide
/// shielded sighash. If two actions share an `rk`, a signature for either action can be
/// copied to the other.
#[cfg(feature = "cypherpunk")]
fn validate_distinct_orchard_protocol_rks(pczt: &Pczt) -> Result<(), ZcashError> {
    let actions = pczt
        .orchard()
        .actions()
        .iter()
        .chain(pczt.ironwood().actions().iter());

    for (index, action) in actions.clone().enumerate() {
        if actions
            .clone()
            .skip(index + 1)
            .any(|other| other.spend().rk() == action.spend().rk())
        {
            return Err(ZcashError::InvalidPczt(
                "duplicate Orchard or Ironwood action rk".to_string(),
            ));
        }
    }

    Ok(())
}

pub(crate) fn pczt_has_ironwood_actions(pczt: &Pczt) -> bool {
    !pczt.ironwood().actions().is_empty()
}

pub(crate) fn pczt_is_v6(pczt: &Pczt) -> bool {
    *pczt.global().tx_version() == constants::V6_TX_VERSION
        && *pczt.global().version_group_id() == constants::V6_VERSION_GROUP_ID
}

pub(crate) fn pczt_should_process_ironwood(pczt: &Pczt) -> bool {
    pczt_is_v6(pczt) || pczt_has_ironwood_actions(pczt)
}

fn validate_sapling_bundle_consistency(pczt: &Pczt) -> Result<(), ZcashError> {
    let value_balance = (*pczt.sapling().value_sum())
        .try_into()
        .ok()
        .and_then(|v| ZatBalance::from_i64(v).ok())
        .ok_or(ZcashError::InvalidPczt(
            "sapling value_sum is invalid".to_string(),
        ))?;
    let sapling_value_sum: i64 = value_balance.into();
    let has_sapling_bundle =
        !pczt.sapling().spends().is_empty() || !pczt.sapling().outputs().is_empty();

    if !has_sapling_bundle && sapling_value_sum != 0 {
        return Err(ZcashError::InvalidPczt(
            "sapling value_sum must be zero when Sapling bundle is empty".to_string(),
        ));
    }

    Ok(())
}

pub(crate) fn check_transparent_derivation<
    P: zcash_vendor::zcash_protocol::consensus::Parameters,
>(
    params: &P,
    account_index: zip32::AccountId,
    xpub: &transparent::keys::AccountPubKey,
    pubkey: &[u8; 33],
    derivation: &transparent::pczt::Bip32Derivation,
    field_label: &str,
) -> Result<(), ZcashError> {
    let target = xpub
        .derive_pubkey_at_bip32_path(params, account_index, derivation.derivation_path())
        .map_err(|_| {
            ZcashError::InvalidPczt(format!(
                "transparent {field_label} bip32 derivation path invalid"
            ))
        })?;
    if &target.serialize() != pubkey {
        return Err(ZcashError::InvalidPczt(format!(
            "transparent {field_label} script pubkey mismatch"
        )));
    }

    Ok(())
}

/// Which shielded pool a bundle belongs to. Orchard and Ironwood share the same
/// `orchard::pczt::Bundle` representation, so this distinguishes them (e.g. in
/// error messages) without a free-form string.
#[cfg(feature = "cypherpunk")]
#[derive(Clone, Copy)]
pub(crate) enum ShieldedPool {
    Orchard,
    Ironwood,
}

#[cfg(feature = "cypherpunk")]
impl ShieldedPool {
    pub(crate) fn label(self) -> &'static str {
        match self {
            ShieldedPool::Orchard => "Orchard",
            ShieldedPool::Ironwood => "Ironwood",
        }
    }
}

/// Returns the supported account declared by a shielded spend derivation that
/// belongs to this seed. Missing or different seed fingerprints are not ours
/// and return `None`; matching fingerprints with paths outside
/// `m/32'/coin_type'/account'` are invalid.
#[cfg(feature = "cypherpunk")]
pub(crate) fn matching_seed_supported_orchard_account(
    seed_fingerprint: &[u8; 32],
    derivation: Option<&zcash_vendor::orchard::pczt::Zip32Derivation>,
    coin_type: u32,
    pool: ShieldedPool,
) -> Result<Option<zcash_vendor::zip32::AccountId>, crate::errors::ZcashError> {
    matching_seed_supported_orchard_account_parts(
        seed_fingerprint,
        derivation.map(|derivation| {
            (
                derivation.seed_fingerprint(),
                derivation.derivation_path().as_slice(),
            )
        }),
        coin_type,
        pool,
    )
}

/// [`matching_seed_supported_orchard_account`] over the derivation's raw
/// (fingerprint, path) parts, for callers holding derivation parts recorded
/// during the batch check instead of a `Zip32Derivation` borrow.
#[cfg(feature = "cypherpunk")]
pub(crate) fn matching_seed_supported_orchard_account_parts(
    seed_fingerprint: &[u8; 32],
    derivation: Option<(&[u8; 32], &[zcash_vendor::zip32::ChildIndex])>,
    coin_type: u32,
    pool: ShieldedPool,
) -> Result<Option<zcash_vendor::zip32::AccountId>, crate::errors::ZcashError> {
    let pool_label = pool.label();
    let Some((derivation_seed_fingerprint, derivation_path)) = derivation else {
        return Ok(None);
    };
    if derivation_seed_fingerprint != seed_fingerprint {
        return Ok(None);
    }

    let unsupported_path = || {
        crate::errors::ZcashError::InvalidPczt(alloc::format!(
            "unsupported {pool_label} spend ZIP 32 derivation path"
        ))
    };

    let [purpose, path_coin_type, account_index] = derivation_path else {
        return Err(unsupported_path());
    };

    if purpose != &zcash_vendor::zip32::ChildIndex::hardened(32)
        || path_coin_type != &zcash_vendor::zip32::ChildIndex::hardened(coin_type)
    {
        return Err(unsupported_path());
    }

    let account_index = account_index
        .index()
        .checked_sub(1 << 31)
        .ok_or_else(unsupported_path)?;
    zcash_vendor::zip32::AccountId::try_from(account_index)
        .map(Some)
        .map_err(|_| unsupported_path())
}

/// Returns whether a PCZT carries anything the transparent-only legacy path
/// cannot handle: a v6+ transaction, or any shielded (Sapling/Orchard/Ironwood)
/// content. These must be checked, parsed, and signed by the cypherpunk build.
#[cfg(any(feature = "multi_coins", not(feature = "cypherpunk")))]
pub(crate) fn pczt_requires_cypherpunk_support(pczt: &zcash_vendor::pczt::Pczt) -> bool {
    *pczt.global().tx_version() >= 6
        || !pczt.sapling().spends().is_empty()
        || !pczt.sapling().outputs().is_empty()
        || !pczt.orchard().actions().is_empty()
        || !pczt.ironwood().actions().is_empty()
}

#[cfg(all(test, feature = "cypherpunk"))]
pub(crate) mod test_support {
    use alloc::{string::String, vec, vec::Vec};

    use ::pczt::roles::{creator::Creator, updater::Updater};
    use incrementalmerkletree::Retention;
    use keystore::algorithms::zcash::{calculate_seed_fingerprint, derive_ufvk};
    use rand_core::OsRng;
    use shardtree::{store::memory::MemoryShardStore, ShardTree};
    use zcash_note_encryption::try_note_decryption;
    use zcash_primitives::transaction::{
        builder::{BuildConfig, Builder, PcztParts, PcztResult},
        fees::zip317,
        TxVersion,
    };
    use zcash_vendor::zcash_protocol::consensus::{BlockHeight, NetworkType, NetworkUpgrade};
    use zcash_vendor::{
        orchard,
        pczt::Pczt,
        zcash_keys::keys::{UnifiedAddressRequest, UnifiedFullViewingKey},
        zcash_protocol::{
            consensus::{BranchId, MainNetwork, Parameters},
            memo::{Memo, MemoBytes},
            value::Zatoshis,
        },
        zip32,
    };

    pub(crate) struct SamplePczt {
        pub(crate) bytes: Vec<u8>,
        pub(crate) seed: Vec<u8>,
        pub(crate) ufvk_text: String,
        pub(crate) seed_fingerprint: [u8; 32],
    }

    #[derive(Clone, Copy, Debug)]
    pub(crate) struct Nu6_3Network;

    impl Parameters for Nu6_3Network {
        fn network_type(&self) -> NetworkType {
            NetworkType::Main
        }

        fn activation_height(&self, nu: NetworkUpgrade) -> Option<BlockHeight> {
            match nu {
                NetworkUpgrade::Nu6_3 => Some(BlockHeight::from_u32(10)),
                _ => MainNetwork.activation_height(nu),
            }
        }
    }

    pub(crate) fn unsupported_orchard_spend_paths() -> Vec<Vec<u32>> {
        vec![
            vec![
                zip32::ChildIndex::hardened(32).index(),
                zip32::ChildIndex::hardened(1).index(),
                zip32::ChildIndex::hardened(0).index(),
            ],
            vec![
                zip32::ChildIndex::hardened(32).index(),
                zip32::ChildIndex::hardened(133).index(),
                zip32::ChildIndex::hardened(0).index(),
                zip32::ChildIndex::hardened(0).index(),
            ],
        ]
    }

    pub(crate) fn orchard_spend_path_for_account(account_index: u32) -> Vec<u32> {
        vec![
            zip32::ChildIndex::hardened(32).index(),
            zip32::ChildIndex::hardened(133).index(),
            zip32::ChildIndex::hardened(account_index).index(),
        ]
    }

    pub(crate) fn ironwood_pczt_with_spend_derivation(
        bytes: &[u8],
        seed_fingerprint: [u8; 32],
        path: Vec<u32>,
    ) -> Vec<u8> {
        Updater::new(Pczt::parse(bytes).unwrap())
            .update_ironwood_with(|mut bundle| {
                for action_index in 0..bundle.bundle().actions().len() {
                    let derivation =
                        orchard::pczt::Zip32Derivation::parse(seed_fingerprint, path.clone())
                            .unwrap();
                    bundle.update_action_with(action_index, |mut action| {
                        action.set_spend_zip32_derivation(derivation);
                        Ok(())
                    })?;
                }
                Ok(())
            })
            .unwrap()
            .finish()
            .serialize()
            .unwrap()
    }

    pub(crate) fn ironwood_pczt_with_dummy_spend_derivation(
        bytes: &[u8],
        seed_fingerprint: [u8; 32],
        path: Vec<u32>,
    ) -> Vec<u8> {
        Updater::new(Pczt::parse(bytes).unwrap())
            .update_ironwood_with(|mut bundle| {
                let dummy_action_indices = bundle
                    .bundle()
                    .actions()
                    .iter()
                    .enumerate()
                    .filter_map(|(index, action)| {
                        matches!(action.spend().value().map(|value| value.inner()), Some(0))
                            .then_some(index)
                    })
                    .collect::<Vec<_>>();
                assert!(!dummy_action_indices.is_empty());

                for action_index in dummy_action_indices {
                    let derivation =
                        orchard::pczt::Zip32Derivation::parse(seed_fingerprint, path.clone())
                            .unwrap();
                    bundle.update_action_with(action_index, |mut action| {
                        action.set_spend_zip32_derivation(derivation);
                        Ok(())
                    })?;
                }
                Ok(())
            })
            .unwrap()
            .finish()
            .serialize()
            .unwrap()
    }

    pub(crate) fn sample_ironwood_pczt() -> SamplePczt {
        let params = Nu6_3Network;
        let seed = [7u8; 32];
        let ufvk_text = derive_ufvk(&params, &seed, "m/32'/133'/0'").unwrap();
        let ufvk = UnifiedFullViewingKey::decode(&params, &ufvk_text).unwrap();
        let orchard_fvk = ufvk.orchard().unwrap().clone();
        let orchard_ivk = orchard_fvk.to_ivk(orchard::keys::Scope::External);
        let orchard_ovk = orchard_fvk.to_ovk(orchard::keys::Scope::External);
        let recipient = orchard_fvk.address_at(0u32, orchard::keys::Scope::External);

        let value = orchard::value::NoteValue::from_raw(1_000_000);
        let note = {
            let mut orchard_builder = orchard::builder::Builder::new(
                orchard::builder::BundleType::DEFAULT,
                orchard::bundle::BundleVersion::ironwood_v3(),
                orchard::bundle::BundleVersion::ironwood_v3().default_flags(),
                orchard::Anchor::empty_tree(),
            )
            .expect("default flags are representable under the bundle version");
            orchard_builder
                .add_output(None, recipient, value, Memo::Empty.encode().into_bytes())
                .unwrap();
            let (bundle, meta) = orchard_builder.build::<i64>(&mut OsRng).unwrap().unwrap();
            let action = bundle
                .actions()
                .get(meta.output_action_index(0).unwrap())
                .unwrap();
            let domain = orchard::note_encryption::IronwoodDomain::for_action(action);
            let (note, _, _) =
                try_note_decryption(&domain, &orchard_ivk.prepare(), action).unwrap();
            note
        };

        let (anchor, merkle_path) = {
            let cmx: orchard::note::ExtractedNoteCommitment = note.commitment().into();
            let leaf = orchard::tree::MerkleHashOrchard::from_cmx(&cmx);
            let mut tree = ShardTree::<_, 32, 16>::new(
                MemoryShardStore::<orchard::tree::MerkleHashOrchard, u32>::empty(),
                100,
            );
            tree.append(leaf, Retention::Marked).unwrap();
            tree.checkpoint(9_999_999).unwrap();
            let merkle_path = tree
                .witness_at_checkpoint_depth(0.into(), 0)
                .unwrap()
                .unwrap();
            let anchor = merkle_path.root(leaf);
            (anchor.into(), merkle_path.into())
        };

        let mut builder = Builder::new(
            &params,
            10_000_000.into(),
            BuildConfig::Standard {
                sapling_anchor: None,
                orchard_anchor: None,
                ironwood_anchor: Some(anchor),
                orchard_pool_bundle_type: orchard::builder::BundleType::DEFAULT,
            },
        );
        builder
            .add_ironwood_spend::<zip317::FeeRule>(orchard_fvk.clone(), note, merkle_path)
            .unwrap();
        builder
            .add_ironwood_output::<zip317::FeeRule>(
                Some(orchard_ovk),
                recipient,
                Zatoshis::const_from_u64(990_000),
                MemoBytes::empty(),
            )
            .unwrap();
        let PcztResult {
            pczt_parts,
            ironwood_meta,
            ..
        } = builder
            .build_for_pczt(OsRng, &zip317::FeeRule::standard())
            .unwrap();
        let spend_action_index = ironwood_meta.spend_action_index(0).unwrap();
        let seed_fingerprint = calculate_seed_fingerprint(&seed).unwrap();
        let derivation = orchard::pczt::Zip32Derivation::parse(
            seed_fingerprint,
            vec![
                zip32::ChildIndex::hardened(32).index(),
                zip32::ChildIndex::hardened(133).index(),
                zip32::ChildIndex::hardened(0).index(),
            ],
        )
        .unwrap();
        let pczt = Updater::new(Creator::build_from_parts(pczt_parts).unwrap())
            .update_ironwood_with(|mut bundle| {
                bundle.update_action_with(spend_action_index, |mut action| {
                    action.set_spend_zip32_derivation(derivation);
                    Ok(())
                })
            })
            .unwrap()
            .finish();

        SamplePczt {
            bytes: pczt.serialize().unwrap(),
            seed: seed.to_vec(),
            ufvk_text,
            seed_fingerprint,
        }
    }

    // Orchard spend -> Ironwood output, matching one compact-eligible transfer.
    pub(crate) fn sample_migration_pczt() -> SamplePczt {
        sample_migration_pczt_with_options(0, MemoBytes::empty(), None)
    }

    /// Builds a migration whose funded output carries the given memo.
    pub(crate) fn sample_migration_pczt_with_output_memo(output_memo: MemoBytes) -> SamplePczt {
        sample_migration_pczt_with_options(0, output_memo, None)
    }

    /// Builds a migration whose funded output belongs to the given account.
    pub(crate) fn sample_migration_pczt_to_account(output_account: u32) -> SamplePczt {
        sample_migration_pczt_with_options(output_account, MemoBytes::empty(), None)
    }

    /// Adds a zero-value output, optionally marked for ordinary display.
    pub(crate) fn sample_migration_pczt_with_zero_output(
        memo: MemoBytes,
        displayable: bool,
    ) -> SamplePczt {
        sample_migration_pczt_with_options(0, MemoBytes::empty(), Some((memo, displayable)))
    }

    /// Builds a migration sample with a configurable funded recipient and optional zero output.
    fn sample_migration_pczt_with_options(
        output_account: u32,
        output_memo: MemoBytes,
        zero_output: Option<(MemoBytes, bool)>,
    ) -> SamplePczt {
        let params = Nu6_3Network;
        let seed = [7u8; 32];
        let ufvk_text = derive_ufvk(&params, &seed, "m/32'/133'/0'").unwrap();
        let ufvk = UnifiedFullViewingKey::decode(&params, &ufvk_text).unwrap();
        let orchard_fvk = ufvk.orchard().unwrap().clone();
        let orchard_ivk = orchard_fvk.to_ivk(orchard::keys::Scope::External);
        let spend_recipient = orchard_fvk.address_at(0u32, orchard::keys::Scope::External);
        let output_ufvk_text = derive_ufvk(
            &params,
            &seed,
            &alloc::format!("m/32'/133'/{output_account}'"),
        )
        .unwrap();
        let output_ufvk = UnifiedFullViewingKey::decode(&params, &output_ufvk_text).unwrap();
        let output_fvk = output_ufvk.orchard().unwrap();
        // The compact-review fixture uses an internal Ironwood receiver. Keep
        // the foreign-account variant decryptable for its ordinary-review test
        // by using the selected account's external OVK.
        let recipient = output_fvk.address_at(0u32, orchard::keys::Scope::Internal);
        let orchard_ovk = orchard_fvk.to_ovk(if output_account == 0 {
            orchard::keys::Scope::Internal
        } else {
            orchard::keys::Scope::External
        });
        let zero_recipient = output_fvk.address_at(0u32, orchard::keys::Scope::External);
        let output_user_address = output_ufvk
            .default_address(UnifiedAddressRequest::AllAvailableKeys)
            .unwrap()
            .0
            .encode(&params);

        // The Orchard note being migrated: output (990_000) + cross-pool fee (20_000),
        // so there is no change output.
        let value = orchard::value::NoteValue::from_raw(1_010_000);
        let note = {
            let mut orchard_builder = orchard::builder::Builder::new(
                orchard::builder::BundleType::Coinbase,
                orchard::bundle::BundleVersion::orchard_v2(),
                orchard::bundle::Flags::SPENDS_DISABLED,
                orchard::Anchor::empty_tree(),
            )
            .expect("spends-disabled flags are valid for a coinbase bundle");
            orchard_builder
                .add_output(
                    None,
                    spend_recipient,
                    value,
                    Memo::Empty.encode().into_bytes(),
                )
                .unwrap();
            let (bundle, meta) = orchard_builder.build::<i64>(&mut OsRng).unwrap().unwrap();
            let action = bundle
                .actions()
                .get(meta.output_action_index(0).unwrap())
                .unwrap();
            let domain = orchard::note_encryption::OrchardDomain::for_action(action);
            let (note, _, _) =
                try_note_decryption(&domain, &orchard_ivk.prepare(), action).unwrap();
            note
        };

        let (anchor, merkle_path) = {
            let cmx: orchard::note::ExtractedNoteCommitment = note.commitment().into();
            let leaf = orchard::tree::MerkleHashOrchard::from_cmx(&cmx);
            let mut tree = ShardTree::<_, 32, 16>::new(
                MemoryShardStore::<orchard::tree::MerkleHashOrchard, u32>::empty(),
                100,
            );
            tree.append(leaf, Retention::Marked).unwrap();
            tree.checkpoint(9_999_999).unwrap();
            let merkle_path = tree
                .witness_at_checkpoint_depth(0.into(), 0)
                .unwrap()
                .unwrap();
            let anchor = merkle_path.root(leaf);
            (anchor.into(), merkle_path.into())
        };

        let mut builder = Builder::new(
            &params,
            10_000_000.into(),
            BuildConfig::Standard {
                sapling_anchor: None,
                orchard_anchor: Some(anchor),
                ironwood_anchor: Some(orchard::Anchor::empty_tree()),
                orchard_pool_bundle_type: orchard::builder::BundleType::DEFAULT,
            },
        );
        builder
            .add_orchard_spend::<zip317::FeeRule>(orchard_fvk.clone(), note, merkle_path)
            .unwrap();
        builder
            .add_ironwood_output::<zip317::FeeRule>(
                Some(orchard_ovk),
                recipient,
                Zatoshis::const_from_u64(990_000),
                output_memo,
            )
            .unwrap();
        if let Some((memo, _)) = zero_output.as_ref() {
            builder
                .add_ironwood_output::<zip317::FeeRule>(
                    None,
                    zero_recipient,
                    Zatoshis::ZERO,
                    memo.clone(),
                )
                .unwrap();
        }
        let PcztResult {
            pczt_parts,
            orchard_meta,
            ironwood_meta,
            ..
        } = builder
            .build_for_pczt(OsRng, &zip317::FeeRule::standard())
            .unwrap();
        let spend_action_index = orchard_meta.spend_action_index(0).unwrap();
        let displayable_zero_action = match zero_output.as_ref() {
            Some((_, true)) => Some(ironwood_meta.output_action_index(1).unwrap()),
            _ => None,
        };
        let seed_fingerprint = calculate_seed_fingerprint(&seed).unwrap();
        let derivation = orchard::pczt::Zip32Derivation::parse(
            seed_fingerprint,
            vec![
                zip32::ChildIndex::hardened(32).index(),
                zip32::ChildIndex::hardened(133).index(),
                zip32::ChildIndex::hardened(0).index(),
            ],
        )
        .unwrap();
        let pczt = Updater::new(Creator::build_from_parts(pczt_parts).unwrap())
            .update_orchard_with(|mut bundle| {
                bundle.update_action_with(spend_action_index, |mut action| {
                    action.set_spend_zip32_derivation(derivation);
                    Ok(())
                })
            })
            .unwrap()
            .finish();
        let pczt = if let Some(action_index) = displayable_zero_action {
            Updater::new(pczt)
                .update_ironwood_with(|mut bundle| {
                    bundle.update_action_with(action_index, |mut action| {
                        action.set_output_user_address(output_user_address);
                        Ok(())
                    })
                })
                .unwrap()
                .finish()
        } else {
            pczt
        };

        SamplePczt {
            bytes: pczt.serialize().unwrap(),
            seed: seed.to_vec(),
            ufvk_text,
            seed_fingerprint,
        }
    }

    pub(crate) fn sample_orchard_change_pczt() -> SamplePczt {
        sample_orchard_change_pczt_for_account(0)
    }

    pub(crate) fn sample_orchard_foreign_change_pczt() -> SamplePczt {
        sample_orchard_change_pczt_for_account(1)
    }

    fn sample_orchard_change_pczt_for_account(output_account: u32) -> SamplePczt {
        let params = MainNetwork;
        let seed = [7u8; 32];
        let ufvk_text = derive_ufvk(&params, &seed, "m/32'/133'/0'").unwrap();
        let ufvk = UnifiedFullViewingKey::decode(&params, &ufvk_text).unwrap();
        let orchard_fvk = ufvk.orchard().unwrap().clone();
        let orchard_ivk = orchard_fvk.to_ivk(orchard::keys::Scope::External);

        let output_ufvk_text = derive_ufvk(
            &params,
            &seed,
            &alloc::format!("m/32'/133'/{output_account}'"),
        )
        .unwrap();
        let output_ufvk = UnifiedFullViewingKey::decode(&params, &output_ufvk_text).unwrap();
        let output_fvk = output_ufvk.orchard().unwrap().clone();
        let recipient_scope = orchard::keys::Scope::External;
        let spend_recipient = orchard_fvk.address_at(0u32, recipient_scope);
        let output_recipient = output_fvk.address_at(0u32, recipient_scope);
        let orchard_ovk = output_fvk.to_ovk(recipient_scope);

        let value = orchard::value::NoteValue::from_raw(1_000_000);
        let note = {
            let mut orchard_builder = orchard::builder::Builder::new(
                orchard::builder::BundleType::Coinbase,
                orchard::bundle::BundleVersion::orchard_v2(),
                orchard::bundle::Flags::SPENDS_DISABLED,
                orchard::Anchor::empty_tree(),
            )
            .expect("spends-disabled flags are valid for a coinbase bundle");
            orchard_builder
                .add_output(
                    None,
                    spend_recipient,
                    value,
                    Memo::Empty.encode().into_bytes(),
                )
                .unwrap();
            let (bundle, meta) = orchard_builder.build::<i64>(&mut OsRng).unwrap().unwrap();
            let action = bundle
                .actions()
                .get(meta.output_action_index(0).unwrap())
                .unwrap();
            let domain = orchard::note_encryption::OrchardDomain::for_action(action);
            let (note, _, _) =
                try_note_decryption(&domain, &orchard_ivk.prepare(), action).unwrap();
            note
        };

        let (anchor, merkle_path) = {
            let cmx: orchard::note::ExtractedNoteCommitment = note.commitment().into();
            let leaf = orchard::tree::MerkleHashOrchard::from_cmx(&cmx);
            let mut tree = ShardTree::<_, 32, 16>::new(
                MemoryShardStore::<orchard::tree::MerkleHashOrchard, u32>::empty(),
                100,
            );
            tree.append(leaf, Retention::Marked).unwrap();
            tree.checkpoint(9_999_999).unwrap();
            let merkle_path = tree
                .witness_at_checkpoint_depth(0.into(), 0)
                .unwrap()
                .unwrap();
            let anchor = merkle_path.root(leaf);
            (anchor.into(), merkle_path.into())
        };

        let mut builder = orchard::builder::Builder::new(
            orchard::builder::BundleType::DEFAULT,
            orchard::bundle::BundleVersion::orchard_v3(),
            orchard::bundle::BundleVersion::orchard_v3().default_flags(),
            anchor,
        )
        .expect("default flags are representable under the bundle version");
        builder
            .add_spend(orchard_fvk.clone(), note, merkle_path)
            .unwrap();
        builder
            .add_change_output(
                output_fvk,
                Some(orchard_ovk),
                output_recipient,
                orchard::value::NoteValue::from_raw(990_000),
                Memo::Empty.encode().into_bytes(),
            )
            .unwrap();
        let (orchard_bundle, _) = builder.build_for_pczt(&mut OsRng).unwrap();
        let seed_fingerprint = calculate_seed_fingerprint(&seed).unwrap();
        let pczt = Creator::build_from_parts(PcztParts {
            params,
            version: TxVersion::V6,
            consensus_branch_id: BranchId::Nu6_3,
            lock_time: 0,
            expiry_height: BlockHeight::from_u32(10_000_000),
            transparent: None,
            sapling: None,
            orchard: Some(orchard_bundle),
            ironwood: None,
        })
        .unwrap();
        let pczt = Updater::new(pczt)
            .update_orchard_with(|mut bundle| {
                let signing_action_accounts = bundle
                    .bundle()
                    .actions()
                    .iter()
                    .enumerate()
                    .filter_map(|(index, action)| {
                        action.spend().dummy_sk().is_none().then(|| {
                            let account = if action.spend().value().unwrap().inner() == 0 {
                                output_account
                            } else {
                                0
                            };
                            (index, account)
                        })
                    })
                    .collect::<Vec<_>>();
                assert_eq!(signing_action_accounts.len(), 2);

                for (action_index, account) in signing_action_accounts {
                    let derivation = orchard::pczt::Zip32Derivation::parse(
                        seed_fingerprint,
                        orchard_spend_path_for_account(account),
                    )
                    .unwrap();
                    bundle.update_action_with(action_index, |mut action| {
                        action.set_spend_zip32_derivation(derivation);
                        Ok(())
                    })?;
                }
                Ok(())
            })
            .unwrap()
            .finish();

        SamplePczt {
            bytes: pczt.serialize().unwrap(),
            seed: seed.to_vec(),
            ufvk_text,
            seed_fingerprint,
        }
    }
}

#[cfg(all(test, feature = "multi_coins", not(feature = "cypherpunk")))]
pub(crate) mod legacy_test_support {
    use alloc::{
        string::{String, ToString},
        vec,
        vec::Vec,
    };

    use ::pczt::roles::{creator::Creator, updater::Updater};
    use bitcoin::secp256k1::Secp256k1;
    use keystore::algorithms::{
        secp256k1::get_extended_public_key_by_seed, zcash::calculate_seed_fingerprint,
    };
    use rand_core::OsRng;
    use zcash_primitives::transaction::{
        builder::{BuildConfig, Builder, PcztResult},
        fees::zip317,
    };
    use zcash_vendor::{
        pczt::Pczt,
        transparent::{
            bundle as transparent,
            keys::{AccountPrivKey, IncomingViewingKey},
        },
        zcash_protocol::{
            consensus::{MainNetwork, NetworkUpgrade, Parameters},
            value::Zatoshis,
        },
        zip32,
    };

    pub(crate) struct LegacyTransparentSample {
        pub(crate) bytes: Vec<u8>,
        pub(crate) seed: Vec<u8>,
        pub(crate) seed_fingerprint: [u8; 32],
        pub(crate) xpub: String,
        pub(crate) input_pubkey: [u8; 33],
    }

    pub(crate) fn legacy_transparent_path_for_account(account_index: u32) -> Vec<u32> {
        vec![
            44 | zcash_vendor::bip32::ChildNumber::HARDENED_FLAG,
            133 | zcash_vendor::bip32::ChildNumber::HARDENED_FLAG,
            account_index | zcash_vendor::bip32::ChildNumber::HARDENED_FLAG,
            0,
            0,
        ]
    }

    pub(crate) fn legacy_transparent_pczt_with_input_derivation(
        bytes: &[u8],
        seed_fingerprint: [u8; 32],
        input_pubkey: [u8; 33],
        path: Vec<u32>,
    ) -> Vec<u8> {
        let derivation =
            zcash_vendor::transparent::pczt::Bip32Derivation::parse(seed_fingerprint, path)
                .unwrap();
        Updater::new(Pczt::parse(bytes).unwrap())
            .update_transparent_with(|mut bundle| {
                bundle.update_input_with(0, |mut input| {
                    input.set_bip32_derivation(input_pubkey, derivation);
                    Ok(())
                })
            })
            .unwrap()
            .finish()
            .serialize()
            .unwrap()
    }

    pub(crate) fn legacy_transparent_sample() -> LegacyTransparentSample {
        let params = MainNetwork;
        let seed = [7u8; 32];
        let account = AccountPrivKey::from_seed(&params, &seed, zip32::AccountId::ZERO).unwrap();
        let (input_addr, address_index) = account
            .to_account_pubkey()
            .derive_external_ivk()
            .unwrap()
            .default_address();
        let input_sk = account.derive_external_secret_key(address_index).unwrap();
        let secp = Secp256k1::signing_only();
        let input_pubkey = input_sk.public_key(&secp);

        let recipient_account =
            AccountPrivKey::from_seed(&params, &[8u8; 32], zip32::AccountId::ZERO).unwrap();
        let (recipient, _) = recipient_account
            .to_account_pubkey()
            .derive_external_ivk()
            .unwrap()
            .default_address();
        let transparent_recipient = recipient
            .to_zcash_address(MainNetwork.network_type())
            .encode();

        let coin = transparent::TxOut::new(
            Zatoshis::const_from_u64(1_000_000),
            input_addr.script().into(),
        );
        let mut builder = Builder::new(
            &params,
            params.activation_height(NetworkUpgrade::Nu5).unwrap(),
            BuildConfig::Standard {
                sapling_anchor: None,
                orchard_anchor: None,
                ironwood_anchor: None,
                orchard_pool_bundle_type: orchard::builder::BundleType::DEFAULT,
            },
        );
        builder
            .add_transparent_p2pkh_input(
                input_pubkey,
                transparent::OutPoint::new([1u8; 32], 1),
                coin,
            )
            .unwrap();
        builder
            .add_transparent_output(&recipient, Zatoshis::const_from_u64(990_000))
            .unwrap();

        let PcztResult { pczt_parts, .. } = builder
            .build_for_pczt(OsRng, &zip317::FeeRule::standard())
            .unwrap();
        let seed_fingerprint = calculate_seed_fingerprint(&seed).unwrap();
        let input_pubkey = input_pubkey.serialize();
        let pczt = Updater::new(Creator::build_from_parts(pczt_parts).unwrap())
            .update_transparent_with(|mut bundle| {
                let derivation = zcash_vendor::transparent::pczt::Bip32Derivation::parse(
                    seed_fingerprint,
                    legacy_transparent_path_for_account(0),
                )
                .unwrap();
                bundle.update_input_with(0, |mut input| {
                    input.set_bip32_derivation(input_pubkey, derivation);
                    Ok(())
                })?;
                bundle.update_output_with(0, |mut output| {
                    output.set_user_address(transparent_recipient.clone());
                    Ok(())
                })
            })
            .unwrap()
            .finish();

        let xpub = get_extended_public_key_by_seed(&seed, &"M/44'/133'/0'".into())
            .unwrap()
            .to_string();

        LegacyTransparentSample {
            bytes: pczt.serialize().unwrap(),
            seed: seed.to_vec(),
            seed_fingerprint,
            xpub,
            input_pubkey,
        }
    }
}
