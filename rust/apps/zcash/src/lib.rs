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
use alloc::format;
#[cfg(feature = "cypherpunk")]
use pczt::sign::SpendAuthCache;
use pczt::structs::ParsedPczt;
#[cfg(feature = "cypherpunk")]
use pczt::structs::{ParsedFrom, ParsedOrchard, ParsedTo};
use zcash_vendor::{
    zcash_keys::keys::{UnifiedAddressRequest, UnifiedFullViewingKey},
    zcash_protocol::consensus::{self},
    zip32,
};

#[cfg(feature = "cypherpunk")]
use zcash_vendor::pczt::roles::signer::SpendAuthSignature;
#[cfg(any(test, feature = "multi_coins", feature = "cypherpunk"))]
use zcash_vendor::pczt::Pczt;
#[cfg(feature = "cypherpunk")]
use zcash_vendor::sha2::{Digest, Sha256};

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

/// Checks one PCZT from a batch request, enforcing the batch shielded-action
/// policy, and returns its normalized encoding. See `check_pczt_cypherpunk`
/// for the normalization contract.
///
/// Test-only parity reference: production checks a batch PCZT through the
/// display-producing `check_batch_pczt_with_display`, and this independent
/// check-only composition (whose bytes are byte-identical) is what the parity
/// tests diff that fused engine against.
#[cfg(all(test, feature = "cypherpunk"))]
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
    let has_my_transparent_input = pczt::check::check_pczt_transparent(
        params,
        seed_fingerprint,
        account_index,
        xpub,
        &pczt,
        false,
    )?;
    let (signable_actions, pczt) =
        signable_shielded_actions(params, pczt, seed_fingerprint, account_index, policy)?;
    if signable_actions.is_empty() && !has_my_transparent_input {
        return Err(ZcashError::PcztNoMyInputs);
    }

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
    reject_unsupported_pczt(&pczt)?;
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
fn reject_unsupported_pczt(pczt: &Pczt) -> Result<()> {
    {
        // The multi-coins check path only verifies transparent data. Reject shielded
        // content and unknown transaction formats so check, parse, and sign enforce
        // the same transparent-only boundary.
        if pczt::pczt_is_unsupported_by_transparent_only(pczt) {
            return Err(ZcashError::InvalidPczt(
                "PCZT is not supported by transparent-only checking".to_string(),
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

/// Caches the decoded UFVK and wallet viewing keys shared by every PCZT in a
/// batch.
/// Values are initialized lazily to preserve validation ordering.
#[cfg(feature = "cypherpunk")]
pub struct BatchCheckContext<'a> {
    ufvk_text: &'a str,
    ufvk: core::cell::OnceCell<UnifiedFullViewingKey>,
    wallet_keys: core::cell::OnceCell<pczt::parse::WalletKeys>,
}

#[cfg(feature = "cypherpunk")]
impl<'a> BatchCheckContext<'a> {
    /// Creates a lazy key cache for one checked batch.
    pub fn new(ufvk_text: &'a str) -> Self {
        Self {
            ufvk_text,
            ufvk: core::cell::OnceCell::new(),
            wallet_keys: core::cell::OnceCell::new(),
        }
    }

    fn ufvk<P: consensus::Parameters>(&self, params: &P) -> Result<&UnifiedFullViewingKey> {
        if let Some(ufvk) = self.ufvk.get() {
            return Ok(ufvk);
        }
        let ufvk = UnifiedFullViewingKey::decode(params, self.ufvk_text)
            .map_err(|e| ZcashError::InvalidDataError(e.to_string()))?;
        Ok(self.ufvk.get_or_init(|| ufvk))
    }

    fn wallet_keys<P: consensus::Parameters>(
        &self,
        params: &P,
    ) -> Result<&pczt::parse::WalletKeys> {
        if let Some(keys) = self.wallet_keys.get() {
            return Ok(keys);
        }
        let keys = pczt::parse::WalletKeys::derive(self.ufvk(params)?)?;
        Ok(self.wallet_keys.get_or_init(|| keys))
    }
}

/// Validates one batch PCZT and returns its normalized transaction, display,
/// signable actions, and selected account.
#[cfg(feature = "cypherpunk")]
fn check_and_parse_batch_pczt_internal<P: consensus::Parameters>(
    params: &P,
    pczt_bytes: &[u8],
    ctx: &BatchCheckContext<'_>,
    seed_fingerprint: &[u8; 32],
    account_index: u32,
) -> Result<(
    Pczt,
    ParsedPczt,
    Vec<SignableShieldedAction>,
    zip32::AccountId,
)> {
    let mut pczt = pczt::parse_pczt(pczt_bytes)?;
    // Resolve compact field representations before the single-pass validation.
    pczt.resolve_fields().map_err(|e| {
        ZcashError::InvalidPczt(alloc::format!("resolve compact PCZT fields: {e:?}"))
    })?;
    let account_index = zip32::AccountId::try_from(account_index)
        .map_err(|_e| ZcashError::InvalidDataError("invalid account index".to_string()))?;
    let ufvk = ctx.ufvk(params)?;
    let xpub = ufvk.transparent().ok_or(ZcashError::InvalidDataError(
        "transparent xpub is not present".to_string(),
    ))?;
    // `validate_supported_pczt` stays ahead of the wallet-key derivation so the
    // first PCZT keeps the check-only path's error ordering; the context then
    // caches the derived keys for every later PCZT.
    pczt::validate_supported_pczt(&pczt)?;
    let keys = ctx.wallet_keys(params)?;
    // Validate shielded actions while collecting their display rows.
    let (
        pczt::check::CheckedShieldedParse {
            orchard: checked_orchard,
            ironwood: checked_ironwood,
            checked_actions,
        },
        pczt,
    ) = pczt::check::check_and_parse_pczt_shielded(
        params,
        seed_fingerprint,
        account_index,
        ufvk,
        keys,
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
    // Apply the batch shape and signable action policy to the checked actions.
    reject_unsupported_batch_pczt(&pczt)?;
    let signable_actions = signable_actions_from_checked_actions(
        params,
        &checked_actions,
        seed_fingerprint,
        account_index,
        ShieldedActionPolicy::Batch,
    )?;
    // Release cloned derivation paths before building the retained display.
    drop(checked_actions);

    if signable_actions.is_empty() {
        return Err(ZcashError::PcztNoMyInputs);
    }
    // Assemble the display from the shielded rows collected above.
    let parsed = pczt::parse::parse_pczt_cypherpunk_with_checked_shielded(
        params,
        seed_fingerprint,
        &pczt,
        checked_orchard,
        checked_ironwood,
    )?;
    Ok((pczt, parsed, signable_actions, account_index))
}

/// Checks one batch PCZT and returns normalized bytes, display rows, an optional
/// compact migration classification, and an opaque signability decision bound
/// to the normalized bytes and check context. Validation, display, and
/// signability share one shielded action pass; signing reuses that decision
/// instead of rebuilding the shielded bundles.
#[cfg(feature = "cypherpunk")]
pub fn check_batch_pczt_with_display<P: consensus::Parameters>(
    params: &P,
    pczt_bytes: &[u8],
    ctx: &BatchCheckContext<'_>,
    seed_fingerprint: &[u8; 32],
    account_index: u32,
) -> Result<(
    Vec<u8>,
    ParsedPczt,
    Option<BatchMigrationTransferSummary>,
    CheckedBatchPcztSignability,
)> {
    let (pczt, parsed, actions, account_index) = check_and_parse_batch_pczt_internal(
        params,
        pczt_bytes,
        ctx,
        seed_fingerprint,
        account_index,
    )?;
    // Classify from the complete display model produced by this check pass.
    // The check already bound every funded spend to the selected account.
    let migration_summary = migration_transfer_summary(&parsed);
    let normalized = pczt
        .serialize()
        .map_err(|e| ZcashError::InvalidPczt(alloc::format!("serialize normalized PCZT: {e:?}")))?;
    let signability = CheckedBatchPcztSignability::new(&normalized, account_index, actions);
    Ok((normalized, parsed, migration_summary, signability))
}

/// Checks and parses one batch PCZT using the shared check-time display path.
#[cfg(feature = "cypherpunk")]
pub fn check_and_parse_batch_pczt_cypherpunk<P: consensus::Parameters>(
    params: &P,
    pczt_bytes: &[u8],
    ufvk_text: &str,
    seed_fingerprint: &[u8; 32],
    account_index: u32,
) -> Result<ParsedPczt> {
    let (_, parsed, _, _) = check_batch_pczt_with_display(
        params,
        pczt_bytes,
        &BatchCheckContext::new(ufvk_text),
        seed_fingerprint,
        account_index,
    )?;
    Ok(parsed)
}

/// Values for one compact-eligible Orchard-to-Ironwood transfer, in zatoshis.
#[cfg(feature = "cypherpunk")]
#[derive(Clone, Copy, Debug, Eq, PartialEq)]
pub struct BatchMigrationTransferSummary {
    input: u64,
    output: u64,
    fee: u64,
}

/// Totals and per-transfer rows for the compact migration review.
#[cfg(feature = "cypherpunk")]
#[derive(Clone, Debug, Default, Eq, PartialEq)]
struct BatchMigrationSummary {
    total_output: u64,
    total_fee: u64,
    transfers: Vec<BatchMigrationTransferSummary>,
}

#[cfg(feature = "cypherpunk")]
impl BatchMigrationSummary {
    /// Adds a compact-eligible transfer using checked arithmetic.
    fn add_transfer(&mut self, transfer: BatchMigrationTransferSummary) -> Result<()> {
        self.total_output = self
            .total_output
            .checked_add(transfer.output)
            .ok_or_else(|| ZcashError::InvalidPczt("migration output overflow".to_string()))?;
        self.total_fee = self
            .total_fee
            .checked_add(transfer.fee)
            .ok_or_else(|| ZcashError::InvalidPczt("migration fee overflow".to_string()))?;
        self.transfers.push(transfer);
        Ok(())
    }

    /// Builds the display model for the compact migration review.
    fn to_parsed_pczt(&self) -> ParsedPczt {
        debug_assert!(!self.transfers.is_empty());

        let orchard = ParsedOrchard::new(
            self.transfers
                .iter()
                .enumerate()
                .map(|(index, transfer)| {
                    ParsedFrom::new(
                        Some(format!(
                            "Migration #{} Orchard note from selected account",
                            index + 1
                        )),
                        pczt::parse::format_zec_value(transfer.input as f64),
                        transfer.input,
                        true,
                    )
                })
                .collect(),
            Vec::new(),
        );
        let ironwood = ParsedOrchard::new(
            Vec::new(),
            self.transfers
                .iter()
                .enumerate()
                .map(|(index, transfer)| {
                    ParsedTo::new(
                        format!("Migration #{} wallet Ironwood output", index + 1),
                        pczt::parse::format_zec_value(transfer.output as f64),
                        transfer.output,
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

/// One parsed batch item and its optional compact migration representation.
#[cfg(feature = "cypherpunk")]
struct ParsedBatchItem {
    parsed: ParsedPczt,
    migration: Option<BatchMigrationTransferSummary>,
}

/// Returns the compact representation only when the ordinary review contains
/// exactly the details represented by a migration row.
///
/// This classifier is valid only after the batch check has bound every funded
/// spend to the selected account. `is_mine` is display metadata and is used
/// here as an additional shape check, not as authorization.
#[cfg(feature = "cypherpunk")]
fn migration_transfer_summary(parsed: &ParsedPczt) -> Option<BatchMigrationTransferSummary> {
    if parsed.get_transparent().is_some() || parsed.get_has_sapling() {
        return None;
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

    match (orchard_from.as_slice(), ironwood_to.as_slice()) {
        ([from], [to])
            if from.get_is_mine()
                && orchard_to.is_empty()
                && ironwood_from.is_empty()
                && to.get_amount() != 0
                && to.get_is_change()
                && no_memo(to) =>
        {
            Some(BatchMigrationTransferSummary {
                input: from.get_amount(),
                output: to.get_amount(),
                fee: from.get_amount().checked_sub(to.get_amount())?,
            })
        }
        _ => None,
    }
}

/// Replaces compact-eligible transfers with one summary while retaining every
/// ordinary transaction as its own full review page.
#[cfg(feature = "cypherpunk")]
fn compact_batch_migration_review(items: Vec<ParsedBatchItem>) -> Vec<ParsedPczt> {
    let migration_count = items.iter().filter(|item| item.migration.is_some()).count();
    if items.len() <= 1 || migration_count == 0 {
        return items.into_iter().map(|item| item.parsed).collect();
    }

    let mut summary = BatchMigrationSummary::default();
    for transfer in items.iter().filter_map(|item| item.migration) {
        // Overflow is not expected for a valid transaction batch. Full review
        // remains safe and complete if the compact totals cannot be represented.
        if summary.add_transfer(transfer).is_err() {
            return items.into_iter().map(|item| item.parsed).collect();
        }
    }

    let mut compact = Vec::with_capacity(items.len() - migration_count + 1);
    for item in items {
        if item.migration.is_none() {
            compact.push(item.parsed);
        }
    }
    compact.push(summary.to_parsed_pczt());
    compact
}

/// Compacts the display rows cached by the batch check without re-parsing any
/// PCZT. Classification is independent of the original PCZT order.
#[cfg(feature = "cypherpunk")]
pub fn compact_checked_batch_migration_review(
    items: impl IntoIterator<Item = (ParsedPczt, Option<BatchMigrationTransferSummary>)>,
) -> Vec<ParsedPczt> {
    compact_batch_migration_review(
        items
            .into_iter()
            .map(|(parsed, migration)| ParsedBatchItem { parsed, migration })
            .collect(),
    )
}

/// Parses checked batch PCZTs and compacts eligible Orchard-to-Ironwood
/// self-transfers without relying on PCZT position.
///
/// Every input must be normalized bytes produced by the batch check. Ordinary
/// transactions retain full review pages regardless of how many are present.
#[cfg(feature = "cypherpunk")]
pub fn parse_batch_with_migration_summary_cypherpunk<'a, P: consensus::Parameters>(
    params: &P,
    pczts: impl IntoIterator<Item = &'a [u8]>,
    ufvk_text: &str,
    seed_fingerprint: &[u8; 32],
) -> Result<Vec<ParsedPczt>> {
    let ufvk = UnifiedFullViewingKey::decode(params, ufvk_text)
        .map_err(|e| ZcashError::InvalidDataError(e.to_string()))?;
    let mut items = Vec::new();
    for pczt_bytes in pczts {
        let pczt = pczt::parse_pczt(pczt_bytes)?;
        // Parse once. The same complete display model determines whether a
        // transaction can be represented by a compact migration row.
        let parsed = pczt::parse::parse_pczt_cypherpunk(params, seed_fingerprint, &ufvk, &pczt)?;
        let migration = migration_transfer_summary(&parsed);
        items.push(ParsedBatchItem { parsed, migration });
    }
    if items.is_empty() {
        return Err(ZcashError::InvalidPczt(
            "batch review has no transactions".to_string(),
        ));
    }
    Ok(compact_batch_migration_review(items))
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
    use alloc::{collections::BTreeMap, string::String, vec::Vec};

    use super::*;
    use serde::{Deserialize, Serialize};
    use zcash_vendor::zcash_protocol::consensus::{BranchId, MainNetwork};

    #[derive(Serialize, Deserialize)]
    struct PcztGlobalPrefix {
        global: GlobalMirror,
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

    fn pczt_with_consensus_branch_id(bytes: &[u8], branch_id: BranchId) -> Vec<u8> {
        let (mut prefix, rest) =
            postcard::take_from_bytes::<PcztGlobalPrefix>(&bytes[8..]).unwrap();
        prefix.global.consensus_branch_id = branch_id.into();

        let mut encoded = bytes[..8].to_vec();
        encoded = postcard::to_extend(&prefix, encoded).unwrap();
        encoded.extend_from_slice(rest);
        encoded
    }

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
    fn legacy_check_accepts_transparent_only_v6_pczt() {
        let sample = pczt::legacy_test_support::legacy_transparent_v6_sample();
        assert!(pczt::pczt_is_v6(&Pczt::parse(&sample.bytes).unwrap()));

        check_pczt_multi_coins(
            &MainNetwork,
            &sample.bytes,
            &sample.xpub,
            &sample.seed_fingerprint,
            0,
        )
        .expect("transparent-only v6 PCZT should pass checking");
    }

    #[test]
    fn legacy_rejects_v6_before_nu6_3() {
        let sample = pczt::legacy_test_support::legacy_transparent_v6_sample();
        let bytes = pczt_with_consensus_branch_id(&sample.bytes, BranchId::Nu6);
        let pczt = Pczt::parse(&bytes).unwrap();
        assert!(pczt::pczt_is_v6(&pczt));

        assert_invalid_pczt_message(
            check_pczt_multi_coins(
                &MainNetwork,
                &bytes,
                &sample.xpub,
                &sample.seed_fingerprint,
                0,
            ),
            "PCZT is not supported by transparent-only checking",
        );
        assert_invalid_pczt_message(
            parse_pczt_multi_coins(&MainNetwork, &bytes, &sample.seed_fingerprint),
            "PCZT is not supported by transparent-only parsing",
        );
        assert_eq!(
            sign_pczt(&bytes, &sample.seed),
            Err(ZcashError::SigningError(
                "PCZT is not supported by transparent-only signing".to_string()
            ))
        );
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

/// Opaque signability result produced by the full batch check and bound to the
/// exact normalized PCZT bytes and selected account that produced it.
#[cfg(feature = "cypherpunk")]
#[derive(Debug)]
pub struct CheckedBatchPcztSignability {
    pczt_digest: [u8; 32],
    selected_account: zip32::AccountId,
    required_actions: Vec<SignableShieldedAction>,
}

#[cfg(feature = "cypherpunk")]
impl CheckedBatchPcztSignability {
    fn new(
        checked_pczt: &[u8],
        selected_account: zip32::AccountId,
        required_actions: Vec<SignableShieldedAction>,
    ) -> Self {
        Self {
            pczt_digest: Sha256::digest(checked_pczt).into(),
            selected_account,
            required_actions,
        }
    }

    fn signing_context(
        &self,
        checked_pczt: &[u8],
    ) -> Result<(zip32::AccountId, &[SignableShieldedAction])> {
        let pczt_digest: [u8; 32] = Sha256::digest(checked_pczt).into();
        if self.pczt_digest != pczt_digest {
            return Err(ZcashError::InvalidDataError(
                "checked batch signability does not match the PCZT".to_string(),
            ));
        }
        if self.required_actions.is_empty() {
            return Err(ZcashError::PcztNoMyInputs);
        }
        Ok((self.selected_account, &self.required_actions))
    }
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

    // A batch response contains signatures produced by this device for the
    // reviewed request. Reject incoming signatures instead of carrying
    // bytes supplied by the host into that response.
    for (pool, bundle) in [("Orchard", pczt.orchard()), ("Ironwood", pczt.ironwood())] {
        if bundle
            .actions()
            .iter()
            .any(|action| action.spend().spend_auth_sig().is_some())
        {
            return Err(ZcashError::InvalidPczt(alloc::format!(
                "Zcash batch request must not contain {pool} spend authorization signatures"
            )));
        }
    }

    Ok(())
}

#[cfg(feature = "cypherpunk")]
#[allow(clippy::too_many_arguments)]
/// Decides whether one shielded action is signable by (`seed_fingerprint`,
/// `account_index`) under `policy`. This is the shared predicate behind
/// [`collect_signable_shielded_actions`] (the direct bundle walk) and
/// [`signable_actions_from_checked_actions`] (the batch check's retained
/// actions), so the two can never diverge.
///
/// Zero-valued spends do not authorize value and are omitted from the account
/// policy. The signer still signs any such action whose derivation and `rk`
/// match a key available from the seed.
fn signable_action_decision<P: consensus::Parameters>(
    params: &P,
    pool: SignableShieldedPool,
    index: usize,
    spend_value: Option<u64>,
    spend_has_dummy_sk: bool,
    spend_derivation: Option<(&[u8; 32], &[zcash_vendor::zip32::ChildIndex])>,
    seed_fingerprint: &[u8; 32],
    account_index: zip32::AccountId,
    policy: ShieldedActionPolicy,
) -> core::result::Result<Option<SignableShieldedAction>, ZcashError> {
    if spend_has_dummy_sk {
        return Ok(None);
    }

    let value = spend_value.ok_or_else(|| {
        ZcashError::InvalidPczt(alloc::format!("missing {} spend value", pool.label()))
    })?;
    if value == 0 {
        return Ok(None);
    }

    let matched_account = pczt::matching_seed_supported_orchard_account_parts(
        seed_fingerprint,
        spend_derivation,
        params.network_type().coin_type(),
        pool.shielded_pool(),
    )?;
    match matched_account {
        Some(matched_account) if matched_account == account_index => {}
        Some(_) => return Err(ZcashError::PcztNoMyInputs),
        None if policy == ShieldedActionPolicy::Batch => {
            return Err(ZcashError::PcztNoMyInputs);
        }
        None => return Ok(None),
    }

    Ok(Some(SignableShieldedAction { pool, index }))
}

/// Applies [`collect_signable_shielded_actions`]' decision to the checked
/// actions retained by the batch validation pass, avoiding another walk of the
/// in-memory bundles. The actions retain bundle then action order, preserving
/// error precedence.
#[cfg(feature = "cypherpunk")]
fn signable_actions_from_checked_actions<P: consensus::Parameters>(
    params: &P,
    checked_actions: &[pczt::check::ShieldedAction],
    seed_fingerprint: &[u8; 32],
    account_index: zip32::AccountId,
    policy: ShieldedActionPolicy,
) -> Result<Vec<SignableShieldedAction>> {
    let mut actions = Vec::new();
    for checked_action in checked_actions {
        let pool = match checked_action.pool {
            pczt::ShieldedPool::Orchard => SignableShieldedPool::Orchard,
            pczt::ShieldedPool::Ironwood => SignableShieldedPool::Ironwood,
        };
        if let Some(action) = signable_action_decision(
            params,
            pool,
            checked_action.index,
            checked_action.spend_value,
            checked_action.spend_has_dummy_sk,
            checked_action
                .spend_derivation
                .as_ref()
                .map(|(fingerprint, path)| (fingerprint, path.as_slice())),
            seed_fingerprint,
            account_index,
            policy,
        )? {
            actions.push(action);
        }
    }
    Ok(actions)
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
        if let Some(signable) = signable_action_decision(
            params,
            pool,
            index,
            action.spend().value().map(|value| value.inner()),
            action.spend().dummy_sk().is_some(),
            action
                .spend()
                .zip32_derivation()
                .as_ref()
                .map(|derivation| {
                    (
                        derivation.seed_fingerprint(),
                        derivation.derivation_path().as_slice(),
                    )
                }),
            seed_fingerprint,
            account_index,
            policy,
        )
        .map_err(OrchardError::Custom)?
        {
            actions.push(signable);
        }
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
        &SpendAuthCache::new(),
    )
}

/// Signs a checked, normalized PCZT and confirms in memory that every
/// supported shielded action owned by (`seed_fingerprint`, `account_index`)
/// received a spend authorization signature. Batch policy: additionally rejects
/// PCZT shapes the batch flow does not support, including Orchard or Ironwood
/// spend authorization signatures, and requires at least one owned
/// signable shielded action. Parses `checked_pczt` exactly once and returns the
/// redacted, version-stamped response bytes. Derives keys into a fresh
/// [`SpendAuthCache`]; batch loops should use
/// [`sign_checked_batch_pczt_with_cache`] so PCZTs for the selected account
/// share one cached derivation.
#[cfg(feature = "cypherpunk")]
pub fn sign_checked_batch_pczt<P: consensus::Parameters>(
    params: &P,
    checked_pczt: &[u8],
    seed: &[u8],
    seed_fingerprint: &[u8; 32],
    account_index: u32,
) -> Result<Vec<u8>> {
    sign_checked_batch_pczt_with_cache(
        params,
        checked_pczt,
        seed,
        seed_fingerprint,
        account_index,
        &SpendAuthCache::new(),
    )
}

/// [`sign_checked_batch_pczt`] with a caller-provided [`SpendAuthCache`]. Create
/// one cache per signing request and pass it to every PCZT using the same
/// seed. The normal batch path reuses its selected account key, avoiding
/// repeated ZIP 32 derivation. An account change scrubs and replaces the slot.
#[cfg(feature = "cypherpunk")]
pub fn sign_checked_batch_pczt_with_cache<P: consensus::Parameters>(
    params: &P,
    checked_pczt: &[u8],
    seed: &[u8],
    seed_fingerprint: &[u8; 32],
    account_index: u32,
    ask_cache: &SpendAuthCache,
) -> Result<Vec<u8>> {
    sign_checked_pczt_with_policy(
        params,
        checked_pczt,
        seed,
        seed_fingerprint,
        account_index,
        ShieldedActionPolicy::Batch,
        ask_cache,
    )
}

/// Signs one firmware-owned checked batch PCZT using the signability decision
/// retained by [`check_batch_pczt_with_display`]. The decision is accepted only
/// for those exact normalized bytes and carries the selected account into the
/// low-level signer. The signer still independently matches each action's
/// derivation and `rk` to the seed-derived signing key.
#[cfg(feature = "cypherpunk")]
pub fn sign_checked_batch_pczt_with_cached_signability(
    checked_pczt: &[u8],
    checked_signability: &CheckedBatchPcztSignability,
    seed: &[u8],
    ask_cache: &SpendAuthCache,
) -> Result<Vec<u8>> {
    let (selected_account, required_actions) = checked_signability.signing_context(checked_pczt)?;
    let pczt = pczt::parse_pczt(checked_pczt)?;
    reject_unsupported_batch_pczt(&pczt)?;
    sign_pczt_with_required_actions(pczt, seed, selected_account, required_actions, ask_cache)
}

#[cfg(feature = "cypherpunk")]
#[allow(clippy::too_many_arguments)]
fn sign_checked_pczt_with_policy<P: consensus::Parameters>(
    params: &P,
    checked_pczt: &[u8],
    seed: &[u8],
    seed_fingerprint: &[u8; 32],
    account_index: u32,
    policy: ShieldedActionPolicy,
    ask_cache: &SpendAuthCache,
) -> Result<Vec<u8>> {
    let pczt = pczt::parse_pczt(checked_pczt)?;
    let account_index = zip32::AccountId::try_from(account_index)
        .map_err(|_e| ZcashError::InvalidDataError("invalid account index".to_string()))?;
    let (signable_actions, pczt) =
        signable_shielded_actions(params, pczt, seed_fingerprint, account_index, policy)?;
    if policy == ShieldedActionPolicy::Batch && signable_actions.is_empty() {
        return Err(ZcashError::PcztNoMyInputs);
    }
    sign_pczt_with_required_actions(pczt, seed, account_index, &signable_actions, ask_cache)
}

#[cfg(feature = "cypherpunk")]
fn sign_pczt_with_required_actions(
    pczt: Pczt,
    seed: &[u8],
    selected_account: zip32::AccountId,
    required_actions: &[SignableShieldedAction],
    ask_cache: &SpendAuthCache,
) -> Result<Vec<u8>> {
    let signed =
        pczt::sign::sign_and_redact_pczt_with_cache(pczt, seed, Some(selected_account), ask_cache)?;
    let signed = if required_actions.is_empty() {
        signed
    } else {
        ensure_shielded_actions_are_signed(signed, required_actions)?
    };
    signed
        .serialize()
        .map_err(|e| ZcashError::SigningError(alloc::format!("serialize signed PCZT: {e:?}")))
}

/// Extracts every Orchard-protocol spend authorization signature from a signed
/// PCZT. Errors if the PCZT is unparseable or carries no such signature.
#[cfg(feature = "cypherpunk")]
pub fn extract_compact_sigs_from_signed_pczt(
    signed_pczt: &[u8],
) -> Result<Vec<SpendAuthSignature>> {
    let signed_pczt = pczt::parse_pczt(signed_pczt)
        .map_err(|_| ZcashError::InvalidPczt("invalid signed pczt data".to_string()))?;
    let sigs =
        zcash_vendor::pczt::roles::signer::extract_orchard_spend_auth_signatures(&signed_pczt);

    if sigs.is_empty() {
        return Err(ZcashError::SigningError(
            "signed PCZT has no spend authorization signatures".to_string(),
        ));
    }

    Ok(sigs)
}

#[cfg(feature = "cypherpunk")]
#[cfg(test)]
mod tests {
    use alloc::{collections::BTreeMap, string::String, vec, vec::Vec};

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

    fn replace_unique_serialized_field(encoded: &mut [u8], original: &[u8], replacement: &[u8]) {
        assert_eq!(original.len(), replacement.len());
        let mut matches = encoded
            .windows(original.len())
            .enumerate()
            .filter_map(|(index, window)| (window == original).then_some(index));
        let index = matches
            .next()
            .expect("the original field must occur in the serialized PCZT");
        assert!(
            matches.next().is_none(),
            "the original field must occur exactly once"
        );
        encoded[index..index + original.len()].copy_from_slice(replacement);
    }

    fn assert_duplicate_rk_rejected(
        sample: &pczt::test_support::SamplePczt,
        malformed_pczt: &[u8],
    ) {
        let expected = "duplicate Orchard or Ironwood action rk";
        let ctx = BatchCheckContext::new(&sample.ufvk_text);

        for result in [
            parse_pczt_cypherpunk(
                &pczt::test_support::Nu6_3Network,
                malformed_pczt,
                &sample.ufvk_text,
                &sample.seed_fingerprint,
            )
            .map(|_| ()),
            check_pczt_cypherpunk(
                &pczt::test_support::Nu6_3Network,
                malformed_pczt,
                &sample.ufvk_text,
                &sample.seed_fingerprint,
                0,
            )
            .map(|_| ()),
            check_batch_pczt_with_display(
                &pczt::test_support::Nu6_3Network,
                malformed_pczt,
                &ctx,
                &sample.seed_fingerprint,
                0,
            )
            .map(|_| ()),
            sign_pczt(malformed_pczt, &sample.seed).map(|_| ()),
            sign_checked_pczt(
                &pczt::test_support::Nu6_3Network,
                malformed_pczt,
                &sample.seed,
                &sample.seed_fingerprint,
                0,
            )
            .map(|_| ()),
            sign_checked_batch_pczt(
                &pczt::test_support::Nu6_3Network,
                malformed_pczt,
                &sample.seed,
                &sample.seed_fingerprint,
                0,
            )
            .map(|_| ()),
        ] {
            assert_invalid_pczt_message(result, expected);
        }
    }

    #[test]
    fn test_all_paths_reject_duplicate_orchard_rk() {
        use zcash_vendor::{pasta_curves::group::ff::PrimeField, pczt::roles::verifier::Verifier};

        let sample = pczt::test_support::sample_orchard_change_pczt();
        let parsed = Pczt::parse(&sample.bytes).expect("sample PCZT should parse");
        let actions = parsed.orchard().actions();
        assert_eq!(actions.len(), 2, "sample must contain two Orchard actions");
        let retained_rk = *actions[0].spend().rk();
        let replaced_rk = *actions[1].spend().rk();
        assert_ne!(retained_rk, replaced_rk);

        let mut alphas = Vec::new();
        Verifier::new(parsed)
            .with_orchard::<ZcashError, _>(|bundle| {
                for action in bundle.actions() {
                    action.spend().verify_rk(None)?;
                    alphas.push(
                        action
                            .spend()
                            .alpha()
                            .as_ref()
                            .expect("sample spend must contain alpha")
                            .to_repr(),
                    );
                }
                Ok(())
            })
            .expect("sample Orchard bundle should verify");
        assert_eq!(alphas.len(), 2);
        assert_ne!(alphas[0], alphas[1]);

        let mut malformed_pczt = sample.bytes.clone();
        replace_unique_serialized_field(&mut malformed_pczt, &alphas[1], &alphas[0]);
        replace_unique_serialized_field(&mut malformed_pczt, &replaced_rk, &retained_rk);
        let reparsed = Pczt::parse(&malformed_pczt).expect("modified PCZT should still parse");
        assert_eq!(
            reparsed.orchard().actions()[0].spend().rk(),
            reparsed.orchard().actions()[1].spend().rk(),
        );
        Verifier::new(reparsed)
            .with_orchard::<ZcashError, _>(|bundle| {
                for action in bundle.actions() {
                    action.spend().verify_rk(None)?;
                }
                Ok(())
            })
            .expect("both duplicate rk values should match their copied alpha");

        assert_duplicate_rk_rejected(&sample, &malformed_pczt);
    }

    #[test]
    fn test_all_paths_reject_duplicate_rk_across_orchard_and_ironwood() {
        let sample = pczt::test_support::sample_migration_pczt();
        let parsed = Pczt::parse(&sample.bytes).expect("sample PCZT should parse");
        let orchard_rk = *parsed.orchard().actions()[0].spend().rk();
        let ironwood_rk = *parsed.ironwood().actions()[0].spend().rk();
        assert_ne!(orchard_rk, ironwood_rk);

        let mut malformed_pczt = sample.bytes.clone();
        replace_unique_serialized_field(&mut malformed_pczt, &ironwood_rk, &orchard_rk);
        let reparsed = Pczt::parse(&malformed_pczt).expect("modified PCZT should still parse");
        assert_eq!(
            reparsed.orchard().actions()[0].spend().rk(),
            reparsed.ironwood().actions()[0].spend().rk(),
        );

        assert_duplicate_rk_rejected(&sample, &malformed_pczt);
    }

    fn malformed_pczt_with_empty_sapling_bundle_and_nonzero_value_sum() -> Vec<u8> {
        use ::pczt::roles::creator::Creator;
        use zcash_vendor::zcash_protocol::consensus::{BranchId, NetworkConstants};

        let bytes = Creator::new(
            BranchId::Nu6.into(),
            10,
            MainNetwork.coin_type(),
            Some([0; 32]),
            Some([0; 32]),
        )
        .unwrap()
        .build()
        .unwrap()
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
            // Exercise the legacy cross-address-enabled Orchard format. NU6.3
            // rejects this spoof at construction before the wallet check runs.
            2_000_000.into(),
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
        let ctx = BatchCheckContext::new(&sample.ufvk_text);

        for result in [
            check_pczt_cypherpunk(
                &pczt::test_support::Nu6_3Network,
                &sample.bytes,
                &sample.ufvk_text,
                &sample.seed_fingerprint,
                0,
            )
            .map(|_| ()),
            check_batch_pczt_with_display(
                &pczt::test_support::Nu6_3Network,
                &sample.bytes,
                &ctx,
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
    fn test_v1_pczt_orchard_check_parse_and_sign() {
        let sample = pczt::test_support::sample_legacy_orchard_change_pczt();
        let v1_pczt = ::pczt::v1::Pczt::try_from(Pczt::parse(&sample.bytes).unwrap())
            .unwrap()
            .serialize();
        assert_eq!(&v1_pczt[..8], b"PCZT\x01\0\0\0");

        let parsed = parse_pczt_cypherpunk(
            &MainNetwork,
            &v1_pczt,
            &sample.ufvk_text,
            &sample.seed_fingerprint,
        )
        .unwrap();
        assert!(parsed.get_ironwood().is_none());
        let orchard = parsed
            .get_orchard()
            .expect("v1 Orchard bundle should decode");
        assert_eq!(orchard.get_from().len(), 1);
        assert!(orchard.get_from()[0].get_is_mine());
        assert_eq!(orchard.get_from()[0].get_value(), "0.01 ZEC");
        assert_eq!(orchard.get_to().len(), 1);
        assert_eq!(orchard.get_to()[0].get_value(), "0.0099 ZEC");
        assert_eq!(parsed.get_fee_value(), "0.0001 ZEC");

        let normalized = check_pczt_cypherpunk(
            &MainNetwork,
            &v1_pczt,
            &sample.ufvk_text,
            &sample.seed_fingerprint,
            0,
        )
        .unwrap();
        let signed = sign_checked_pczt(
            &MainNetwork,
            &normalized,
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
    }

    #[test]
    fn test_parse_ignores_and_check_rejects_unsupported_ironwood_spend_zip32_path() {
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
            assert_invalid_pczt_message(
                check_pczt_cypherpunk(
                    &pczt::test_support::Nu6_3Network,
                    &pczt,
                    &sample.ufvk_text,
                    &sample.seed_fingerprint,
                    0,
                ),
                "unsupported Ironwood spend ZIP 32 derivation path",
            );
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

    fn assert_existing_batch_signature_error<T: core::fmt::Debug>(result: Result<T>, pool: &str) {
        assert_eq!(
            result.unwrap_err(),
            ZcashError::InvalidPczt(alloc::format!(
                "Zcash batch request must not contain {pool} spend authorization signatures"
            ))
        );
    }

    fn pczt_with_existing_funded_signature(
        mut sample: pczt::test_support::SamplePczt,
        pool: SignableShieldedPool,
    ) -> pczt::test_support::SamplePczt {
        use zcash_vendor::{
            orchard::keys::{SpendAuthorizingKey, SpendingKey},
            pczt::roles::signer::Signer,
            zip32,
        };

        let original = Pczt::parse(&sample.bytes).expect("sample PCZT should parse");
        let account = zip32::AccountId::ZERO;
        let (required_actions, original) = match pool {
            SignableShieldedPool::Orchard => signable_shielded_actions(
                &MainNetwork,
                original,
                &sample.seed_fingerprint,
                account,
                ShieldedActionPolicy::Single,
            ),
            SignableShieldedPool::Ironwood => signable_shielded_actions(
                &pczt::test_support::Nu6_3Network,
                original,
                &sample.seed_fingerprint,
                account,
                ShieldedActionPolicy::Single,
            ),
        }
        .expect("sample PCZT should have a signable funded action");
        assert_eq!(required_actions.len(), 1);

        let spending_key = SpendingKey::from_zip32_seed(&sample.seed, 133, account).unwrap();
        let ask = SpendAuthorizingKey::from(&spending_key);
        let mut signer = Signer::new(original).expect("sample PCZT should be signable");
        let action_index = required_actions[0].index;
        let signing_result = match pool {
            SignableShieldedPool::Orchard => signer.sign_orchard(action_index, &ask),
            SignableShieldedPool::Ironwood => signer.sign_ironwood(action_index, &ask),
        };
        signing_result.expect("funded action should sign");
        let pczt = signer.finish();
        let actions = match pool {
            SignableShieldedPool::Orchard => pczt.orchard().actions(),
            SignableShieldedPool::Ironwood => pczt.ironwood().actions(),
        };
        assert_eq!(
            actions
                .iter()
                .filter(|action| action.spend().spend_auth_sig().is_some())
                .count(),
            1,
        );
        assert!(actions
            .iter()
            .any(|action| action.spend().spend_auth_sig().is_none()));
        sample.bytes = pczt
            .serialize()
            .expect("partially signed PCZT should serialize");
        sample
    }

    #[test]
    fn test_batch_rejects_existing_funded_spend_auth_signature() {
        for (sample, pool) in [
            (
                pczt::test_support::sample_orchard_change_pczt(),
                SignableShieldedPool::Orchard,
            ),
            (
                pczt::test_support::sample_ironwood_pczt(),
                SignableShieldedPool::Ironwood,
            ),
        ] {
            let sample = pczt_with_existing_funded_signature(sample, pool);

            assert_existing_batch_signature_error(
                check_batch_pczt_with_display(
                    &pczt::test_support::Nu6_3Network,
                    &sample.bytes,
                    &BatchCheckContext::new(&sample.ufvk_text),
                    &sample.seed_fingerprint,
                    0,
                ),
                pool.label(),
            );
            assert_existing_batch_signature_error(
                sign_checked_batch_pczt(
                    &pczt::test_support::Nu6_3Network,
                    &sample.bytes,
                    &sample.seed,
                    &sample.seed_fingerprint,
                    0,
                ),
                pool.label(),
            );
        }
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

        let compact_sigs =
            extract_compact_sigs_from_signed_pczt(&signed).expect("compact sigs should extract");
        assert!(compact_sigs
            .iter()
            .any(|sig| sig.value_pool() == zcash_vendor::orchard::ValuePool::Ironwood));
    }

    #[test]
    fn test_cached_batch_signability_binds_pczt_and_account() {
        let sample = pczt::test_support::sample_orchard_change_pczt();
        let (normalized, _, _, mut signability) = check_batch_pczt_with_display(
            &pczt::test_support::Nu6_3Network,
            &sample.bytes,
            &BatchCheckContext::new(&sample.ufvk_text),
            &sample.seed_fingerprint,
            0,
        )
        .unwrap();
        // The C bridge performs this canonical round trip when rebuilding the
        // checked batch envelope.
        let canonical = Pczt::parse(&normalized).unwrap().serialize().unwrap();
        let signed = sign_checked_batch_pczt_with_cached_signability(
            &canonical,
            &signability,
            &sample.seed,
            &SpendAuthCache::new(),
        )
        .unwrap();
        assert_eq!(
            extract_compact_sigs_from_signed_pczt(&signed)
                .unwrap()
                .len(),
            2,
            "the signer must still sign the wallet-controlled zero-value action"
        );

        let mut different_pczt = canonical.clone();
        different_pczt[0] ^= 1;
        assert!(matches!(
            sign_checked_batch_pczt_with_cached_signability(
                &different_pczt,
                &signability,
                &sample.seed,
                &SpendAuthCache::new(),
            ),
            Err(ZcashError::InvalidDataError(_))
        ));

        signability.selected_account = zip32::AccountId::try_from(1).unwrap();
        assert!(matches!(
            sign_checked_batch_pczt_with_cached_signability(
                &canonical,
                &signability,
                &sample.seed,
                &SpendAuthCache::new(),
            ),
            Err(ZcashError::PcztNoMyInputs)
        ));
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
    fn test_check_and_sign_reject_unselected_account_spend() {
        let sample = pczt::test_support::sample_migration_pczt_from_account(1);
        let check_result = check_pczt_cypherpunk(
            &pczt::test_support::Nu6_3Network,
            &sample.bytes,
            &sample.ufvk_text,
            &sample.seed_fingerprint,
            0,
        );
        assert!(matches!(check_result, Err(ZcashError::PcztNoMyInputs)));

        let sign_result = sign_checked_pczt(
            &pczt::test_support::Nu6_3Network,
            &sample.bytes,
            &sample.seed,
            &sample.seed_fingerprint,
            0,
        );
        assert!(matches!(sign_result, Err(ZcashError::PcztNoMyInputs)));
    }

    #[test]
    fn test_check_and_sign_pczt_reject_foreign_seed() {
        let sample = pczt::test_support::sample_orchard_change_pczt();
        let foreign_seed = [9u8; 32];
        let foreign_fingerprint = calculate_seed_fingerprint(&foreign_seed).unwrap();

        let check_result = check_pczt_cypherpunk(
            &pczt::test_support::Nu6_3Network,
            &sample.bytes,
            &sample.ufvk_text,
            &foreign_fingerprint,
            0,
        );
        assert!(matches!(check_result, Err(ZcashError::PcztNoMyInputs)));

        let normalized = check_pczt_cypherpunk(
            &pczt::test_support::Nu6_3Network,
            &sample.bytes,
            &sample.ufvk_text,
            &sample.seed_fingerprint,
            0,
        )
        .unwrap();

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
    fn test_check_pczt_accepts_owned_transparent_input() {
        let sample = pczt::legacy_test_support::legacy_transparent_sample();
        let ufvk = derive_ufvk(&MainNetwork, &sample.seed, "m/32'/133'/0'").unwrap();

        check_pczt_cypherpunk(
            &MainNetwork,
            &sample.bytes,
            &ufvk,
            &sample.seed_fingerprint,
            0,
        )
        .expect("an owned transparent input satisfies singleton ownership");
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

        let (_, parsed, _, _) = check_batch_pczt_with_display(
            &pczt::test_support::Nu6_3Network,
            &sample.bytes,
            &BatchCheckContext::new(&sample.ufvk_text),
            &sample.seed_fingerprint,
            0,
        )
        .expect("Ironwood batch PCZT should parse");
        assert!(parsed.get_orchard().is_some() || parsed.get_ironwood().is_some());

        assert_eq!(
            check_batch_pczt_with_display(
                &pczt::test_support::Nu6_3Network,
                &sample.bytes,
                &BatchCheckContext::new(&sample.ufvk_text),
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

        let (_, parsed, _, _) = check_batch_pczt_with_display(
            &pczt::test_support::Nu6_3Network,
            &sample.bytes,
            &BatchCheckContext::new(&sample.ufvk_text),
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
            check_batch_pczt_with_display(
                &pczt::test_support::Nu6_3Network,
                &sample.bytes,
                &BatchCheckContext::new(&sample.ufvk_text),
                &sample.seed_fingerprint,
                1,
            )
            .unwrap_err(),
            ZcashError::PcztNoMyInputs
        );
    }

    // An undecryptable funded output must fail the fused check and display path.
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
                .expect("migration transfer must contain a non-zero Ironwood output")
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
                check_batch_pczt_with_display(
                    &pczt::test_support::Nu6_3Network,
                    &corrupted,
                    &BatchCheckContext::new(&sample.ufvk_text),
                    &sample.seed_fingerprint,
                    0,
                ),
                Err(ZcashError::InvalidPczt(message)) if message.contains("undecryptable")
            ),
            "single-pass batch review must also reject the undecryptable output"
        );
    }

    #[test]
    fn test_batch_migration_summary_accepts_orchard_to_ironwood_transfer() {
        let sample = pczt::test_support::sample_migration_pczt();
        let parsed = check_and_parse_batch_pczt_cypherpunk(
            &pczt::test_support::Nu6_3Network,
            &sample.bytes,
            &sample.ufvk_text,
            &sample.seed_fingerprint,
            0,
        )
        .expect("migration transfer should pass batch review");
        let transfer =
            migration_transfer_summary(&parsed).expect("migration transfer should summarize");
        assert_eq!(
            transfer,
            BatchMigrationTransferSummary {
                input: 1_010_000,
                output: 990_000,
                fee: 20_000,
            }
        );

        let mut summary = BatchMigrationSummary::default();
        summary.add_transfer(transfer).unwrap();

        assert_eq!(
            summary,
            BatchMigrationSummary {
                total_output: 990_000,
                total_fee: 20_000,
                transfers: vec![transfer],
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
    fn test_batch_migration_summary_aggregates_all_migration_only_batch() {
        let samples = [
            pczt::test_support::sample_migration_pczt(),
            pczt::test_support::sample_migration_pczt(),
            pczt::test_support::sample_migration_pczt(),
        ];
        let checked = samples
            .iter()
            .map(|sample| {
                check_batch_pczt_cypherpunk(
                    &pczt::test_support::Nu6_3Network,
                    &sample.bytes,
                    &sample.ufvk_text,
                    &sample.seed_fingerprint,
                    0,
                )
                .unwrap()
            })
            .collect::<Vec<_>>();
        let parsed = parse_batch_with_migration_summary_cypherpunk(
            &pczt::test_support::Nu6_3Network,
            checked.iter().map(Vec::as_slice),
            &samples[0].ufvk_text,
            &samples[0].seed_fingerprint,
        )
        .expect("all migration transfers should aggregate");

        assert_eq!(parsed.len(), 1);
        let summary = &parsed[0];
        assert_eq!(summary.get_total_transfer_value(), "0.0297 ZEC");
        assert_eq!(summary.get_fee_value(), "0.0006 ZEC");
        let inputs = summary.get_orchard().unwrap().get_from();
        let outputs = summary.get_ironwood().unwrap().get_to();
        assert_eq!(inputs.len(), 3);
        assert!(inputs.iter().all(|input| input.get_amount() == 1_010_000));
        assert_eq!(outputs.len(), 3);
        assert!(outputs.iter().all(|output| output.get_amount() == 990_000));
    }

    #[test]
    fn test_batch_migration_summary_is_order_independent() {
        let split = pczt::test_support::sample_orchard_change_pczt();
        let migration_1 = pczt::test_support::sample_migration_pczt();
        let migration_2 = pczt::test_support::sample_migration_pczt();
        let ufvk_text = split.ufvk_text.clone();
        let seed_fingerprint = split.seed_fingerprint;
        let check = |bytes: &[u8]| {
            check_batch_pczt_cypherpunk(
                &pczt::test_support::Nu6_3Network,
                bytes,
                &ufvk_text,
                &seed_fingerprint,
                0,
            )
            .unwrap()
        };
        let split = check(&split.bytes);
        let migration_1 = check(&migration_1.bytes);
        let migration_2 = check(&migration_2.bytes);
        let orders = [
            [
                split.as_slice(),
                migration_1.as_slice(),
                migration_2.as_slice(),
            ],
            [
                migration_1.as_slice(),
                split.as_slice(),
                migration_2.as_slice(),
            ],
            [
                migration_1.as_slice(),
                migration_2.as_slice(),
                split.as_slice(),
            ],
        ];

        for order in orders {
            let parsed = parse_batch_with_migration_summary_cypherpunk(
                &pczt::test_support::Nu6_3Network,
                order,
                &ufvk_text,
                &seed_fingerprint,
            )
            .expect("PCZT order must not affect migration classification");

            assert_eq!(parsed.len(), 2);
            let summary = &parsed[1];
            assert_eq!(summary.get_ironwood().unwrap().get_to().len(), 2);
            assert_eq!(summary.get_total_transfer_value(), "0.0198 ZEC");
            assert_eq!(summary.get_fee_value(), "0.0004 ZEC");
        }
    }

    #[test]
    fn test_batch_migration_summary_keeps_single_pczt_uncompacted() {
        let sample = pczt::test_support::sample_migration_pczt();
        let checked = check_batch_pczt_cypherpunk(
            &pczt::test_support::Nu6_3Network,
            &sample.bytes,
            &sample.ufvk_text,
            &sample.seed_fingerprint,
            0,
        )
        .unwrap();
        let parsed = parse_batch_with_migration_summary_cypherpunk(
            &pczt::test_support::Nu6_3Network,
            core::iter::once(checked.as_slice()),
            &sample.ufvk_text,
            &sample.seed_fingerprint,
        )
        .unwrap();

        assert_eq!(parsed.len(), 1);
        assert_eq!(
            parsed[0].get_ironwood().unwrap().get_to()[0].get_address(),
            "<internal-address>"
        );
    }

    #[test]
    fn test_batch_migration_summary_rejects_empty_batch() {
        let sample = pczt::test_support::sample_migration_pczt();
        assert_invalid_pczt_message(
            parse_batch_with_migration_summary_cypherpunk(
                &pczt::test_support::Nu6_3Network,
                core::iter::empty(),
                &sample.ufvk_text,
                &sample.seed_fingerprint,
            ),
            "batch review has no transactions",
        );
    }

    #[test]
    fn test_batch_migration_summary_compacts_three_splits_and_thirty_migrations() {
        let ordinary = pczt::test_support::sample_orchard_change_pczt();
        let migration = pczt::test_support::sample_migration_pczt();
        let context = BatchCheckContext::new(&ordinary.ufvk_text);
        let (_, ordinary, ordinary_migration, _) = check_batch_pczt_with_display(
            &pczt::test_support::Nu6_3Network,
            &ordinary.bytes,
            &context,
            &ordinary.seed_fingerprint,
            0,
        )
        .unwrap();
        let (_, migration, migration_summary, _) = check_batch_pczt_with_display(
            &pczt::test_support::Nu6_3Network,
            &migration.bytes,
            &context,
            &migration.seed_fingerprint,
            0,
        )
        .unwrap();
        assert!(ordinary_migration.is_none());
        assert!(migration_summary.is_some());

        let mut checked = Vec::with_capacity(33);
        for index in 1..=3 {
            let mut split = ordinary.clone();
            split.set_total_transfer_value(format!("split-{index}"));
            checked.push((split, None));
        }
        checked.extend(core::iter::repeat_with(|| (migration.clone(), migration_summary)).take(30));

        let parsed = compact_checked_batch_migration_review(checked);

        assert_eq!(parsed.len(), 4);
        assert_eq!(parsed[0].get_total_transfer_value(), "split-1");
        assert_eq!(parsed[1].get_total_transfer_value(), "split-2");
        assert_eq!(parsed[2].get_total_transfer_value(), "split-3");
        assert!(parsed[..3].iter().all(|item| item.get_ironwood().is_none()));

        let summary = &parsed[3];
        assert_eq!(summary.get_total_transfer_value(), "0.297 ZEC");
        assert_eq!(summary.get_fee_value(), "0.006 ZEC");
        assert_eq!(summary.get_orchard().unwrap().get_from().len(), 30);
        assert_eq!(summary.get_ironwood().unwrap().get_to().len(), 30);
        assert_eq!(
            summary.get_ironwood().unwrap().get_to()[29].get_address(),
            "Migration #30 wallet Ironwood output"
        );
    }

    // A memo on the funded output forces fallback to the review that displays it.
    #[test]
    fn test_batch_migration_summary_rejects_memo_carrying_output() {
        use zcash_vendor::zcash_protocol::memo::MemoBytes;

        let sample = pczt::test_support::sample_migration_pczt_with_output_memo(
            MemoBytes::from_bytes(b"covert note").expect("memo text fits"),
        );

        let parsed = check_and_parse_batch_pczt_cypherpunk(
            &pczt::test_support::Nu6_3Network,
            &sample.bytes,
            &sample.ufvk_text,
            &sample.seed_fingerprint,
            0,
        )
        .expect("review for each PCZT must accept the memo-carrying transfer");
        let shown_memo = parsed
            .get_ironwood()
            .expect("migration must show Ironwood outputs")
            .get_to()
            .first()
            .expect("migration must show the real output")
            .get_memo();
        assert_eq!(shown_memo.as_deref(), Some("covert note"));
        assert!(migration_transfer_summary(&parsed).is_none());

        let migration = pczt::test_support::sample_migration_pczt();
        let checked = [&migration, &sample]
            .into_iter()
            .map(|item| {
                check_batch_pczt_cypherpunk(
                    &pczt::test_support::Nu6_3Network,
                    &item.bytes,
                    &item.ufvk_text,
                    &item.seed_fingerprint,
                    0,
                )
                .unwrap()
            })
            .collect::<Vec<_>>();
        let batch = parse_batch_with_migration_summary_cypherpunk(
            &pczt::test_support::Nu6_3Network,
            checked.iter().map(Vec::as_slice),
            &sample.ufvk_text,
            &sample.seed_fingerprint,
        )
        .unwrap();
        assert_eq!(batch.len(), 2);
        assert!(batch.iter().any(|item| {
            item.get_ironwood()
                .map(|pool| {
                    pool.get_to()
                        .iter()
                        .any(|to| to.get_memo().as_deref() == Some("covert note"))
                })
                .unwrap_or(false)
        }));
    }

    #[test]
    fn test_batch_migration_summary_rejects_foreign_funded_output() {
        let sample = pczt::test_support::sample_migration_pczt_to_account(1);

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
        assert!(migration_transfer_summary(&parsed).is_none());
    }

    #[test]
    fn test_batch_migration_summary_ignores_dummy_equivalent_zero_output() {
        use zcash_vendor::zcash_protocol::memo::MemoBytes;

        let sample = pczt::test_support::sample_migration_pczt_with_zero_output(
            MemoBytes::from_bytes(b"hidden dummy memo").unwrap(),
            false,
        );
        let parsed = check_and_parse_batch_pczt_cypherpunk(
            &pczt::test_support::Nu6_3Network,
            &sample.bytes,
            &sample.ufvk_text,
            &sample.seed_fingerprint,
            0,
        )
        .expect("ordinary review should accept the zero output");
        let transfer = migration_transfer_summary(&parsed)
            .expect("dummy-equivalent zero output should not block the summary");
        assert_eq!(transfer.output, 990_000);
        assert_eq!(transfer.fee, 20_000);
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
        assert!(migration_transfer_summary(&parsed).is_none());
    }

    // Transparent or Sapling components force fallback because the compact summary
    // cannot display them or include transparent values in its fee.
    #[test]
    fn test_migration_classifier_rejects_transparent_and_sapling() {
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
        migration_transfer_summary(&build(None, false))
            .expect("a shielded-only Orchard-to-Ironwood transfer must still summarize");

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
        assert!(migration_transfer_summary(&build(Some(transparent), false)).is_none());

        // A Sapling component must likewise force fallback.
        assert!(migration_transfer_summary(&build(None, true)).is_none());
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
                .expect("migration transfer must contain a non-zero Ironwood output")
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

        let summary_err = parse_batch_with_migration_summary_cypherpunk(
            &pczt::test_support::Nu6_3Network,
            core::iter::once(corrupted.as_slice()),
            &sample.ufvk_text,
            &sample.seed_fingerprint,
        )
        .expect_err("summary must reject a migration transfer with an undecryptable output");
        assert!(
            matches!(&summary_err, ZcashError::InvalidPczt(message) if message.contains("undecryptable")),
            "expected an undecryptable-output rejection, got {summary_err:?}"
        );

        // The ordinary review must enforce the same output recovery rule.
        assert!(
            matches!(
                check_batch_pczt_with_display(
                    &pczt::test_support::Nu6_3Network,
                    &corrupted,
                    &BatchCheckContext::new(&sample.ufvk_text),
                    &sample.seed_fingerprint,
                    0,
                ),
                Err(ZcashError::InvalidPczt(message)) if message.contains("undecryptable")
            ),
            "ordinary review for each PCZT must also reject the undecryptable output"
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

        let parsed = check_and_parse_batch_pczt_cypherpunk(
            &pczt::test_support::Nu6_3Network,
            &redacted,
            &sample.ufvk_text,
            &sample.seed_fingerprint,
            0,
        )
        .expect("redacted migration transfer should pass batch review");
        let transfer = migration_transfer_summary(&parsed)
            .expect("redacted migration transfer should summarize");

        assert_eq!(transfer.input, 1_010_000);
        assert_eq!(transfer.output, 990_000);
        assert_eq!(transfer.fee, 20_000);

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
        assert_batch_unsupported_sapling_error(check_batch_pczt_with_display(
            &pczt::test_support::Nu6_3Network,
            &sample.bytes,
            &BatchCheckContext::new(&sample.ufvk_text),
            &sample.seed_fingerprint,
            0,
        ));
    }

    /// The display-producing check must return byte-identical normalized bytes
    /// to the reference `check_batch_pczt_cypherpunk` for the same input, across
    /// an Orchard change tx, an Ironwood tx, and an Orchard->Ironwood migration.
    #[test]
    fn test_check_batch_with_display_bytes_match_reference_check() {
        let samples = [
            pczt::test_support::sample_orchard_change_pczt(),
            pczt::test_support::sample_ironwood_pczt(),
            pczt::test_support::sample_migration_pczt(),
        ];
        let ctx = BatchCheckContext::new(&samples[0].ufvk_text);
        for sample in &samples {
            let reference = check_batch_pczt_cypherpunk(
                &pczt::test_support::Nu6_3Network,
                &sample.bytes,
                &sample.ufvk_text,
                &sample.seed_fingerprint,
                0,
            )
            .unwrap();
            let (bytes, _parsed, _summary, _) = check_batch_pczt_with_display(
                &pczt::test_support::Nu6_3Network,
                &sample.bytes,
                &ctx,
                &sample.seed_fingerprint,
                0,
            )
            .unwrap();
            assert_eq!(
                bytes, reference,
                "display check bytes must match the reference check"
            );
        }
    }

    /// The display rows produced during the combined check must equal a fresh
    /// `parse_pczt_cypherpunk` over the normalized bytes (no PartialEq on
    /// `ParsedPczt`, so compare the derived-`Debug` renderings, which cover every
    /// display field).
    #[test]
    fn test_check_batch_with_display_rows_match_fresh_parse() {
        for sample in [
            pczt::test_support::sample_orchard_change_pczt(),
            pczt::test_support::sample_ironwood_pczt(),
            pczt::test_support::sample_migration_pczt(),
        ] {
            let (bytes, parsed, _summary, _) = check_batch_pczt_with_display(
                &pczt::test_support::Nu6_3Network,
                &sample.bytes,
                &BatchCheckContext::new(&sample.ufvk_text),
                &sample.seed_fingerprint,
                0,
            )
            .unwrap();
            let reparsed = parse_pczt_cypherpunk(
                &pczt::test_support::Nu6_3Network,
                &bytes,
                &sample.ufvk_text,
                &sample.seed_fingerprint,
            )
            .unwrap();
            assert_eq!(
                alloc::format!("{parsed:?}"),
                alloc::format!("{reparsed:?}"),
                "display rows must equal a fresh parse of the normalized bytes"
            );
        }
    }

    /// A migration-shaped row retains the values used by the compact summary.
    #[test]
    fn test_check_batch_with_display_caches_migration_classification() {
        let sample = pczt::test_support::sample_migration_pczt();
        let (_bytes, _parsed, summary, _) = check_batch_pczt_with_display(
            &pczt::test_support::Nu6_3Network,
            &sample.bytes,
            &BatchCheckContext::new(&sample.ufvk_text),
            &sample.seed_fingerprint,
            0,
        )
        .unwrap();
        let summary = summary.expect("migration transfer should be compact-eligible");
        assert_eq!(summary.input, 1_010_000);
        assert_eq!(summary.output, 990_000);
        assert_eq!(summary.fee, 20_000);
    }

    /// A non-migration-shaped PCZT (here, a memo-carrying migration PCZT that the
    /// amounts-only summary cannot render) must still pass the check successfully but
    /// yield `None` for the aggregate summary, so the caller falls back to the
    /// display for that PCZT, which shows the memo.
    #[test]
    fn test_check_batch_with_display_non_migration_yields_no_summary() {
        use zcash_vendor::zcash_protocol::memo::MemoBytes;

        let sample = pczt::test_support::sample_migration_pczt_with_output_memo(
            MemoBytes::from_bytes(b"covert note").expect("memo text fits"),
        );
        let (_bytes, parsed, summary, _) = check_batch_pczt_with_display(
            &pczt::test_support::Nu6_3Network,
            &sample.bytes,
            &BatchCheckContext::new(&sample.ufvk_text),
            &sample.seed_fingerprint,
            0,
        )
        .unwrap();
        assert!(
            summary.is_none(),
            "a memo-carrying PCZT must not fold into the aggregate summary"
        );
        let shown = parsed
            .get_ironwood()
            .expect("migration must show Ironwood outputs")
            .get_to()
            .first()
            .expect("migration must show the real output")
            .get_memo();
        assert_eq!(shown.as_deref(), Some("covert note"));
    }
}
