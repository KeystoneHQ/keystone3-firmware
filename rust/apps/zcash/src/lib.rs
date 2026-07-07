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
use pczt::structs::ParsedPczt;
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

/// Validates a Partially Created Zcash Transaction (PCZT) against a Unified Full Viewing Key.
///
/// # Parameters
/// * `params` - The consensus parameters for the Zcash network (mainnet or testnet)
/// * `pczt` - The binary representation of the PCZT to validate
/// * `ufvk_text` - The string representation of the Unified Full Viewing Key
/// * `seed_fingerprint` - A 32-byte fingerprint of the seed used to derive keys
/// * `account_index` - The account index for the keys to check against
///
/// # Returns
/// * `Result<()>` - Ok if the PCZT is valid for the given UFVK, or an error otherwise
///
/// # Errors
/// * `ZcashError::InvalidDataError` - If the UFVK cannot be decoded or the account index is invalid
/// * `ZcashError::InvalidPczt` - If the PCZT data is malformed or cannot be parsed
/// * Other errors from the underlying validation process
#[cfg(feature = "cypherpunk")]
pub fn check_pczt_cypherpunk<P: consensus::Parameters>(
    params: &P,
    pczt: &[u8],
    ufvk_text: &str,
    seed_fingerprint: &[u8; 32],
    account_index: u32,
) -> Result<()> {
    let pczt = pczt::parse_pczt(pczt)?;
    check_parsed_pczt_cypherpunk(params, &pczt, ufvk_text, seed_fingerprint, account_index)
}

/// `check_pczt_cypherpunk` against an already-parsed PCZT, so preflight can
/// parse once and reuse the parsed value for normalization.
#[cfg(feature = "cypherpunk")]
fn check_parsed_pczt_cypherpunk<P: consensus::Parameters>(
    params: &P,
    pczt: &Pczt,
    ufvk_text: &str,
    seed_fingerprint: &[u8; 32],
    account_index: u32,
) -> Result<()> {
    let account_index = zip32::AccountId::try_from(account_index)
        .map_err(|_e| ZcashError::InvalidDataError("invalid account index".to_string()))?;
    let ufvk = UnifiedFullViewingKey::decode(params, ufvk_text)
        .map_err(|e| ZcashError::InvalidDataError(e.to_string()))?;
    let xpub = ufvk.transparent().ok_or(ZcashError::InvalidDataError(
        "transparent xpub is not present".to_string(),
    ))?;
    pczt::check::check_pczt_orchard(params, seed_fingerprint, account_index, &ufvk, pczt)?;
    pczt::check::check_pczt_transparent(
        params,
        seed_fingerprint,
        account_index,
        xpub,
        pczt,
        false,
    )?;
    Ok(())
}

/// Parses, policy-checks, and re-serializes a PCZT in one pass.
///
/// Returns the normalized (current-version) encoding of the checked PCZT: the
/// bytes C retains as the `checked_PCZT`, which display and signing consume
/// without re-running these checks.
#[cfg(feature = "cypherpunk")]
pub fn preflight_pczt_cypherpunk<P: consensus::Parameters>(
    params: &P,
    pczt_bytes: &[u8],
    ufvk_text: &str,
    seed_fingerprint: &[u8; 32],
    account_index: u32,
) -> Result<Vec<u8>> {
    let pczt = pczt::parse_pczt(pczt_bytes)?;
    // FUTURE(omitted-field-recompute): recompute-or-check omitted fields here,
    // mutating `pczt` so the normalized bytes carry the verified values forward.
    check_parsed_pczt_cypherpunk(params, &pczt, ufvk_text, seed_fingerprint, account_index)?;
    Ok(pczt.serialize())
}

/// Batch preflight for one `ZcashSignBatch` message: parses once, runs the full
/// policy checks, enforces the batch shielded-action policy (the PCZT must be
/// batch-signable by this account), and returns the normalized encoding. See
/// `preflight_pczt_cypherpunk` for the normalization contract.
#[cfg(feature = "cypherpunk")]
pub fn preflight_batch_pczt_cypherpunk<P: consensus::Parameters>(
    params: &P,
    pczt_bytes: &[u8],
    ufvk_text: &str,
    seed_fingerprint: &[u8; 32],
    account_index: u32,
) -> Result<Vec<u8>> {
    let pczt = pczt::parse_pczt(pczt_bytes)?;
    // FUTURE(omitted-field-recompute): recompute-or-check omitted fields here,
    // as in preflight_pczt_cypherpunk.
    check_parsed_pczt_cypherpunk(params, &pczt, ufvk_text, seed_fingerprint, account_index)?;
    let account_id = zip32::AccountId::try_from(account_index)
        .map_err(|_e| ZcashError::InvalidDataError("invalid account index".to_string()))?;
    let (actions, pczt) = signable_shielded_actions(
        params,
        pczt,
        seed_fingerprint,
        account_id,
        ShieldedActionPolicy::Batch,
    )?;
    if actions.is_empty() {
        return Err(ZcashError::PcztNoMyInputs);
    }
    Ok(pczt.serialize())
}

#[cfg(feature = "multi_coins")]
pub fn check_pczt_multi_coins<P: consensus::Parameters>(
    params: &P,
    pczt: &[u8],
    xpub: &str,
    seed_fingerprint: &[u8; 32],
    account_index: u32,
) -> Result<()> {
    let pczt = pczt::parse_pczt(pczt)?;
    check_parsed_pczt_multi_coins(params, &pczt, xpub, seed_fingerprint, account_index)
}

/// `check_pczt_multi_coins` against an already-parsed PCZT, so preflight can
/// parse once and reuse the parsed value for normalization.
#[cfg(feature = "multi_coins")]
fn check_parsed_pczt_multi_coins<P: consensus::Parameters>(
    params: &P,
    pczt: &Pczt,
    xpub: &str,
    seed_fingerprint: &[u8; 32],
    account_index: u32,
) -> Result<()> {
    reject_legacy_check_unsupported_pczt(pczt)?;
    let account_pubkey = transparent_account_pubkey_from_xpub(xpub)?;
    let account_index = zip32::AccountId::try_from(account_index)
        .map_err(|_e| ZcashError::InvalidDataError("invalid account index".to_string()))?;

    pczt::check::check_pczt_transparent(
        params,
        seed_fingerprint,
        account_index,
        &account_pubkey,
        pczt,
        true,
    )?;
    Ok(())
}

/// Parses, policy-checks, and re-serializes a PCZT in one pass.
///
/// Returns the normalized (current-version) encoding of the checked PCZT: the
/// bytes C retains as the `checked_PCZT`, which display and signing consume
/// without re-running these checks.
#[cfg(feature = "multi_coins")]
pub fn preflight_pczt_multi_coins<P: consensus::Parameters>(
    params: &P,
    pczt_bytes: &[u8],
    xpub: &str,
    seed_fingerprint: &[u8; 32],
    account_index: u32,
) -> Result<Vec<u8>> {
    let pczt = pczt::parse_pczt(pczt_bytes)?;
    // FUTURE(omitted-field-recompute): recompute-or-check omitted fields here,
    // mutating `pczt` so the normalized bytes carry the verified values forward.
    check_parsed_pczt_multi_coins(params, &pczt, xpub, seed_fingerprint, account_index)?;
    Ok(pczt.serialize())
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
    #[cfg(zcash_unstable = "nu6.3")]
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

        let normalized = preflight_pczt_multi_coins(
            &MainNetwork,
            &sample.bytes,
            &sample.xpub,
            &sample.seed_fingerprint,
            0,
        )
        .expect("selected account PCZT should preflight");
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

    #[cfg(zcash_unstable = "nu6.3")]
    #[test]
    fn legacy_check_rejects_v6_pczt() {
        let pczt = Creator::new_v6(
            BranchId::Nu6_3.into(),
            10,
            MainNetwork.coin_type(),
            [0; 32],
            [0; 32],
            [1; 32],
        )
        .build();

        let result = check_pczt_multi_coins(
            &MainNetwork,
            &pczt.serialize(),
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
    #[cfg(zcash_unstable = "nu6.3")]
    Ironwood,
}

#[cfg(feature = "cypherpunk")]
impl SignableShieldedPool {
    fn label(self) -> &'static str {
        match self {
            SignableShieldedPool::Orchard => "Orchard",
            #[cfg(zcash_unstable = "nu6.3")]
            SignableShieldedPool::Ironwood => "Ironwood",
        }
    }

    fn shielded_pool(self) -> pczt::ShieldedPool {
        match self {
            SignableShieldedPool::Orchard => pczt::ShieldedPool::Orchard,
            #[cfg(zcash_unstable = "nu6.3")]
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

    #[cfg(zcash_unstable = "nu6.3")]
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

    #[cfg(zcash_unstable = "nu6.3")]
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

    #[cfg(zcash_unstable = "nu6.3")]
    let should_process_ironwood = pczt::pczt_should_process_ironwood(&signed_pczt);
    let verifier = Verifier::new(signed_pczt)
        .with_orchard::<ZcashError, _>(|bundle| {
            ensure_actions_are_signed(bundle, SignableShieldedPool::Orchard, signable_actions)
        })
        .map_err(map_shielded_verifier_error)?;

    #[cfg(zcash_unstable = "nu6.3")]
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

/// Signs a preflight-checked, normalized PCZT and confirms in memory that every
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

/// Signs a preflight-checked, normalized PCZT and confirms in memory that every
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
    let signed = pczt::sign::sign_pczt_to_pczt(pczt, seed)?;
    let signed = if signable_actions.is_empty() {
        signed
    } else {
        ensure_shielded_actions_are_signed(signed, &signable_actions)?
    };
    Ok(signed.serialize())
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

    #[derive(Serialize, Deserialize)]
    struct PcztMirror {
        global: GlobalMirror,
        transparent: ::pczt::transparent::Bundle,
        sapling: SaplingBundleMirror,
        orchard: ::pczt::orchard::Bundle,
        #[cfg(zcash_unstable = "nu6.3")]
        ironwood: ::pczt::orchard::Bundle,
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

    #[cfg(zcash_unstable = "nu6.3")]
    fn v5_pczt_with_ironwood_actions() -> Vec<u8> {
        let sample = pczt::test_support::sample_ironwood_pczt();
        let mut bytes = sample.bytes;
        let mut pczt: PcztMirror = postcard::from_bytes(&bytes[8..]).unwrap();
        assert!(!pczt.ironwood.actions().is_empty());

        pczt.global.tx_version = constants::V5_TX_VERSION;
        pczt.global.version_group_id = constants::V5_VERSION_GROUP_ID;

        bytes.truncate(8);
        postcard::to_extend(&pczt, bytes).unwrap()
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

        let mut bytes = Creator::new(
            BranchId::Nu6.into(),
            10,
            MainNetwork.coin_type(),
            [0; 32],
            [0; 32],
        )
        .build()
        .serialize();
        let mut pczt: PcztMirror = postcard::from_bytes(&bytes[8..]).unwrap();
        assert!(pczt.sapling.spends.is_empty());
        assert!(pczt.sapling.outputs.is_empty());

        pczt.sapling.value_sum = 1;

        bytes.truncate(8);
        postcard::to_extend(&pczt, bytes).unwrap()
    }

    /// A PCZT whose Sapling bundle is empty but declares a non-zero value sum is malformed
    /// and must be rejected before signing.
    #[test]
    fn test_check_pczt_rejects_empty_sapling_bundle_with_nonzero_value_sum() {
        let seed = [9u8; 32];
        let malformed_pczt = malformed_pczt_with_empty_sapling_bundle_and_nonzero_value_sum();
        let ufvk = derive_ufvk(&MainNetwork, &seed, "m/32'/133'/0'").unwrap();
        let seed_fingerprint = calculate_seed_fingerprint(&seed).unwrap();

        let result = check_pczt_cypherpunk(&MainNetwork, &malformed_pczt, &ufvk, &seed_fingerprint, 0);

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
        let pczt_bytes = Creator::build_from_parts(pczt_parts).unwrap().serialize();
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

        match preflight_pczt_cypherpunk(&params, &pczt_bytes, &ufvk_text, &seed_fingerprint, 0) {
            Err(ZcashError::InvalidPczt(msg)) if msg.contains(expected) => {}
            other => panic!("preflight must reject internal-OVK change spoofing, got: {other:?}"),
        }
    }

    #[test]
    fn test_get_address() {
        let address = get_address(&MainNetwork, "uview1s2e0495jzhdarezq4h4xsunfk4jrq7gzg22tjjmkzpd28wgse4ejm6k7yfg8weanaghmwsvc69clwxz9f9z2hwaz4gegmna0plqrf05zkeue0nevnxzm557rwdkjzl4pl4hp4q9ywyszyjca8jl54730aymaprt8t0kxj8ays4fs682kf7prj9p24dnlcgqtnd2vnskkm7u8cwz8n0ce7yrwx967cyp6dhkc2wqprt84q0jmwzwnufyxe3j0758a9zgk9ssrrnywzkwfhu6ap6cgx3jkxs3un53n75s3");
        assert_eq!(address.unwrap(), "u1tqdskj32l9udfp0rysmca6gpz73fdqc2rmeenyhh0nfrq4vgak284ehkxefw5cf9495rdur0tparuntevp6nnetzjkyzv08m524e4swwk94asas7hm2ad5w5c64zz00hmr7nux0yhaz");
    }

    #[cfg(zcash_unstable = "nu6.3")]
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

    #[cfg(zcash_unstable = "nu6.3")]
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

    #[cfg(zcash_unstable = "nu6.3")]
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

    #[cfg(zcash_unstable = "nu6.3")]
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

    #[cfg(zcash_unstable = "nu6.3")]
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

    #[cfg(zcash_unstable = "nu6.3")]
    #[test]
    fn test_preflight_pczt_normalizes_and_is_idempotent() {
        let sample = pczt::test_support::sample_orchard_change_pczt();
        let normalized = preflight_pczt_cypherpunk(
            &pczt::test_support::Nu6_3Network,
            &sample.bytes,
            &sample.ufvk_text,
            &sample.seed_fingerprint,
            0,
        )
        .unwrap();

        // Normalized bytes are a valid PCZT that passes the same preflight and
        // re-normalizes to identical bytes.
        let renormalized = preflight_pczt_cypherpunk(
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
    fn test_preflight_pczt_rejects_invalid_data() {
        let seed = [7u8; 32];
        let ufvk = derive_ufvk(&MainNetwork, &seed, "m/32'/133'/0'").unwrap();
        let seed_fingerprint = calculate_seed_fingerprint(&seed).unwrap();

        let result = preflight_pczt_cypherpunk(
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

    #[cfg(zcash_unstable = "nu6.3")]
    fn pczt_with_sapling_output() -> pczt::test_support::SamplePczt {
        let mut sample = pczt::test_support::sample_orchard_change_pczt();
        let mut pczt: PcztMirror = postcard::from_bytes(&sample.bytes[8..]).unwrap();
        pczt.sapling.outputs.push(SaplingOutputMirror {
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
        pczt.sapling.value_sum = -1;

        sample.bytes.truncate(8);
        sample.bytes = postcard::to_extend(&pczt, sample.bytes).unwrap();
        sample
    }

    fn assert_batch_unsupported_sapling_error<T: core::fmt::Debug>(result: Result<T>) {
        assert_eq!(
            result.unwrap_err(),
            ZcashError::InvalidPczt(BATCH_UNSUPPORTED_SAPLING_ERROR.to_string())
        );
    }

    #[cfg(zcash_unstable = "nu6.3")]
    #[test]
    fn test_sign_checked_batch_pczt_signs_ironwood_spend() {
        let sample = pczt::test_support::sample_ironwood_pczt();
        let normalized = preflight_batch_pczt_cypherpunk(
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

    #[cfg(zcash_unstable = "nu6.3")]
    #[test]
    fn test_sign_checked_pczt_signs_owned_orchard_actions() {
        let sample = pczt::test_support::sample_orchard_change_pczt();
        let normalized = preflight_pczt_cypherpunk(
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

    #[cfg(zcash_unstable = "nu6.3")]
    #[test]
    fn test_sign_checked_pczt_rejects_foreign_seed() {
        let sample = pczt::test_support::sample_orchard_change_pczt();
        let normalized = preflight_pczt_cypherpunk(
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

    #[cfg(zcash_unstable = "nu6.3")]
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

    #[cfg(zcash_unstable = "nu6.3")]
    #[test]
    fn test_preflight_batch_pczt_accepts_orchard_and_ironwood_spends() {
        for sample in [
            pczt::test_support::sample_orchard_change_pczt(),
            pczt::test_support::sample_ironwood_pczt(),
        ] {
            let normalized = preflight_batch_pczt_cypherpunk(
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
                preflight_batch_pczt_cypherpunk(
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

    #[cfg(zcash_unstable = "nu6.3")]
    #[test]
    fn test_preflight_batch_pczt_rejects_sapling_outputs() {
        let sample = pczt_with_sapling_output();
        assert_batch_unsupported_sapling_error(preflight_batch_pczt_cypherpunk(
            &pczt::test_support::Nu6_3Network,
            &sample.bytes,
            &sample.ufvk_text,
            &sample.seed_fingerprint,
            0,
        ));
    }
}
