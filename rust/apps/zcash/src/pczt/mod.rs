pub mod check;
pub mod parse;
pub mod sign;
pub mod structs;

use alloc::{format, string::ToString, vec::Vec};
// Shared imports consumed by the not-yet-refactored `parse`/`sign` submodules via `use super::*`.
// These move into those modules when they are refactored in a later commit.
use keystore::algorithms::secp256k1::get_public_key_by_seed;
use keystore::algorithms::zcash::calculate_seed_fingerprint;
#[cfg(feature = "cypherpunk")]
use keystore::algorithms::zcash::sign_message_orchard;
use zcash_vendor::{
    pczt::Pczt,
    transparent,
    zcash_protocol::value::ZatBalance,
    zip32,
};
#[cfg(feature = "cypherpunk")]
use zcash_vendor::orchard;

use crate::errors::ZcashError;

pub(crate) fn parse_pczt(bytes: &[u8]) -> Result<Pczt, ZcashError> {
    Pczt::parse(bytes).map_err(|_| ZcashError::InvalidPczt("invalid pczt data".to_string()))
}

pub(crate) fn validate_supported_pczt(pczt: &Pczt) -> Result<(), ZcashError> {
    validate_sapling_bundle_consistency(pczt)?;

    Ok(())
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

pub(crate) fn transparent_derivation_matches_selected_account<
    P: zcash_vendor::zcash_protocol::consensus::Parameters,
>(
    params: &P,
    seed_fingerprint: &[u8; 32],
    account_index: zip32::AccountId,
    xpub: &transparent::keys::AccountPubKey,
    pubkey: &[u8; 33],
    derivation: &transparent::pczt::Bip32Derivation,
    field_label: &str,
) -> Result<bool, ZcashError> {
    if seed_fingerprint != derivation.seed_fingerprint() {
        return Ok(false);
    }

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

    Ok(true)
}
