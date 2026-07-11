#![no_std]
extern crate alloc;

pub mod errors;
pub mod pczt;
pub mod version;

use errors::{Result, ZcashError};

use alloc::{
    string::{String, ToString},
    vec::Vec,
};
// The aggregate migration review is the only consumer of these; keep them off
// the non-cypherpunk build so it stays warning-free.
#[cfg(feature = "cypherpunk")]
use alloc::{format, vec};
use pczt::structs::ParsedPczt;
#[cfg(feature = "cypherpunk")]
use pczt::structs::{ParsedFrom, ParsedOrchard, ParsedTo};
use zcash_vendor::{
    zcash_keys::keys::{UnifiedAddressRequest, UnifiedFullViewingKey},
    zcash_protocol::consensus::{self},
    zip32,
};

#[cfg(any(test, feature = "multi_coins", feature = "cypherpunk"))]
use zcash_vendor::pczt::Pczt;

#[cfg(feature = "cypherpunk")]
use zcash_vendor::zcash_protocol::consensus::NetworkConstants;

/// Generates a Zcash address from a Unified Full Viewing Key (UFVK).
///
/// # Parameters
/// * `params` - The consensus parameters for the Zcash network (mainnet or testnet)
/// * `ufvk_text` - The string representation of the Unified Full Viewing Key
///
/// # Returns
/// * `Result<String>` - The encoded Zcash address if successful, or an error if the UFVK is invalid
///                      or if there was an issue generating the address
///
/// # Errors
/// * `ZcashError::GenerateAddressError` - If the UFVK cannot be decoded or if the address cannot be generated
pub fn get_address<P: consensus::Parameters>(params: &P, ufvk_text: &str) -> Result<String> {
    let ufvk = UnifiedFullViewingKey::decode(params, ufvk_text)
        .map_err(|e| ZcashError::GenerateAddressError(e.to_string()))?;
    let (address, _) = ufvk
        .default_address(UnifiedAddressRequest::AllAvailableKeys)
        .map_err(|e| ZcashError::GenerateAddressError(e.to_string()))?;
    Ok(address.encode(params))
}

/// Parses a PCZT, checks policy, and serializes it again in one pass.
///
/// # Parameters
/// * `params` - The consensus parameters for the Zcash network (mainnet or testnet)
/// * `pczt_bytes` - The binary representation of the PCZT to validate
/// * `ufvk_text` - The string representation of the Unified Full Viewing Key
/// * `seed_fingerprint` - A 32-byte fingerprint of the seed used to derive keys
/// * `account_index` - The account index for the keys to check against
///
/// # Returns
/// * `Result<Vec<u8>>` - The normalized encoding of the checked PCZT
///
/// # Errors
/// * `ZcashError::InvalidDataError` - If the UFVK cannot be decoded or the account index is invalid
/// * `ZcashError::InvalidPczt` - If the PCZT data is malformed or cannot be parsed
/// * Other errors from the underlying validation process
///
/// The returned bytes are what C retains as the `checked_PCZT`, which display
/// and signing consume without re-running these checks.
#[cfg(feature = "cypherpunk")]
pub fn check_pczt_cypherpunk<P: consensus::Parameters>(
    params: &P,
    pczt_bytes: &[u8],
    ufvk_text: &str,
    seed_fingerprint: &[u8; 32],
    account_index: u32,
) -> Result<Vec<u8>> {
    check_pczt_cypherpunk_with_policy(
        params,
        pczt_bytes,
        ufvk_text,
        seed_fingerprint,
        account_index,
        ShieldedActionPolicy::Single,
    )
}

/// Batch check for one `ZcashSignBatch` message: parses once, runs the full
/// policy checks, enforces the batch shielded-action policy (the PCZT must be
/// batch-signable by this account), and returns the normalized encoding. See
/// `check_pczt_cypherpunk` for the normalization contract.
#[cfg(feature = "cypherpunk")]
pub fn check_batch_pczt_cypherpunk<P: consensus::Parameters>(
    params: &P,
    pczt_bytes: &[u8],
    ufvk_text: &str,
    seed_fingerprint: &[u8; 32],
    account_index: u32,
) -> Result<Vec<u8>> {
    check_pczt_cypherpunk_with_policy(
        params,
        pczt_bytes,
        ufvk_text,
        seed_fingerprint,
        account_index,
        ShieldedActionPolicy::Batch,
    )
}

#[cfg(feature = "cypherpunk")]
fn check_pczt_cypherpunk_with_policy<P: consensus::Parameters>(
    params: &P,
    pczt_bytes: &[u8],
    ufvk_text: &str,
    seed_fingerprint: &[u8; 32],
    account_index: u32,
    policy: ShieldedActionPolicy,
) -> Result<Vec<u8>> {
    let mut pczt = pczt::parse_pczt(pczt_bytes)?;
    // Resolve compact field representations (memo-plaintext ciphertexts,
    // omitted cv_net) once, up front: the checks below then see complete
    // actions, and `serialize()` bakes the resolved values into the
    // normalized bytes so display and signing never re-resolve.
    pczt.resolve_fields().map_err(|e| {
        ZcashError::InvalidPczt(alloc::format!("resolve compact PCZT fields: {e:?}"))
    })?;

    let account_index = zip32::AccountId::try_from(account_index)
        .map_err(|_e| ZcashError::InvalidDataError("invalid account index".to_string()))?;
    let ufvk = UnifiedFullViewingKey::decode(params, ufvk_text)
        .map_err(|e| ZcashError::InvalidDataError(e.to_string()))?;
    let xpub = ufvk.transparent().ok_or(ZcashError::InvalidDataError(
        "transparent xpub is not present".to_string(),
    ))?;
    pczt::check::check_pczt_orchard(params, seed_fingerprint, account_index, &ufvk, &pczt)?;
    pczt::check::check_pczt_transparent(
        params,
        seed_fingerprint,
        account_index,
        xpub,
        &pczt,
        false,
    )?;

    let pczt = match policy {
        ShieldedActionPolicy::Single => pczt,
        ShieldedActionPolicy::Batch => {
            let (actions, pczt) =
                signable_shielded_actions(params, pczt, seed_fingerprint, account_index, policy)?;
            if actions.is_empty() {
                return Err(ZcashError::PcztNoMyInputs);
            }
            pczt
        }
    };

    pczt.serialize()
        .map_err(|e| ZcashError::InvalidPczt(alloc::format!("serialize normalized PCZT: {e:?}")))
}

/// Parses a multi-coins PCZT, checks policy, and serializes it again in one
/// pass.
///
/// Returns the normalized encoding of the checked PCZT. The returned bytes are
/// what C retains as the `checked_PCZT`, which display and signing consume
/// without re-running these checks.
#[cfg(feature = "multi_coins")]
pub fn check_pczt_multi_coins<P: consensus::Parameters>(
    params: &P,
    pczt_bytes: &[u8],
    xpub: &str,
    seed_fingerprint: &[u8; 32],
    account_index: u32,
) -> Result<Vec<u8>> {
    let pczt = pczt::parse_pczt(pczt_bytes)?;
    // FUTURE(omitted-field-recompute): recompute-or-check omitted fields here,
    // mutating `pczt` so the normalized bytes carry the verified values forward.
    // transparent-only build: pczt's orchard feature (and resolve_fields) is not compiled here.
    reject_legacy_check_unsupported_pczt(&pczt)?;
    let account_pubkey = transparent_account_pubkey_from_xpub(xpub)?;
    let account_index = zip32::AccountId::try_from(account_index)
        .map_err(|_e| ZcashError::InvalidDataError("invalid account index".to_string()))?;

    pczt::check::check_pczt_transparent(
        params,
        seed_fingerprint,
        account_index,
        &account_pubkey,
        &pczt,
        true,
    )?;
    pczt.serialize()
        .map_err(|e| ZcashError::InvalidPczt(alloc::format!("serialize normalized PCZT: {e:?}")))
}

#[cfg(feature = "multi_coins")]
fn transparent_account_pubkey_from_xpub(
    xpub: &str,
) -> Result<zcash_vendor::transparent::keys::AccountPubKey> {
    use core::str::FromStr;
    use zcash_vendor::{bip32, transparent};

    let xpub: bip32::ExtendedPublicKey<bitcoin::secp256k1::PublicKey> =
        bip32::ExtendedPublicKey::from_str(xpub)
            .map_err(|e| ZcashError::InvalidDataError(e.to_string()))?;

    let key = {
        let chain_code = xpub.attrs().chain_code;
        let pubkey = xpub.public_key().serialize();
        let mut bytes = [0u8; 65];
        bytes[..32].copy_from_slice(&chain_code);
        bytes[32..].copy_from_slice(&pubkey);
        bytes
    };

    transparent::keys::AccountPubKey::deserialize(&key)
        .map_err(|e| ZcashError::InvalidDataError(e.to_string()))
}

#[cfg(feature = "multi_coins")]
fn reject_legacy_check_unsupported_pczt(pczt: &Pczt) -> Result<()> {
    {
        // The legacy multi-coins check path only verifies transparent data. Reject any
        // shielded (Sapling/Orchard/Ironwood) or V6 PCZT so check, parse, and sign
        // enforce the same transparent-only boundary.
        if pczt::pczt_requires_cypherpunk_support(pczt) {
            return Err(ZcashError::InvalidPczt(
                "Shielded or V6 PCZTs require cypherpunk checking support".to_string(),
            ));
        }
    }
    Ok(())
}

/// Parses a Partially Created Zcash Transaction (PCZT) and extracts its details.
///
/// This function takes a binary PCZT and a Unified Full Viewing Key (UFVK), parses the transaction,
/// and returns a structured representation of the transaction's contents.
///
/// # Parameters
/// * `params` - The consensus parameters for the Zcash network (mainnet or testnet)
/// * `pczt` - The binary representation of the PCZT to parse
/// * `ufvk_text` - The string representation of the Unified Full Viewing Key
/// * `seed_fingerprint` - A 32-byte fingerprint of the seed used to derive keys
/// # Returns
/// * `Result<ParsedPczt>` - A structured representation of the PCZT if successful
///
/// # Errors
/// * `ZcashError::InvalidDataError` - If the UFVK cannot be decoded
/// * `ZcashError::InvalidPczt` - If the PCZT data is malformed or cannot be parsed
/// * Other errors from the underlying parsing process
#[cfg(feature = "cypherpunk")]
pub fn parse_pczt_cypherpunk<P: consensus::Parameters>(
    params: &P,
    pczt: &[u8],
    ufvk_text: &str,
    seed_fingerprint: &[u8; 32],
) -> Result<ParsedPczt> {
    let ufvk = UnifiedFullViewingKey::decode(params, ufvk_text)
        .map_err(|e| ZcashError::InvalidDataError(e.to_string()))?;
    let pczt = pczt::parse_pczt(pczt)?;
    pczt::parse::parse_pczt_cypherpunk(params, seed_fingerprint, &ufvk, &pczt)
}

/// Validates a batch PCZT for the selected account and returns its display data.
#[cfg(feature = "cypherpunk")]
pub fn check_and_parse_batch_pczt_cypherpunk<P: consensus::Parameters>(
    params: &P,
    pczt_bytes: &[u8],
    ufvk_text: &str,
    seed_fingerprint: &[u8; 32],
    account_index: u32,
) -> Result<ParsedPczt> {
    let mut pczt = pczt::parse_pczt(pczt_bytes)?;
    // Resolve compact field representations up front so the single-pass check
    // sees complete actions, matching the standalone batch check.
    pczt.resolve_fields().map_err(|e| {
        ZcashError::InvalidPczt(alloc::format!("resolve compact PCZT fields: {e:?}"))
    })?;
    let account_index = zip32::AccountId::try_from(account_index)
        .map_err(|_e| ZcashError::InvalidDataError("invalid account index".to_string()))?;
    let ufvk = UnifiedFullViewingKey::decode(params, ufvk_text)
        .map_err(|e| ZcashError::InvalidDataError(e.to_string()))?;
    let xpub = ufvk.transparent().ok_or(ZcashError::InvalidDataError(
        "transparent xpub is not present".to_string(),
    ))?;

    // Validate shielded actions while collecting their display rows.
    let (checked_shielded, pczt) = pczt::check::check_and_parse_pczt_shielded(
        params,
        seed_fingerprint,
        account_index,
        &ufvk,
        pczt,
    )?;

    // Check the remaining transparent bundle against the same account.
    pczt::check::check_pczt_transparent(
        params,
        seed_fingerprint,
        account_index,
        xpub,
        &pczt,
        false,
    )?;

    // Reuse the PCZT returned by signability validation for display assembly.
    let (signable_actions, pczt) = signable_shielded_actions(
        params,
        pczt,
        seed_fingerprint,
        account_index,
        ShieldedActionPolicy::Batch,
    )?;

    if signable_actions.is_empty() {
        Err(ZcashError::PcztNoMyInputs)
    } else {
        // Assemble the display from the shielded rows collected above.
        pczt::parse::parse_pczt_cypherpunk_with_checked_shielded(
            params,
            seed_fingerprint,
            &pczt,
            checked_shielded.orchard,
            checked_shielded.ironwood,
        )
    }
}

/// Values for one checked migration child, in zatoshis.
#[cfg(feature = "cypherpunk")]
#[derive(Clone, Copy, Debug, Eq, PartialEq)]
struct BatchMigrationChildSummary {
    input: u64,
    output: u64,
    fee: u64,
}

/// Totals and per-child rows for the compact migration review.
#[cfg(feature = "cypherpunk")]
#[derive(Clone, Debug, Default, Eq, PartialEq)]
struct BatchMigrationSummary {
    migrations: u32,
    total_input: u64,
    total_output: u64,
    total_fee: u64,
    children: Vec<BatchMigrationChildSummary>,
}

#[cfg(feature = "cypherpunk")]
impl BatchMigrationSummary {
    /// Adds a checked child summary using checked arithmetic.
    fn add_child(&mut self, child: &BatchMigrationSummary) -> Result<()> {
        self.migrations = self
            .migrations
            .checked_add(child.migrations)
            .ok_or_else(|| ZcashError::InvalidPczt("migration count overflow".to_string()))?;
        self.total_input = self
            .total_input
            .checked_add(child.total_input)
            .ok_or_else(|| ZcashError::InvalidPczt("migration input overflow".to_string()))?;
        self.total_output = self
            .total_output
            .checked_add(child.total_output)
            .ok_or_else(|| ZcashError::InvalidPczt("migration output overflow".to_string()))?;
        self.total_fee = self
            .total_fee
            .checked_add(child.total_fee)
            .ok_or_else(|| ZcashError::InvalidPczt("migration fee overflow".to_string()))?;
        if child.children.is_empty() {
            self.children.push(BatchMigrationChildSummary {
                input: child.total_input,
                output: child.total_output,
                fee: child.total_fee,
            });
        } else {
            self.children.extend(child.children.iter().copied());
        }
        Ok(())
    }

    /// Builds the display model for the compact migration review.
    fn to_parsed_pczt(&self) -> ParsedPczt {
        let children = if self.children.is_empty() {
            vec![BatchMigrationChildSummary {
                input: self.total_input,
                output: self.total_output,
                fee: self.total_fee,
            }]
        } else {
            self.children.clone()
        };

        let orchard = ParsedOrchard::new(
            children
                .iter()
                .enumerate()
                .map(|(index, child)| {
                    ParsedFrom::new(
                        Some(format!(
                            "Migration #{} Orchard note from selected account",
                            index + 1
                        )),
                        pczt::parse::format_zec_value(child.input as f64),
                        child.input,
                        true,
                    )
                })
                .collect(),
            Vec::new(),
        );
        let ironwood = ParsedOrchard::new(
            Vec::new(),
            children
                .iter()
                .enumerate()
                .map(|(index, child)| {
                    ParsedTo::new(
                        format!("Migration #{} wallet Ironwood output", index + 1),
                        pczt::parse::format_zec_value(child.output as f64),
                        child.output,
                        true,
                        false,
                        None,
                    )
                })
                .collect(),
        );

        ParsedPczt::new(
            None,
            Some(orchard),
            Some(ironwood),
            pczt::parse::format_zec_value(self.total_output as f64),
            pczt::parse::format_zec_value(self.total_fee as f64),
            false,
        )
    }
}

/// Requires an action value to be present, returning it (zero is a valid
/// value; callers classify zero themselves).
#[cfg(feature = "cypherpunk")]
fn require_action_value(value: Option<u64>, label: &str) -> Result<u64> {
    value.ok_or_else(|| ZcashError::InvalidPczt(format!("missing {label} value")))
}

/// Validates the funded migration shape and computes its totals.
#[cfg(feature = "cypherpunk")]
fn summarize_migration_actions(
    ufvk: &UnifiedFullViewingKey,
    pczt: &Pczt,
) -> Result<BatchMigrationSummary> {
    use zcash_vendor::pczt::roles::verifier::{OrchardError, Verifier};

    // Reject transparent components at the wire level so their values cannot be
    // omitted from the fee.
    if !pczt.transparent().inputs().is_empty() {
        return Err(ZcashError::InvalidPczt(
            "migration summary does not support transparent inputs".to_string(),
        ));
    }
    if !pczt.transparent().outputs().is_empty() {
        return Err(ZcashError::InvalidPczt(
            "migration summary does not support transparent outputs".to_string(),
        ));
    }

    let mut orchard_spends = 0u32;
    let mut orchard_outputs = 0u32;
    let mut ironwood_spends = 0u32;
    let mut ironwood_outputs = 0u32;
    let mut total_input = 0u64;
    let mut total_output = 0u64;

    let map_verifier_error = |error: OrchardError<ZcashError>| match error {
        OrchardError::Custom(error) => error,
        error => ZcashError::InvalidDataError(format!("{error:?}")),
    };

    // Values are read through the Verifier's parsed view; the wire structs of
    // the pinned pczt revision expose no spend-value getter.
    let verifier = Verifier::new(pczt.clone())
        .with_orchard(|bundle| {
            for action in bundle.actions().iter() {
                let spend_value = require_action_value(
                    action.spend().value().map(|v| v.inner()),
                    "Orchard spend",
                )
                .map_err(OrchardError::Custom)?;
                if spend_value != 0 {
                    orchard_spends = orchard_spends.checked_add(1).ok_or_else(|| {
                        OrchardError::Custom(ZcashError::InvalidPczt(
                            "Orchard spend count overflow".to_string(),
                        ))
                    })?;
                    total_input = total_input.checked_add(spend_value).ok_or_else(|| {
                        OrchardError::Custom(ZcashError::InvalidPczt(
                            "migration input overflow".to_string(),
                        ))
                    })?;
                }

                let output_value = require_action_value(
                    action.output().value().map(|v| v.inner()),
                    "Orchard output",
                )
                .map_err(OrchardError::Custom)?;
                if output_value != 0 {
                    orchard_outputs = orchard_outputs.checked_add(1).ok_or_else(|| {
                        OrchardError::Custom(ZcashError::InvalidPczt(
                            "Orchard output count overflow".to_string(),
                        ))
                    })?;
                }
            }
            Ok(())
        })
        .map_err(map_verifier_error)?;

    verifier
        .with_ironwood(|bundle| {
            for action in bundle.actions().iter() {
                let spend_value = require_action_value(
                    action.spend().value().map(|v| v.inner()),
                    "Ironwood spend",
                )
                .map_err(OrchardError::Custom)?;
                if spend_value != 0 {
                    ironwood_spends = ironwood_spends.checked_add(1).ok_or_else(|| {
                        OrchardError::Custom(ZcashError::InvalidPczt(
                            "Ironwood spend count overflow".to_string(),
                        ))
                    })?;
                }

                let output_value = require_action_value(
                    action.output().value().map(|v| v.inner()),
                    "Ironwood output",
                )
                .map_err(OrchardError::Custom)?;
                if output_value == 0 {
                    continue;
                }

                let recipient = action.output().recipient().ok_or_else(|| {
                    OrchardError::Custom(ZcashError::InvalidPczt(
                        "missing Ironwood output recipient".to_string(),
                    ))
                })?;
                if !pczt::parse::is_wallet_orchard_address(ufvk, &recipient)
                    .map_err(OrchardError::Custom)?
                {
                    return Err(OrchardError::Custom(ZcashError::InvalidPczt(
                        "migration Ironwood output is not wallet-owned".to_string(),
                    )));
                }

                ironwood_outputs = ironwood_outputs.checked_add(1).ok_or_else(|| {
                    OrchardError::Custom(ZcashError::InvalidPczt(
                        "Ironwood output count overflow".to_string(),
                    ))
                })?;
                total_output = total_output.checked_add(output_value).ok_or_else(|| {
                    OrchardError::Custom(ZcashError::InvalidPczt(
                        "migration output overflow".to_string(),
                    ))
                })?;
            }
            Ok(())
        })
        .map_err(map_verifier_error)?;

    if orchard_spends != 1 || orchard_outputs != 0 || ironwood_spends != 0 || ironwood_outputs != 1
    {
        return Err(ZcashError::InvalidPczt(format!(
            "unsupported migration summary shape orchard_spends={orchard_spends} orchard_outputs={orchard_outputs} ironwood_spends={ironwood_spends} ironwood_outputs={ironwood_outputs}"
        )));
    }

    let total_fee = total_input
        .checked_sub(total_output)
        .ok_or_else(|| ZcashError::InvalidPczt("migration output exceeds input".to_string()))?;

    Ok(BatchMigrationSummary {
        migrations: 1,
        total_input,
        total_output,
        total_fee,
        children: vec![BatchMigrationChildSummary {
            input: total_input,
            output: total_output,
            fee: total_fee,
        }],
    })
}

/// Summarizes one checked Orchard-to-Ironwood migration child.
///
/// The caller must pass normalized bytes produced by the batch check. This
/// rejects any detail the compact review cannot display, including funded
/// Orchard outputs, funded Ironwood spends, and memos on the funded output.
#[cfg(feature = "cypherpunk")]
fn summarize_batch_migration_pczt_cypherpunk<P: consensus::Parameters>(
    params: &P,
    pczt: &[u8],
    ufvk_text: &str,
    seed_fingerprint: &[u8; 32],
) -> Result<BatchMigrationSummary> {
    let ufvk = UnifiedFullViewingKey::decode(params, ufvk_text)
        .map_err(|e| ZcashError::InvalidDataError(e.to_string()))?;
    let pczt = pczt::parse_pczt(pczt)?;
    // Reuse ordinary parsing so the summary has the same recovery and display checks.
    let parsed = pczt::parse::parse_pczt_cypherpunk(params, seed_fingerprint, &ufvk, &pczt)?;
    require_migration_display_shape(&parsed)?;
    summarize_migration_actions(&ufvk, &pczt)
}

/// Parses the first PCZT normally and aggregates later migration children.
///
/// All inputs must be normalized bytes produced by the batch check. Returns an
/// error when the compact representation cannot be built.
#[cfg(feature = "cypherpunk")]
pub fn parse_batch_with_migration_summary_cypherpunk<'a, P: consensus::Parameters>(
    params: &P,
    first_pczt: &[u8],
    migration_pczts: impl IntoIterator<Item = &'a [u8]>,
    ufvk_text: &str,
    seed_fingerprint: &[u8; 32],
) -> Result<Vec<ParsedPczt>> {
    let first = parse_pczt_cypherpunk(params, first_pczt, ufvk_text, seed_fingerprint)?;
    let mut summary = BatchMigrationSummary::default();
    let mut has_migrations = false;
    for child_pczt in migration_pczts {
        has_migrations = true;
        let child = summarize_batch_migration_pczt_cypherpunk(
            params,
            child_pczt,
            ufvk_text,
            seed_fingerprint,
        )?;
        summary.add_child(&child)?;
    }
    if !has_migrations {
        return Err(ZcashError::InvalidPczt(
            "migration review has no child transactions".to_string(),
        ));
    }

    Ok(vec![first, summary.to_parsed_pczt()])
}

/// Rejects children whose ordinary review contains details the compact review
/// would hide. The accepted shape has one Orchard spend row, one funded
/// Ironwood output without a memo, and no transparent or Sapling components.
#[cfg(feature = "cypherpunk")]
fn require_migration_display_shape(parsed: &ParsedPczt) -> Result<()> {
    let reject = |what: &str| {
        Err(ZcashError::InvalidPczt(format!(
            "migration summary cannot represent {what}; use the per-message review"
        )))
    };

    // A migration child must be shielded-only Orchard→Ironwood. A transparent
    // bundle or any Sapling component is invisible in the amounts-only summary
    // (a transparent input would additionally understate the displayed fee), so
    // fall back to the per-message review, which displays them. `get_transparent`
    // is `Some` only for a non-empty bundle, so shielded-only children pass.
    if parsed.get_transparent().is_some() {
        return reject("transparent components");
    }
    if parsed.get_has_sapling() {
        return reject("Sapling components");
    }

    let no_memo = |to: &ParsedTo| matches!(to.get_memo().as_deref(), None | Some(""));

    let orchard = parsed.get_orchard();
    let ironwood = parsed.get_ironwood();
    let orchard_from = orchard
        .as_ref()
        .map(|rows| rows.get_from())
        .unwrap_or_default();
    let orchard_to = orchard
        .as_ref()
        .map(|rows| rows.get_to())
        .unwrap_or_default();
    let ironwood_from = ironwood
        .as_ref()
        .map(|rows| rows.get_from())
        .unwrap_or_default();
    let ironwood_to = ironwood
        .as_ref()
        .map(|rows| rows.get_to())
        .unwrap_or_default();

    if orchard_from.len() != 1 || !ironwood_from.is_empty() {
        return reject("this spend shape");
    }
    // The migrated note is the only Orchard row; a displayable Orchard output
    // (beyond builder dummies, which the row pass already drops) means the
    // per-message review had something to show.
    if !orchard_to.is_empty() {
        return reject("Orchard outputs");
    }
    match ironwood_to.as_slice() {
        [only] if only.get_amount() != 0 && no_memo(only) => Ok(()),
        [only] if only.get_amount() != 0 => reject("an output memo"),
        _ => reject("this output shape"),
    }
}

#[cfg(test)]
mod additional_tests {
    use super::*;
    use zcash_vendor::zcash_protocol::consensus::MAIN_NETWORK;

    #[cfg(feature = "cypherpunk")]
    #[test]
    fn test_get_address() {
        let ufvk_text = "uview10zf3gnxd08cne6g7ryh6lln79duzsayg0qxktvyc3l6uutfk0agmyclm5g82h5z0lqv4c2gzp0eu0qc0nxzurxhj4ympwn3gj5c3dc9g7ca4eh3q09fw9kka7qplzq0wnauekf45w9vs4g22khtq57sc8k6j6s70kz0rtqlyat6zsjkcqfrlm9quje8vzszs8y9mjvduf7j2vx329hk2v956g6svnhqswxfp3n760mw233w7ffgsja2szdhy5954hsfldalf28wvav0tctxwkmkgrk43tq2p7sqchzc6";
        let addr = get_address(&MAIN_NETWORK, ufvk_text).expect("should generate address");
        // We can print this address to see what it is, and then pin it in the test.
        // For now, let's just assert it is valid and not empty.
        assert!(!addr.is_empty());
        assert!(addr.starts_with("u1")); // Mainnet unified address starts with u1
    }

    #[test]
    fn test_get_address_invalid_ufvk() {
        let ufvk_text = "invalid_ufvk";
        let result = get_address(&MAIN_NETWORK, ufvk_text);
        assert!(result.is_err());
    }
}

#[cfg(feature = "multi_coins")]
pub fn parse_pczt_multi_coins<P: consensus::Parameters>(
    params: &P,
    pczt: &[u8],
    seed_fingerprint: &[u8; 32],
) -> Result<ParsedPczt> {
    let pczt = pczt::parse_pczt(pczt)?;

    pczt::parse::parse_pczt_multi_coins(params, seed_fingerprint, &pczt)
}

/// Signs a Partially Created Zcash Transaction (PCZT) using a seed.
///
/// This function takes a binary PCZT and a seed, parses the transaction,
/// and returns a signed PCZT.
///
/// # Parameters
/// * `pczt` - The binary representation of the PCZT to sign
/// * `seed` - The seed to sign the PCZT with
///
/// # Returns
/// * `Result<Vec<u8>>` - The signed PCZT if successful, or an error otherwise
///
/// # Errors
/// * `ZcashError::InvalidPczt` - If the PCZT data is malformed or cannot be parsed
/// * Other errors from the underlying signing process
pub fn sign_pczt(pczt: &[u8], seed: &[u8]) -> Result<Vec<u8>> {
    let pczt = pczt::parse_pczt(pczt)?;
    pczt::sign::sign_pczt(pczt, seed)
}

#[cfg(all(test, feature = "multi_coins", not(feature = "cypherpunk")))]
mod legacy_tests {
    use super::*;
    use zcash_vendor::{
        pczt::roles::creator::Creator,
        zcash_protocol::consensus::{BranchId, MainNetwork, NetworkConstants},
    };

    fn assert_invalid_pczt_message<T: core::fmt::Debug>(result: Result<T>, expected: &str) {
        match result {
            Err(ZcashError::InvalidPczt(message)) if message == expected => {}
            other => panic!("unexpected InvalidPczt result: {other:?}"),
        }
    }

    #[test]
    fn legacy_parse_uses_seed_fingerprint_and_check_validates_transparent_account() {
        let sample = pczt::legacy_test_support::legacy_transparent_sample();

        let parsed = parse_pczt_multi_coins(&MainNetwork, &sample.bytes, &sample.seed_fingerprint)
            .expect("selected account PCZT should parse");
        assert!(parsed
            .get_transparent()
            .unwrap()
            .get_from()
            .first()
            .unwrap()
            .get_is_mine());
        check_pczt_multi_coins(
            &MainNetwork,
            &sample.bytes,
            &sample.xpub,
            &sample.seed_fingerprint,
            0,
        )
        .expect("selected account PCZT should check");

        let normalized = check_pczt_multi_coins(
            &MainNetwork,
            &sample.bytes,
            &sample.xpub,
            &sample.seed_fingerprint,
            0,
        )
        .expect("selected account PCZT should pass the check");
        assert!(
            parse_pczt_multi_coins(&MainNetwork, &normalized, &sample.seed_fingerprint).is_ok()
        );

        let account_one_pczt =
            pczt::legacy_test_support::legacy_transparent_pczt_with_input_derivation(
                &sample.bytes,
                sample.seed_fingerprint,
                sample.input_pubkey,
                pczt::legacy_test_support::legacy_transparent_path_for_account(1),
            );

        parse_pczt_multi_coins(&MainNetwork, &account_one_pczt, &sample.seed_fingerprint)
            .expect("parse uses seed fingerprint ownership only");
        assert_invalid_pczt_message(
            check_pczt_multi_coins(
                &MainNetwork,
                &account_one_pczt,
                &sample.xpub,
                &sample.seed_fingerprint,
                0,
            ),
            "transparent input bip32 derivation path invalid",
        );
    }

    #[test]
    fn legacy_check_rejects_v6_pczt() {
        let pczt = Creator::new(
            BranchId::Nu6_3.into(),
            10,
            MainNetwork.coin_type(),
            [0; 32],
            [0; 32],
        )
        .unwrap()
        .build();

        let result = check_pczt_multi_coins(
            &MainNetwork,
            &pczt.serialize().unwrap(),
            "not-an-xpub",
            &[7u8; 32],
            0,
        );

        assert!(matches!(
            result,
            Err(ZcashError::InvalidPczt(msg))
                if msg == "Shielded or V6 PCZTs require cypherpunk checking support"
        ));
    }
}

#[cfg(feature = "cypherpunk")]
fn map_shielded_verifier_error(
    e: zcash_vendor::pczt::roles::verifier::OrchardError<ZcashError>,
) -> ZcashError {
    use zcash_vendor::pczt::roles::verifier::OrchardError;

    match e {
        OrchardError::Custom(e) => e,
        _ => ZcashError::InvalidPczt(alloc::format!("{e:?}")),
    }
}

#[cfg(feature = "cypherpunk")]
#[derive(Clone, Copy, Debug, Eq, PartialEq)]
enum SignableShieldedPool {
    Orchard,
    Ironwood,
}

#[cfg(feature = "cypherpunk")]
impl SignableShieldedPool {
    fn label(self) -> &'static str {
        match self {
            SignableShieldedPool::Orchard => "Orchard",
            SignableShieldedPool::Ironwood => "Ironwood",
        }
    }

    fn shielded_pool(self) -> pczt::ShieldedPool {
        match self {
            SignableShieldedPool::Orchard => pczt::ShieldedPool::Orchard,
            SignableShieldedPool::Ironwood => pczt::ShieldedPool::Ironwood,
        }
    }
}

#[cfg(feature = "cypherpunk")]
#[derive(Clone, Copy, Debug, Eq, PartialEq)]
struct SignableShieldedAction {
    pool: SignableShieldedPool,
    index: usize,
}

#[cfg(feature = "cypherpunk")]
#[derive(Clone, Copy, Debug, Eq, PartialEq)]
enum ShieldedActionPolicy {
    Batch,
    Single,
}

#[cfg(feature = "cypherpunk")]
fn reject_unsupported_batch_pczt(pczt: &Pczt) -> Result<()> {
    if !pczt.sapling().spends().is_empty() || !pczt.sapling().outputs().is_empty() {
        return Err(ZcashError::InvalidPczt(
            "Zcash batch PCZT must not contain Sapling spends or outputs".to_string(),
        ));
    }

    if !pczt.transparent().inputs().is_empty() {
        return Err(ZcashError::InvalidPczt(
            "Zcash batch PCZT must not contain transparent inputs".to_string(),
        ));
    }

    Ok(())
}

#[cfg(feature = "cypherpunk")]
fn collect_signable_shielded_actions<P: consensus::Parameters>(
    params: &P,
    bundle: &zcash_vendor::orchard::pczt::Bundle,
    pool: SignableShieldedPool,
    seed_fingerprint: &[u8; 32],
    account_index: zip32::AccountId,
    policy: ShieldedActionPolicy,
    actions: &mut Vec<SignableShieldedAction>,
) -> core::result::Result<(), zcash_vendor::pczt::roles::verifier::OrchardError<ZcashError>> {
    use zcash_vendor::pczt::roles::verifier::OrchardError;

    for (index, action) in bundle.actions().iter().enumerate() {
        if action.spend().dummy_sk().is_some() {
            continue;
        }

        let value = action.spend().value().ok_or_else(|| {
            OrchardError::Custom(ZcashError::InvalidPczt(alloc::format!(
                "missing {} spend value",
                pool.label(),
            )))
        })?;
        if value.inner() == 0 {
            continue;
        }

        let matches_account = pczt::matching_seed_supported_orchard_account(
            seed_fingerprint,
            action.spend().zip32_derivation().as_ref(),
            params.network_type().coin_type(),
            pool.shielded_pool(),
        )
        .map_err(OrchardError::Custom)?
            == Some(account_index);
        if !matches_account {
            if policy == ShieldedActionPolicy::Batch {
                return Err(OrchardError::Custom(ZcashError::PcztNoMyInputs));
            }
            continue;
        }

        actions.push(SignableShieldedAction { pool, index });
    }

    Ok(())
}

#[cfg(feature = "cypherpunk")]
fn ensure_actions_are_signed(
    bundle: &zcash_vendor::orchard::pczt::Bundle,
    pool: SignableShieldedPool,
    signable_actions: &[SignableShieldedAction],
) -> core::result::Result<(), zcash_vendor::pczt::roles::verifier::OrchardError<ZcashError>> {
    use zcash_vendor::pczt::roles::verifier::OrchardError;

    for action_ref in signable_actions.iter().filter(|action| action.pool == pool) {
        let action = bundle.actions().get(action_ref.index).ok_or_else(|| {
            OrchardError::Custom(ZcashError::SigningError(alloc::format!(
                "signed PCZT is missing an {} action",
                pool.label(),
            )))
        })?;
        if action.spend().spend_auth_sig().is_none() {
            return Err(OrchardError::Custom(ZcashError::SigningError(
                alloc::format!(
                    "signed PCZT is missing an {} spend authorization signature",
                    pool.label(),
                ),
            )));
        }
    }

    Ok(())
}

#[cfg(feature = "cypherpunk")]
fn signable_shielded_actions<P: consensus::Parameters>(
    params: &P,
    pczt: Pczt,
    seed_fingerprint: &[u8; 32],
    account_index: zip32::AccountId,
    policy: ShieldedActionPolicy,
) -> Result<(Vec<SignableShieldedAction>, Pczt)> {
    use zcash_vendor::pczt::roles::verifier::Verifier;

    if policy == ShieldedActionPolicy::Batch {
        reject_unsupported_batch_pczt(&pczt)?;
    }

    let should_process_ironwood = pczt::pczt_should_process_ironwood(&pczt);
    let mut actions = Vec::new();
    let verifier = Verifier::new(pczt)
        .with_orchard::<ZcashError, _>(|bundle| {
            collect_signable_shielded_actions(
                params,
                bundle,
                SignableShieldedPool::Orchard,
                seed_fingerprint,
                account_index,
                policy,
                &mut actions,
            )
        })
        .map_err(map_shielded_verifier_error)?;

    let verifier = if should_process_ironwood {
        verifier
            .with_ironwood::<ZcashError, _>(|bundle| {
                collect_signable_shielded_actions(
                    params,
                    bundle,
                    SignableShieldedPool::Ironwood,
                    seed_fingerprint,
                    account_index,
                    policy,
                    &mut actions,
                )
            })
            .map_err(map_shielded_verifier_error)?
    } else {
        verifier
    };
    let pczt = verifier.finish();

    Ok((actions, pczt))
}

#[cfg(feature = "cypherpunk")]
fn ensure_shielded_actions_are_signed(
    signed_pczt: Pczt,
    signable_actions: &[SignableShieldedAction],
) -> Result<Pczt> {
    use zcash_vendor::pczt::roles::verifier::Verifier;

    let should_process_ironwood = pczt::pczt_should_process_ironwood(&signed_pczt);
    let verifier = Verifier::new(signed_pczt)
        .with_orchard::<ZcashError, _>(|bundle| {
            ensure_actions_are_signed(bundle, SignableShieldedPool::Orchard, signable_actions)
        })
        .map_err(map_shielded_verifier_error)?;

    let verifier = if should_process_ironwood {
        verifier
            .with_ironwood::<ZcashError, _>(|bundle| {
                ensure_actions_are_signed(bundle, SignableShieldedPool::Ironwood, signable_actions)
            })
            .map_err(map_shielded_verifier_error)?
    } else {
        verifier
    };

    Ok(verifier.finish())
}

/// Signs a checked, normalized PCZT and confirms in memory that every
/// supported shielded action owned by (`seed_fingerprint`, `account_index`)
/// received a spend authorization signature. Single-transaction policy: a PCZT
/// with no owned shielded action still signs if any action matched the seed.
/// Parses `checked_pczt` exactly once and returns the redacted, version-stamped
/// response bytes.
#[cfg(feature = "cypherpunk")]
pub fn sign_checked_pczt<P: consensus::Parameters>(
    params: &P,
    checked_pczt: &[u8],
    seed: &[u8],
    seed_fingerprint: &[u8; 32],
    account_index: u32,
) -> Result<Vec<u8>> {
    sign_checked_pczt_with_policy(
        params,
        checked_pczt,
        seed,
        seed_fingerprint,
        account_index,
        ShieldedActionPolicy::Single,
    )
}

/// Signs a checked, normalized PCZT and confirms in memory that every
/// supported shielded action owned by (`seed_fingerprint`, `account_index`)
/// received a spend authorization signature. Batch policy: additionally rejects
/// PCZT shapes the batch flow does not support and requires at least one owned
/// signable shielded action. Parses `checked_pczt` exactly once and returns the
/// redacted, version-stamped response bytes.
#[cfg(feature = "cypherpunk")]
pub fn sign_checked_batch_pczt<P: consensus::Parameters>(
    params: &P,
    checked_pczt: &[u8],
    seed: &[u8],
    seed_fingerprint: &[u8; 32],
    account_index: u32,
) -> Result<Vec<u8>> {
    sign_checked_pczt_with_policy(
        params,
        checked_pczt,
        seed,
        seed_fingerprint,
        account_index,
        ShieldedActionPolicy::Batch,
    )
}

#[cfg(feature = "cypherpunk")]
fn sign_checked_pczt_with_policy<P: consensus::Parameters>(
    params: &P,
    checked_pczt: &[u8],
    seed: &[u8],
    seed_fingerprint: &[u8; 32],
    account_index: u32,
    policy: ShieldedActionPolicy,
) -> Result<Vec<u8>> {
    let pczt = pczt::parse_pczt(checked_pczt)?;
    let account_index = zip32::AccountId::try_from(account_index)
        .map_err(|_e| ZcashError::InvalidDataError("invalid account index".to_string()))?;
    let (signable_actions, pczt) =
        signable_shielded_actions(params, pczt, seed_fingerprint, account_index, policy)?;
    if policy == ShieldedActionPolicy::Batch && signable_actions.is_empty() {
        return Err(ZcashError::PcztNoMyInputs);
    }
    let signed = pczt::sign::sign_and_redact_pczt(pczt, seed)?;
    let signed = if signable_actions.is_empty() {
        signed
    } else {
        ensure_shielded_actions_are_signed(signed, &signable_actions)?
    };
    signed
        .serialize()
        .map_err(|e| ZcashError::SigningError(alloc::format!("serialize signed PCZT: {e:?}")))
}

#[cfg(feature = "cypherpunk")]
#[cfg(test)]
mod tests {
    use alloc::{collections::BTreeMap, string::String, vec::Vec};

    use consensus::MainNetwork;
    use keystore::algorithms::zcash::{calculate_seed_fingerprint, derive_ufvk};
    use serde::{Deserialize, Serialize};
    use zcash_vendor::zcash_protocol::constants;

    use super::*;
    extern crate std;

    // Test-only decoder for the v2 postcard prefix these fixtures mutate. The
    // trailing Orchard and Ironwood bundles use private wire types, so callers
    // preserve those bytes unchanged via `postcard::take_from_bytes`.
    #[derive(Serialize, Deserialize)]
    struct PcztWirePrefix {
        global: GlobalMirror,
        transparent: Option<::pczt::transparent::Bundle>,
        sapling: Option<SaplingBundleMirror>,
    }

    #[derive(Serialize, Deserialize)]
    struct GlobalMirror {
        tx_version: u32,
        version_group_id: u32,
        consensus_branch_id: u32,
        fallback_lock_time: Option<u32>,
        expiry_height: u32,
        coin_type: u32,
        tx_modifiable: u8,
        proprietary: BTreeMap<String, Vec<u8>>,
    }

    #[derive(Serialize, Deserialize)]
    struct SaplingBundleMirror {
        spends: Vec<SaplingSpendMirror>,
        outputs: Vec<SaplingOutputMirror>,
        value_sum: i128,
        anchor: [u8; 32],
        bsk: Option<[u8; 32]>,
    }

    #[derive(Serialize, Deserialize)]
    struct SaplingSpendMirror;

    #[serde_with::serde_as]
    #[derive(Serialize, Deserialize)]
    struct SaplingOutputMirror {
        cv: [u8; 32],
        cmu: [u8; 32],
        ephemeral_key: [u8; 32],
        enc_ciphertext: Vec<u8>,
        out_ciphertext: Vec<u8>,
        #[serde_as(as = "Option<[_; 144]>")]
        zkproof: Option<[u8; 144]>,
        #[serde_as(as = "Option<[_; 43]>")]
        recipient: Option<[u8; 43]>,
        value: Option<u64>,
        rseed: Option<[u8; 32]>,
        rcv: Option<[u8; 32]>,
        ock: Option<[u8; 32]>,
        zip32_derivation: Option<Zip32DerivationMirror>,
        user_address: Option<String>,
        proprietary: BTreeMap<String, Vec<u8>>,
    }

    #[derive(Serialize, Deserialize)]
    struct Zip32DerivationMirror {
        seed_fingerprint: [u8; 32],
        derivation_path: Vec<u32>,
    }

    fn v5_pczt_with_ironwood_actions() -> Vec<u8> {
        let sample = pczt::test_support::sample_ironwood_pczt();
        let bytes = sample.bytes;
        assert!(!::pczt::Pczt::parse(&bytes)
            .unwrap()
            .ironwood()
            .actions()
            .is_empty());
        let (mut prefix, rest) = postcard::take_from_bytes::<PcztWirePrefix>(&bytes[8..]).unwrap();

        prefix.global.tx_version = constants::V5_TX_VERSION;
        prefix.global.version_group_id = constants::V5_VERSION_GROUP_ID;

        let mut out = bytes[..8].to_vec();
        out = postcard::to_extend(&prefix, out).unwrap();
        out.extend_from_slice(rest);
        out
    }

    fn assert_invalid_pczt_message<T: core::fmt::Debug>(result: Result<T>, expected: &str) {
        assert_eq!(
            result.unwrap_err(),
            ZcashError::InvalidPczt(expected.to_string())
        );
    }

    fn malformed_pczt_with_empty_sapling_bundle_and_nonzero_value_sum() -> Vec<u8> {
        use ::pczt::roles::creator::Creator;
        use zcash_vendor::zcash_protocol::consensus::{BranchId, NetworkConstants};

        let bytes = Creator::new(
            BranchId::Nu6.into(),
            10,
            MainNetwork.coin_type(),
            [0; 32],
            [0; 32],
        )
        .unwrap()
        .build()
        .serialize()
        .unwrap();
        let (mut prefix, rest) = postcard::take_from_bytes::<PcztWirePrefix>(&bytes[8..]).unwrap();
        // v2 omits empty bundles, so the freshly created PCZT has no Sapling bundle.
        // Attach one that is empty except for a non-zero value sum, which `check` rejects.
        assert!(prefix.sapling.is_none());
        prefix.sapling = Some(SaplingBundleMirror {
            spends: Vec::new(),
            outputs: Vec::new(),
            value_sum: 1,
            anchor: [0u8; 32],
            bsk: None,
        });

        let mut out = bytes[..8].to_vec();
        out = postcard::to_extend(&prefix, out).unwrap();
        out.extend_from_slice(rest);
        out
    }

    /// A PCZT whose Sapling bundle is empty but declares a non-zero value sum is malformed
    /// and must be rejected before signing.
    #[test]
    fn test_check_pczt_rejects_empty_sapling_bundle_with_nonzero_value_sum() {
        let seed = [9u8; 32];
        let malformed_pczt = malformed_pczt_with_empty_sapling_bundle_and_nonzero_value_sum();
        let ufvk = derive_ufvk(&MainNetwork, &seed, "m/32'/133'/0'").unwrap();
        let seed_fingerprint = calculate_seed_fingerprint(&seed).unwrap();

        let result =
            check_pczt_cypherpunk(&MainNetwork, &malformed_pczt, &ufvk, &seed_fingerprint, 0);

        assert_invalid_pczt_message(
            result,
            "sapling value_sum must be zero when Sapling bundle is empty",
        );
    }

    /// Regression test for internal-OVK change spoofing.
    ///
    /// An Orchard output paid to a non-wallet recipient but encrypted with the wallet's
    /// *internal* OVK must be rejected: otherwise funds leaving the wallet could be displayed
    /// (and signed) as if they were the user's own change. Both the parse path (what the user
    /// sees) and the check path (pre-sign validation) must reject it.
    #[test]
    fn test_parse_pczt_rejects_orchard_internal_ovk_change_spoofing() {
        use ::pczt::roles::creator::Creator;
        use bitcoin::secp256k1::Secp256k1;
        use rand_core::OsRng;
        use zcash_primitives::transaction::{
            builder::{BuildConfig, Builder, PcztResult},
            fees::zip317,
        };
        use zcash_vendor::{
            orchard,
            transparent::{
                bundle as transparent,
                keys::{AccountPrivKey, IncomingViewingKey},
            },
            zcash_protocol::{memo::MemoBytes, value::Zatoshis},
            zip32,
        };

        let params = MainNetwork;

        let victim_seed = [7u8; 32];
        let ufvk_text = derive_ufvk(&params, &victim_seed, "m/32'/133'/0'").unwrap();
        let ufvk = UnifiedFullViewingKey::decode(&params, &ufvk_text).unwrap();
        let victim_fvk = ufvk.orchard().unwrap().clone();

        let victim_account =
            AccountPrivKey::from_seed(&params, &victim_seed, zip32::AccountId::ZERO).unwrap();
        let (victim_addr, address_index) = victim_account
            .to_account_pubkey()
            .derive_external_ivk()
            .unwrap()
            .default_address();
        let victim_sk = victim_account
            .derive_external_secret_key(address_index)
            .unwrap();
        let secp = Secp256k1::signing_only();
        let victim_pubkey = victim_sk.public_key(&secp);

        // Attacker-controlled Orchard recipient that does NOT belong to the victim wallet.
        let attacker_sk = orchard::keys::SpendingKey::from_bytes([2; 32]).unwrap();
        let attacker_fvk = orchard::keys::FullViewingKey::from(&attacker_sk);
        let attacker_recipient = attacker_fvk.address_at(0u32, orchard::keys::Scope::External);
        let victim_change = victim_fvk.address_at(0u32, orchard::keys::Scope::Internal);

        let coin = transparent::TxOut::new(
            Zatoshis::const_from_u64(1_000_000),
            victim_addr.script().into(),
        );
        let mut builder = Builder::new(
            &params,
            10_000_000.into(),
            BuildConfig::Standard {
                sapling_anchor: None,
                orchard_anchor: Some(orchard::Anchor::empty_tree()),
                ironwood_anchor: None,
                orchard_pool_bundle_type: orchard::builder::BundleType::DEFAULT,
            },
        );
        builder
            .add_transparent_p2pkh_input(
                victim_pubkey,
                transparent::OutPoint::new([1u8; 32], 0),
                coin,
            )
            .unwrap();
        // Pay the attacker, but encrypt the output with the victim's INTERNAL ovk (the spoof).
        builder
            .add_orchard_output::<zip317::FeeRule>(
                Some(victim_fvk.to_ovk(orchard::keys::Scope::Internal)),
                attacker_recipient,
                Zatoshis::const_from_u64(100_000),
                MemoBytes::empty(),
            )
            .unwrap();
        // A genuine internal-ovk change output back to the victim.
        builder
            .add_orchard_output::<zip317::FeeRule>(
                Some(victim_fvk.to_ovk(orchard::keys::Scope::Internal)),
                victim_change,
                Zatoshis::const_from_u64(885_000),
                MemoBytes::empty(),
            )
            .unwrap();

        let PcztResult { pczt_parts, .. } = builder
            .build_for_pczt(OsRng, &zip317::FeeRule::standard())
            .unwrap();
        let pczt_bytes = Creator::build_from_parts(pczt_parts)
            .unwrap()
            .serialize()
            .unwrap();
        let seed_fingerprint = calculate_seed_fingerprint(&victim_seed).unwrap();

        let expected =
            "output was recoverable with an internal OVK but does not belong to this wallet";

        match parse_pczt_cypherpunk(&params, &pczt_bytes, &ufvk_text, &seed_fingerprint) {
            Err(ZcashError::InvalidPczt(msg)) if msg.contains(expected) => {}
            other => panic!("parse must reject internal-OVK change spoofing, got: {other:?}"),
        }

        match check_pczt_cypherpunk(&params, &pczt_bytes, &ufvk_text, &seed_fingerprint, 0) {
            Err(ZcashError::InvalidPczt(msg)) if msg.contains(expected) => {}
            other => panic!("check must reject internal-OVK change spoofing, got: {other:?}"),
        }
    }

    #[test]
    fn test_check_rejects_foreign_restricted_orchard_output() {
        let sample = pczt::test_support::sample_orchard_foreign_change_pczt();
        let expected =
            "funded Orchard output paired with a zero-value spend does not belong to the selected account";

        for result in [
            check_pczt_cypherpunk(
                &pczt::test_support::Nu6_3Network,
                &sample.bytes,
                &sample.ufvk_text,
                &sample.seed_fingerprint,
                0,
            )
            .map(|_| ()),
            check_and_parse_batch_pczt_cypherpunk(
                &pczt::test_support::Nu6_3Network,
                &sample.bytes,
                &sample.ufvk_text,
                &sample.seed_fingerprint,
                0,
            )
            .map(|_| ()),
        ] {
            match result {
                Err(ZcashError::InvalidPczt(message)) if message == expected => {}
                other => panic!("check must reject foreign restricted output, got: {other:?}"),
            }
        }
    }

    #[test]
    fn test_get_address() {
        let address = get_address(&MainNetwork, "uview1s2e0495jzhdarezq4h4xsunfk4jrq7gzg22tjjmkzpd28wgse4ejm6k7yfg8weanaghmwsvc69clwxz9f9z2hwaz4gegmna0plqrf05zkeue0nevnxzm557rwdkjzl4pl4hp4q9ywyszyjca8jl54730aymaprt8t0kxj8ays4fs682kf7prj9p24dnlcgqtnd2vnskkm7u8cwz8n0ce7yrwx967cyp6dhkc2wqprt84q0jmwzwnufyxe3j0758a9zgk9ssrrnywzkwfhu6ap6cgx3jkxs3un53n75s3");
        assert_eq!(address.unwrap(), "u1tqdskj32l9udfp0rysmca6gpz73fdqc2rmeenyhh0nfrq4vgak284ehkxefw5cf9495rdur0tparuntevp6nnetzjkyzv08m524e4swwk94asas7hm2ad5w5c64zz00hmr7nux0yhaz");
    }

    #[test]
    fn test_pczt_ironwood_to_ironwood() {
        let sample = pczt::test_support::sample_ironwood_pczt();
        let seed_fingerprint = sample.seed_fingerprint;
        let parsed_pczt = parse_pczt_cypherpunk(
            &pczt::test_support::Nu6_3Network,
            &sample.bytes,
            &sample.ufvk_text,
            &seed_fingerprint,
        )
        .unwrap();

        assert!(parsed_pczt.get_ironwood().is_some());
        assert!(parsed_pczt.get_orchard().is_none());
        assert_eq!(parsed_pczt.get_fee_value(), "0.0001 ZEC");

        check_pczt_cypherpunk(
            &pczt::test_support::Nu6_3Network,
            &sample.bytes,
            &sample.ufvk_text,
            &seed_fingerprint,
            0,
        )
        .unwrap();

        let signed = sign_pczt(&sample.bytes, &sample.seed).expect("Ironwood PCZT should sign");
        let signed_pczt = Pczt::parse(&signed).expect("signed PCZT must parse");
        assert!(
            signed_pczt
                .ironwood()
                .actions()
                .iter()
                .any(|action| action.spend().spend_auth_sig().is_some()),
            "Ironwood spend authorization signature must be present",
        );
    }

    #[test]
    fn test_parse_pczt_orchard_decodes_spend_and_change() {
        let sample = pczt::test_support::sample_orchard_change_pczt();
        let parsed = parse_pczt_cypherpunk(
            &pczt::test_support::Nu6_3Network,
            &sample.bytes,
            &sample.ufvk_text,
            &sample.seed_fingerprint,
        )
        .unwrap();

        // Decodes as an Orchard (not Ironwood) bundle.
        assert!(parsed.get_ironwood().is_none());
        let orchard = parsed.get_orchard().expect("orchard bundle should decode");

        // The wallet's own spend is recognized, with its value.
        let from = orchard.get_from();
        assert_eq!(from.len(), 1);
        assert!(from[0].get_is_mine());
        assert!(from[0].get_address().is_none());
        assert_eq!(from[0].get_value(), "0.01 ZEC");

        // The output value and recipient are decoded (a wallet output, not change).
        let to = orchard.get_to();
        assert_eq!(to.len(), 1);
        assert_eq!(to[0].get_value(), "0.0099 ZEC");
        assert!(to[0].get_address().starts_with("u1"));
        assert!(!to[0].get_is_change());

        assert_eq!(parsed.get_fee_value(), "0.0001 ZEC");

        // The same PCZT also passes the pre-sign checks.
        check_pczt_cypherpunk(
            &pczt::test_support::Nu6_3Network,
            &sample.bytes,
            &sample.ufvk_text,
            &sample.seed_fingerprint,
            0,
        )
        .unwrap();
    }

    #[test]
    fn test_parse_and_check_ignore_unsupported_ironwood_spend_zip32_path() {
        let sample = pczt::test_support::sample_ironwood_pczt();
        let parsed_pczt = parse_pczt_cypherpunk(
            &pczt::test_support::Nu6_3Network,
            &sample.bytes,
            &sample.ufvk_text,
            &sample.seed_fingerprint,
        )
        .unwrap();
        assert!(parsed_pczt
            .get_ironwood()
            .unwrap()
            .get_from()
            .first()
            .unwrap()
            .get_is_mine());

        for path in pczt::test_support::unsupported_orchard_spend_paths() {
            let pczt = pczt::test_support::ironwood_pczt_with_spend_derivation(
                &sample.bytes,
                sample.seed_fingerprint,
                path,
            );

            parse_pczt_cypherpunk(
                &pczt::test_support::Nu6_3Network,
                &pczt,
                &sample.ufvk_text,
                &sample.seed_fingerprint,
            )
            .expect("parse uses seed fingerprint ownership only");
            check_pczt_cypherpunk(
                &pczt::test_support::Nu6_3Network,
                &pczt,
                &sample.ufvk_text,
                &sample.seed_fingerprint,
                0,
            )
            .expect("check ignores non-selected shielded spend paths");
        }
    }

    #[test]
    fn test_parse_and_check_ignore_dummy_ironwood_spend_zip32_metadata() {
        let sample = pczt::test_support::sample_ironwood_pczt();
        let mut paths = pczt::test_support::unsupported_orchard_spend_paths();
        paths.push(pczt::test_support::orchard_spend_path_for_account(1));

        for path in paths {
            let pczt = pczt::test_support::ironwood_pczt_with_dummy_spend_derivation(
                &sample.bytes,
                sample.seed_fingerprint,
                path,
            );

            let parsed_pczt = parse_pczt_cypherpunk(
                &pczt::test_support::Nu6_3Network,
                &pczt,
                &sample.ufvk_text,
                &sample.seed_fingerprint,
            )
            .unwrap();
            assert!(parsed_pczt
                .get_ironwood()
                .unwrap()
                .get_from()
                .first()
                .unwrap()
                .get_is_mine());
            check_pczt_cypherpunk(
                &pczt::test_support::Nu6_3Network,
                &pczt,
                &sample.ufvk_text,
                &sample.seed_fingerprint,
                0,
            )
            .unwrap();
        }
    }

    #[test]
    fn test_parse_check_and_sign_reject_v5_pczt_with_ironwood_actions() {
        let sample = pczt::test_support::sample_ironwood_pczt();
        let malformed_pczt = v5_pczt_with_ironwood_actions();

        assert_invalid_pczt_message(
            parse_pczt_cypherpunk(
                &pczt::test_support::Nu6_3Network,
                &malformed_pczt,
                &sample.ufvk_text,
                &sample.seed_fingerprint,
            ),
            "Ironwood actions require a v6 PCZT",
        );
        assert_invalid_pczt_message(
            check_pczt_cypherpunk(
                &pczt::test_support::Nu6_3Network,
                &malformed_pczt,
                &sample.ufvk_text,
                &sample.seed_fingerprint,
                0,
            ),
            "Ironwood actions require a v6 PCZT",
        );
        assert_invalid_pczt_message(
            sign_pczt(&malformed_pczt, &sample.seed),
            "Ironwood actions require a v6 PCZT",
        );
    }

    #[test]
    fn test_get_address_invalid_ufvk() {
        let invalid_ufvk = "invalid_ufvk_string";
        let result = get_address(&MainNetwork, invalid_ufvk);
        assert!(result.is_err());
        assert!(matches!(
            result.unwrap_err(),
            ZcashError::GenerateAddressError(_)
        ));
    }

    #[test]
    fn test_check_pczt_invalid_data() {
        let invalid_pczt = b"invalid_pczt_data";
        let seed = hex::decode("d561f5aba9db8b100a9a84197322e522f952171a388ad74eaab1ab9db815be3335c3099a0a2bb0fee57e630db5ed7251412b6bd4b905cf518627411fee3f32dd").unwrap();
        let ufvk = derive_ufvk(&MainNetwork, &seed, "m/32'/133'/0'").unwrap();
        let seed_fingerprint = calculate_seed_fingerprint(&seed).unwrap();

        let result = check_pczt_cypherpunk(
            &MainNetwork,
            invalid_pczt,
            &ufvk.to_string(),
            &seed_fingerprint,
            0,
        );
        assert!(result.is_err());
        assert!(matches!(result.unwrap_err(), ZcashError::InvalidPczt(_)));
    }

    #[test]
    fn test_check_pczt_normalizes_and_is_idempotent() {
        let sample = pczt::test_support::sample_orchard_change_pczt();
        let normalized = check_pczt_cypherpunk(
            &pczt::test_support::Nu6_3Network,
            &sample.bytes,
            &sample.ufvk_text,
            &sample.seed_fingerprint,
            0,
        )
        .unwrap();

        // Normalized bytes are a valid PCZT that passes the same check and
        // re-normalizes to identical bytes.
        let renormalized = check_pczt_cypherpunk(
            &pczt::test_support::Nu6_3Network,
            &normalized,
            &sample.ufvk_text,
            &sample.seed_fingerprint,
            0,
        )
        .unwrap();
        assert_eq!(normalized, renormalized);
    }

    #[test]
    fn test_check_pczt_rejects_invalid_data() {
        let seed = [7u8; 32];
        let ufvk = derive_ufvk(&MainNetwork, &seed, "m/32'/133'/0'").unwrap();
        let seed_fingerprint = calculate_seed_fingerprint(&seed).unwrap();

        let result = check_pczt_cypherpunk(
            &MainNetwork,
            b"invalid_pczt_data",
            &ufvk,
            &seed_fingerprint,
            0,
        );
        assert!(matches!(result.unwrap_err(), ZcashError::InvalidPczt(_)));
    }

    #[test]
    fn test_parse_pczt_invalid_data() {
        let invalid_pczt = b"invalid_pczt_data";
        let seed = hex::decode("d561f5aba9db8b100a9a84197322e522f952171a388ad74eaab1ab9db815be3335c3099a0a2bb0fee57e630db5ed7251412b6bd4b905cf518627411fee3f32dd").unwrap();
        let ufvk = derive_ufvk(&MainNetwork, &seed, "m/32'/133'/0'").unwrap();
        let seed_fingerprint = calculate_seed_fingerprint(&seed).unwrap();

        let result = parse_pczt_cypherpunk(
            &MainNetwork,
            invalid_pczt,
            &ufvk.to_string(),
            &seed_fingerprint,
        );
        assert!(result.is_err());
        assert!(matches!(result.unwrap_err(), ZcashError::InvalidPczt(_)));
    }

    #[test]
    fn test_sign_pczt_invalid_data() {
        let invalid_pczt = b"invalid_pczt_data";
        let seed = hex::decode("d561f5aba9db8b100a9a84197322e522f952171a388ad74eaab1ab9db815be3335c3099a0a2bb0fee57e630db5ed7251412b6bd4b905cf518627411fee3f32dd").unwrap();

        let result = sign_pczt(invalid_pczt, &seed);
        assert!(result.is_err());
        assert!(matches!(result.unwrap_err(), ZcashError::InvalidPczt(_)));
    }

    const BATCH_UNSUPPORTED_SAPLING_ERROR: &str =
        "Zcash batch PCZT must not contain Sapling spends or outputs";

    fn pczt_with_sapling_output() -> pczt::test_support::SamplePczt {
        let mut sample = pczt::test_support::sample_orchard_change_pczt();
        let (mut prefix, rest) =
            postcard::take_from_bytes::<PcztWirePrefix>(&sample.bytes[8..]).unwrap();
        // The orchard-change sample has an empty Sapling bundle, which v2 omits on the wire;
        // synthesize one with a single output so the batch check rejects it.
        let mut sapling = prefix.sapling.take().unwrap_or(SaplingBundleMirror {
            spends: Vec::new(),
            outputs: Vec::new(),
            value_sum: 0,
            anchor: [0u8; 32],
            bsk: None,
        });
        sapling.outputs.push(SaplingOutputMirror {
            cv: [0; 32],
            cmu: [0; 32],
            ephemeral_key: [0; 32],
            enc_ciphertext: Vec::new(),
            out_ciphertext: Vec::new(),
            zkproof: None,
            recipient: None,
            value: Some(1),
            rseed: None,
            rcv: None,
            ock: None,
            zip32_derivation: None,
            user_address: None,
            proprietary: BTreeMap::new(),
        });
        sapling.value_sum = -1;
        prefix.sapling = Some(sapling);

        let mut out = sample.bytes[..8].to_vec();
        out = postcard::to_extend(&prefix, out).unwrap();
        out.extend_from_slice(rest);
        sample.bytes = out;
        sample
    }

    fn assert_batch_unsupported_sapling_error<T: core::fmt::Debug>(result: Result<T>) {
        assert_eq!(
            result.unwrap_err(),
            ZcashError::InvalidPczt(BATCH_UNSUPPORTED_SAPLING_ERROR.to_string())
        );
    }

    #[test]
    fn test_sign_checked_batch_pczt_signs_ironwood_spend() {
        let sample = pczt::test_support::sample_ironwood_pczt();
        let normalized = check_batch_pczt_cypherpunk(
            &pczt::test_support::Nu6_3Network,
            &sample.bytes,
            &sample.ufvk_text,
            &sample.seed_fingerprint,
            0,
        )
        .unwrap();
        let signed = sign_checked_batch_pczt(
            &pczt::test_support::Nu6_3Network,
            &normalized,
            &sample.seed,
            &sample.seed_fingerprint,
            0,
        )
        .unwrap();
        assert!(Pczt::parse(&signed)
            .unwrap()
            .ironwood()
            .actions()
            .iter()
            .any(|action| action.spend().spend_auth_sig().is_some()));
    }

    #[test]
    fn test_sign_checked_pczt_signs_owned_orchard_actions() {
        let sample = pczt::test_support::sample_orchard_change_pczt();
        let normalized = check_pczt_cypherpunk(
            &pczt::test_support::Nu6_3Network,
            &sample.bytes,
            &sample.ufvk_text,
            &sample.seed_fingerprint,
            0,
        )
        .unwrap();

        let signed = sign_checked_pczt(
            &pczt::test_support::Nu6_3Network,
            &normalized,
            &sample.seed,
            &sample.seed_fingerprint,
            0,
        )
        .unwrap();

        let parsed = Pczt::parse(&signed).expect("signed PCZT must parse");
        let signed_actions = parsed
            .orchard()
            .actions()
            .iter()
            .filter(|action| action.spend().spend_auth_sig().is_some())
            .count();
        assert_eq!(signed_actions, 2);
    }

    #[test]
    fn test_sign_checked_pczt_rejects_foreign_seed() {
        let sample = pczt::test_support::sample_orchard_change_pczt();
        let normalized = check_pczt_cypherpunk(
            &pczt::test_support::Nu6_3Network,
            &sample.bytes,
            &sample.ufvk_text,
            &sample.seed_fingerprint,
            0,
        )
        .unwrap();
        let foreign_seed = [9u8; 32];
        let foreign_fingerprint = calculate_seed_fingerprint(&foreign_seed).unwrap();

        let result = sign_checked_pczt(
            &pczt::test_support::Nu6_3Network,
            &normalized,
            &foreign_seed,
            &foreign_fingerprint,
            0,
        );
        assert!(matches!(result, Err(ZcashError::PcztNoMyInputs)));
    }

    #[test]
    fn test_sign_checked_batch_pczt_signs_and_rejects_sapling() {
        let sample = pczt::test_support::sample_orchard_change_pczt();
        let signed = sign_checked_batch_pczt(
            &pczt::test_support::Nu6_3Network,
            &sample.bytes,
            &sample.seed,
            &sample.seed_fingerprint,
            0,
        )
        .unwrap();
        assert!(Pczt::parse(&signed)
            .unwrap()
            .orchard()
            .actions()
            .iter()
            .any(|action| action.spend().spend_auth_sig().is_some()));

        // Batch policy: account 1 owns nothing in this PCZT.
        assert_eq!(
            sign_checked_batch_pczt(
                &pczt::test_support::Nu6_3Network,
                &sample.bytes,
                &sample.seed,
                &sample.seed_fingerprint,
                1,
            )
            .unwrap_err(),
            ZcashError::PcztNoMyInputs
        );

        let sapling_sample = pczt_with_sapling_output();
        assert_batch_unsupported_sapling_error(sign_checked_batch_pczt(
            &pczt::test_support::Nu6_3Network,
            &sapling_sample.bytes,
            &sapling_sample.seed,
            &sapling_sample.seed_fingerprint,
            0,
        ));
    }

    #[test]
    fn test_check_batch_pczt_accepts_orchard_and_ironwood_spends() {
        for sample in [
            pczt::test_support::sample_orchard_change_pczt(),
            pczt::test_support::sample_ironwood_pczt(),
        ] {
            let normalized = check_batch_pczt_cypherpunk(
                &pczt::test_support::Nu6_3Network,
                &sample.bytes,
                &sample.ufvk_text,
                &sample.seed_fingerprint,
                0,
            )
            .unwrap();
            assert!(Pczt::parse(&normalized).is_ok());

            // Account 1 owns nothing in these PCZTs: batch policy rejects.
            assert_eq!(
                check_batch_pczt_cypherpunk(
                    &pczt::test_support::Nu6_3Network,
                    &sample.bytes,
                    &sample.ufvk_text,
                    &sample.seed_fingerprint,
                    1,
                )
                .unwrap_err(),
                ZcashError::PcztNoMyInputs
            );
        }
    }

    #[test]
    fn test_batch_check_and_parse_accepts_ironwood_spend() {
        let sample = pczt::test_support::sample_ironwood_pczt();

        let parsed = check_and_parse_batch_pczt_cypherpunk(
            &pczt::test_support::Nu6_3Network,
            &sample.bytes,
            &sample.ufvk_text,
            &sample.seed_fingerprint,
            0,
        )
        .expect("Ironwood batch PCZT should parse");
        assert!(parsed.get_orchard().is_some() || parsed.get_ironwood().is_some());

        assert_eq!(
            check_and_parse_batch_pczt_cypherpunk(
                &pczt::test_support::Nu6_3Network,
                &sample.bytes,
                &sample.ufvk_text,
                &sample.seed_fingerprint,
                1,
            )
            .unwrap_err(),
            ZcashError::PcztNoMyInputs
        );
    }

    #[test]
    fn test_batch_check_and_parse_accepts_orchard_to_ironwood_migration() {
        let sample = pczt::test_support::sample_migration_pczt();

        let parsed = check_and_parse_batch_pczt_cypherpunk(
            &pczt::test_support::Nu6_3Network,
            &sample.bytes,
            &sample.ufvk_text,
            &sample.seed_fingerprint,
            0,
        )
        .expect("migration batch PCZT should parse");
        assert!(!parsed
            .get_orchard()
            .expect("migration must show Orchard inputs")
            .get_from()
            .is_empty());
        assert!(!parsed
            .get_ironwood()
            .expect("migration must show Ironwood outputs")
            .get_to()
            .is_empty());
        assert_eq!(parsed.get_fee_value(), "0.0002 ZEC");

        assert_eq!(
            check_and_parse_batch_pczt_cypherpunk(
                &pczt::test_support::Nu6_3Network,
                &sample.bytes,
                &sample.ufvk_text,
                &sample.seed_fingerprint,
                1,
            )
            .unwrap_err(),
            ZcashError::PcztNoMyInputs
        );
    }

    #[test]
    fn test_check_rejects_undecryptable_ironwood_output() {
        use zcash_vendor::pczt::Pczt;

        /// Flips a byte inside the first verbatim occurrence of `needle`.
        fn corrupt_first_occurrence(haystack: &mut [u8], needle: &[u8]) -> bool {
            if needle.is_empty() || needle.len() > haystack.len() {
                return false;
            }
            for start in 0..=haystack.len() - needle.len() {
                if &haystack[start..start + needle.len()] == needle {
                    haystack[start + needle.len() / 2] ^= 0xff;
                    return true;
                }
            }
            false
        }

        let sample = pczt::test_support::sample_migration_pczt();

        // Extract the non-zero Ironwood output's ciphertext bytes.
        let enc_ciphertext = {
            let pczt = Pczt::parse(&sample.bytes).expect("sample PCZT should parse");
            pczt.ironwood()
                .actions()
                .iter()
                .find(|action| matches!(action.output().value(), Some(value) if *value != 0))
                .expect("migration child must contain a non-zero Ironwood output")
                .output()
                .enc_ciphertext()
                .clone()
                .into_encrypted()
                .expect("the sample's Ironwood output carries a full enc_ciphertext")
        };

        // Corrupt only the ciphertext: cmx, cv_net, the value balance, and the
        // plaintext recipient are all untouched, so every other check still
        // passes and only decryption/recoverability fails.
        let mut corrupted = sample.bytes.clone();
        assert!(
            corrupt_first_occurrence(&mut corrupted, &enc_ciphertext),
            "sample must embed the Ironwood output enc_ciphertext verbatim"
        );
        assert!(
            Pczt::parse(&corrupted).is_ok(),
            "corruption must keep the PCZT structurally well-formed"
        );

        let check_err = check_pczt_cypherpunk(
            &pczt::test_support::Nu6_3Network,
            &corrupted,
            &sample.ufvk_text,
            &sample.seed_fingerprint,
            0,
        )
        .expect_err("check must reject a PCZT with an undecryptable output");
        assert!(
            matches!(&check_err, ZcashError::InvalidPczt(message) if message.contains("undecryptable")),
            "expected an undecryptable-output rejection, got {check_err:?}"
        );

        // Both review paths enforce the same output-recoverability contract.
        assert!(
            matches!(
                check_and_parse_batch_pczt_cypherpunk(
                    &pczt::test_support::Nu6_3Network,
                    &corrupted,
                    &sample.ufvk_text,
                    &sample.seed_fingerprint,
                    0,
                ),
                Err(ZcashError::InvalidPczt(message)) if message.contains("undecryptable")
            ),
            "single-pass batch review must also reject the undecryptable output"
        );
    }

    #[test]
    fn test_batch_migration_summary_accepts_orchard_to_ironwood_child() {
        let sample = pczt::test_support::sample_migration_pczt();

        let summary = summarize_batch_migration_pczt_cypherpunk(
            &pczt::test_support::Nu6_3Network,
            &sample.bytes,
            &sample.ufvk_text,
            &sample.seed_fingerprint,
        )
        .expect("migration child should summarize");

        assert_eq!(
            summary,
            BatchMigrationSummary {
                migrations: 1,
                total_input: 1_010_000,
                total_output: 990_000,
                total_fee: 20_000,
                children: vec![BatchMigrationChildSummary {
                    input: 1_010_000,
                    output: 990_000,
                    fee: 20_000,
                }],
            }
        );

        let parsed = summary.to_parsed_pczt();
        assert_eq!(parsed.get_total_transfer_value(), "0.0099 ZEC");
        assert_eq!(parsed.get_fee_value(), "0.0002 ZEC");
        assert_eq!(
            parsed
                .get_orchard()
                .expect("summary should show Orchard inputs")
                .get_from()
                .len(),
            1
        );
        assert_eq!(
            parsed
                .get_ironwood()
                .expect("summary should show Ironwood outputs")
                .get_to()
                .len(),
            1
        );
        assert_eq!(
            parsed
                .get_orchard()
                .expect("summary should show Orchard inputs")
                .get_from()[0]
                .get_address()
                .as_deref(),
            Some("Migration #1 Orchard note from selected account")
        );
        assert_eq!(
            parsed
                .get_ironwood()
                .expect("summary should show Ironwood outputs")
                .get_to()[0]
                .get_address(),
            "Migration #1 wallet Ironwood output"
        );
        assert!(parsed
            .get_ironwood()
            .expect("summary should show Ironwood outputs")
            .get_to()[0]
            .get_is_change());
    }

    #[test]
    fn test_batch_migration_summary_aggregates_multiple_children() {
        let samples = [
            pczt::test_support::sample_migration_pczt(),
            pczt::test_support::sample_migration_pczt(),
            pczt::test_support::sample_migration_pczt(),
        ];
        let parsed = parse_batch_with_migration_summary_cypherpunk(
            &pczt::test_support::Nu6_3Network,
            &samples[0].bytes,
            samples[1..].iter().map(|sample| sample.bytes.as_slice()),
            &samples[0].ufvk_text,
            &samples[0].seed_fingerprint,
        )
        .expect("two migration children should aggregate");

        assert_eq!(parsed.len(), 2);
        let summary = &parsed[1];
        assert_eq!(summary.get_total_transfer_value(), "0.0198 ZEC");
        assert_eq!(summary.get_fee_value(), "0.0004 ZEC");
        let inputs = summary.get_orchard().unwrap().get_from();
        let outputs = summary.get_ironwood().unwrap().get_to();
        assert_eq!(inputs.len(), 2);
        assert!(inputs.iter().all(|input| input.get_amount() == 1_010_000));
        assert_eq!(outputs.len(), 2);
        assert!(outputs.iter().all(|output| output.get_amount() == 990_000));
    }

    #[test]
    fn test_batch_migration_summary_requires_a_child() {
        let first = pczt::test_support::sample_migration_pczt();
        assert_invalid_pczt_message(
            parse_batch_with_migration_summary_cypherpunk(
                &pczt::test_support::Nu6_3Network,
                &first.bytes,
                core::iter::empty(),
                &first.ufvk_text,
                &first.seed_fingerprint,
            ),
            "migration review has no child transactions",
        );
    }

    // A memo on the funded output forces fallback to the review that displays it.
    #[test]
    fn test_batch_migration_summary_rejects_memo_carrying_output() {
        use zcash_vendor::zcash_protocol::memo::MemoBytes;

        let sample = pczt::test_support::sample_migration_pczt_with_output_memo(
            MemoBytes::from_bytes(b"covert note").expect("memo text fits"),
        );

        let summary_err = summarize_batch_migration_pczt_cypherpunk(
            &pczt::test_support::Nu6_3Network,
            &sample.bytes,
            &sample.ufvk_text,
            &sample.seed_fingerprint,
        )
        .expect_err("summary must refuse a memo it cannot render");
        assert!(
            matches!(&summary_err, ZcashError::InvalidPczt(message) if message.contains("per-message review")),
            "expected a display-shape rejection, got {summary_err:?}"
        );

        // Parity: the fallback per-message review shows the memo.
        let parsed = check_and_parse_batch_pczt_cypherpunk(
            &pczt::test_support::Nu6_3Network,
            &sample.bytes,
            &sample.ufvk_text,
            &sample.seed_fingerprint,
            0,
        )
        .expect("per-message review must accept the memo-carrying child");
        let shown_memo = parsed
            .get_ironwood()
            .expect("migration must show Ironwood outputs")
            .get_to()
            .first()
            .expect("migration must show the real output")
            .get_memo();
        assert_eq!(shown_memo.as_deref(), Some("covert note"));

        let first = pczt::test_support::sample_migration_pczt();
        assert!(parse_batch_with_migration_summary_cypherpunk(
            &pczt::test_support::Nu6_3Network,
            &first.bytes,
            core::iter::once(sample.bytes.as_slice()),
            &sample.ufvk_text,
            &sample.seed_fingerprint,
        )
        .is_err());
    }

    #[test]
    fn test_batch_migration_summary_rejects_foreign_funded_output() {
        let sample = pczt::test_support::sample_migration_pczt_to_account(1);

        let summary_err = summarize_batch_migration_pczt_cypherpunk(
            &pczt::test_support::Nu6_3Network,
            &sample.bytes,
            &sample.ufvk_text,
            &sample.seed_fingerprint,
        )
        .expect_err("summary must refuse a foreign funded output");
        assert!(matches!(
            &summary_err,
            ZcashError::InvalidPczt(message) if message.contains("not wallet-owned")
        ));

        let parsed = check_and_parse_batch_pczt_cypherpunk(
            &pczt::test_support::Nu6_3Network,
            &sample.bytes,
            &sample.ufvk_text,
            &sample.seed_fingerprint,
            0,
        )
        .expect("ordinary review should show a foreign output");
        let outputs = parsed.get_ironwood().unwrap().get_to();
        let output = &outputs[0];
        assert_eq!(output.get_amount(), 990_000);
        assert!(!output.get_is_change());
    }

    #[test]
    fn test_batch_migration_summary_ignores_dummy_equivalent_zero_output() {
        use zcash_vendor::zcash_protocol::memo::MemoBytes;

        let sample = pczt::test_support::sample_migration_pczt_with_zero_output(
            MemoBytes::from_bytes(b"hidden dummy memo").unwrap(),
            false,
        );
        let summary = summarize_batch_migration_pczt_cypherpunk(
            &pczt::test_support::Nu6_3Network,
            &sample.bytes,
            &sample.ufvk_text,
            &sample.seed_fingerprint,
        )
        .expect("dummy-equivalent zero output should not block the summary");
        assert_eq!(summary.total_output, 990_000);
        assert_eq!(summary.total_fee, 20_000);

        let parsed = check_and_parse_batch_pczt_cypherpunk(
            &pczt::test_support::Nu6_3Network,
            &sample.bytes,
            &sample.ufvk_text,
            &sample.seed_fingerprint,
            0,
        )
        .expect("ordinary review should accept the zero output");
        let outputs = parsed.get_ironwood().unwrap().get_to();
        assert_eq!(outputs.len(), 1);
        assert_eq!(outputs[0].get_amount(), 990_000);
        assert!(outputs[0].get_memo().is_none());
    }

    #[test]
    fn test_batch_migration_summary_falls_back_for_displayable_zero_output() {
        use zcash_vendor::zcash_protocol::memo::MemoBytes;

        let sample = pczt::test_support::sample_migration_pczt_with_zero_output(
            MemoBytes::from_bytes(b"visible zero memo").unwrap(),
            true,
        );
        let summary_err = summarize_batch_migration_pczt_cypherpunk(
            &pczt::test_support::Nu6_3Network,
            &sample.bytes,
            &sample.ufvk_text,
            &sample.seed_fingerprint,
        )
        .expect_err("displayable zero output should force ordinary review");
        assert!(matches!(
            &summary_err,
            ZcashError::InvalidPczt(message) if message.contains("output shape")
        ));

        let parsed = check_and_parse_batch_pczt_cypherpunk(
            &pczt::test_support::Nu6_3Network,
            &sample.bytes,
            &sample.ufvk_text,
            &sample.seed_fingerprint,
            0,
        )
        .expect("ordinary review should show the zero output");
        let outputs = parsed.get_ironwood().unwrap().get_to();
        assert_eq!(outputs.len(), 2);
        let zero = outputs
            .iter()
            .find(|output| output.get_amount() == 0)
            .expect("zero output should be displayed");
        assert_eq!(zero.get_memo().as_deref(), Some("visible zero memo"));
    }

    // Transparent or Sapling components force fallback because the compact summary
    // cannot display them or include transparent values in its fee.
    #[test]
    fn test_migration_display_shape_rejects_transparent_and_sapling() {
        use crate::pczt::structs::ParsedTransparent;

        // The shielded-only shape the summary can represent.
        let orchard = ParsedOrchard::new(
            vec![ParsedFrom::new(
                None,
                "0.0101 ZEC".to_string(),
                1_010_000,
                true,
            )],
            vec![],
        );
        let ironwood = ParsedOrchard::new(
            vec![],
            vec![ParsedTo::new(
                "wallet Ironwood output".to_string(),
                "0.0099 ZEC".to_string(),
                990_000,
                true,
                false,
                None,
            )],
        );
        let build = |transparent: Option<ParsedTransparent>, has_sapling: bool| {
            ParsedPczt::new(
                transparent,
                Some(orchard.clone()),
                Some(ironwood.clone()),
                "0.0099 ZEC".to_string(),
                "0.0002 ZEC".to_string(),
                has_sapling,
            )
        };

        // Control: the pure shielded migration folds into the summary.
        require_migration_display_shape(&build(None, false))
            .expect("a shielded-only Orchard->Ironwood child must still summarize");

        // A wallet-owned transparent input the amounts-only summary would hide
        // (its value silently dropped from the fee) must force fallback.
        let transparent = ParsedTransparent::new(
            vec![ParsedFrom::new(
                Some("t1wallet".to_string()),
                "0.0005 ZEC".to_string(),
                50_000,
                true,
            )],
            vec![],
        );
        assert!(
            matches!(
                require_migration_display_shape(&build(Some(transparent), false)),
                Err(ZcashError::InvalidPczt(message)) if message.contains("transparent components")
            ),
            "a transparent component must fall back to the per-message review",
        );

        // A Sapling component must likewise force fallback.
        assert!(
            matches!(
                require_migration_display_shape(&build(None, true)),
                Err(ZcashError::InvalidPczt(message)) if message.contains("Sapling components")
            ),
            "a Sapling component must fall back to the per-message review",
        );
    }

    // An undecryptable funded output must fail both compact and ordinary review.
    #[test]
    fn test_batch_migration_summary_rejects_undecryptable_ironwood_output() {
        use zcash_vendor::pczt::Pczt;

        /// Flips a byte inside the first verbatim occurrence of `needle`.
        fn corrupt_first_occurrence(haystack: &mut [u8], needle: &[u8]) -> bool {
            if needle.is_empty() || needle.len() > haystack.len() {
                return false;
            }
            for start in 0..=haystack.len() - needle.len() {
                if &haystack[start..start + needle.len()] == needle {
                    haystack[start + needle.len() / 2] ^= 0xff;
                    return true;
                }
            }
            false
        }

        let sample = pczt::test_support::sample_migration_pczt();

        // Extract the funded Ironwood output's encrypted ciphertext bytes.
        let enc_ciphertext = {
            let pczt = Pczt::parse(&sample.bytes).expect("sample PCZT should parse");
            pczt.ironwood()
                .actions()
                .iter()
                .find(|action| matches!(action.output().value(), Some(value) if *value != 0))
                .expect("migration child must contain a non-zero Ironwood output")
                .output()
                .enc_ciphertext()
                .clone()
                .into_encrypted()
                .expect("the sample's Ironwood output carries a full enc_ciphertext")
        };

        // Corrupt only the ciphertext: cmx, cv_net, the value balance, and the
        // plaintext recipient are all untouched, so every other check still
        // passes and only decryption/recoverability fails.
        let mut corrupted = sample.bytes.clone();
        assert!(
            corrupt_first_occurrence(&mut corrupted, &enc_ciphertext),
            "sample must embed the Ironwood output enc_ciphertext verbatim"
        );
        assert!(
            Pczt::parse(&corrupted).is_ok(),
            "corruption must keep the PCZT structurally well-formed"
        );

        let summary_err = summarize_batch_migration_pczt_cypherpunk(
            &pczt::test_support::Nu6_3Network,
            &corrupted,
            &sample.ufvk_text,
            &sample.seed_fingerprint,
        )
        .expect_err("summary must reject a migration child with an undecryptable output");
        assert!(
            matches!(&summary_err, ZcashError::InvalidPczt(message) if message.contains("undecryptable")),
            "expected an undecryptable-output rejection, got {summary_err:?}"
        );

        // The ordinary review must enforce the same output recovery rule.
        assert!(
            matches!(
                check_and_parse_batch_pczt_cypherpunk(
                    &pczt::test_support::Nu6_3Network,
                    &corrupted,
                    &sample.ufvk_text,
                    &sample.seed_fingerprint,
                    0,
                ),
                Err(ZcashError::InvalidPczt(message)) if message.contains("undecryptable")
            ),
            "ordinary per-message review must also reject the undecryptable output"
        );
    }

    #[test]
    fn test_batch_migration_summary_accepts_optional_spend_fvk() {
        use zcash_vendor::pczt::{roles::redactor::Redactor, Pczt};

        let sample = pczt::test_support::sample_migration_pczt();
        let pczt = Pczt::parse(&sample.bytes).expect("sample PCZT should parse");
        let redacted = Redactor::new(pczt)
            .redact_orchard_with(|mut r| {
                r.redact_actions(|mut ar| {
                    ar.clear_spend_fvk();
                });
            })
            .redact_ironwood_with(|mut r| {
                r.redact_actions(|mut ar| {
                    ar.clear_spend_fvk();
                });
            })
            .finish()
            .serialize()
            .expect("redacted PCZT should serialize");

        let summary = summarize_batch_migration_pczt_cypherpunk(
            &pczt::test_support::Nu6_3Network,
            &redacted,
            &sample.ufvk_text,
            &sample.seed_fingerprint,
        )
        .expect("redacted migration child should summarize");

        assert_eq!(summary.migrations, 1);
        assert_eq!(summary.total_input, 1_010_000);
        assert_eq!(summary.total_output, 990_000);
        assert_eq!(summary.total_fee, 20_000);

        let signed = sign_checked_batch_pczt(
            &pczt::test_support::Nu6_3Network,
            &redacted,
            &sample.seed,
            &sample.seed_fingerprint,
            0,
        )
        .expect("request redacted only by optional spend FVK should sign");
        let parsed = Pczt::parse(&signed).expect("signed PCZT should parse");
        assert!(
            parsed
                .orchard()
                .actions()
                .iter()
                .any(|action| action.spend().spend_auth_sig().is_some()),
            "redacted migration request must still produce an Orchard spend signature"
        );
    }

    #[test]
    fn test_check_resolves_compact_pczt_and_signs() {
        use zcash_vendor::pczt::roles::redactor::Redactor;

        let sample = pczt::test_support::sample_migration_pczt();
        // Compact the sample the way the wallet's batch redaction will: drop cv_net and
        // the v6 anchors, and swap each Ironwood output's ciphertext down to its memo
        // plaintext. `resolve_fields` in the check must undo all of it.
        let compact = {
            let parsed = Pczt::parse(&sample.bytes).unwrap();
            let redacted = Redactor::new(parsed)
                .redact_orchard_with(|mut r| {
                    r.redact_actions(|mut ar| ar.clear_cv_net());
                    r.clear_anchor();
                })
                .redact_ironwood_with(|mut r| {
                    r.redact_actions(|mut ar| {
                        ar.clear_cv_net();
                        ar.replace_enc_ciphertext_with_decrypted_memo_plaintext(
                            orchard::note::NoteVersion::V3,
                        );
                    });
                    r.clear_anchor();
                })
                .finish();
            redacted.serialize().unwrap()
        };
        assert!(compact.len() < sample.bytes.len());
        // Confirm the swap actually compacted an Ironwood output (otherwise the
        // ciphertext round-trip below would be vacuous).
        assert!(Pczt::parse(&compact)
            .unwrap()
            .ironwood()
            .actions()
            .iter()
            .any(|action| matches!(
                action.output().enc_ciphertext(),
                ::pczt::orchard::EncCiphertext::MemoPlaintext(_)
            )));

        let normalized = check_pczt_cypherpunk(
            &pczt::test_support::Nu6_3Network,
            &compact,
            &sample.ufvk_text,
            &sample.seed_fingerprint,
            0,
        )
        .expect("the check must resolve compact fields first");

        let reparsed = Pczt::parse(&normalized).expect("normalized bytes must parse");
        assert!(reparsed
            .orchard()
            .actions()
            .iter()
            .all(|action| action.cv_net().is_some()));
        // resolve_fields recomputed every Ironwood output's full ciphertext.
        assert!(reparsed.ironwood().actions().iter().all(|action| matches!(
            action.output().enc_ciphertext(),
            ::pczt::orchard::EncCiphertext::Encrypted(_)
        )));
        let signed = sign_checked_pczt(
            &pczt::test_support::Nu6_3Network,
            &normalized,
            &sample.seed,
            &sample.seed_fingerprint,
            0,
        )
        .expect("resolved normalized PCZT must sign");
        assert!(Pczt::parse(&signed)
            .unwrap()
            .orchard()
            .actions()
            .iter()
            .any(|action| action.spend().spend_auth_sig().is_some()));
    }

    #[test]
    fn test_check_batch_pczt_rejects_sapling_outputs() {
        let sample = pczt_with_sapling_output();
        assert_batch_unsupported_sapling_error(check_batch_pczt_cypherpunk(
            &pczt::test_support::Nu6_3Network,
            &sample.bytes,
            &sample.ufvk_text,
            &sample.seed_fingerprint,
            0,
        ));
    }
}
