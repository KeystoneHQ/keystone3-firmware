// checking logic for PCZT

use alloc::{format, string::ToString, vec};

use crate::errors::ZcashError;

#[cfg(feature = "cypherpunk")]
use zcash_vendor::{
    orchard::{self, keys::FullViewingKey, value::ValueSum, Address},
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
    let should_process_ironwood = super::pczt_should_process_ironwood(pczt);
    let verifier = Verifier::new(pczt.clone())
        .with_orchard(|bundle| {
            check_shielded_bundle(
                params,
                seed_fingerprint,
                account_index,
                ufvk,
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
    bundle: &orchard::pczt::Bundle,
    pool: ShieldedPool,
) -> Result<(), ZcashError> {
    let pool_label = pool.label();
    bundle.actions().iter().try_for_each(|action| {
        check_action(params, seed_fingerprint, account_index, ufvk, action, pool)?;
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

#[cfg(feature = "cypherpunk")]
// check orchard action
fn check_action<P: consensus::Parameters>(
    params: &P,
    seed_fingerprint: &[u8; 32],
    account_index: zip32::AccountId,
    ufvk: &UnifiedFullViewingKey,
    action: &orchard::pczt::Action,
    pool: ShieldedPool,
) -> Result<(), ZcashError> {
    let pool_label = pool.label();
    // Check `cv_net` first so we know that the `value` fields for both the spend and the
    // output are present and correct.
    action.verify_cv_net().map_err(|e| {
        ZcashError::InvalidPczt(format!("invalid cv_net in {pool_label} action: {e:?}"))
    })?;

    let fvk = ufvk.orchard().ok_or(ZcashError::InvalidDataError(
        "orchard fvk is not present".to_string(),
    ))?;
    check_action_spend(
        params,
        seed_fingerprint,
        account_index,
        fvk,
        action.spend(),
        pool,
    )?;
    check_action_output(params, ufvk, action, pool)?;
    Ok(())
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
    let pool_label = pool.label();
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
fn is_wallet_orchard_address(fvk: &FullViewingKey, address: &Address) -> bool {
    let external_ivk = fvk.to_ivk(zcash_vendor::zip32::Scope::External);
    let internal_ivk = fvk.to_ivk(zcash_vendor::zip32::Scope::Internal);

    external_ivk.diversifier_index(address).is_some()
        || internal_ivk.diversifier_index(address).is_some()
}

#[cfg(feature = "cypherpunk")]
// check output cmx and internal-ovk output ownership constraints
fn check_action_output<P: consensus::Parameters>(
    params: &P,
    ufvk: &UnifiedFullViewingKey,
    action: &orchard::pczt::Action,
    pool: ShieldedPool,
) -> Result<(), ZcashError> {
    let pool_label = pool.label();
    action
        .output()
        .verify_note_commitment(action.spend())
        .map_err(|e| ZcashError::InvalidPczt(format!("invalid {pool_label} action cmx: {e:?}")))?;

    let fvk = ufvk.orchard().ok_or(ZcashError::InvalidDataError(
        "orchard fvk is not present".to_string(),
    ))?;
    let external_ovk = fvk.to_ovk(zcash_vendor::zip32::Scope::External).clone();
    let internal_ovk = fvk.to_ovk(zcash_vendor::zip32::Scope::Internal).clone();
    let transparent_internal_ovk = ufvk
        .transparent()
        .map(|k| orchard::keys::OutgoingViewingKey::from(k.internal_ovk().as_bytes()));

    let mut keys = vec![(Some(external_ovk), false), (Some(internal_ovk), true)];
    if let Some(ovk) = transparent_internal_ovk {
        keys.push((Some(ovk), true));
    }

    for (vk, is_internal_ovk) in keys {
        if let Some((_, address, _)) =
            super::parse::decode_output_enc_ciphertext(action, vk.as_ref())?
        {
            if let Some(user_address) = action.output().user_address() {
                super::parse::validate_orchard_user_address(params, user_address, &address)?;
            }
            if is_internal_ovk && !is_wallet_orchard_address(fvk, &address) {
                return Err(ZcashError::InvalidPczt(format!(
                    "{pool_label} output was recoverable with an internal OVK but does not belong to this wallet"
                )));
            }
            break;
        }
    }

    if let (Some(user_address), Some(recipient)) =
        (action.output().user_address(), action.output().recipient())
    {
        super::parse::validate_orchard_user_address(params, user_address, recipient)?;
    }

    Ok(())
}
