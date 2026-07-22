pub mod check;
pub mod parse;
pub mod sign;
pub mod structs;

use alloc::{format, string::ToString, vec::Vec};
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
    validate_empty_orchard_protocol_bundle_balances(pczt)?;

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

fn validate_empty_orchard_protocol_bundle_balances(pczt: &Pczt) -> Result<(), ZcashError> {
    for (pool, bundle) in [("Orchard", pczt.orchard()), ("Ironwood", pczt.ironwood())] {
        if bundle.actions().is_empty() && bundle.value_sum().0 != 0 {
            return Err(ZcashError::InvalidPczt(format!(
                "{pool} value_sum must be zero when {pool} bundle is empty"
            )));
        }
    }

    Ok(())
}

/// Ensures every Orchard protocol action has a distinct randomized validating key.
///
/// Within each pool, spend authorization signatures cover the same transaction-wide
/// shielded sighash. Orchard and Ironwood use different sighashes, but honest construction
/// should not reuse an `rk` across either pool, so validate their union defensively.
#[cfg(feature = "cypherpunk")]
fn validate_distinct_orchard_protocol_rks(pczt: &Pczt) -> Result<(), ZcashError> {
    let actions = pczt
        .orchard()
        .actions()
        .iter()
        .chain(pczt.ironwood().actions().iter())
        .collect::<Vec<_>>();

    for i in 0..actions.len() {
        for j in (i + 1)..actions.len() {
            if actions[i].spend().rk() == actions[j].spend().rk() {
                return Err(ZcashError::InvalidPczt(
                    "duplicate Orchard or Ironwood action rk".to_string(),
                ));
            }
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
impl core::fmt::Display for ShieldedPool {
    fn fmt(&self, f: &mut core::fmt::Formatter<'_>) -> core::fmt::Result {
        match self {
            ShieldedPool::Orchard => f.write_str("Orchard"),
            ShieldedPool::Ironwood => f.write_str("Ironwood"),
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
    let Some((derivation_seed_fingerprint, derivation_path)) = derivation else {
        return Ok(None);
    };
    if derivation_seed_fingerprint != seed_fingerprint {
        return Ok(None);
    }

    let unsupported_path = || {
        crate::errors::ZcashError::InvalidPczt(alloc::format!(
            "unsupported {pool} spend ZIP 32 derivation path"
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

/// Returns whether a PCZT carries anything the transparent-only path cannot
/// handle: shielded content or an unknown transaction format. Transparent-only
/// v6 is supported by the shared ZIP 244 sighash implementation.
#[cfg(any(feature = "multi_coins", not(feature = "cypherpunk")))]
pub(crate) fn pczt_is_unsupported_by_transparent_only(pczt: &zcash_vendor::pczt::Pczt) -> bool {
    use zcash_vendor::zcash_protocol::{
        consensus::{BranchId, OrchardProtocolRevision},
        constants::{V5_TX_VERSION, V5_VERSION_GROUP_ID, V6_TX_VERSION, V6_VERSION_GROUP_ID},
    };

    let orchard_revision = BranchId::try_from(*pczt.global().consensus_branch_id())
        .ok()
        .and_then(|branch_id| branch_id.orchard_protocol_revision());
    let supported_transaction_format = match (
        *pczt.global().tx_version(),
        *pczt.global().version_group_id(),
        orchard_revision,
    ) {
        (V5_TX_VERSION, V5_VERSION_GROUP_ID, Some(_)) => true,
        (V6_TX_VERSION, V6_VERSION_GROUP_ID, Some(OrchardProtocolRevision::V3)) => true,
        _ => false,
    };

    !supported_transaction_format
        || !pczt.sapling().spends().is_empty()
        || !pczt.sapling().outputs().is_empty()
        || !pczt.orchard().actions().is_empty()
        || !pczt.ironwood().actions().is_empty()
}

#[cfg(all(test, feature = "cypherpunk"))]
mod consistency_tests {
    use alloc::{format, vec::Vec};

    use ::pczt::roles::creator::Creator;
    use serde::{Deserialize, Serialize};
    use zcash_vendor::zcash_protocol::consensus::{BranchId, MainNetwork, NetworkConstants};

    use super::*;

    #[derive(Serialize, Deserialize)]
    struct EmptyPcztWire {
        global: ::pczt::common::Global,
        transparent: Option<::pczt::transparent::Bundle>,
        sapling: Option<::pczt::sapling::Bundle>,
        orchard: Option<EmptyShieldedBundleWire>,
        ironwood: Option<EmptyShieldedBundleWire>,
    }

    #[derive(Serialize, Deserialize)]
    struct EmptyShieldedBundleWire {
        actions: Vec<()>,
        flags: u8,
        value_sum: (u64, bool),
        anchor: Option<[u8; 32]>,
        note_version: NoteVersionWire,
        zkproof: Option<Vec<u8>>,
        bsk: Option<[u8; 32]>,
    }

    #[derive(Serialize, Deserialize)]
    enum NoteVersionWire {
        V2,
        V3,
    }

    fn empty_bundle_with_nonzero_value_sum(pool: ShieldedPool) -> Vec<u8> {
        let branch_id = match pool {
            ShieldedPool::Orchard => BranchId::Nu6,
            ShieldedPool::Ironwood => BranchId::Nu6_3,
        };
        let bytes = Creator::new(branch_id.into(), 10, MainNetwork.coin_type(), None, None)
            .unwrap()
            .build()
            .unwrap()
            .serialize()
            .unwrap();
        let mut wire: EmptyPcztWire = postcard::from_bytes(&bytes[8..]).unwrap();
        let bundle = EmptyShieldedBundleWire {
            actions: Vec::new(),
            flags: match pool {
                ShieldedPool::Orchard => 0b0000_0011,
                ShieldedPool::Ironwood => 0b0000_0111,
            },
            value_sum: (1, false),
            anchor: None,
            note_version: match pool {
                ShieldedPool::Orchard => NoteVersionWire::V2,
                ShieldedPool::Ironwood => NoteVersionWire::V3,
            },
            zkproof: None,
            bsk: None,
        };
        match pool {
            ShieldedPool::Orchard => wire.orchard = Some(bundle),
            ShieldedPool::Ironwood => wire.ironwood = Some(bundle),
        }

        postcard::to_extend(&wire, bytes[..8].to_vec()).unwrap()
    }

    #[test]
    fn rejects_nonzero_value_sum_on_empty_orchard_protocol_bundles() {
        for pool in [ShieldedPool::Orchard, ShieldedPool::Ironwood] {
            let pczt = parse_pczt(&empty_bundle_with_nonzero_value_sum(pool)).unwrap();

            assert_eq!(
                validate_supported_pczt(&pczt),
                Err(ZcashError::InvalidPczt(format!(
                    "{pool} value_sum must be zero when {pool} bundle is empty"
                )))
            );
        }
    }
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
        sample_migration_pczt_with_options(0, 0, MemoBytes::empty(), None)
    }

    /// Builds a migration whose funded output carries the given memo.
    pub(crate) fn sample_migration_pczt_with_output_memo(output_memo: MemoBytes) -> SamplePczt {
        sample_migration_pczt_with_options(0, 0, output_memo, None)
    }

    /// Builds a migration whose funded output belongs to the given account.
    pub(crate) fn sample_migration_pczt_to_account(output_account: u32) -> SamplePczt {
        sample_migration_pczt_with_options(0, output_account, MemoBytes::empty(), None)
    }

    /// Builds a migration whose spend belongs to the given account while account 0 is selected.
    pub(crate) fn sample_migration_pczt_from_account(spend_account: u32) -> SamplePczt {
        sample_migration_pczt_with_options(spend_account, 0, MemoBytes::empty(), None)
    }

    /// Adds a zero-value output, optionally marked for ordinary display.
    pub(crate) fn sample_migration_pczt_with_zero_output(
        memo: MemoBytes,
        displayable: bool,
    ) -> SamplePczt {
        sample_migration_pczt_with_options(0, 0, MemoBytes::empty(), Some((memo, displayable)))
    }

    /// Builds a migration sample with a configurable funded recipient and optional zero output.
    fn sample_migration_pczt_with_options(
        spend_account: u32,
        output_account: u32,
        output_memo: MemoBytes,
        zero_output: Option<(MemoBytes, bool)>,
    ) -> SamplePczt {
        let params = Nu6_3Network;
        let seed = [7u8; 32];
        let ufvk_text = derive_ufvk(&params, &seed, "m/32'/133'/0'").unwrap();
        let ufvk = UnifiedFullViewingKey::decode(&params, &ufvk_text).unwrap();
        let selected_fvk = ufvk.orchard().unwrap().clone();
        let spend_ufvk_text = derive_ufvk(
            &params,
            &seed,
            &alloc::format!("m/32'/133'/{spend_account}'"),
        )
        .unwrap();
        let spend_ufvk = UnifiedFullViewingKey::decode(&params, &spend_ufvk_text).unwrap();
        let orchard_fvk = spend_ufvk.orchard().unwrap().clone();
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
        let orchard_ovk = selected_fvk.to_ovk(if spend_account == 0 && output_account == 0 {
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
                zip32::ChildIndex::hardened(spend_account).index(),
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

    /// Fixed v1 bytes that remain independent of changes to the current PCZT builders.
    pub(crate) fn sample_legacy_transparent_orchard_pczt() -> SamplePczt {
        const PCZT_HEX: &str = concat!(
            "50435a5401000000058ace9cb502b4a1db960c0100a8897a850100000101010101010101010101010101010101010101",
            "010101010101010101010101010000000000c0843d1976a914ba4ef3bbc4125b49a6abd7415b967c59e9b667fe88ac00",
            "000101023069edcd102ea24cb79c64fc6f28b599901bfbc1b4018fda1cb1644263469aff0f213b9facb6c1cf1b1884be",
            "c4380deffd25fe0a5f8cfac12249474cfaa07a5d05ac80808008858180800880808080080000000000000001b0b63c19",
            "76a914a4a95f46235f144e92ac1a7e2f601640fd25894388ac00000123743159744669796f35467442707a78327a7241",
            "586a52597336364556664432447743470000000000000000000000000000000000000000000000000000000000000000",
            "000000000002d7de77f6ffe88bc38e68fb0a754a8487344a12f5138492f1284b22092000501bca0d93ecc6298e39c882",
            "41e4788bcff09c8766e5cc643df29dfb4181d060cb2db3b123b6f1b8a19016b8df2daf7b0ba5fbe61055db093d216658",
            "b8a77126a18d0001f5520a1777474e15757f4a8cbb6b84bf926e24e8c12eda940a9cc69b99fc06cc84e87d0949cb12e2",
            "22101901c0843d01068957ba39a3428fda2844fc8c8e5cde846908c9291fa36a5a157ef6af45551801cb715a9c98b3f2",
            "bba68725aebd130616f2055571131313c33a30d4e67430957001fea7e5f33f92dbdaeb60f04893da113cbd058832f724",
            "b0a521f70c6b32e9e2095587d375f5772a54e29eac5f11352f3ead177237470370b3de8cfc331257681c109f29f7435b",
            "32df20eff2429ef6f617dcb07705d75701d63d3ea34cdfa9ef3e01000200000000000000000000000000000000000000",
            "000000000000000000000000d1ab2507c809c2713c000f525e9fbdcb06c958384e51b9cc7f792dde6c97f411c7413f46",
            "14cd64043abbab7cc1095c9bb104231cea89e2c3e0df83769556d0302111fc397753e5fd50ec74816df27d6ada7ed2a9",
            "ac3816aab2573c8fac794204806afbfeb45c64d4f2384c51eff30764b84599ae56a7ab3d4a46d9ce3aeab431873e4157",
            "f2c0f0c645e899360069fcc9d2ed9bc11bf59827af0230ed52edab1827ab1320953ae1ad70c8c15a1253a0a86fbc8a0a",
            "a36a84207293f8a495ffc4024e14563df191a2a65b4b37113b5230680555051b22d74a8e1f1d706f90f3133bb3bbe4f9",
            "93d18a0f4eb7f4174b1d8555ce3396855d04676f1ce4f06dda07371f4ef5bde9c6f0d76aeb9e27e93fba28c679dfcb99",
            "1cbcb8395a2b57924cbd170ea3c02568acebf5ca1ec30d6a7d7cd217a47d6a1b8311bf9462a5f939c6b743073ef9b30b",
            "ae6122da1605bad6ec5d49b41d4d40caa96c1cf6302b66c5d2d10d3922ae2800cb93abe63b70c172de70362d9830e538",
            "00398884a7a64ff68ed99e0b187110d92672c24cedb0979cdfc917a6053b310d145c031c7292bb1d65b7661b3f98adbe",
            "364f148b0cc2042cafc6be1166fae39090ab4b354bfb6217b964453b63f8dbd10df936f1734973e0b3bd25f4ed440566",
            "c923085903f696bc6347ec0f2182163eac4061885a313568148dfae564e478066dcbe389a0ddb1ecb7f5dc34bd9dc068",
            "1918a3f3f9cd1f9e06aa1ad68927da63acc13b92a2578b2738a6d331ca2ced953b7fb95e3ba986333da9e69cd355223c",
            "929731094b6c2174c7638d2e55354b96b56f9e45aae1e0094d71ee248dabf668117778bdc3c19ca5331a4e1a7097b04c",
            "2aa045a0deffcaca41c5ac92e694466578f5909e72bb78d33310f705e81d6821ff813bd410867a3f22e8e5cb7ac5599a",
            "610af5c354eb392877362e01157de8567f7c4996b8c4fdc94938fd808c3b2a5ccb79d1a63858adaa9a6dd824fe1fce51",
            "cd6120c12c124695c4f98b275918fceae6eb209873ed73fe73775d0b1f91982912012669f74d0cfa1030ff37b152324e",
            "5b8346b3335a0aaeb63a0a2d5dec15f52af17da3931396183cbbbfbea7ed950714540aec06c645c754975522e8ae2ad9",
            "1d463bab75ee941d33cc5817b613c63cda943a4c07f600591b088a25d53fdee371cef596766823f4a518a583b1158243",
            "afe89700f0da76da46d0060f15d2444cefe7914c9a61e829c730eceb216288fee825f6b3b6298f6f6b6bd62e4c57a617",
            "a0aa10ea7a83aa6b6b0ed685b6a3d9e5b8fd14f56cdc18021b12253f3fd4915c19bd831a7920be55d969b2ac23359e25",
            "59da77de2373f06ca014ba2787d063cd07ee4944222b7762840eb94c688bec743fa8bdf7715c8fe29f104c2a0139c304",
            "0591d5490335cb982852d9d9db581779db3141cf2198ababa1c15a172c010f213b9facb6c1cf1b1884bec4380deffd25",
            "fe0a5f8cfac12249474cfaa07a5d03a0808080088581808008808080800800002d97e62e15dd29c7d00a16f9d555e468",
            "37aaebcdaaefd7d341e142a9e45e2425ce7f12eac4afa1e7f6a807c2a552e88a2459655d57486ec1c6c5e739da72d22f",
            "c404003f915d95cd8e53c5fa9e4856f0b24bcb639132ddb981241079ed6e9cdfb2197b1bfcb234b23012bb5c7aaf134f",
            "1ce888360cbb8663acfce17a6c847fc033f630ff9c061bebb9da1732f30e7eb4af50d9f238428896831aa3a323593d34",
            "863eaf5e2fb1819eedc8ee8a1f280a9672f0a8970eb509ef0df107ce8a621331413ddecb3ac2e54501068e824e423a57",
            "374e92679e6898db5e76666c87bcae52c8b2b56b62c48b5639f638166426fcc12da61ef0d0be3ee2fca43fe608e30fc3",
            "121a043cb757cca2c743037800f30560d94395b5f41a7c7c783c3c7255b297ae99fe16c06f0a659da4ec6d67772e87dc",
            "d4623bbdf40c99d8c526e2605684c64fd30cdbcfa950ad82711011d2a781dd701957d860969093573eea6ca36b698c3e",
            "ad422c962a5dfb8967b71274fdbf36ab5134bb78df7bf89a72b339a0ef632a4b3361cfec2a8b8af9a8be3b2f8097b3f6",
            "bf873249cc2aa588b81ec39c733cb7f7e7bf69792781477a7ca429c20401785e6ffc2368c127a183589a3bcab8e58568",
            "0d9746e43161b83f066eda7e24da96e88f36161ddae9a6eb9a08b272efa14c106d100eaeefbf61671fdca859e91f3b01",
            "8600deef3469481c0cb4077ddb21dfa54f779d1b39a08d21255c9158be500acf976f0a4beef6ed62dbcf60339eff5d58",
            "a489a67ab3559dd28d0e92382f7bda1b1c599a3d07e9c4408b2b08734b8635791cb60d0bf16fa5b64a9b03a278a6a613",
            "17d115be42d48cd37ae395444d6966deec1a47e7728a43dbf296622a2db97dfbcd0fc76ecabd7d831a3b878de5b6dc8c",
            "5bb3c2f2135a5063bf5055326e1f61e81967fdebc92e26179a5eb596da2948badc09ca94ed0d73bdf9ed250c43d70ca1",
            "ebae520e069390ae527b3f02be5bb8052c6f4f968bc5aecb23d753b021dd9f7863a60bfa5a134101dae4e5dc1cc3ec8a",
            "427138f7d59e13b8c21da9b2efd152aaa83ac7dce3b7de048af411a1fff8c8b7fb4f2e01b8dd3c016a45a187bcc465a4",
            "5a2f8b2b63a5b3296f22b316bd24e7f0fa4eb44bff5e1dfd00000000017fbc144dafdd2423ce223ae359052b61085e8a",
            "8da4b3e4188c9eaf89e108bb217159134427cdc6afa5d0ed3d9729e7ac122011ce0805d8924974ea5a3381a02da01904",
            "d5a6b72ae0ee1c2a56c32f3a98ed2b1563bddf70e9d31d0d14c2c1bd0b9d73b752d50c4ccb9013c98fd228098e47640c",
            "cf96e33ddda1874ff4acac0e890001663a9335eaadf30de4a0c57917476142d8344fd30e78f9ffce3a5256bba83f4d84",
            "c7589964d089033c8f12010001d7a7f858ffb16f4124a57f5d7d5e3c75919efd26619421cdc116cb77ea36aa3d0176d7",
            "898d75e50d30ccb98c5616642a2ad7c908e0f43f44519885151afada70f10154c1c128b406f34be416ca8ddc52f0d4a7",
            "c7318e9eeece33265e30b256cf1a3c93cd916f299b1129571f506df32af6bce7f97690108926c3fcd6a10930733f2c10",
            "af50998bd7060533695bf10bd470f42333f2ce10648ad8a828281eab4e382401f0b093c30b86420dc27ace6a500c9c35",
            "859cf9a57403d9b1ddec048652912e6a30aea8583516e059921d3e38a823d7df257152a124743caa09eddab02e1169c9",
            "f8bd4a45140a7e56356a3b3a228fd9a5a730f871eba0f3983e1dc28a3bf2f88f3bf302760a999cce88e27151507b47dc",
            "0c79a5e6596c5c14a14461c5d7b9312bc73fb6192c772a9c1305df036f75ffb6f4d497d43e33ae80aab01f8e620e35da",
            "05dc3fa011e02240f8c6ab0b8cec024b342b6d21a6b35c37dbb396b7becdaf6b90614c233e65bd636063192181d7d9f9",
            "1843239b9d95ab5859f0abbddd3163bfc53215d10bc15a2c548359e6356e25f1f4f1422f8969a5618447776f56285ecb",
            "3c43fed11ec73926a555bb7e75e765f73f3820098044c6a084d8d883031e8db9e2fa9a6c0b9c43666027d0cef4765b02",
            "7c2481a5be19a05ae8d36d4e6cc32fadc995c9d50a0e5ada2cc387903a4e178261bbc168b0f0854194d7e851ba6ce5a9",
            "efaa6c842323272eaad5245f913d970c69ef20a3a581087b20dc9f16170b175df9e727662b09bb71dda5d918d6754767",
            "980a9475acebd5c301534c29f9b7d8c9e3c1085d3582e65c64240409161c096874a10381c06fcb14a9f5606734ca7237",
            "e7faa1f4016b8cc1dd24bb5628587bb328493a52f498a66963f59bffaf8c4ede0f2c0e30113ce58f6d550274e371b945",
            "c76cee8c06436453aa725ac7d94e2a109bf0fc360f88cf9be0b163fa582e56d6e582ea72acf68c7cdf98b7c099ecd365",
            "58fccf1a016180d515b4ed742267158a81661211bfcf64dbeca3601107635292e09877bd044e94149d0a9184a5ff060b",
            "8409a1cb4bdbff30bb5bffb350fa82674255e6a20849d99cf25cf31ccf5abea96de641f7509dc1daebcdd16ad08b96b9",
            "ed8d47cf1457c1ade6b892f23c3cbe0b924c761f19ec6bc7efa6924c893c3972f84eb0aa3dc7cd7b3d88b3e08be850c4",
            "9aa33a1e672dc7b204aacf7a81ac461df202b97a3170fdc972c6bb19902051df2db8d635b2c4e6ad62ef1d3dfb5da816",
            "a1a3b9c00e5c486e2881cce37261f34268277694bad8af2005f3f4e8cb7fd8eacc57cb3923f5538b448a5e8723cdb446",
            "d976c2451eb8cdeb6d402a341b512db0863ca6df16b501671b46eb4eed88fa0c72dc532d263eaedf4ae89ac334265d84",
            "8c830e73168813f5d05c16c2c60329bf5f288a20db2716d1017897aee19c23a558c4aeae1991eb4885c2e003ffc2834b",
            "df4f07e2e6708824f4903ef4d7595672c68b23a830ba20a81eead158627dccf045e10c78cd055e7f15006f029300a08f",
            "de4824a005066e3567b72245c547d2953121e334f54b9990543ddea0395ee997f26c0b8d30d9ed146b83122323a64cdb",
            "5bfa95c2b38d01365ad8412fdc67848a0123e6f73e207330ffb52bd38ce531eae1a2d978665fd89fc402e84ab78eb572",
            "40329de10001688234fd1d9fe781607ae78eedbe68e33ab9233f91e7f2392ed2bad56abb61380001eb7a0da0effe1eb5",
            "6108c8e542e9eb4ebf05740e3776a6caafe25c88b1245b6f0093128312cee979e30cad333190f0e73616924b250daf78",
            "8c528a7e954a491108801562a9acd5f569fee3a89c0472d14d705109366e019b786a1078278b429197c404735247d474",
            "f4fd3ca0c11a5d334d421d3e2b710c725290d5dd005da6eb83daa8c3c8a5cfe9aa2e69eea60170383207a1d1e8fbeb5c",
            "51e177b0b7849d8e3f73cdb931f37d89350617586ed88f7e993271388c749674a8d57ebdcbd688676cd1ec0f359f6f54",
            "df92ca61a93d61e1439a640a66e32f1429c3b7492308ba05dc5bf601c154f45389f236fb203b04f977a12695122334f4",
            "0785cf31464b3c16def65256c06678bb96cd07456577a6fa5f966a8b64d728492b7c3b944c5062f05197274e8065b7ef",
            "864ff7e0bef15476d3bd855d6cacae99c83a0330f66b42f3f3cce8859c28759aa52bd9456e95cd4a2589c95e8d82bdce",
            "726b128bc0865ba3913473b9beeacbc478e33aef2f5a529018c0611af2aaf77a6e2beeaad86b067d70d71a45c757b57c",
            "c82871790c5b70b23f40c6f73499ed3c5d68abe32009d618811b10f7f604df66460a7fe26fb0d4c3b771aa0ba763070c",
            "78997d6c712f5775e4092844876f5c7849d755e69283ca4788c38427570fbf956f4afbb42c0cc1c2bbc6e49660a1f4eb",
            "2e5d44183f52abd216d32db5dd5702801065ac6a8edd3f34ea25e16d084b379014946bd809bca84b1861df2d262001dc",
            "80b132a5a4cee03a5c7c8694cd383c3a89e928af8e70ff66162a43002fcd193c346ff64a4cd72a75478712151b06b78f",
            "ff0a32f3b4efb6639b68eae3a247defa7c16c55b7a38785544e18a50ad7eb378b29e27d4c447b51f7b2d03693622ed5d",
            "a3482e18673d22c8e5963491943122eb338538dba12dcf2e8b5bd69043e043beaadd0eb133bdca4454a648dd07235650",
            "3cc5776971573054c7863a384539afd5e65466cf682378518360e428d669d73a60f168fe1fff33abdb5602c1e088eb28",
            "d16a79144c089c230b167fcc953678ac3992df58119b0c378c806aa8bc2cd00701a492bd2f05b8c8147f431e5c6fc951",
            "7393f9f7527dd1a97d0ae104c31143e80940b6ea23a7c5293f109db5010001202811a564c8023a720cfeb8acf07e7a66",
            "95271230e12055ae9b1bd15ec7a7c10000000001541f53a50812ca83a8b61a9d7cc6e84839dfa1ffcf2d8b3b9266b228",
            "ad067214038827002078c89963306be0c1793873e73636a0371992af9d75489945d522591dbf98140000",
        );

        let params = MainNetwork;
        let seed = [7u8; 32];
        SamplePczt {
            bytes: hex::decode(PCZT_HEX).expect("hard-coded v1 PCZT fixture is valid hex"),
            seed: seed.to_vec(),
            ufvk_text: derive_ufvk(&params, &seed, "m/32'/133'/0'").unwrap(),
            seed_fingerprint: calculate_seed_fingerprint(&seed).unwrap(),
        }
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

#[cfg(all(test, any(feature = "multi_coins", feature = "cypherpunk")))]
pub(crate) mod legacy_test_support {
    #[cfg(feature = "multi_coins")]
    use alloc::string::{String, ToString};
    use alloc::{vec, vec::Vec};

    use ::pczt::roles::{creator::Creator, updater::Updater};
    use bitcoin::secp256k1::Secp256k1;
    #[cfg(feature = "multi_coins")]
    use keystore::algorithms::secp256k1::get_extended_public_key_by_seed;
    use keystore::algorithms::zcash::calculate_seed_fingerprint;
    use rand_core::OsRng;
    use zcash_primitives::transaction::{
        builder::{BuildConfig, Builder, PcztResult},
        fees::zip317,
    };
    #[cfg(feature = "multi_coins")]
    use zcash_vendor::pczt::Pczt;
    use zcash_vendor::{
        transparent::{
            bundle as transparent,
            keys::{AccountPrivKey, IncomingViewingKey},
        },
        zcash_protocol::{
            consensus::{BlockHeight, MainNetwork, NetworkType, NetworkUpgrade, Parameters},
            value::Zatoshis,
        },
        zip32,
    };

    pub(crate) struct LegacyTransparentSample {
        pub(crate) bytes: Vec<u8>,
        pub(crate) seed: Vec<u8>,
        pub(crate) seed_fingerprint: [u8; 32],
        #[cfg(feature = "multi_coins")]
        pub(crate) xpub: String,
        #[cfg(feature = "multi_coins")]
        pub(crate) input_pubkey: [u8; 33],
    }

    #[derive(Clone, Copy)]
    struct Nu6_3Network;

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

    pub(crate) fn legacy_transparent_path_for_account(account_index: u32) -> Vec<u32> {
        vec![
            44 | zcash_vendor::bip32::ChildNumber::HARDENED_FLAG,
            133 | zcash_vendor::bip32::ChildNumber::HARDENED_FLAG,
            account_index | zcash_vendor::bip32::ChildNumber::HARDENED_FLAG,
            0,
            0,
        ]
    }

    #[cfg(feature = "multi_coins")]
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
        let target_height = params.activation_height(NetworkUpgrade::Nu5).unwrap();
        transparent_sample(params, target_height)
    }

    pub(crate) fn legacy_transparent_v6_sample() -> LegacyTransparentSample {
        transparent_sample(Nu6_3Network, BlockHeight::from_u32(10))
    }

    fn transparent_sample<P: Parameters>(
        params: P,
        target_height: BlockHeight,
    ) -> LegacyTransparentSample {
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
        let transparent_recipient = recipient.to_zcash_address(params.network_type()).encode();

        let coin = transparent::TxOut::new(
            Zatoshis::const_from_u64(1_000_000),
            input_addr.script().into(),
        );
        let mut builder = Builder::new(
            &params,
            target_height,
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

        #[cfg(feature = "multi_coins")]
        let xpub = get_extended_public_key_by_seed(&seed, &"M/44'/133'/0'".into())
            .unwrap()
            .to_string();

        LegacyTransparentSample {
            bytes: pczt.serialize().unwrap(),
            seed: seed.to_vec(),
            seed_fingerprint,
            #[cfg(feature = "multi_coins")]
            xpub,
            #[cfg(feature = "multi_coins")]
            input_pubkey,
        }
    }
}
