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
use core::slice;
use cryptoxide::hashing::sha256;
use cty::c_char;
use keystore::algorithms::{
    ed25519::slip10_ed25519::get_private_key_by_seed,
    zcash::{calculate_seed_fingerprint, derive_ufvk},
};
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

// Cap both per-PCZT overhead and variable-size payload data to leave headroom
// in shared device memory while processing a batch.
#[cfg(feature = "cypherpunk")]
const ZCASH_BATCH_MAX_PCZTS: usize = 50;
#[cfg(feature = "cypherpunk")]
const ZCASH_BATCH_MAX_TOTAL_BYTES: usize = 512 * 1024;

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

/// Enforces the count, canonical byte, and exact payload uniqueness limits.
#[cfg(feature = "cypherpunk")]
fn validate_zcash_batch_payloads(payloads: &[Vec<u8>]) -> Result<(), RustCError> {
    if payloads.is_empty() {
        return Err(RustCError::InvalidData(
            "Zcash batch has no PCZTs".to_string(),
        ));
    }
    if payloads.len() > ZCASH_BATCH_MAX_PCZTS {
        return Err(RustCError::UnsupportedTransaction(format!(
            "Zcash batch supports at most {ZCASH_BATCH_MAX_PCZTS} PCZTs"
        )));
    }

    let mut total_payload_bytes = 0usize;
    let mut payload_digests = Vec::with_capacity(payloads.len());
    for (index, payload) in payloads.iter().enumerate() {
        if payload.is_empty() {
            return Err(RustCError::InvalidData(format!(
                "Zcash batch PCZT {index} has no payload"
            )));
        }
        total_payload_bytes = total_payload_bytes.saturating_add(payload.len());
        if total_payload_bytes > ZCASH_BATCH_MAX_TOTAL_BYTES {
            return Err(RustCError::UnsupportedTransaction(format!(
                "Zcash batch PCZTs exceed {ZCASH_BATCH_MAX_TOTAL_BYTES} bytes"
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

/// Bounds the outer request before parsing or retaining its checked state.
#[cfg(feature = "cypherpunk")]
fn validate_zcash_batch_envelope(request_id: &[u8], data: &[u8]) -> Result<(), RustCError> {
    if request_id.is_empty() {
        return Err(RustCError::InvalidData(
            "Zcash batch request id must not be empty".to_string(),
        ));
    }
    if request_id.len().saturating_add(data.len()) > ZCASH_BATCH_MAX_TOTAL_BYTES {
        return Err(RustCError::UnsupportedTransaction(format!(
            "Zcash batch request exceeds {ZCASH_BATCH_MAX_TOTAL_BYTES} bytes"
        )));
    }
    Ok(())
}

/// Parses the bounded outer registry into the PCZT crate's batch request.
#[cfg(feature = "cypherpunk")]
fn parse_zcash_batch_registry(registry: &ZcashSignBatch) -> Result<BatchSignRequest, RustCError> {
    validate_zcash_batch_envelope(registry.get_request_id(), registry.get_data())?;
    BatchSignRequest::parse(registry.get_data())
        .map_err(|e| RustCError::InvalidData(format!("invalid PCZT batch request: {e:?}")))
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

/// Wraps the PCZT crate's signature response with its echoed request id.
#[cfg(feature = "cypherpunk")]
fn encode_zcash_batch_sig_result(
    request_id: Vec<u8>,
    data: Vec<u8>,
) -> Result<Vec<u8>, RustCError> {
    ZcashBatchSigResult::new(request_id, data)
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
    let ufvk_text = unsafe { recover_c_char(ufvk) };
    let seed_fingerprint = extract_array!(seed_fingerprint, u8, 32);
    let seed_fingerprint = seed_fingerprint.try_into().unwrap();

    let payloads = match validate_zcash_batch(&batch) {
        Ok(payloads) => payloads,
        Err(e) => return TransactionCheckResult::from(e).c_ptr(),
    };

    let mut checked_pczts = Vec::with_capacity(payloads.len());
    for payload in payloads {
        match app_zcash::check_batch_pczt_cypherpunk(
            &MainNetwork,
            &payload,
            &ufvk_text,
            seed_fingerprint,
            account_index,
        ) {
            Ok(normalized) => {
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
            }
            Err(e) => return TransactionCheckResult::from(e).c_ptr(),
        }
    }

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
    *checked_batch = ZcashCheckedPczt::new(normalized_batch).c_ptr();
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
    let checked = extract_ptr_with_type!(checked_batch, ZcashCheckedPczt);
    let bytes = match checked.checked_bytes() {
        Ok(bytes) => bytes,
        Err(e) => return TransactionParseResult::from(e).c_ptr(),
    };
    let batch = match parse_checked_zcash_batch(bytes) {
        Ok((_, batch)) => batch,
        Err(e) => return TransactionParseResult::from(e).c_ptr(),
    };
    let ufvk_text = unsafe { recover_c_char(ufvk) };
    let seed_fingerprint = extract_array!(seed_fingerprint, u8, 32);
    let seed_fingerprint = seed_fingerprint.try_into().unwrap();

    // Serialize the normalized PCZTs once, then parse each into a complete
    // display model. Eligible Orchard-to-Ironwood transfers are folded by
    // content; ambiguous batches keep their ordinary review pages.
    let payloads = match batch
        .pczts()
        .iter()
        .map(serialize_batch_pczt)
        .collect::<Result<Vec<_>, _>>()
    {
        Ok(payloads) => payloads,
        Err(e) => return TransactionParseResult::from(e).c_ptr(),
    };
    let parsed_items = match app_zcash::parse_batch_with_migration_summary_cypherpunk(
        &MainNetwork,
        payloads.iter().map(Vec::as_slice),
        &ufvk_text,
        seed_fingerprint,
    ) {
        Ok(items) => items,
        Err(e) => return TransactionParseResult::from(e).c_ptr(),
    };
    // Convert only after every PCZT has parsed. These FFI values own heap
    // allocations freed by `free_TransactionParseResult_DisplayZcashBatch`, not
    // Rust `Drop`; an early return after partial conversion would leak memory.
    let display_items: Vec<DisplayPczt> = parsed_items.iter().map(DisplayPczt::from).collect();

    TransactionParseResult::success(DisplayZcashBatch::from(display_items).c_ptr()).c_ptr()
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
    let expected_seed_fingerprint = extract_array!(seed_fingerprint, u8, 32);
    let expected_seed_fingerprint: &[u8; 32] = expected_seed_fingerprint.try_into().unwrap();
    let mut seed = extract_array_mut!(seed, u8, seed_len as usize);

    let result = match checked.checked_bytes() {
        Ok(bytes) => match parse_checked_zcash_batch(bytes) {
            Ok((request_id, batch)) => match calculate_seed_fingerprint(seed) {
                Ok(seed_fingerprint) => {
                    if &seed_fingerprint != expected_seed_fingerprint {
                        seed.zeroize();
                        return UREncodeResult::from(RustCError::MasterFingerprintMismatch).c_ptr();
                    }

                    let mut results = Vec::new();
                    let mut error = None;
                    // Preserve request order and emit nothing unless every PCZT signs.
                    for pczt in batch.pczts() {
                        let payload = match serialize_batch_pczt(pczt) {
                            Ok(payload) => payload,
                            Err(e) => {
                                error = Some(UREncodeResult::from(e).c_ptr());
                                break;
                            }
                        };
                        match app_zcash::sign_checked_batch_pczt(
                            &MainNetwork,
                            &payload,
                            seed,
                            &seed_fingerprint,
                            account_index,
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

#[no_mangle]
pub unsafe extern "C" fn rust_aes256_cbc_encrypt(
    data: PtrString,
    password: PtrString,
    iv: PtrBytes,
    iv_len: u32,
) -> *mut SimpleResponse<c_char> {
    let data = unsafe { recover_c_char(data) };
    let data = data.as_bytes();
    let password = unsafe { recover_c_char(password) };
    let iv = extract_array!(iv, u8, iv_len as usize);
    let key = sha256(password.as_bytes());
    let iv = GenericArray::from_slice(iv);
    let key = GenericArray::from_slice(&key);
    let ct = Aes256CbcEnc::new(key, iv).encrypt_padded_vec_mut::<Pkcs7>(data);
    SimpleResponse::success(convert_c_char(hex::encode(ct))).simple_c_ptr()
}

#[no_mangle]
pub unsafe extern "C" fn rust_aes256_cbc_decrypt(
    hex_data: PtrString,
    password: PtrString,
    iv: PtrBytes,
    iv_len: u32,
) -> *mut SimpleResponse<c_char> {
    let hex_data = unsafe { recover_c_char(hex_data) };
    let data = hex::decode(hex_data).unwrap();
    let password = unsafe { recover_c_char(password) };
    let iv = extract_array!(iv, u8, iv_len as usize);
    let key = sha256(password.as_bytes());
    let iv = GenericArray::from_slice(iv);
    let key = GenericArray::from_slice(&key);

    match Aes256CbcDec::new(key, iv).decrypt_padded_vec_mut::<Pkcs7>(&data) {
        Ok(pt) => {
            SimpleResponse::success(convert_c_char(String::from_utf8(pt).unwrap())).simple_c_ptr()
        }
        Err(_e) => SimpleResponse::from(RustCError::InvalidHex("decrypt failed".to_string()))
            .simple_c_ptr(),
    }
}

#[no_mangle]
pub unsafe extern "C" fn rust_derive_iv_from_seed(
    seed: PtrBytes,
    seed_len: u32,
) -> *mut SimpleResponse<u8> {
    let seed = extract_array!(seed, u8, seed_len as usize);
    let iv_path = "m/44'/1557192335'/0'/2'/0'".to_string();
    let iv = get_private_key_by_seed(seed, &iv_path).unwrap();
    let mut iv_bytes = [0; 16];
    iv_bytes.copy_from_slice(&iv[..16]);
    SimpleResponse::success(Box::into_raw(Box::new(iv_bytes)) as *mut u8).simple_c_ptr()
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
    fn test_validate_zcash_batch_accepts_max_pczts() {
        let payloads = test_zcash_payloads(ZCASH_BATCH_MAX_PCZTS);

        validate_zcash_batch_payloads(&payloads).unwrap();
    }

    #[cfg(feature = "cypherpunk")]
    #[test]
    fn test_validate_zcash_batch_rejects_oversized_total_payload() {
        // Three PCZTs whose summed payloads cross the byte bound: the count cap
        // alone no longer bounds RAM, so the byte bound must reject this.
        let big = vec![0xAB; ZCASH_BATCH_MAX_TOTAL_BYTES / 2];
        let payloads = vec![big.clone(), [big.as_slice(), &[0x01]].concat(), vec![0x02]];

        let error = validate_zcash_batch_payloads(&payloads).unwrap_err();
        assert!(matches!(
            error,
            RustCError::UnsupportedTransaction(message)
                if message.contains("PCZTs exceed")
        ));
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
    fn test_validate_zcash_batch_rejects_too_many_pczts() {
        let payloads = test_zcash_payloads(ZCASH_BATCH_MAX_PCZTS + 1);

        assert!(matches!(
            validate_zcash_batch_payloads(&payloads),
            Err(RustCError::UnsupportedTransaction(message))
                if message.contains("supports at most")
        ));
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

        let oversized = ZcashSignBatch::new(vec![0xaa], vec![0; ZCASH_BATCH_MAX_TOTAL_BYTES]);
        assert!(matches!(
            parse_zcash_batch_registry(&oversized),
            Err(RustCError::UnsupportedTransaction(message))
                if message.contains("batch request exceeds")
        ));
    }

    #[cfg(feature = "cypherpunk")]
    #[test]
    fn test_encode_zcash_batch_sig_result_wraps_pczt_response() {
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
        let mut password = convert_c_char("password".to_string());
        let mut seed = hex::decode("5eb00bbddcf069084889a8ab9155568165f5c453ccb85e70811aaed6f6da5fc19a5ac40b389cd370d086206dec8aa6c43daea6690f20ad3d8d48b2d2ce9e38e4").unwrap();
        let iv = unsafe { rust_derive_iv_from_seed(seed.as_mut_ptr(), 64) };
        let mut iv = unsafe { slice::from_raw_parts_mut((*iv).data, 16) };
        let iv_len = 16;
        let ct = unsafe { rust_aes256_cbc_encrypt(data, password, iv.as_mut_ptr(), iv_len as u32) };
        assert!(!ct.is_null());
        let ct_vec = unsafe { (*ct).data };
        let value = unsafe { recover_c_char(ct_vec) };
        assert_eq!(value, "4989eed8515d7d3fcc16b009d8cdff9e");
    }

    #[test]
    fn test_aes256_cbc_decrypt() {
        //8dd387c3b2656d9f24ace7c3daf6fc26a1c161098460f8dddd37545fc951f9cd7da6c75c71ae52f32ceb8827eca2169ef4a643d2ccb9f01389d281a85850e2ddd100630ab1ca51310c3e6ccdd3029d0c48db18cdc971dba8f0daff9ad281b56221ffefc7d32333ea310a1f74f99dea444f8a089002cf1f0cd6a4ddf608a7b5388dc09f9417612657b9bf335a466f951547f9707dd129b3c24c900a26010f51c543eba10e9aabef7062845dc6969206b25577a352cb4d984db67c54c7615fe60769726bffa59fd8bd0b66fe29ee3c358af13cf0796c2c062bc79b73271eb0366f0536e425f8e42307ead4c695804fd3281aca5577d9a621e3a8047b14128c280c45343b5bbb783a065d94764e90ad6820fe81a200637401c256b1fb8f58a9d412d303b89c647411662907cdc55ed93adb
        //73e6ca87d5cd5622cdc747367905efe7
        //68487dc295052aa79c530e283ce698b8c6bb1b42ff0944252e1910dbecdc5425
        let mut seed = hex::decode("5eb00bbddcf069084889a8ab9155568165f5c453ccb85e70811aaed6f6da5fc19a5ac40b389cd370d086206dec8aa6c43daea6690f20ad3d8d48b2d2ce9e38e4").unwrap();
        // First encrypt to get ciphertext
        let enc_data = convert_c_char("hello world".to_string());
        let enc_password = convert_c_char("password".to_string());
        let iv_resp = unsafe { rust_derive_iv_from_seed(seed.as_mut_ptr(), 64) };
        let mut iv_enc = unsafe { slice::from_raw_parts_mut((*iv_resp).data, 16) };
        let ct =
            unsafe { rust_aes256_cbc_encrypt(enc_data, enc_password, iv_enc.as_mut_ptr(), 16) };
        let ct_hex = unsafe { recover_c_char((*ct).data) };
        assert_eq!(ct_hex, "4989eed8515d7d3fcc16b009d8cdff9e");

        // Now decrypt
        let data = convert_c_char(ct_hex);
        let password = convert_c_char("password".to_string());
        let iv = unsafe { rust_derive_iv_from_seed(seed.as_mut_ptr(), 64) };
        let iv = unsafe { slice::from_raw_parts_mut((*iv).data, 16) };
        let iv_len = 16;
        let pt = unsafe { rust_aes256_cbc_decrypt(data, password, iv.as_mut_ptr(), iv_len as u32) };
        assert!(!pt.is_null());
        let ct_vec = unsafe { (*pt).data };
        let value = unsafe { recover_c_char(ct_vec) };
        assert_eq!(value, "hello world");
    }

    #[test]
    fn test_dep_aes256() {
        let mut data = b"hello world";
        let seed = hex::decode("5eb00bbddcf069084889a8ab9155568165f5c453ccb85e70811aaed6f6da5fc19a5ac40b389cd370d086206dec8aa6c43daea6690f20ad3d8d48b2d2ce9e38e4").unwrap();
        let iv_path = "m/44'/1557192335'/0'/2'/0'".to_string();
        let iv = get_private_key_by_seed(&seed, &iv_path).unwrap();
        let mut iv_bytes = [0; 16];
        iv_bytes.copy_from_slice(&iv[..16]);
        let key = sha256(b"password");
        let iv = GenericArray::from_slice(&iv_bytes);
        let key = GenericArray::from_slice(&key);

        let encrypter = Aes256CbcEnc::new(key, iv);
        let decrypter = Aes256CbcDec::new(key, iv);

        let ct = encrypter.encrypt_padded_vec_mut::<Pkcs7>(data);
        let pt = decrypter.decrypt_padded_vec_mut::<Pkcs7>(&ct).unwrap();

        assert_eq!(String::from_utf8(pt).unwrap(), "hello world");
    }
}
