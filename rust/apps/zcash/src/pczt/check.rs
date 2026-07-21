// checking logic for PCZT

use alloc::{format, string::ToString, vec};

use crate::errors::ZcashError;

#[cfg(feature = "cypherpunk")]
use zcash_vendor::{
    orchard::{self, keys::FullViewingKey, value::ValueSum},
    zcash_keys::keys::UnifiedFullViewingKey,
};

use zcash_vendor::{
    pczt::{self, roles::verifier::Verifier, Pczt},
    ripemd::Ripemd160,
    sha2::{Digest, Sha256},
    transparent::{self, address::TransparentAddress, keys::AccountPubKey},
    zcash_address::{ToAddress, ZcashAddress},
    zcash_protocol::consensus::{self},
    zip32,
};

#[cfg(feature = "cypherpunk")]
use zcash_vendor::zcash_protocol::consensus::NetworkConstants;

#[cfg(feature = "cypherpunk")]
use super::parse::WalletKeys;
#[cfg(feature = "cypherpunk")]
use super::structs::{ParsedOrchard, ParsedTo};
#[cfg(feature = "cypherpunk")]
use super::ShieldedPool;

#[cfg(feature = "cypherpunk")]
fn map_orchard_verifier_error(
    error: pczt::roles::verifier::OrchardError<ZcashError>,
) -> ZcashError {
    match error {
        pczt::roles::verifier::OrchardError::Custom(error) => error,
        error => ZcashError::InvalidDataError(format!("{error:?}")),
    }
}

#[cfg(feature = "cypherpunk")]
pub fn check_pczt_orchard<P: consensus::Parameters>(
    params: &P,
    seed_fingerprint: &[u8; 32],
    account_index: zip32::AccountId,
    ufvk: &UnifiedFullViewingKey,
    pczt: &Pczt,
) -> Result<(), ZcashError> {
    super::validate_supported_pczt(pczt)?;
    let keys = WalletKeys::derive(ufvk)?;
    let should_process_ironwood = super::pczt_should_process_ironwood(pczt);
    let verifier = Verifier::new(pczt.clone())
        .with_orchard(|bundle| {
            check_shielded_bundle(
                params,
                seed_fingerprint,
                account_index,
                ufvk,
                &keys,
                bundle,
                ShieldedPool::Orchard,
            )
            .map_err(pczt::roles::verifier::OrchardError::Custom)?;
            Ok(())
        })
        .map_err(map_orchard_verifier_error)?;
    if should_process_ironwood {
        verifier
            .with_ironwood(|bundle| {
                check_shielded_bundle(
                    params,
                    seed_fingerprint,
                    account_index,
                    ufvk,
                    &keys,
                    bundle,
                    ShieldedPool::Ironwood,
                )
                .map_err(pczt::roles::verifier::OrchardError::Custom)?;
                Ok(())
            })
            .map_err(map_orchard_verifier_error)?;
    }
    Ok(())
}

/// Shielded display rows and checked actions produced by one validation pass.
/// A pool is `None` when it contains no displayable actions.
#[cfg(feature = "cypherpunk")]
pub(crate) struct CheckedShieldedParse {
    pub(crate) orchard: Option<ParsedOrchard>,
    pub(crate) ironwood: Option<ParsedOrchard>,
    pub(crate) checked_actions: alloc::vec::Vec<ShieldedAction>,
}

/// Identifies a checked Orchard or Ironwood action and retains the spend fields
/// needed to evaluate its signability after the remaining PCZT checks.
#[cfg(feature = "cypherpunk")]
pub(crate) struct ShieldedAction {
    pub(crate) pool: ShieldedPool,
    pub(crate) index: usize,
    pub(crate) spend_value: Option<u64>,
    pub(crate) spend_has_dummy_sk: bool,
    pub(crate) spend_derivation: Option<([u8; 32], alloc::vec::Vec<zip32::ChildIndex>)>,
}

/// Validates shielded actions while collecting their [`ParsedOrchard`] display
/// rows and the [`ShieldedAction`] values needed by the later batch policy. The
/// result feeds [`super::parse::parse_pczt_cypherpunk_with_checked_shielded`]
/// without another shielded bundle walk.
///
/// The caller supplies the batch's [`WalletKeys`] and must run
/// `validate_supported_pczt` first to preserve validation ordering. The
/// PCZT owned by the Verifier is returned with the checked actions for later
/// checks.
#[cfg(feature = "cypherpunk")]
pub(crate) fn check_and_parse_pczt_shielded<P: consensus::Parameters>(
    params: &P,
    seed_fingerprint: &[u8; 32],
    account_index: zip32::AccountId,
    ufvk: &UnifiedFullViewingKey,
    keys: &WalletKeys,
    pczt: Pczt,
) -> Result<(CheckedShieldedParse, Pczt), ZcashError> {
    let mut parsed_orchard = None;
    let mut checked_actions = alloc::vec::Vec::new();
    let mut parsed_ironwood = None;
    let should_process_ironwood = super::pczt_should_process_ironwood(&pczt);

    // Validate Orchard while collecting the rows shown during review.
    let verifier = Verifier::new(pczt)
        .with_orchard(|bundle| {
            parsed_orchard = check_and_parse_shielded_bundle(
                params,
                seed_fingerprint,
                account_index,
                ufvk,
                keys,
                bundle,
                ShieldedPool::Orchard,
                &mut checked_actions,
            )
            .map_err(pczt::roles::verifier::OrchardError::Custom)?;
            Ok(())
        })
        .map_err(map_orchard_verifier_error)?;

    // Continue through Ironwood when this transaction version enables it.
    let verifier = if should_process_ironwood {
        verifier
            .with_ironwood(|bundle| {
                parsed_ironwood = check_and_parse_shielded_bundle(
                    params,
                    seed_fingerprint,
                    account_index,
                    ufvk,
                    keys,
                    bundle,
                    ShieldedPool::Ironwood,
                    &mut checked_actions,
                )
                .map_err(pczt::roles::verifier::OrchardError::Custom)?;
                Ok(())
            })
            .map_err(map_orchard_verifier_error)?
    } else {
        verifier
    };

    // Return the verifier-owned PCZT for the remaining checks.
    Ok((
        CheckedShieldedParse {
            orchard: parsed_orchard,
            ironwood: parsed_ironwood,
            checked_actions,
        },
        verifier.finish(),
    ))
}

pub fn check_pczt_transparent<P: consensus::Parameters>(
    params: &P,
    seed_fingerprint: &[u8; 32],
    account_index: zip32::AccountId,
    xpub: &AccountPubKey,
    pczt: &Pczt,
    check_sfp: bool,
) -> Result<bool, ZcashError> {
    super::validate_supported_pczt(pczt)?;
    let mut has_my_input = false;
    Verifier::new(pczt.clone())
        .with_transparent(|bundle| {
            has_my_input = check_transparent(
                params,
                seed_fingerprint,
                account_index,
                xpub,
                bundle,
                check_sfp,
            )
            .map_err(pczt::roles::verifier::TransparentError::Custom)?;
            Ok(())
        })
        .map_err(|e| match e {
            pczt::roles::verifier::TransparentError::Custom(e) => e,
            _e => ZcashError::InvalidDataError(format!("{:?}", _e)),
        })?;
    Ok(has_my_input)
}

fn check_transparent<P: consensus::Parameters>(
    params: &P,
    seed_fingerprint: &[u8; 32],
    account_index: zip32::AccountId,
    xpub: &AccountPubKey,
    bundle: &transparent::pczt::Bundle,
    check_sfp: bool,
) -> Result<bool, ZcashError> {
    let mut has_my_input = false;
    bundle.inputs().iter().try_for_each(|input| {
        let _has = check_transparent_input(params, seed_fingerprint, account_index, xpub, input)?;
        if _has {
            has_my_input = true;
        }
        Ok::<_, ZcashError>(())
    })?;
    bundle.outputs().iter().try_for_each(|output| {
        check_transparent_output(params, seed_fingerprint, account_index, xpub, output)?;
        Ok::<_, ZcashError>(())
    })?;
    if check_sfp && !has_my_input {
        return Err(ZcashError::PcztNoMyInputs);
    }
    Ok(has_my_input)
}

fn check_transparent_input<P: consensus::Parameters>(
    params: &P,
    seed_fingerprint: &[u8; 32],
    account_index: zip32::AccountId,
    xpub: &AccountPubKey,
    input: &transparent::pczt::Input,
) -> Result<bool, ZcashError> {
    let script = input.script_pubkey().clone();
    // p2sh transparent input is not supported yet
    let Some(TransparentAddress::PublicKeyHash(hash)) =
        TransparentAddress::from_script_from_chain(&script)
    else {
        return Err(ZcashError::InvalidPczt(
            "transparent input script pubkey is not a public key hash".to_string(),
        ));
    };

    // 1: find my derivation (none means the input isn't ours, pass)
    input
        .bip32_derivation()
        .iter()
        .find(|(_pubkey, derivation)| seed_fingerprint == derivation.seed_fingerprint())
        .map_or(Ok(false), |(pubkey, derivation)| {
            // 2: derive my pubkey
            super::check_transparent_derivation(
                params,
                account_index,
                xpub,
                pubkey,
                derivation,
                "input",
            )?;
            // 3: check script pubkey
            if hash[..] != Ripemd160::digest(Sha256::digest(pubkey))[..] {
                return Err(ZcashError::InvalidPczt(
                    "transparent input script pubkey mismatch".to_string(),
                ));
            }
            Ok(true)
        })
}

fn check_transparent_output<P: consensus::Parameters>(
    params: &P,
    seed_fingerprint: &[u8; 32],
    account_index: zip32::AccountId,
    xpub: &AccountPubKey,
    output: &transparent::pczt::Output,
) -> Result<(), ZcashError> {
    let script = output.script_pubkey().clone();
    match TransparentAddress::from_script_pubkey(&script) {
        Some(TransparentAddress::PublicKeyHash(hash)) => {
            //check user_address and script_pubkey
            match output.user_address() {
                Some(user_address) => {
                    let ta =
                        ZcashAddress::from_transparent_p2pkh(params.network_type(), hash).encode();
                    if user_address != &ta {
                        return Err(ZcashError::InvalidPczt(
                            "transparent output user_address mismatch".to_string(),
                        ));
                    }
                }
                None => {
                    return Err(ZcashError::InvalidPczt(
                        "transparent output user_address is None".to_string(),
                    ))
                }
            }

            let pubkey = output
                .bip32_derivation()
                .keys()
                .find(|pubkey| hash[..] == Ripemd160::digest(Sha256::digest(pubkey))[..]);
            match pubkey {
                Some(pubkey) => {
                    match output.bip32_derivation().get(pubkey) {
                        Some(bip32_derivation) => {
                            if seed_fingerprint == bip32_derivation.seed_fingerprint() {
                                super::check_transparent_derivation(
                                    params,
                                    account_index,
                                    xpub,
                                    pubkey,
                                    bip32_derivation,
                                    "output",
                                )?;
                                Ok(())
                            } else {
                                //not my output, pass
                                Ok(())
                            }
                        }
                        //not my output, pass
                        None => Ok(()),
                    }
                }
                //not my output, pass
                None => Ok(()),
            }
        }
        Some(TransparentAddress::ScriptHash(hash)) => {
            //check user_address
            match output.user_address() {
                Some(user_address) => {
                    let ta =
                        ZcashAddress::from_transparent_p2sh(params.network_type(), hash).encode();
                    if user_address != &ta {
                        return Err(ZcashError::InvalidPczt(
                            "transparent output user_address mismatch".to_string(),
                        ));
                    }
                }
                None => {
                    return Err(ZcashError::InvalidPczt(
                        "transparent output user_address is None".to_string(),
                    ))
                }
            }
            // 1: find my derivation
            let my_derivation = output
                .bip32_derivation()
                .iter()
                .find(|(_pubkey, derivation)| seed_fingerprint == derivation.seed_fingerprint());
            match my_derivation {
                None => {
                    //not my output, pass
                    Ok(())
                }
                Some((pubkey, derivation)) => {
                    super::check_transparent_derivation(
                        params,
                        account_index,
                        xpub,
                        pubkey,
                        derivation,
                        "output",
                    )?;
                    // TODO: find a proper way to check script pubkey
                    Ok(())
                }
            }
        }
        _ => Err(ZcashError::InvalidPczt(
            "transparent output script pubkey is not a public key hash".to_string(),
        )),
    }
}

#[cfg(feature = "cypherpunk")]
// check orchard bundle
fn check_shielded_bundle<P: consensus::Parameters>(
    params: &P,
    seed_fingerprint: &[u8; 32],
    account_index: zip32::AccountId,
    ufvk: &UnifiedFullViewingKey,
    keys: &WalletKeys,
    bundle: &orchard::pczt::Bundle,
    pool: ShieldedPool,
) -> Result<(), ZcashError> {
    let pool_label = pool;
    let fvk = ufvk.orchard().ok_or(ZcashError::InvalidDataError(
        "orchard fvk is not present".to_string(),
    ))?;
    bundle.actions().iter().try_for_each(|action| {
        check_action(
            params,
            seed_fingerprint,
            account_index,
            fvk,
            keys,
            action,
            bundle.flags(),
            pool,
        )?;
        Ok::<_, ZcashError>(())
    })?;

    // At this point, we know that every `value` field in the Orchard bundle is present.
    // Check that `value_sum` is correct so we can use it for fee calculations later.
    let calculated_value_balance = bundle
        .actions()
        .iter()
        .map(|action| {
            action.spend().value().expect("present") - action.output().value().expect("present")
        })
        .sum::<Result<ValueSum, _>>();

    match calculated_value_balance {
        Ok(value_balance) if &value_balance == bundle.value_sum() => Ok(()),
        _ => Err(ZcashError::InvalidPczt(format!(
            "invalid {pool_label} bundle value balance"
        ))),
    }
}

/// Validates a shielded bundle while collecting its non-dummy display rows.
/// Returns `None` when the bundle contains no displayable actions.
#[cfg(feature = "cypherpunk")]
#[allow(clippy::too_many_arguments)]
fn check_and_parse_shielded_bundle<P: consensus::Parameters>(
    params: &P,
    seed_fingerprint: &[u8; 32],
    account_index: zip32::AccountId,
    ufvk: &UnifiedFullViewingKey,
    keys: &WalletKeys,
    bundle: &orchard::pczt::Bundle,
    pool: ShieldedPool,
    checked_actions: &mut alloc::vec::Vec<ShieldedAction>,
) -> Result<Option<ParsedOrchard>, ZcashError> {
    let pool_label = pool;
    let fvk = ufvk.orchard().ok_or(ZcashError::InvalidDataError(
        "orchard fvk is not present".to_string(),
    ))?;

    let mut parsed_orchard = ParsedOrchard::new(vec![], vec![]);
    // Validate and decode each action in the canonical order.
    bundle
        .actions()
        .iter()
        .enumerate()
        .try_for_each(|(index, action)| {
            let parsed_to = check_action(
                params,
                seed_fingerprint,
                account_index,
                fvk,
                keys,
                action,
                bundle.flags(),
                pool,
            )?;

            // Add only real spends to the review.
            if let Some(value) = action.spend().value() {
                if value.inner() != 0 {
                    let parsed_from =
                        super::parse::parse_orchard_spend(seed_fingerprint, action.spend())?;
                    parsed_orchard.add_from(parsed_from);
                }
            }

            if !parsed_to.get_is_dummy() {
                parsed_orchard.add_to(parsed_to);
            }

            checked_actions.push(ShieldedAction {
                pool,
                index,
                spend_value: action.spend().value().map(|value| value.inner()),
                spend_has_dummy_sk: action.spend().dummy_sk().is_some(),
                spend_derivation: action
                    .spend()
                    .zip32_derivation()
                    .as_ref()
                    .map(|derivation| {
                        (
                            *derivation.seed_fingerprint(),
                            derivation.derivation_path().clone(),
                        )
                    }),
            });

            Ok::<_, ZcashError>(())
        })?;

    // Recompute the bundle balance from the values just reviewed.
    let calculated_value_balance = bundle
        .actions()
        .iter()
        .map(|action| {
            action.spend().value().expect("present") - action.output().value().expect("present")
        })
        .sum::<Result<ValueSum, _>>();

    match calculated_value_balance {
        Ok(value_balance) if &value_balance == bundle.value_sum() => {
            if parsed_orchard.get_from().is_empty() && parsed_orchard.get_to().is_empty() {
                Ok(None)
            } else {
                Ok(Some(parsed_orchard))
            }
        }
        _ => Err(ZcashError::InvalidPczt(format!(
            "invalid {pool_label} bundle value balance"
        ))),
    }
}

#[cfg(feature = "cypherpunk")]
// check orchard action
fn check_action<P: consensus::Parameters>(
    params: &P,
    seed_fingerprint: &[u8; 32],
    account_index: zip32::AccountId,
    fvk: &FullViewingKey,
    keys: &WalletKeys,
    action: &orchard::pczt::Action,
    flags: &orchard::bundle::Flags,
    pool: ShieldedPool,
) -> Result<ParsedTo, ZcashError> {
    let pool_label = pool;
    // Check `cv_net` first so we know that the `value` fields for both the spend and the
    // output are present and correct.
    action.verify_cv_net().map_err(|e| {
        ZcashError::InvalidPczt(format!("invalid cv_net in {pool_label} action: {e:?}"))
    })?;

    check_action_spend(
        params,
        seed_fingerprint,
        account_index,
        fvk,
        action.spend(),
        pool,
    )?;
    check_action_output(params, keys, action, flags, pool)
}

#[cfg(feature = "cypherpunk")]
// check spend nullifier
fn check_action_spend<P: consensus::Parameters>(
    params: &P,
    seed_fingerprint: &[u8; 32],
    account_index: zip32::AccountId,
    fvk: &FullViewingKey,
    spend: &orchard::pczt::Spend,
    pool: ShieldedPool,
) -> Result<(), ZcashError> {
    let pool_label = pool;
    if let (Some(value), Some(zip32_derivation)) = (spend.value(), spend.zip32_derivation()) {
        if value.inner() != 0 && zip32_derivation.seed_fingerprint() == seed_fingerprint {
            let matched_account = super::matching_seed_supported_orchard_account(
                seed_fingerprint,
                Some(zip32_derivation),
                params.network_type().coin_type(),
                pool,
            )?;
            if matched_account != Some(account_index) {
                return Err(ZcashError::PcztNoMyInputs);
            }
        }
    }

    // We can only verify the `nullifier` and `rk` fields of a spend if we know its FVK.
    let can_verify_nf_rk = match (spend.value(), spend.fvk(), spend.zip32_derivation()) {
        // Dummy notes use randomly-generated FVKs, so if one is already present then
        // don't validate using the account's FVK.
        (Some(value), Some(_), _) if value.inner() == 0 => Some(None),
        // If the spend is marked as matching the selected account's FVK, verify with it.
        (Some(value), _, Some(zip32_derivation))
            if value.inner() != 0
                && zip32_derivation.seed_fingerprint() == seed_fingerprint
                && zip32_derivation.derivation_path()
                    == &[
                        zip32::ChildIndex::hardened(32),
                        zip32::ChildIndex::hardened(params.network_type().coin_type()),
                        account_index.into(),
                    ] =>
        {
            Some(Some(fvk))
        }
        // Don't verify `nullifier` or `rk` for spends that lack value data.
        _ => None,
    };

    if let Some(expected_fvk) = can_verify_nf_rk {
        spend.verify_nullifier(expected_fvk).map_err(|e| {
            ZcashError::InvalidPczt(format!("invalid {pool_label} action nullifier: {e:?}"))
        })?;
        spend.verify_rk(expected_fvk).map_err(|e| {
            ZcashError::InvalidPczt(format!("invalid {pool_label} action rk: {e:?}"))
        })?;
    }

    Ok(())
}

#[cfg(feature = "cypherpunk")]
fn check_action_output<P: consensus::Parameters>(
    params: &P,
    keys: &WalletKeys,
    action: &orchard::pczt::Action,
    flags: &orchard::bundle::Flags,
    pool: ShieldedPool,
) -> Result<ParsedTo, ZcashError> {
    let pool_label = pool;
    action
        .output()
        .verify_note_commitment(action.spend())
        .map_err(|e| ZcashError::InvalidPczt(format!("invalid {pool_label} action cmx: {e:?}")))?;

    // Decode and validate the recipient, rejecting non-zero outputs the device cannot review.
    let parsed_to = super::parse::parse_orchard_output(params, keys, action, pool)?;
    check_restricted_zero_value_output(keys, action, flags, pool)?;

    Ok(parsed_to)
}

#[cfg(feature = "cypherpunk")]
fn check_restricted_zero_value_output(
    keys: &WalletKeys,
    action: &orchard::pczt::Action,
    flags: &orchard::bundle::Flags,
    pool: ShieldedPool,
) -> Result<(), ZcashError> {
    // A restricted action binds its spend and output to the same expanded receiver.
    // A funded output paired with a zero-valued spend must therefore be ours.
    let is_zero_spend = matches!(action.spend().value(), Some(value) if value.inner() == 0);
    let is_funded_output = matches!(action.output().value(), Some(value) if value.inner() != 0);
    if flags.cross_address_enabled() || !is_zero_spend || !is_funded_output {
        return Ok(());
    }

    let recipient = action.output().recipient().ok_or_else(|| {
        ZcashError::InvalidPczt(format!("missing recipient for funded {} output", pool))
    })?;
    if !super::parse::is_wallet_orchard_address(keys, &recipient)? {
        return Err(ZcashError::InvalidPczt(format!(
            "funded {} output paired with a zero-value spend does not belong to the selected account",
            pool
        )));
    }

    Ok(())
}
