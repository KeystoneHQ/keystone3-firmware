pub mod structs;

use crate::common::{
    errors::RustCError,
    free::Free,
    structs::{SimpleResponse, TransactionCheckResult, TransactionParseResult},
    types::{Ptr, PtrBytes, PtrString, PtrT, PtrUR},
    ur::{UREncodeResult, FRAGMENT_MAX_LENGTH_DEFAULT, FRAGMENT_UNLIMITED_LENGTH},
    utils::{convert_c_char, recover_c_char},
};
use crate::{extract_array, extract_array_mut};
use crate::{extract_ptr_with_type, make_free_method};
use alloc::{boxed::Box, format, string::String, string::ToString, vec::Vec};
use app_zcash::get_address;
#[cfg(feature = "cypherpunk")]
use app_zcash::pczt::{sign::SpendAuthCache, structs::ParsedPczt};
#[cfg(feature = "cypherpunk")]
use app_zcash::version::KEYSTONE_FW_VERSION;
use core::slice;
use cryptoxide::hashing::sha256;
use cty::c_char;
use keystore::algorithms::{
    ed25519::slip10_ed25519::get_private_key_by_seed,
    zcash::{calculate_seed_fingerprint, derive_ufvk},
};
#[cfg(feature = "cypherpunk")]
use structs::BatchDisplayCache;
use structs::DisplayPczt;
use structs::DisplayZcashBatch;
use structs::ZcashCheckedPczt;
use ur_registry::traits::RegistryItem;
use ur_registry::zcash::zcash_batch_sig_result::ZcashBatchSigResult;
use ur_registry::zcash::zcash_pczt::ZcashPczt;
use ur_registry::zcash::zcash_sign_batch::ZcashSignBatch;
#[cfg(feature = "cypherpunk")]
use zcash_vendor::pczt::roles::signer::batch::{BatchSignRequest, BatchSignResponse};
use zcash_vendor::{pczt::Pczt, zcash_protocol::consensus::MainNetwork};
use zeroize::Zeroize;

/// Aggregate budget for canonical batch bytes, standalone PCZT headers, and
/// worst-case resolved-field growth.
#[cfg(feature = "cypherpunk")]
const ZCASH_BATCH_MAX_RESOLVED_BYTES: usize = 512 * 1024;
/// Maximum Orchard actions whose signatures may be retained for the response QR.
#[cfg(feature = "cypherpunk")]
const ZCASH_BATCH_MAX_RESPONSE_ACTIONS: usize = 96;
/// Four-byte magic and four-byte version header added to each standalone PCZT.
#[cfg(feature = "cypherpunk")]
const ZCASH_BATCH_PER_PCZT_ENCODING_OVERHEAD: usize = 4 + core::mem::size_of::<u32>();

#[no_mangle]
pub unsafe extern "C" fn derive_zcash_ufvk(
    seed: PtrBytes,
    seed_len: u32,
    account_path: PtrString,
) -> *mut SimpleResponse<c_char> {
    let seed = extract_array!(seed, u8, seed_len as usize);
    let account_path = unsafe { recover_c_char(account_path) };
    let ufvk_text = derive_ufvk(&MainNetwork, seed, &account_path);
    let result = match ufvk_text {
        Ok(text) => SimpleResponse::success(convert_c_char(text)).simple_c_ptr(),
        Err(e) => SimpleResponse::from(e).simple_c_ptr(),
    };
    result
}

#[no_mangle]
pub unsafe extern "C" fn calculate_zcash_seed_fingerprint(
    seed: PtrBytes,
    seed_len: u32,
) -> *mut SimpleResponse<u8> {
    let mut seed = extract_array_mut!(seed, u8, seed_len as usize);
    let sfp = calculate_seed_fingerprint(seed);
    let result = match sfp {
        Ok(bytes) => {
            SimpleResponse::success(Box::into_raw(Box::new(bytes)) as *mut u8).simple_c_ptr()
        }
        Err(e) => SimpleResponse::from(e).simple_c_ptr(),
    };
    seed.zeroize();
    result
}

#[no_mangle]
pub unsafe extern "C" fn generate_zcash_default_address(
    ufvk_text: PtrString,
) -> *mut SimpleResponse<c_char> {
    let ufvk_text = unsafe { recover_c_char(ufvk_text) };
    let address = get_address(&MainNetwork, &ufvk_text);
    match address {
        Ok(text) => SimpleResponse::success(convert_c_char(text)).simple_c_ptr(),
        Err(e) => SimpleResponse::from(e).simple_c_ptr(),
    }
}

#[no_mangle]
#[cfg(feature = "cypherpunk")]
pub unsafe extern "C" fn check_zcash_tx_cypherpunk(
    tx: PtrUR,
    ufvk: PtrString,
    seed_fingerprint: PtrBytes,
    account_index: u32,
    disabled: bool,
    checked_pczt: Ptr<Ptr<ZcashCheckedPczt>>,
) -> *mut TransactionCheckResult {
    *checked_pczt = core::ptr::null_mut();
    if disabled {
        return TransactionCheckResult::from(RustCError::UnsupportedTransaction(
            "Zcash requires at least 256-bit entropy (use 33-word Shamir shares)".to_string(),
        ))
        .c_ptr();
    }
    let pczt = extract_ptr_with_type!(tx, ZcashPczt);
    let ufvk_text = unsafe { recover_c_char(ufvk) };
    let seed_fingerprint = extract_array!(seed_fingerprint, u8, 32);
    let seed_fingerprint = seed_fingerprint.try_into().unwrap();
    match app_zcash::check_pczt_cypherpunk(
        &MainNetwork,
        &pczt.get_data(),
        &ufvk_text,
        seed_fingerprint,
        account_index,
    ) {
        Ok(normalized) => {
            *checked_pczt = ZcashCheckedPczt::new(normalized).c_ptr();
            TransactionCheckResult::new().c_ptr()
        }
        Err(e) => TransactionCheckResult::from(e).c_ptr(),
    }
}

#[cfg(feature = "multi-coins")]
#[no_mangle]
pub unsafe extern "C" fn check_zcash_tx_multi_coins(
    tx: PtrUR,
    xpub: PtrString,
    seed_fingerprint: PtrBytes,
    account_index: u32,
    disabled: bool,
    checked_pczt: Ptr<Ptr<ZcashCheckedPczt>>,
) -> *mut TransactionCheckResult {
    *checked_pczt = core::ptr::null_mut();
    if disabled {
        return TransactionCheckResult::from(RustCError::UnsupportedTransaction(
            "Zcash requires at least 256-bit entropy (use 33-word Shamir shares)".to_string(),
        ))
        .c_ptr();
    }
    let pczt = extract_ptr_with_type!(tx, ZcashPczt);
    let xpub_text = unsafe { recover_c_char(xpub) };
    let seed_fingerprint = extract_array!(seed_fingerprint, u8, 32);
    let seed_fingerprint = seed_fingerprint.try_into().unwrap();
    match app_zcash::check_pczt_multi_coins(
        &MainNetwork,
        &pczt.get_data(),
        &xpub_text,
        seed_fingerprint,
        account_index,
    ) {
        Ok(normalized) => {
            *checked_pczt = ZcashCheckedPczt::new(normalized).c_ptr();
            TransactionCheckResult::new().c_ptr()
        }
        Err(e) => TransactionCheckResult::from(e).c_ptr(),
    }
}

#[cfg(feature = "cypherpunk")]
#[no_mangle]
pub unsafe extern "C" fn parse_zcash_tx_cypherpunk(
    checked_pczt: Ptr<ZcashCheckedPczt>,
    ufvk: PtrString,
    seed_fingerprint: PtrBytes,
) -> Ptr<TransactionParseResult<DisplayPczt>> {
    if checked_pczt.is_null() {
        return TransactionParseResult::from(RustCError::InvalidData(
            "no checked PCZT available".to_string(),
        ))
        .c_ptr();
    }
    let checked = extract_ptr_with_type!(checked_pczt, ZcashCheckedPczt);
    let bytes = match checked.checked_bytes() {
        Ok(bytes) => bytes,
        Err(e) => return TransactionParseResult::from(e).c_ptr(),
    };
    let ufvk_text = unsafe { recover_c_char(ufvk) };
    let seed_fingerprint = extract_array!(seed_fingerprint, u8, 32);
    let seed_fingerprint = seed_fingerprint.try_into().unwrap();
    match app_zcash::parse_pczt_cypherpunk(&MainNetwork, bytes, &ufvk_text, seed_fingerprint) {
        Ok(pczt) => TransactionParseResult::success(DisplayPczt::from(&pczt).c_ptr()).c_ptr(),
        Err(e) => TransactionParseResult::from(e).c_ptr(),
    }
}

#[cfg(feature = "multi-coins")]
#[no_mangle]
pub unsafe extern "C" fn parse_zcash_tx_multi_coins(
    checked_pczt: Ptr<ZcashCheckedPczt>,
    seed_fingerprint: PtrBytes,
) -> Ptr<TransactionParseResult<DisplayPczt>> {
    if checked_pczt.is_null() {
        return TransactionParseResult::from(RustCError::InvalidData(
            "no checked PCZT available".to_string(),
        ))
        .c_ptr();
    }
    let checked = extract_ptr_with_type!(checked_pczt, ZcashCheckedPczt);
    let bytes = match checked.checked_bytes() {
        Ok(bytes) => bytes,
        Err(e) => return TransactionParseResult::from(e).c_ptr(),
    };
    let seed_fingerprint = extract_array!(seed_fingerprint, u8, 32);
    let seed_fingerprint = seed_fingerprint.try_into().unwrap();
    match app_zcash::parse_pczt_multi_coins(&MainNetwork, bytes, seed_fingerprint) {
        Ok(pczt) => TransactionParseResult::success(DisplayPczt::from(&pczt).c_ptr()).c_ptr(),
        Err(e) => TransactionParseResult::from(e).c_ptr(),
    }
}

/// Enforces non-empty and unique canonical payloads.
#[cfg(feature = "cypherpunk")]
fn validate_zcash_batch_payloads(payloads: &[Vec<u8>]) -> Result<(), RustCError> {
    if payloads.is_empty() {
        return Err(RustCError::InvalidData(
            "Zcash batch has no PCZTs".to_string(),
        ));
    }
    let mut payload_digests = Vec::with_capacity(payloads.len());
    for (index, payload) in payloads.iter().enumerate() {
        if payload.is_empty() {
            return Err(RustCError::InvalidData(format!(
                "Zcash batch PCZT {index} has no payload"
            )));
        }
        let digest = sha256(payload);
        if payload_digests.contains(&digest) {
            return Err(RustCError::InvalidData(
                "Zcash batch contains duplicate PCZTs".to_string(),
            ));
        }
        payload_digests.push(digest);
    }

    Ok(())
}

/// Serializes one logical batch PCZT into its canonical standalone encoding.
#[cfg(feature = "cypherpunk")]
fn serialize_batch_pczt(pczt: &Pczt) -> Result<Vec<u8>, RustCError> {
    pczt.clone()
        .serialize()
        .map_err(|e| RustCError::InvalidData(format!("encode PCZT in batch request: {e:?}")))
}

/// Applies firmware batch limits to the request body owned by the PCZT crate.
#[cfg(feature = "cypherpunk")]
fn validate_zcash_batch(batch: &BatchSignRequest) -> Result<Vec<Vec<u8>>, RustCError> {
    let payloads = batch
        .pczts()
        .iter()
        .map(serialize_batch_pczt)
        .collect::<Result<Vec<_>, _>>()?;
    validate_zcash_batch_payloads(&payloads)?;
    Ok(payloads)
}

/// Applies the aggregate byte budget before parsing or retaining a batch.
#[cfg(feature = "cypherpunk")]
fn validate_zcash_batch_envelope(request_id: &[u8], data: &[u8]) -> Result<(), RustCError> {
    if request_id.is_empty() {
        return Err(RustCError::InvalidData(
            "Zcash batch request id must not be empty".to_string(),
        ));
    }
    let encoded_bytes = request_id.len().checked_add(data.len()).ok_or_else(|| {
        RustCError::UnsupportedTransaction("Zcash batch size overflow".to_string())
    })?;
    if encoded_bytes > ZCASH_BATCH_MAX_RESOLVED_BYTES {
        return Err(RustCError::UnsupportedTransaction(format!(
            "Zcash batch exceeds its {ZCASH_BATCH_MAX_RESOLVED_BYTES}-byte resource budget"
        )));
    }
    Ok(())
}

/// Counts Orchard actions across both Orchard-protocol value pools.
#[cfg(feature = "cypherpunk")]
fn zcash_batch_action_count(batch: &BatchSignRequest) -> Result<usize, RustCError> {
    batch
        .pczts()
        .iter()
        .map(|pczt| {
            pczt.orchard()
                .actions()
                .len()
                .checked_add(pczt.ironwood().actions().len())
        })
        .reduce(|total, actions| {
            total.and_then(|total| actions.and_then(|actions| total.checked_add(actions)))
        })
        .unwrap_or(Some(0))
        .ok_or_else(|| RustCError::UnsupportedTransaction("Zcash batch size overflow".to_string()))
}

/// Bounds signatures retained for the signed-response QR.
#[cfg(feature = "cypherpunk")]
fn validate_zcash_batch_response_actions(batch: &BatchSignRequest) -> Result<(), RustCError> {
    validate_zcash_batch_response_action_count(zcash_batch_action_count(batch)?)
}

#[cfg(feature = "cypherpunk")]
fn validate_zcash_batch_response_action_count(total_actions: usize) -> Result<(), RustCError> {
    if total_actions > ZCASH_BATCH_MAX_RESPONSE_ACTIONS {
        return Err(RustCError::UnsupportedTransaction(format!(
            "Zcash batch exceeds {ZCASH_BATCH_MAX_RESPONSE_ACTIONS} Orchard actions"
        )));
    }
    Ok(())
}

/// Estimates the normalized bytes retained after compact fields are restored.
#[cfg(feature = "cypherpunk")]
fn estimate_zcash_batch_resolved_bytes(
    request_id: &[u8],
    batch: &BatchSignRequest,
) -> Result<usize, RustCError> {
    let canonical_bytes = batch
        .serialize()
        .map_err(|e| RustCError::InvalidData(format!("encode PCZT batch request: {e:?}")))?
        .len();
    let pczt_encoding_overhead = batch
        .pczts()
        .len()
        .checked_mul(ZCASH_BATCH_PER_PCZT_ENCODING_OVERHEAD)
        .ok_or_else(|| {
            RustCError::UnsupportedTransaction("Zcash batch size overflow".to_string())
        })?;
    let resolved_action_growth = zcash_batch_action_count(batch)?
        .checked_mul(app_zcash::COMPACT_PCZT_MAX_RESOLVED_ACTION_GROWTH)
        .ok_or_else(|| {
            RustCError::UnsupportedTransaction("Zcash batch size overflow".to_string())
        })?;

    request_id
        .len()
        .checked_add(canonical_bytes)
        .and_then(|bytes| bytes.checked_add(pczt_encoding_overhead))
        .and_then(|bytes| bytes.checked_add(resolved_action_growth))
        .ok_or_else(|| RustCError::UnsupportedTransaction("Zcash batch size overflow".to_string()))
}

/// Enforces one aggregate budget before any compact fields are restored.
#[cfg(feature = "cypherpunk")]
fn validate_zcash_batch_resource_budget(
    request_id: &[u8],
    batch: &BatchSignRequest,
) -> Result<(), RustCError> {
    if estimate_zcash_batch_resolved_bytes(request_id, batch)? > ZCASH_BATCH_MAX_RESOLVED_BYTES {
        return Err(RustCError::UnsupportedTransaction(format!(
            "Zcash batch may exceed its {ZCASH_BATCH_MAX_RESOLVED_BYTES}-byte resource budget after field resolution"
        )));
    }
    Ok(())
}

/// Parses a batch whose encoded envelope fits the aggregate resource budget.
#[cfg(feature = "cypherpunk")]
fn parse_zcash_batch_registry(registry: &ZcashSignBatch) -> Result<BatchSignRequest, RustCError> {
    validate_zcash_batch_envelope(registry.get_request_id(), registry.get_data())?;
    let batch = BatchSignRequest::parse(registry.get_data())
        .map_err(|e| RustCError::InvalidData(format!("invalid PCZT batch request: {e:?}")))?;
    validate_zcash_batch_response_actions(&batch)?;
    Ok(batch)
}

/// Reopens the exact normalized envelope retained by the check step.
#[cfg(feature = "cypherpunk")]
fn parse_checked_zcash_batch(data: &[u8]) -> Result<(Vec<u8>, BatchSignRequest), RustCError> {
    let registry = ZcashSignBatch::try_from(data.to_vec()).map_err(|e| {
        RustCError::InvalidData(format!("decode checked Zcash batch envelope: {e:?}"))
    })?;
    let batch = parse_zcash_batch_registry(&registry)?;
    Ok((registry.get_request_id().to_vec(), batch))
}

/// Retains normalized PCZTs and their request id as checked firmware state.
#[cfg(feature = "cypherpunk")]
fn encode_checked_zcash_batch(request_id: &[u8], data: Vec<u8>) -> Result<Vec<u8>, RustCError> {
    validate_zcash_batch_envelope(request_id, &data)?;
    ZcashSignBatch::new(request_id.to_vec(), data)
        .try_into()
        .map_err(|e| {
            RustCError::InvalidData(format!("encode normalized Zcash batch envelope: {e:?}"))
        })
}

/// Wraps the PCZT crate's signature response with its echoed request id and
/// the firmware version that produced the signatures.
#[cfg(feature = "cypherpunk")]
fn encode_zcash_batch_sig_result(
    request_id: Vec<u8>,
    data: Vec<u8>,
) -> Result<Vec<u8>, RustCError> {
    ZcashBatchSigResult::new(request_id, data, KEYSTONE_FW_VERSION.encode())
        .try_into()
        .map_err(|e| RustCError::InvalidData(format!("encode Zcash batch result envelope: {e:?}")))
}

#[cfg(feature = "cypherpunk")]
#[no_mangle]
pub unsafe extern "C" fn check_zcash_batch_tx_cypherpunk(
    tx: PtrUR,
    ufvk: PtrString,
    seed_fingerprint: PtrBytes,
    account_index: u32,
    disabled: bool,
    checked_batch: Ptr<Ptr<ZcashCheckedPczt>>,
) -> *mut TransactionCheckResult {
    *checked_batch = core::ptr::null_mut();
    if disabled {
        return TransactionCheckResult::from(RustCError::UnsupportedTransaction(
            "Zcash requires at least 256-bit entropy (use 33-word Shamir shares)".to_string(),
        ))
        .c_ptr();
    }
    let registry = extract_ptr_with_type!(tx, ZcashSignBatch);
    let batch = match parse_zcash_batch_registry(registry) {
        Ok(batch) => batch,
        Err(e) => return TransactionCheckResult::from(e).c_ptr(),
    };
    let request_id = registry.get_request_id().to_vec();
    if let Err(e) = validate_zcash_batch_resource_budget(&request_id, &batch) {
        return TransactionCheckResult::from(e).c_ptr();
    }
    let ufvk_text = unsafe { recover_c_char(ufvk) };
    let seed_fingerprint = extract_array!(seed_fingerprint, u8, 32);
    let seed_fingerprint = seed_fingerprint.try_into().unwrap();

    let payloads = match validate_zcash_batch(&batch) {
        Ok(payloads) => payloads,
        Err(e) => return TransactionCheckResult::from(e).c_ptr(),
    };

    // Each check returns normalized bytes, display rows, an optional compact
    // migration classification, and the bound signability decision from the
    // same shielded action pass.
    let mut checked_pczts = Vec::with_capacity(payloads.len());
    let mut rows: Vec<ParsedPczt> = Vec::with_capacity(payloads.len());
    let mut migration_summaries = Vec::with_capacity(payloads.len());
    let mut signability = Vec::with_capacity(payloads.len());
    // One check context for the whole batch: the UFVK decode and the wallet
    // Orchard key derivation depend only on the device UFVK, so they run once
    // here instead of once per PCZT (see BatchCheckContext).
    let check_ctx = app_zcash::BatchCheckContext::new(&ufvk_text);
    for payload in payloads {
        match app_zcash::check_batch_pczt_with_display(
            &MainNetwork,
            &payload,
            &check_ctx,
            seed_fingerprint,
            account_index,
        ) {
            Ok((normalized, parsed, migration_summary, checked_signability)) => {
                let pczt = match Pczt::parse(&normalized) {
                    Ok(pczt) => pczt,
                    Err(e) => {
                        return TransactionCheckResult::from(RustCError::InvalidData(format!(
                            "parse normalized PCZT in batch request: {e:?}"
                        )))
                        .c_ptr();
                    }
                };
                checked_pczts.push(pczt);
                rows.push(parsed);
                migration_summaries.push(migration_summary);
                signability.push(checked_signability);
            }
            Err(e) => return TransactionCheckResult::from(e).c_ptr(),
        }
    }

    // Compact eligible Orchard-to-Ironwood transfers by content, independent of
    // their PCZT positions. Ambiguous batches retain every ordinary row.
    let display_rows = app_zcash::compact_checked_batch_migration_review(
        rows.into_iter().zip(migration_summaries),
    );
    let display =
        BatchDisplayCache::new(display_rows, signability, *seed_fingerprint, account_index);

    // Rebuild the Postcard request around the normalized PCZTs so parse/sign
    // consume exactly what was checked, then preserve the outer request id for
    // the eventual batch result.
    let normalized_request = match BatchSignRequest::new(checked_pczts).serialize() {
        Ok(bytes) => bytes,
        Err(e) => {
            return TransactionCheckResult::from(RustCError::InvalidData(format!(
                "encode normalized PCZT batch request: {e:?}"
            )))
            .c_ptr();
        }
    };
    let normalized_batch = match encode_checked_zcash_batch(&request_id, normalized_request) {
        Ok(bytes) => bytes,
        Err(e) => return TransactionCheckResult::from(e).c_ptr(),
    };
    *checked_batch = ZcashCheckedPczt::new_with_display(normalized_batch, display).c_ptr();
    TransactionCheckResult::new().c_ptr()
}

#[cfg(feature = "cypherpunk")]
#[no_mangle]
pub unsafe extern "C" fn parse_zcash_batch_tx_cypherpunk(
    checked_batch: Ptr<ZcashCheckedPczt>,
    ufvk: PtrString,
    seed_fingerprint: PtrBytes,
    disabled: bool,
) -> Ptr<TransactionParseResult<DisplayZcashBatch>> {
    if disabled {
        return TransactionParseResult::from(RustCError::UnsupportedTransaction(
            "Zcash requires at least 256-bit entropy (use 33-word Shamir shares)".to_string(),
        ))
        .c_ptr();
    }
    if checked_batch.is_null() {
        return TransactionParseResult::from(RustCError::InvalidData(
            "no checked Zcash batch available".to_string(),
        ))
        .c_ptr();
    }
    // `ufvk` and `seed_fingerprint` are retained for ABI/signature stability but
    // unused now that parse converts the display rows the check pass cached
    // instead of re-decrypting every output.
    let _ = (ufvk, seed_fingerprint);
    let checked = extract_ptr_with_type!(checked_batch, ZcashCheckedPczt);
    if let Err(e) = checked.checked_bytes() {
        return TransactionParseResult::from(e).c_ptr();
    }
    if checked.display.is_null() {
        // Can't happen for a batch checked container (the batch check always
        // stores a cache), but fail closed rather than silently re-deriving.
        return TransactionParseResult::from(RustCError::InvalidData(
            "no checked Zcash batch display available".to_string(),
        ))
        .c_ptr();
    }
    // Convert the cached rows into fresh owned FFI structs. There is no fallible
    // step after the first `DisplayPczt` is built, so the "materialize only after
    // all fallible steps" leak-safety is trivially preserved.
    let display_items = batch_display_items(&*checked.display);

    TransactionParseResult::success(DisplayZcashBatch::from(display_items).c_ptr()).c_ptr()
}

/// Converts the cached review rows into fresh FFI display structs. The cache
/// already contains either the compacted review or all ordinary PCZT rows.
#[cfg(feature = "cypherpunk")]
fn batch_display_items(cache: &BatchDisplayCache) -> Vec<DisplayPczt> {
    cache.rows().iter().map(DisplayPczt::from).collect()
}

#[cfg(feature = "cypherpunk")]
unsafe fn sign_zcash_batch_tx_cypherpunk_dynamic(
    checked_batch: Ptr<ZcashCheckedPczt>,
    seed_fingerprint: PtrBytes,
    account_index: u32,
    disabled: bool,
    seed: PtrBytes,
    seed_len: u32,
    max_fragment_length: usize,
    allow_multipart: bool,
) -> *mut UREncodeResult {
    if disabled {
        return UREncodeResult::from(RustCError::UnsupportedTransaction(
            "Zcash requires at least 256-bit entropy (use 33-word Shamir shares)".to_string(),
        ))
        .c_ptr();
    }
    if checked_batch.is_null() {
        return UREncodeResult::from(RustCError::InvalidData(
            "no checked Zcash batch available for signing".to_string(),
        ))
        .c_ptr();
    }
    let checked = extract_ptr_with_type!(checked_batch, ZcashCheckedPczt);
    if checked.display.is_null() {
        return UREncodeResult::from(RustCError::InvalidData(
            "no checked Zcash batch signability available".to_string(),
        ))
        .c_ptr();
    }
    let expected_seed_fingerprint = extract_array!(seed_fingerprint, u8, 32);
    let expected_seed_fingerprint: &[u8; 32] = expected_seed_fingerprint.try_into().unwrap();
    let checked_signability =
        match (&*checked.display).signability(expected_seed_fingerprint, account_index) {
            Some(signability) => signability,
            None => {
                return UREncodeResult::from(RustCError::InvalidData(
                    "checked Zcash batch signing context mismatch".to_string(),
                ))
                .c_ptr();
            }
        };
    let mut seed = extract_array_mut!(seed, u8, seed_len as usize);

    let result = match checked.checked_bytes() {
        Ok(bytes) => match parse_checked_zcash_batch(bytes) {
            Ok((request_id, batch)) => match calculate_seed_fingerprint(seed) {
                Ok(seed_fingerprint) => {
                    if &seed_fingerprint != expected_seed_fingerprint {
                        seed.zeroize();
                        return UREncodeResult::from(RustCError::MasterFingerprintMismatch).c_ptr();
                    }
                    if checked_signability.len() != batch.pczts().len() {
                        seed.zeroize();
                        return UREncodeResult::from(RustCError::InvalidData(
                            "checked Zcash batch signability count mismatch".to_string(),
                        ))
                        .c_ptr();
                    }

                    let mut results = Vec::new();
                    let mut error = None;
                    // One scrubbed spend-auth slot for the whole request. The
                    // selected account key stays cached across every batch PCZT.
                    let ask_cache = SpendAuthCache::new();
                    // Preserve request order and emit nothing unless every PCZT signs.
                    for (pczt, checked_signability) in batch.pczts().iter().zip(checked_signability)
                    {
                        let payload = match serialize_batch_pczt(pczt) {
                            Ok(payload) => payload,
                            Err(e) => {
                                error = Some(UREncodeResult::from(e).c_ptr());
                                break;
                            }
                        };
                        match app_zcash::sign_checked_batch_pczt_with_cached_signability(
                            &payload,
                            checked_signability,
                            seed,
                            &ask_cache,
                        ) {
                            Ok(payload) => {
                                match app_zcash::extract_compact_sigs_from_signed_pczt(&payload) {
                                    Ok(compact_sigs) => {
                                        results.push(compact_sigs);
                                    }
                                    Err(e) => {
                                        error = Some(UREncodeResult::from(e).c_ptr());
                                        break;
                                    }
                                }
                            }
                            Err(e) => {
                                error = Some(UREncodeResult::from(e).c_ptr());
                                break;
                            }
                        }
                    }
                    // End the secret's lifetime before response serialization
                    // and UR encoding, while retaining it across every PCZT.
                    drop(ask_cache);

                    if let Some(error) = error {
                        error
                    } else {
                        let response = BatchSignResponse::new(results);
                        match response.serialize() {
                            Ok(bytes) => {
                                let registry_type =
                                    ZcashBatchSigResult::get_registry_type().get_type();
                                match encode_zcash_batch_sig_result(request_id, bytes) {
                                    Ok(cbor) => {
                                        if allow_multipart {
                                            UREncodeResult::encode(
                                                cbor,
                                                registry_type,
                                                max_fragment_length,
                                            )
                                            .c_ptr()
                                        } else {
                                            UREncodeResult::encode_full_response(
                                                cbor,
                                                registry_type,
                                            )
                                            .c_ptr()
                                        }
                                    }
                                    Err(e) => UREncodeResult::from(e).c_ptr(),
                                }
                            }
                            Err(e) => UREncodeResult::from(RustCError::InvalidData(format!(
                                "encode PCZT batch response: {e:?}"
                            )))
                            .c_ptr(),
                        }
                    }
                }
                Err(e) => UREncodeResult::from(e).c_ptr(),
            },
            Err(e) => UREncodeResult::from(e).c_ptr(),
        },
        Err(e) => UREncodeResult::from(e).c_ptr(),
    };
    seed.zeroize();
    result
}

#[cfg(feature = "cypherpunk")]
#[no_mangle]
pub unsafe extern "C" fn sign_zcash_batch_tx_cypherpunk(
    checked_batch: Ptr<ZcashCheckedPczt>,
    seed_fingerprint: PtrBytes,
    account_index: u32,
    disabled: bool,
    seed: PtrBytes,
    seed_len: u32,
) -> *mut UREncodeResult {
    sign_zcash_batch_tx_cypherpunk_dynamic(
        checked_batch,
        seed_fingerprint,
        account_index,
        disabled,
        seed,
        seed_len,
        FRAGMENT_MAX_LENGTH_DEFAULT,
        true,
    )
}

#[cfg(feature = "cypherpunk")]
#[no_mangle]
pub unsafe extern "C" fn sign_zcash_batch_tx_cypherpunk_unlimited(
    checked_batch: Ptr<ZcashCheckedPczt>,
    seed_fingerprint: PtrBytes,
    account_index: u32,
    disabled: bool,
    seed: PtrBytes,
    seed_len: u32,
) -> *mut UREncodeResult {
    sign_zcash_batch_tx_cypherpunk_dynamic(
        checked_batch,
        seed_fingerprint,
        account_index,
        disabled,
        seed,
        seed_len,
        FRAGMENT_UNLIMITED_LENGTH,
        false,
    )
}

#[no_mangle]
pub unsafe extern "C" fn sign_zcash_tx(
    checked_pczt: Ptr<ZcashCheckedPczt>,
    seed: PtrBytes,
    seed_len: u32,
) -> *mut UREncodeResult {
    sign_zcash_tx_dynamic(checked_pczt, seed, seed_len, FRAGMENT_MAX_LENGTH_DEFAULT)
}

#[no_mangle]
pub unsafe extern "C" fn sign_zcash_tx_unlimited(
    checked_pczt: Ptr<ZcashCheckedPczt>,
    seed: PtrBytes,
    seed_len: u32,
) -> *mut UREncodeResult {
    sign_zcash_tx_dynamic(checked_pczt, seed, seed_len, FRAGMENT_UNLIMITED_LENGTH)
}

unsafe fn sign_zcash_tx_dynamic(
    checked_pczt: Ptr<ZcashCheckedPczt>,
    seed: PtrBytes,
    seed_len: u32,
    max_fragment_length: usize,
) -> *mut UREncodeResult {
    if checked_pczt.is_null() {
        return UREncodeResult::from(RustCError::InvalidData(
            "no checked PCZT available for signing".to_string(),
        ))
        .c_ptr();
    }
    let checked = extract_ptr_with_type!(checked_pczt, ZcashCheckedPczt);
    let mut seed = extract_array_mut!(seed, u8, seed_len as usize);
    let result = match checked.checked_bytes() {
        Ok(bytes) => match app_zcash::sign_pczt(bytes, seed) {
            Ok(pczt) => match ZcashPczt::new(pczt).try_into() {
                Err(e) => UREncodeResult::from(e).c_ptr(),
                Ok(v) => UREncodeResult::encode(
                    v,
                    ZcashPczt::get_registry_type().get_type(),
                    max_fragment_length,
                )
                .c_ptr(),
            },
            Err(e) => UREncodeResult::from(e).c_ptr(),
        },
        Err(e) => UREncodeResult::from(e).c_ptr(),
    };
    seed.zeroize();
    result
}

#[cfg(feature = "cypherpunk")]
unsafe fn sign_zcash_tx_cypherpunk_dynamic(
    checked_pczt: Ptr<ZcashCheckedPczt>,
    seed_fingerprint: PtrBytes,
    account_index: u32,
    disabled: bool,
    seed: PtrBytes,
    seed_len: u32,
    max_fragment_length: usize,
) -> *mut UREncodeResult {
    if disabled {
        return UREncodeResult::from(RustCError::UnsupportedTransaction(
            "Zcash requires at least 256-bit entropy (use 33-word Shamir shares)".to_string(),
        ))
        .c_ptr();
    }
    if checked_pczt.is_null() {
        return UREncodeResult::from(RustCError::InvalidData(
            "no checked PCZT available for signing".to_string(),
        ))
        .c_ptr();
    }
    let checked = extract_ptr_with_type!(checked_pczt, ZcashCheckedPczt);
    let expected_seed_fingerprint = extract_array!(seed_fingerprint, u8, 32);
    let expected_seed_fingerprint: &[u8; 32] = expected_seed_fingerprint.try_into().unwrap();
    let mut seed = extract_array_mut!(seed, u8, seed_len as usize);

    let result = match checked.checked_bytes() {
        Ok(pczt_bytes) => match calculate_seed_fingerprint(seed) {
            Ok(seed_fingerprint) => {
                if &seed_fingerprint != expected_seed_fingerprint {
                    seed.zeroize();
                    return UREncodeResult::from(RustCError::MasterFingerprintMismatch).c_ptr();
                }
                match app_zcash::sign_checked_pczt(
                    &MainNetwork,
                    pczt_bytes,
                    seed,
                    &seed_fingerprint,
                    account_index,
                ) {
                    Ok(signed_pczt) => match ZcashPczt::new(signed_pczt).try_into() {
                        Err(e) => UREncodeResult::from(e).c_ptr(),
                        Ok(v) => UREncodeResult::encode(
                            v,
                            ZcashPczt::get_registry_type().get_type(),
                            max_fragment_length,
                        )
                        .c_ptr(),
                    },
                    Err(e) => UREncodeResult::from(e).c_ptr(),
                }
            }
            Err(e) => UREncodeResult::from(e).c_ptr(),
        },
        Err(e) => UREncodeResult::from(e).c_ptr(),
    };
    seed.zeroize();
    result
}

#[cfg(feature = "cypherpunk")]
#[no_mangle]
pub unsafe extern "C" fn sign_zcash_tx_cypherpunk(
    checked_pczt: Ptr<ZcashCheckedPczt>,
    seed_fingerprint: PtrBytes,
    account_index: u32,
    disabled: bool,
    seed: PtrBytes,
    seed_len: u32,
) -> *mut UREncodeResult {
    sign_zcash_tx_cypherpunk_dynamic(
        checked_pczt,
        seed_fingerprint,
        account_index,
        disabled,
        seed,
        seed_len,
        FRAGMENT_MAX_LENGTH_DEFAULT,
    )
}

#[cfg(feature = "cypherpunk")]
#[no_mangle]
pub unsafe extern "C" fn sign_zcash_tx_cypherpunk_unlimited(
    checked_pczt: Ptr<ZcashCheckedPczt>,
    seed_fingerprint: PtrBytes,
    account_index: u32,
    disabled: bool,
    seed: PtrBytes,
    seed_len: u32,
) -> *mut UREncodeResult {
    sign_zcash_tx_cypherpunk_dynamic(
        checked_pczt,
        seed_fingerprint,
        account_index,
        disabled,
        seed,
        seed_len,
        FRAGMENT_UNLIMITED_LENGTH,
    )
}

make_free_method!(TransactionParseResult<DisplayPczt>);
make_free_method!(TransactionParseResult<DisplayZcashBatch>);

/// Frees a `ZcashCheckedPczt` previously returned through a check FFI out-param.
#[no_mangle]
pub unsafe extern "C" fn free_zcash_checked_pczt(ptr: PtrT<ZcashCheckedPczt>) {
    if ptr.is_null() {
        return;
    }
    let checked = alloc::boxed::Box::from_raw(ptr);
    checked.free();
}

use aes::cipher::block_padding::Pkcs7;
use aes::cipher::generic_array::GenericArray;
use aes::cipher::{BlockDecryptMut, BlockEncryptMut, KeyIvInit};

type Aes256CbcEnc = cbc::Encryptor<aes::Aes256>;
type Aes256CbcDec = cbc::Decryptor<aes::Aes256>;

/// Derives an AES-256 key from the wallet seed at a dedicated BIP32 path.
/// The UFVK itself is seed-derived, so keying the ciphertext with the seed loses no security
/// property and removes the weak `sha256(login password)` key derivation (see security review).
#[no_mangle]
pub unsafe extern "C" fn rust_derive_key_from_seed(
    seed: PtrBytes,
    seed_len: u32,
) -> *mut SimpleResponse<u8> {
    let seed = extract_array!(seed, u8, seed_len as usize);
    // Dedicated path, distinct from the legacy fixed-IV path m/44'/1557192335'/0'/2'/0'.
    let key_path = "m/44'/1557192335'/0'/3'/0'".to_string();
    let key = match get_private_key_by_seed(seed, &key_path) {
        Ok(key) => key,
        Err(e) => return SimpleResponse::from(e).simple_c_ptr(),
    };
    SimpleResponse::success(Box::into_raw(Box::new(key)) as *mut u8).simple_c_ptr()
}

/// Raw AES-256-CBC encrypt (pure crypto, no blob layout). Returns hex(ciphertext).
#[no_mangle]
pub unsafe extern "C" fn rust_aes256_cbc_encrypt(
    data: PtrString,
    key: PtrBytes,
    key_len: u32,
    iv: PtrBytes,
    iv_len: u32,
) -> *mut SimpleResponse<c_char> {
    let data = unsafe { recover_c_char(data) };
    let data = data.as_bytes();
    let key = extract_array!(key, u8, key_len as usize);
    let iv = extract_array!(iv, u8, iv_len as usize);
    let iv_generic = GenericArray::from_slice(iv);
    let key = GenericArray::from_slice(&key);
    let ct = Aes256CbcEnc::new(key, iv_generic).encrypt_padded_vec_mut::<Pkcs7>(data);
    SimpleResponse::success(convert_c_char(hex::encode(ct))).simple_c_ptr()
}

/// Raw AES-256-CBC decrypt (pure crypto, no blob layout). Input is hex(ciphertext).
#[no_mangle]
pub unsafe extern "C" fn rust_aes256_cbc_decrypt(
    hex_data: PtrString,
    key: PtrBytes,
    key_len: u32,
    iv: PtrBytes,
    iv_len: u32,
) -> *mut SimpleResponse<c_char> {
    let hex_data = unsafe { recover_c_char(hex_data) };
    let data = match hex::decode(hex_data) {
        Ok(data) => data,
        Err(_) => {
            return SimpleResponse::from(RustCError::InvalidHex("invalid ciphertext".to_string()))
                .simple_c_ptr()
        }
    };
    let key = extract_array!(key, u8, key_len as usize);
    let iv = extract_array!(iv, u8, iv_len as usize);
    let iv = GenericArray::from_slice(iv);
    let key = GenericArray::from_slice(&key);

    match Aes256CbcDec::new(key, iv).decrypt_padded_vec_mut::<Pkcs7>(&data) {
        Ok(pt) => match String::from_utf8(pt) {
            Ok(pt_str) => SimpleResponse::success(convert_c_char(pt_str)).simple_c_ptr(),
            Err(_) => SimpleResponse::from(RustCError::InvalidHex("invalid plaintext".to_string()))
                .simple_c_ptr(),
        },
        Err(_e) => SimpleResponse::from(RustCError::InvalidHex("decrypt failed".to_string()))
            .simple_c_ptr(),
    }
}

/// Storage layout for the UFVK blob: hex(IV_16) || hex(ciphertext). The fresh random IV
/// travels with the blob so the same plaintext+key never produces comparable ciphertext
/// (see security review). Encryption side: pack the caller-provided random IV ahead of ct.
#[no_mangle]
pub unsafe extern "C" fn rust_encrypt_ufvk_blob(
    data: PtrString,
    key: PtrBytes,
    key_len: u32,
    iv: PtrBytes,
    iv_len: u32,
) -> *mut SimpleResponse<c_char> {
    let data = unsafe { recover_c_char(data) };
    let data = data.as_bytes();
    let key = extract_array!(key, u8, key_len as usize);
    let iv = extract_array!(iv, u8, iv_len as usize);
    let iv_generic = GenericArray::from_slice(iv);
    let key = GenericArray::from_slice(&key);
    let ct = Aes256CbcEnc::new(key, iv_generic).encrypt_padded_vec_mut::<Pkcs7>(data);
    // Storage format: "z2" || hex(IV_16) || hex(ciphertext). The magic prefix distinguishes the
    // new layout from legacy pure-hex ciphertext; the fresh random IV travels with the blob so
    // the same plaintext+key never produces comparable ciphertext (see security review).
    let value = format!("z2{}{}", hex::encode(iv), hex::encode(ct));
    SimpleResponse::success(convert_c_char(value)).simple_c_ptr()
}

/// Storage layout for the UFVK blob: hex(IV_16) || hex(ciphertext). Decryption side:
/// split off the leading IV, then decrypt the rest.
#[no_mangle]
pub unsafe extern "C" fn rust_decrypt_ufvk_blob(
    blob: PtrString,
    key: PtrBytes,
    key_len: u32,
) -> *mut SimpleResponse<c_char> {
    let blob = unsafe { recover_c_char(blob) };
    let blob = blob.as_bytes();
    // Magic prefix distinguishes the new layout from legacy pure-hex ciphertext (whose
    // leading bytes would otherwise be misread as the IV). Legacy blobs fail here and are
    // regenerated by the caller instead of being probed heuristically.
    const MAGIC: &[u8] = b"z2";
    if !blob.starts_with(MAGIC) {
        return SimpleResponse::from(RustCError::InvalidHex("invalid blob magic".to_string()))
            .simple_c_ptr();
    }
    let payload = &blob[MAGIC.len()..];
    if payload.len() < 2 * 16 {
        return SimpleResponse::from(RustCError::InvalidHex("invalid blob".to_string()))
            .simple_c_ptr();
    }
    let (iv_hex, ct_hex) = payload.split_at(2 * 16);
    let iv = match hex::decode(iv_hex) {
        Ok(iv) => iv,
        Err(_) => {
            return SimpleResponse::from(RustCError::InvalidHex("invalid iv".to_string()))
                .simple_c_ptr()
        }
    };
    let data = match hex::decode(ct_hex) {
        Ok(data) => data,
        Err(_) => {
            return SimpleResponse::from(RustCError::InvalidHex("invalid ciphertext".to_string()))
                .simple_c_ptr()
        }
    };
    let key = extract_array!(key, u8, key_len as usize);
    let iv = GenericArray::from_slice(&iv);
    let key = GenericArray::from_slice(&key);

    match Aes256CbcDec::new(key, iv).decrypt_padded_vec_mut::<Pkcs7>(&data) {
        Ok(pt) => match String::from_utf8(pt) {
            Ok(pt_str) => SimpleResponse::success(convert_c_char(pt_str)).simple_c_ptr(),
            Err(_) => SimpleResponse::from(RustCError::InvalidHex("invalid plaintext".to_string()))
                .simple_c_ptr(),
        },
        Err(_e) => SimpleResponse::from(RustCError::InvalidHex("decrypt failed".to_string()))
            .simple_c_ptr(),
    }
}

#[cfg(test)]
mod tests {
    use alloc::{format, string::String, vec, vec::Vec};

    use super::*;

    #[cfg(feature = "cypherpunk")]
    fn test_zcash_payloads(count: usize) -> Vec<Vec<u8>> {
        (0..count)
            .map(|index| format!("pczt-{index}").into_bytes())
            .collect()
    }

    #[cfg(feature = "cypherpunk")]
    #[test]
    fn test_validate_zcash_batch_accepts_valid_payloads() {
        let payloads = test_zcash_payloads(2);

        validate_zcash_batch_payloads(&payloads).unwrap();
    }

    #[cfg(feature = "cypherpunk")]
    #[test]
    fn test_validate_zcash_batch_rejects_empty_batch_and_payload() {
        assert_eq!(
            validate_zcash_batch_payloads(&[]).unwrap_err(),
            RustCError::InvalidData("Zcash batch has no PCZTs".to_string())
        );

        assert_eq!(
            validate_zcash_batch_payloads(&[vec![]]).unwrap_err(),
            RustCError::InvalidData("Zcash batch PCZT 0 has no payload".to_string())
        );
    }

    #[cfg(feature = "cypherpunk")]
    #[test]
    fn test_validate_zcash_batch_rejects_duplicate_pczts() {
        let duplicate_payloads = vec![b"pczt".to_vec(), b"pczt".to_vec()];
        assert_eq!(
            validate_zcash_batch_payloads(&duplicate_payloads).unwrap_err(),
            RustCError::InvalidData("Zcash batch contains duplicate PCZTs".to_string())
        );
    }

    #[cfg(feature = "cypherpunk")]
    fn empty_batch_request() -> Vec<u8> {
        BatchSignRequest::new(vec![]).serialize().unwrap()
    }

    #[cfg(feature = "cypherpunk")]
    #[test]
    fn test_checked_zcash_batch_preserves_request_id() {
        let request_id = vec![0xaa, 0xbb];
        let checked = encode_checked_zcash_batch(&request_id, empty_batch_request()).unwrap();

        let (decoded_request_id, batch) = parse_checked_zcash_batch(&checked).unwrap();

        assert_eq!(decoded_request_id, request_id);
        assert!(batch.pczts().is_empty());
    }

    #[cfg(feature = "cypherpunk")]
    #[test]
    fn test_zcash_batch_rejects_invalid_envelope_bounds() {
        let registry = ZcashSignBatch::new(vec![], empty_batch_request());

        assert_eq!(
            parse_zcash_batch_registry(&registry).unwrap_err(),
            RustCError::InvalidData("Zcash batch request id must not be empty".to_string())
        );

        let oversized = ZcashSignBatch::new(vec![0xaa], vec![0; ZCASH_BATCH_MAX_RESOLVED_BYTES]);
        assert!(matches!(
            parse_zcash_batch_registry(&oversized),
            Err(RustCError::UnsupportedTransaction(message))
                if message.contains("resource budget")
        ));
    }

    #[cfg(feature = "cypherpunk")]
    fn padded_zcash_batch_with_orchard_actions(padding_bytes: usize) -> BatchSignRequest {
        use rand_core::OsRng;
        use zcash_primitives::transaction::{builder::PcztParts, TxVersion};
        use zcash_vendor::{
            orchard::{
                self,
                builder::{Builder, BundleType},
                bundle::BundleVersion,
                keys::{FullViewingKey, Scope, SpendingKey},
                value::NoteValue,
                Anchor,
            },
            pczt::roles::{creator::Creator, updater::Updater},
            zcash_protocol::consensus::{BlockHeight, BranchId},
        };

        let spending_key = SpendingKey::from_bytes([7; 32]).unwrap();
        let full_viewing_key = FullViewingKey::from(&spending_key);
        let recipient = full_viewing_key.address_at(0u32, Scope::External);
        let bundle_version = BundleVersion::orchard_v3();
        let mut builder = Builder::new(
            BundleType::DEFAULT,
            bundle_version,
            bundle_version.default_flags(),
            Anchor::empty_tree(),
        )
        .unwrap();
        builder
            .add_change_output(
                full_viewing_key,
                None,
                recipient,
                NoteValue::from_raw(1),
                [0; 512],
            )
            .unwrap();
        let (orchard, _) = builder.build_for_pczt(&mut OsRng).unwrap();
        let pczt = Creator::build_from_parts(PcztParts {
            params: MainNetwork,
            version: TxVersion::V6,
            consensus_branch_id: BranchId::Nu6_3,
            lock_time: 0,
            expiry_height: BlockHeight::from_u32(1),
            transparent: None,
            sapling: None,
            orchard: Some(orchard),
            ironwood: None,
        })
        .unwrap();
        let pczt = Updater::new(pczt)
            .update_global_with(|mut global| {
                global.set_proprietary("padding".to_string(), vec![0; padding_bytes]);
            })
            .finish();

        BatchSignRequest::new(vec![pczt])
    }

    #[cfg(feature = "cypherpunk")]
    #[test]
    fn test_zcash_batch_resource_budget_charges_batch_pczts_and_actions() {
        let batch = padded_zcash_batch_with_orchard_actions(ZCASH_BATCH_MAX_RESOLVED_BYTES - 8192);
        let canonical_bytes = batch.serialize().unwrap().len();
        let action_count = zcash_batch_action_count(&batch).unwrap();
        let action_growth = action_count * app_zcash::COMPACT_PCZT_MAX_RESOLVED_ACTION_GROWTH;
        assert!(action_count > 0);
        let request_id_len = ZCASH_BATCH_MAX_RESOLVED_BYTES
            - canonical_bytes
            - ZCASH_BATCH_PER_PCZT_ENCODING_OVERHEAD
            - action_growth;
        let mut request_id = vec![0xaa; request_id_len];

        validate_zcash_batch_envelope(&request_id, &batch.serialize().unwrap()).unwrap();
        assert_eq!(
            estimate_zcash_batch_resolved_bytes(&request_id, &batch).unwrap(),
            ZCASH_BATCH_MAX_RESOLVED_BYTES
        );
        validate_zcash_batch_resource_budget(&request_id, &batch).unwrap();

        request_id.push(0xaa);
        validate_zcash_batch_envelope(&request_id, &batch.serialize().unwrap()).unwrap();
        assert!(matches!(
            validate_zcash_batch_resource_budget(&request_id, &batch),
            Err(RustCError::UnsupportedTransaction(message))
                if message.contains("after field resolution")
        ));
    }

    #[cfg(feature = "cypherpunk")]
    #[test]
    fn test_zcash_batch_response_action_limit_accepts_96_and_rejects_97() {
        validate_zcash_batch_response_action_count(ZCASH_BATCH_MAX_RESPONSE_ACTIONS).unwrap();
        assert!(
            validate_zcash_batch_response_action_count(ZCASH_BATCH_MAX_RESPONSE_ACTIONS + 1)
                .is_err()
        );
    }

    #[cfg(feature = "cypherpunk")]
    #[test]
    fn test_encode_zcash_batch_sig_result_wraps_response_with_firmware_version() {
        use zcash_vendor::{orchard::ValuePool, pczt::roles::signer::SpendAuthSignature};

        let request_id = vec![0xaa, 0xbb];
        let response = BatchSignResponse::new(vec![
            vec![SpendAuthSignature::from_parts(
                ValuePool::Orchard,
                0,
                [0x11; 64],
            )],
            vec![SpendAuthSignature::from_parts(
                ValuePool::Ironwood,
                3,
                [0x22; 64],
            )],
        ]);
        let response_bytes = response.serialize().unwrap();

        let cbor =
            encode_zcash_batch_sig_result(request_id.clone(), response_bytes.clone()).unwrap();
        let decoded = ZcashBatchSigResult::try_from(cbor).unwrap();

        assert_eq!(decoded.get_request_id(), request_id);
        assert_eq!(decoded.get_data(), response_bytes);
        assert_eq!(
            decoded.get_firmware_version(),
            &KEYSTONE_FW_VERSION.encode()
        );
        assert_eq!(
            BatchSignResponse::parse(decoded.get_data()).unwrap(),
            response
        );
    }

    /// The batch signer fails closed: with no checked batch stored (a null
    /// container) it refuses and produces no signature UR.
    #[cfg(feature = "cypherpunk")]
    #[test]
    fn test_sign_zcash_batch_refuses_without_checked_batch() {
        for unlimited in [false, true] {
            let result = unsafe {
                if unlimited {
                    sign_zcash_batch_tx_cypherpunk_unlimited(
                        core::ptr::null_mut(),
                        core::ptr::null_mut(),
                        0,
                        false,
                        core::ptr::null_mut(),
                        0,
                    )
                } else {
                    sign_zcash_batch_tx_cypherpunk(
                        core::ptr::null_mut(),
                        core::ptr::null_mut(),
                        0,
                        false,
                        core::ptr::null_mut(),
                        0,
                    )
                }
            };
            assert!(!result.is_null());
            assert!(
                unsafe { (*result).data.is_null() },
                "signing without a checked batch must not produce a signature UR"
            );
            unsafe { Box::from_raw(result).free() };
        }
    }

    #[test]
    fn test_aes256_cbc_encrypt() {
        let mut data = convert_c_char("hello world".to_string());
        let mut seed = hex::decode("5eb00bbddcf069084889a8ab9155568165f5c453ccb85e70811aaed6f6da5fc19a5ac40b389cd370d086206dec8aa6c43daea6690f20ad3d8d48b2d2ce9e38e4").unwrap();
        let key_resp = unsafe { rust_derive_key_from_seed(seed.as_mut_ptr(), 64) };
        let mut key = unsafe { slice::from_raw_parts_mut((*key_resp).data, 32) };
        let mut iv_bytes = [0u8; 16];
        iv_bytes.copy_from_slice(&hex::decode("73e6ca87d5cd5622cdc747367905efe7").unwrap());
        let ct = unsafe {
            rust_aes256_cbc_encrypt(data, key.as_mut_ptr(), 32, iv_bytes.as_mut_ptr(), 16)
        };
        assert!(!ct.is_null());
        let ct_vec = unsafe { (*ct).data };
        let value = unsafe { recover_c_char(ct_vec) };
        // Pure ciphertext only: "hello world" -> 16-byte block -> 32 hex chars, no IV prefix.
        assert_eq!(value.len(), 32);
        assert!(hex::decode(value).is_ok());
    }

    #[test]
    fn test_aes256_ufvk_blob() {
        let mut data = convert_c_char("hello world".to_string());
        let mut seed = hex::decode("5eb00bbddcf069084889a8ab9155568165f5c453ccb85e70811aaed6f6da5fc19a5ac40b389cd370d086206dec8aa6c43daea6690f20ad3d8d48b2d2ce9e38e4").unwrap();
        let key_resp = unsafe { rust_derive_key_from_seed(seed.as_mut_ptr(), 64) };
        let mut key = unsafe { slice::from_raw_parts_mut((*key_resp).data, 32) };
        let mut iv_bytes = [0u8; 16];
        iv_bytes.copy_from_slice(&hex::decode("73e6ca87d5cd5622cdc747367905efe7").unwrap());
        // Blob encrypt packs magic + hex(IV) || hex(ciphertext).
        let blob_resp = unsafe {
            rust_encrypt_ufvk_blob(data, key.as_mut_ptr(), 32, iv_bytes.as_mut_ptr(), 16)
        };
        assert!(!blob_resp.is_null());
        let blob = unsafe { recover_c_char((*blob_resp).data) };
        assert_eq!(blob.len(), 2 + 32 + 32); // magic + IV prefix + one-block ciphertext
        assert!(blob.starts_with("z2"));
        assert!(blob[2..].starts_with("73e6ca87d5cd5622cdc747367905efe7"));

        // Blob decrypt splits magic+IV off and recovers the plaintext.
        let blob_input = convert_c_char(blob);
        let pt_resp = unsafe { rust_decrypt_ufvk_blob(blob_input, key.as_mut_ptr(), 32) };
        assert!(!pt_resp.is_null());
        let pt = unsafe { recover_c_char((*pt_resp).data) };
        assert_eq!(pt, "hello world");
    }

    #[test]
    fn test_aes256_ufvk_blob_rejects_legacy_or_garbage() {
        let mut seed = hex::decode("5eb00bbddcf069084889a8ab9155568165f5c453ccb85e70811aaed6f6da5fc19a5ac40b389cd370d086206dec8aa6c43daea6690f20ad3d8d48b2d2ce9e38e4").unwrap();
        let key_resp = unsafe { rust_derive_key_from_seed(seed.as_mut_ptr(), 64) };
        let mut key = unsafe { slice::from_raw_parts_mut((*key_resp).data, 32) };

        // Legacy blob: pure hex ciphertext without the magic prefix must be rejected
        // (so the caller regenerates) instead of being misparsed or panicking.
        let legacy = convert_c_char(
            "73e6ca87d5cd5622cdc747367905efe700000000000000000000000000000000".to_string(),
        );
        let r1 = unsafe { rust_decrypt_ufvk_blob(legacy, key.as_mut_ptr(), 32) };
        assert!(!r1.is_null());
        assert!(unsafe { (*r1).data.is_null() });

        // Truncated blob (magic but too short) must be rejected, not panic.
        let truncated = convert_c_char("z273e6ca87d5cd56".to_string());
        let r2 = unsafe { rust_decrypt_ufvk_blob(truncated, key.as_mut_ptr(), 32) };
        assert!(!r2.is_null());
        assert!(unsafe { (*r2).data.is_null() });
    }

    #[test]
    fn test_aes256_cbc_decrypt() {
        let mut seed = hex::decode("5eb00bbddcf069084889a8ab9155568165f5c453ccb85e70811aaed6f6da5fc19a5ac40b389cd370d086206dec8aa6c43daea6690f20ad3d8d48b2d2ce9e38e4").unwrap();
        let mut iv_bytes = [0u8; 16];
        iv_bytes.copy_from_slice(&hex::decode("73e6ca87d5cd5622cdc747367905efe7").unwrap());
        // Encrypt to get pure ciphertext under the seed-derived key.
        let enc_data = convert_c_char("hello world".to_string());
        let key_resp = unsafe { rust_derive_key_from_seed(seed.as_mut_ptr(), 64) };
        let mut key = unsafe { slice::from_raw_parts_mut((*key_resp).data, 32) };
        let ct = unsafe {
            rust_aes256_cbc_encrypt(enc_data, key.as_mut_ptr(), 32, iv_bytes.as_mut_ptr(), 16)
        };
        let ct_hex = unsafe { recover_c_char((*ct).data) };

        // Decrypt back with the same IV.
        let data = convert_c_char(ct_hex);
        let key_resp = unsafe { rust_derive_key_from_seed(seed.as_mut_ptr(), 64) };
        let mut key = unsafe { slice::from_raw_parts_mut((*key_resp).data, 32) };
        let pt = unsafe {
            rust_aes256_cbc_decrypt(data, key.as_mut_ptr(), 32, iv_bytes.as_mut_ptr(), 16)
        };
        assert!(!pt.is_null());
        let ct_vec = unsafe { (*pt).data };
        let value = unsafe { recover_c_char(ct_vec) };
        assert_eq!(value, "hello world");
    }

    #[test]
    fn test_dep_aes256() {
        let mut data = b"hello world";
        let seed = hex::decode("5eb00bbddcf069084889a8ab9155568165f5c453ccb85e70811aaed6f6da5fc19a5ac40b389cd370d086206dec8aa6c43daea6690f20ad3d8d48b2d2ce9e38e4").unwrap();
        // AES key derived from the seed at the dedicated path.
        let key_bytes =
            get_private_key_by_seed(&seed, &"m/44'/1557192335'/0'/3'/0'".to_string()).unwrap();
        let mut iv_bytes = [0u8; 16];
        iv_bytes.copy_from_slice(&hex::decode("73e6ca87d5cd5622cdc747367905efe7").unwrap());
        let iv = GenericArray::from_slice(&iv_bytes);
        let key = GenericArray::from_slice(&key_bytes);

        let encrypter = Aes256CbcEnc::new(key, iv);
        let decrypter = Aes256CbcDec::new(key, iv);

        let ct = encrypter.encrypt_padded_vec_mut::<Pkcs7>(data);
        let pt = decrypter.decrypt_padded_vec_mut::<Pkcs7>(&ct).unwrap();

        assert_eq!(String::from_utf8(pt).unwrap(), "hello world");
    }

    /// A minimal display row; the real content parity is covered by the app-level
    /// tests, so this only exercises the rust_c cache plumbing.
    #[cfg(feature = "cypherpunk")]
    fn sample_parsed_pczt() -> ParsedPczt {
        ParsedPczt::new(
            None,
            None,
            None,
            "1 ZEC".to_string(),
            "0.0001 ZEC".to_string(),
            false,
        )
    }

    /// The batch parse FFI reads the display cache the check stored and returns one
    /// display per cached row without re-deriving anything. The item count is
    /// asserted through the conversion helper (`TransactionParseResult::data` is
    /// private), then the full FFI is driven end-to-end for the no-crash path.
    #[cfg(feature = "cypherpunk")]
    #[test]
    fn test_parse_zcash_batch_reads_display_cache() {
        let items = batch_display_items(&BatchDisplayCache::new(
            vec![
                sample_parsed_pczt(),
                sample_parsed_pczt(),
                sample_parsed_pczt(),
            ],
            Vec::new(),
            [0; 32],
            0,
        ));
        assert_eq!(
            items.len(),
            3,
            "the cache must yield one display per review row"
        );
        for item in &items {
            unsafe { item.free() };
        }

        let cache = BatchDisplayCache::new(
            vec![sample_parsed_pczt(), sample_parsed_pczt()],
            Vec::new(),
            [0; 32],
            0,
        );
        let checked_ptr =
            ZcashCheckedPczt::new_with_display(b"normalized-batch-bytes".to_vec(), cache).c_ptr();
        let result = unsafe {
            parse_zcash_batch_tx_cypherpunk(
                checked_ptr,
                core::ptr::null_mut(),
                core::ptr::null_mut(),
                false,
            )
        };
        assert!(
            !result.is_null(),
            "parse must produce a result for a cache-bearing container"
        );
        unsafe {
            Box::from_raw(result).free();
            free_zcash_checked_pczt(checked_ptr);
        }
    }

    /// Freeing a checked container that carries a display cache must free both the
    /// bytes and the cache exactly once (reaching the end without an allocator
    /// abort is the assertion).
    #[cfg(feature = "cypherpunk")]
    #[test]
    fn test_free_cache_bearing_container_is_clean() {
        let cache = BatchDisplayCache::new(
            vec![sample_parsed_pczt(), sample_parsed_pczt()],
            Vec::new(),
            [0; 32],
            0,
        );
        let checked_ptr =
            ZcashCheckedPczt::new_with_display(b"normalized-batch-bytes".to_vec(), cache).c_ptr();
        unsafe { free_zcash_checked_pczt(checked_ptr) };
    }
}
