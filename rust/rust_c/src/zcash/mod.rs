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
use ur_registry::traits::RegistryItem;
use ur_registry::zcash::zcash_batch_sig_result::{
    ZcashActionSig, ZcashBatchSigResult, ZcashMsgSig, ZCASH_BATCH_SIG_RESULT_VERSION,
};
use ur_registry::zcash::zcash_pczt::ZcashPczt;
use ur_registry::zcash::zcash_sign_batch::{
    ZcashSignBatch, ZcashSignMessage, ZCASH_SIGN_BATCH_NETWORK_MAINNET, ZCASH_SIGN_BATCH_VERSION,
    ZCASH_SIGN_MESSAGE_KIND_PCZT_V1,
};
use zcash_vendor::zcash_protocol::consensus::MainNetwork;
use zeroize::Zeroize;

// Batch memory is intentionally bounded by message count rather than separate
// byte caps. With the supported pczt-v1 messages, a full 35-message batch used
// about 35% of RAM on target hardware. Revisit this if new message kinds or
// substantially larger payload encodings are added.
const ZCASH_BATCH_MAX_MESSAGES: usize = 35;

/// Fingerprint of the last Zcash batch that completed the review path
/// (`parse_zcash_batch_tx_cypherpunk`), which verifies every message before
/// the user approves what is displayed. The batch signer signs without
/// re-running those checks, so it refuses any batch whose fingerprint differs
/// from the reviewed one — making "the device signs exactly the bytes the user
/// reviewed" an enforced invariant rather than a GUI control-flow assumption.
///
/// The fingerprint is cleared when a review starts (so a failed parse cannot
/// leave a previous batch armed) and recorded on review success. It
/// intentionally survives signing so the signature QR can be regenerated for
/// the same reviewed batch. Recording at parse success rather than at user
/// approval is sound because the GUI cannot start a new review while an
/// approval screen is open: USB resolve-UR requests are refused outside the
/// home/transport views, and reaching the QR scanner tears the review view
/// down.
///
/// Locking discipline: the review path (the only writer) runs on the
/// background task and locks unconditionally; the signer — which the USB flow
/// runs on the higher-priority UI task — only `try_lock`s and refuses to sign
/// on contention, so a higher-priority task can never spin-wait on a
/// preempted lock holder (fail closed instead of livelock).
#[cfg(feature = "cypherpunk")]
static REVIEWED_BATCH_FINGERPRINT: spin::Mutex<Option<[u8; 32]>> = spin::Mutex::new(None);

/// Confirms that `batch_fingerprint` is the reviewed batch's fingerprint.
/// `try_lock` so the (possibly higher-priority) signing task never spin-waits
/// on a preempted lock holder — an unobtainable lock means the reviewed
/// fingerprint cannot be confirmed, so this reports unreviewed (fail closed).
#[cfg(feature = "cypherpunk")]
fn confirm_batch_reviewed(batch_fingerprint: &[u8; 32]) -> bool {
    REVIEWED_BATCH_FINGERPRINT
        .try_lock()
        .is_some_and(|reviewed| reviewed.as_ref() == Some(batch_fingerprint))
}

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
) -> *mut TransactionCheckResult {
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
        Ok(_) => TransactionCheckResult::new().c_ptr(),
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
) -> *mut TransactionCheckResult {
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
        Ok(_) => TransactionCheckResult::new().c_ptr(),
        Err(e) => TransactionCheckResult::from(e).c_ptr(),
    }
}

#[cfg(feature = "cypherpunk")]
#[no_mangle]
pub unsafe extern "C" fn parse_zcash_tx_cypherpunk(
    tx: PtrUR,
    ufvk: PtrString,
    seed_fingerprint: PtrBytes,
) -> Ptr<TransactionParseResult<DisplayPczt>> {
    let pczt = extract_ptr_with_type!(tx, ZcashPczt);
    let ufvk_text = unsafe { recover_c_char(ufvk) };
    let seed_fingerprint = extract_array!(seed_fingerprint, u8, 32);
    let seed_fingerprint = seed_fingerprint.try_into().unwrap();
    match app_zcash::parse_pczt_cypherpunk(
        &MainNetwork,
        &pczt.get_data(),
        &ufvk_text,
        seed_fingerprint,
    ) {
        Ok(pczt) => TransactionParseResult::success(DisplayPczt::from(&pczt).c_ptr()).c_ptr(),
        Err(e) => TransactionParseResult::from(e).c_ptr(),
    }
}

#[cfg(feature = "multi-coins")]
#[no_mangle]
pub unsafe extern "C" fn parse_zcash_tx_multi_coins(
    tx: PtrUR,
    seed_fingerprint: PtrBytes,
) -> Ptr<TransactionParseResult<DisplayPczt>> {
    let pczt = extract_ptr_with_type!(tx, ZcashPczt);
    let seed_fingerprint = extract_array!(seed_fingerprint, u8, 32);
    let seed_fingerprint = seed_fingerprint.try_into().unwrap();
    match app_zcash::parse_pczt_multi_coins(&MainNetwork, &pczt.get_data(), seed_fingerprint) {
        Ok(pczt) => TransactionParseResult::success(DisplayPczt::from(&pczt).c_ptr()).c_ptr(),
        Err(e) => TransactionParseResult::from(e).c_ptr(),
    }
}

/// Validates the batch container and returns its fingerprint: one hash over
/// the header fields plus a fixed-width (id digest, payload digest) pair per
/// message, so any change to what the signer would consume changes the
/// fingerprint. The per-payload digests are already needed for duplicate
/// detection, so this adds no extra payload hashing.
fn validate_zcash_batch(batch: &ZcashSignBatch) -> Result<[u8; 32], RustCError> {
    let messages = batch.get_messages();
    if batch.get_version() != ZCASH_SIGN_BATCH_VERSION {
        return Err(RustCError::UnsupportedTransaction(format!(
            "unsupported Zcash batch version {}",
            batch.get_version()
        )));
    }
    if batch.get_network() != ZCASH_SIGN_BATCH_NETWORK_MAINNET {
        return Err(RustCError::UnsupportedTransaction(
            "only Zcash mainnet batch signing is supported".to_string(),
        ));
    }
    if batch.get_request_id().is_empty() {
        return Err(RustCError::InvalidData(
            "Zcash batch has no request id".to_string(),
        ));
    }
    if !batch.get_atomic() {
        return Err(RustCError::UnsupportedTransaction(
            "Zcash batch signing requires atomic=true".to_string(),
        ));
    }
    if messages.is_empty() {
        return Err(RustCError::InvalidData(
            "Zcash batch has no messages".to_string(),
        ));
    }
    if messages.len() > ZCASH_BATCH_MAX_MESSAGES {
        return Err(RustCError::UnsupportedTransaction(format!(
            "Zcash batch supports at most {ZCASH_BATCH_MAX_MESSAGES} messages"
        )));
    }

    let mut fingerprint_data = Vec::with_capacity(8 + 32 + messages.len() * 64);
    fingerprint_data.extend_from_slice(&batch.get_version().to_le_bytes());
    fingerprint_data.extend_from_slice(&batch.get_network().to_le_bytes());
    fingerprint_data.extend_from_slice(&sha256(batch.get_request_id()));

    let mut seen_messages: Vec<(&[u8], [u8; 32])> = Vec::new();
    for (index, message) in messages.iter().enumerate() {
        if message.get_kind() != ZCASH_SIGN_MESSAGE_KIND_PCZT_V1 {
            return Err(RustCError::UnsupportedTransaction(format!(
                "unsupported Zcash batch message kind {}",
                message.get_kind()
            )));
        }
        if message.get_id().is_empty() {
            return Err(RustCError::InvalidData(format!(
                "Zcash batch message {index} has no id"
            )));
        }
        if message.get_payload().is_empty() {
            return Err(RustCError::InvalidData(format!(
                "Zcash batch message {index} has no payload"
            )));
        }

        let digest = sha256(message.get_payload());
        if let Some(expected_digest) = message.get_payload_digest() {
            if expected_digest.as_slice() != digest.as_slice() {
                return Err(RustCError::InvalidData(format!(
                    "Zcash batch message {index} payload digest mismatch"
                )));
            }
        }

        for (previous_id, previous_digest) in &seen_messages {
            if previous_digest == &digest {
                return Err(RustCError::InvalidData(
                    "Zcash batch contains duplicate payloads".to_string(),
                ));
            }
            if *previous_id == message.get_id().as_slice() {
                return Err(RustCError::InvalidData(
                    "Zcash batch contains duplicate message ids".to_string(),
                ));
            }
        }
        seen_messages.push((message.get_id().as_slice(), digest));
        fingerprint_data.extend_from_slice(&sha256(message.get_id()));
        fingerprint_data.extend_from_slice(&digest);
    }

    Ok(sha256(&fingerprint_data))
}

#[cfg(feature = "cypherpunk")]
fn check_zcash_batch_message_cypherpunk(
    message: &ZcashSignMessage,
    ufvk_text: &str,
    seed_fingerprint: &[u8; 32],
    account_index: u32,
) -> app_zcash::errors::Result<()> {
    app_zcash::check_pczt_cypherpunk(
        &MainNetwork,
        message.get_payload(),
        ufvk_text,
        seed_fingerprint,
        account_index,
    )?;
    app_zcash::ensure_pczt_has_signable_shielded_action(
        &MainNetwork,
        message.get_payload(),
        seed_fingerprint,
        account_index,
    )
}

#[cfg(feature = "cypherpunk")]
fn check_zcash_pczt_message_cypherpunk(
    payload: &[u8],
    ufvk_text: &str,
    seed_fingerprint: &[u8; 32],
    account_index: u32,
) -> app_zcash::errors::Result<()> {
    app_zcash::check_pczt_cypherpunk(
        &MainNetwork,
        payload,
        ufvk_text,
        seed_fingerprint,
        account_index,
    )
}

#[cfg(feature = "cypherpunk")]
#[no_mangle]
pub unsafe extern "C" fn check_zcash_batch_tx_cypherpunk(
    tx: PtrUR,
    ufvk: PtrString,
    seed_fingerprint: PtrBytes,
    account_index: u32,
    disabled: bool,
) -> *mut TransactionCheckResult {
    if disabled {
        return TransactionCheckResult::from(RustCError::UnsupportedTransaction(
            "Zcash requires at least 256-bit entropy (use 33-word Shamir shares)".to_string(),
        ))
        .c_ptr();
    }
    let batch = extract_ptr_with_type!(tx, ZcashSignBatch);
    let ufvk_text = unsafe { recover_c_char(ufvk) };
    let seed_fingerprint = extract_array!(seed_fingerprint, u8, 32);
    let seed_fingerprint = seed_fingerprint.try_into().unwrap();

    if let Err(e) = validate_zcash_batch(batch) {
        return TransactionCheckResult::from(e).c_ptr();
    }

    for message in batch.get_messages() {
        if let Err(e) = check_zcash_batch_message_cypherpunk(
            message,
            &ufvk_text,
            seed_fingerprint,
            account_index,
        ) {
            return TransactionCheckResult::from(e).c_ptr();
        }
    }

    TransactionCheckResult::new().c_ptr()
}

#[cfg(feature = "cypherpunk")]
#[no_mangle]
pub unsafe extern "C" fn parse_zcash_batch_tx_cypherpunk(
    tx: PtrUR,
    ufvk: PtrString,
    seed_fingerprint: PtrBytes,
    account_index: u32,
    disabled: bool,
) -> Ptr<TransactionParseResult<DisplayZcashBatch>> {
    // Starting a new review disarms any previously reviewed batch — before
    // any early return, so no parse attempt can leave an older batch armed.
    *REVIEWED_BATCH_FINGERPRINT.lock() = None;
    if disabled {
        return TransactionParseResult::from(RustCError::UnsupportedTransaction(
            "Zcash requires at least 256-bit entropy (use 33-word Shamir shares)".to_string(),
        ))
        .c_ptr();
    }
    let batch = extract_ptr_with_type!(tx, ZcashSignBatch);
    let ufvk_text = unsafe { recover_c_char(ufvk) };
    let seed_fingerprint = extract_array!(seed_fingerprint, u8, 32);
    let seed_fingerprint = seed_fingerprint.try_into().unwrap();

    let batch_fingerprint = match validate_zcash_batch(batch) {
        Ok(fingerprint) => fingerprint,
        Err(e) => return TransactionParseResult::from(e).c_ptr(),
    };

    #[cfg(zcash_unstable = "nu6.3")]
    if batch.get_messages().len() > 1 {
        match parse_zcash_batch_as_split_plus_migrations(
            batch,
            &ufvk_text,
            seed_fingerprint,
            account_index,
        ) {
            Ok(display_items) => {
                *REVIEWED_BATCH_FINGERPRINT.lock() = Some(batch_fingerprint);
                return TransactionParseResult::success(
                    DisplayZcashBatch::from(display_items).c_ptr(),
                )
                .c_ptr();
            }
            Err(_) => {}
        }
    }

    let mut display_items = Vec::new();
    for message in batch.get_messages() {
        match app_zcash::check_and_parse_batch_pczt_cypherpunk(
            &MainNetwork,
            message.get_payload(),
            &ufvk_text,
            &seed_fingerprint,
            account_index,
        ) {
            Ok(pczt) => display_items.push(DisplayPczt::from(&pczt)),
            Err(e) => return TransactionParseResult::from(e).c_ptr(),
        }
    }

    *REVIEWED_BATCH_FINGERPRINT.lock() = Some(batch_fingerprint);
    TransactionParseResult::success(DisplayZcashBatch::from(display_items).c_ptr()).c_ptr()
}

#[cfg(all(feature = "cypherpunk", zcash_unstable = "nu6.3"))]
fn parse_zcash_batch_as_split_plus_migrations(
    batch: &ZcashSignBatch,
    ufvk_text: &str,
    seed_fingerprint: [u8; 32],
    account_index: u32,
) -> app_zcash::errors::Result<Vec<DisplayPczt>> {
    let messages = batch.get_messages();
    let mut display_items = Vec::new();

    let split_message = &messages[0];
    let split_pczt = app_zcash::check_and_parse_batch_pczt_cypherpunk(
        &MainNetwork,
        split_message.get_payload(),
        ufvk_text,
        &seed_fingerprint,
        account_index,
    )?;
    display_items.push(DisplayPczt::from(&split_pczt));

    let mut summary = app_zcash::BatchMigrationSummary::default();
    for message in messages.iter().skip(1) {
        let child = app_zcash::summarize_batch_migration_pczt_cypherpunk(
            &MainNetwork,
            message.get_payload(),
            ufvk_text,
            &seed_fingerprint,
            account_index,
        )?;
        summary.add_child(&child)?;
    }

    let summary_pczt = summary.to_parsed_pczt();
    display_items.push(DisplayPczt::from(&summary_pczt));

    Ok(display_items)
}

#[cfg(feature = "cypherpunk")]
// `_ufvk` stays in the FFI signature for C-side call compatibility, but batch
// signing consumes only the PCZT bytes, seed material, and selected account.
unsafe fn sign_zcash_batch_tx_cypherpunk_dynamic(
    tx: PtrUR,
    _ufvk: PtrString,
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
    let batch = extract_ptr_with_type!(tx, ZcashSignBatch);
    let expected_seed_fingerprint = extract_array!(seed_fingerprint, u8, 32);
    let expected_seed_fingerprint: &[u8; 32] = expected_seed_fingerprint.try_into().unwrap();
    let mut seed = extract_array_mut!(seed, u8, seed_len as usize);

    let result = match validate_zcash_batch(batch) {
        Ok(batch_fingerprint) => {
            // Only sign the exact batch the user reviewed and approved: the
            // review path verified every message and recorded this
            // fingerprint, and this signer intentionally re-runs only cheap
            // structural checks.
            if !confirm_batch_reviewed(&batch_fingerprint) {
                seed.zeroize();
                return UREncodeResult::from(RustCError::InvalidData(
                    "Zcash batch does not match the reviewed batch".to_string(),
                ))
                .c_ptr();
            }
            let seed_fingerprint = calculate_seed_fingerprint(seed);
            match seed_fingerprint {
                Ok(seed_fingerprint) => {
                    if &seed_fingerprint != expected_seed_fingerprint {
                        seed.zeroize();
                        return UREncodeResult::from(RustCError::MasterFingerprintMismatch).c_ptr();
                    }

                    let mut results = Vec::new();
                    for message in batch.get_messages() {
                        match app_zcash::sign_batch_pczt_cypherpunk(
                            message.get_payload(),
                            seed,
                            &seed_fingerprint,
                            account_index,
                        ) {
                            Ok(payload) => {
                                match app_zcash::extract_compact_sigs_from_signed_pczt(&payload) {
                                    Ok(compact_sigs) => {
                                        let action_sigs = compact_sigs
                                            .into_iter()
                                            .map(|sig| {
                                                ZcashActionSig::new(
                                                    sig.pool,
                                                    sig.action_index,
                                                    sig.sig,
                                                )
                                            })
                                            .collect();
                                        results.push(ZcashMsgSig::new(
                                            message.get_id().clone(),
                                            action_sigs,
                                        ));
                                    }
                                    Err(e) => {
                                        seed.zeroize();
                                        return UREncodeResult::from(e).c_ptr();
                                    }
                                }
                            }
                            Err(e) => {
                                seed.zeroize();
                                return UREncodeResult::from(e).c_ptr();
                            }
                        }
                    }

                    let result = ZcashBatchSigResult::new(
                        ZCASH_BATCH_SIG_RESULT_VERSION,
                        batch.get_request_id().clone(),
                        results,
                    );
                    match TryInto::<Vec<u8>>::try_into(result) {
                        Ok(bytes) => {
                            let registry_type = ZcashBatchSigResult::get_registry_type().get_type();
                            if allow_multipart {
                                UREncodeResult::encode(bytes, registry_type, max_fragment_length)
                            } else {
                                UREncodeResult::encode_full_response(bytes, registry_type)
                            }
                            .c_ptr()
                        }
                        Err(e) => UREncodeResult::from(e).c_ptr(),
                    }
                }
                Err(e) => UREncodeResult::from(e).c_ptr(),
            }
        }
        Err(e) => UREncodeResult::from(e).c_ptr(),
    };
    seed.zeroize();
    result
}

#[cfg(feature = "cypherpunk")]
#[no_mangle]
pub unsafe extern "C" fn sign_zcash_batch_tx_cypherpunk(
    tx: PtrUR,
    ufvk: PtrString,
    seed_fingerprint: PtrBytes,
    account_index: u32,
    disabled: bool,
    seed: PtrBytes,
    seed_len: u32,
) -> *mut UREncodeResult {
    sign_zcash_batch_tx_cypherpunk_dynamic(
        tx,
        ufvk,
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
    tx: PtrUR,
    ufvk: PtrString,
    seed_fingerprint: PtrBytes,
    account_index: u32,
    disabled: bool,
    seed: PtrBytes,
    seed_len: u32,
) -> *mut UREncodeResult {
    sign_zcash_batch_tx_cypherpunk_dynamic(
        tx,
        ufvk,
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
    tx: PtrUR,
    seed: PtrBytes,
    seed_len: u32,
) -> *mut UREncodeResult {
    sign_zcash_tx_dynamic(tx, seed, seed_len, FRAGMENT_MAX_LENGTH_DEFAULT)
}

#[no_mangle]
pub unsafe extern "C" fn sign_zcash_tx_unlimited(
    tx: PtrUR,
    seed: PtrBytes,
    seed_len: u32,
) -> *mut UREncodeResult {
    sign_zcash_tx_dynamic(tx, seed, seed_len, FRAGMENT_UNLIMITED_LENGTH)
}

unsafe fn sign_zcash_tx_dynamic(
    tx: PtrUR,
    seed: PtrBytes,
    seed_len: u32,
    max_fragment_length: usize,
) -> *mut UREncodeResult {
    let pczt = extract_ptr_with_type!(tx, ZcashPczt);
    let mut seed = extract_array_mut!(seed, u8, seed_len as usize);
    let result = match app_zcash::sign_pczt(&pczt.get_data(), seed) {
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
    };
    seed.zeroize();
    result
}

#[cfg(feature = "cypherpunk")]
unsafe fn sign_zcash_tx_cypherpunk_dynamic(
    tx: PtrUR,
    ufvk: PtrString,
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

    let pczt = extract_ptr_with_type!(tx, ZcashPczt);
    let ufvk_text = unsafe { recover_c_char(ufvk) };
    let expected_seed_fingerprint = extract_array!(seed_fingerprint, u8, 32);
    let expected_seed_fingerprint: &[u8; 32] = expected_seed_fingerprint.try_into().unwrap();
    let mut seed = extract_array_mut!(seed, u8, seed_len as usize);
    let pczt_data = pczt.get_data();

    let result = match check_zcash_pczt_message_cypherpunk(
        &pczt_data,
        &ufvk_text,
        expected_seed_fingerprint,
        account_index,
    ) {
        Ok(()) => {
            let seed_fingerprint = calculate_seed_fingerprint(seed);
            match seed_fingerprint {
                Ok(seed_fingerprint) => {
                    if &seed_fingerprint != expected_seed_fingerprint {
                        seed.zeroize();
                        return UREncodeResult::from(RustCError::MasterFingerprintMismatch).c_ptr();
                    }

                    match app_zcash::sign_pczt(&pczt_data, seed) {
                        Ok(signed_pczt) => {
                            if let Err(e) =
                                app_zcash::ensure_owned_supported_shielded_actions_are_signed(
                                    &MainNetwork,
                                    &pczt_data,
                                    &signed_pczt,
                                    &seed_fingerprint,
                                    account_index,
                                )
                            {
                                seed.zeroize();
                                return UREncodeResult::from(e).c_ptr();
                            }

                            match ZcashPczt::new(signed_pczt).try_into() {
                                Err(e) => UREncodeResult::from(e).c_ptr(),
                                Ok(v) => UREncodeResult::encode(
                                    v,
                                    ZcashPczt::get_registry_type().get_type(),
                                    max_fragment_length,
                                )
                                .c_ptr(),
                            }
                        }
                        Err(e) => UREncodeResult::from(e).c_ptr(),
                    }
                }
                Err(e) => UREncodeResult::from(e).c_ptr(),
            }
        }
        Err(e) => UREncodeResult::from(e).c_ptr(),
    };
    seed.zeroize();
    result
}

#[cfg(feature = "cypherpunk")]
#[no_mangle]
pub unsafe extern "C" fn sign_zcash_tx_cypherpunk(
    tx: PtrUR,
    ufvk: PtrString,
    seed_fingerprint: PtrBytes,
    account_index: u32,
    disabled: bool,
    seed: PtrBytes,
    seed_len: u32,
) -> *mut UREncodeResult {
    sign_zcash_tx_cypherpunk_dynamic(
        tx,
        ufvk,
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
    tx: PtrUR,
    ufvk: PtrString,
    seed_fingerprint: PtrBytes,
    account_index: u32,
    disabled: bool,
    seed: PtrBytes,
    seed_len: u32,
) -> *mut UREncodeResult {
    sign_zcash_tx_cypherpunk_dynamic(
        tx,
        ufvk,
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

    fn test_zcash_batch(messages: Vec<ZcashSignMessage>) -> ZcashSignBatch {
        ZcashSignBatch::new(
            ZCASH_SIGN_BATCH_VERSION,
            b"test-request".to_vec(),
            ZCASH_SIGN_BATCH_NETWORK_MAINNET,
            messages,
            Some(true),
        )
    }

    fn test_zcash_message(id: &[u8], payload: &[u8]) -> ZcashSignMessage {
        ZcashSignMessage::new(
            id.to_vec(),
            ZCASH_SIGN_MESSAGE_KIND_PCZT_V1,
            payload.to_vec(),
            Some(sha256(payload).to_vec()),
        )
    }

    #[test]
    fn test_validate_zcash_batch_accepts_valid_envelope() {
        let batch = test_zcash_batch(vec![
            test_zcash_message(b"one", b"pczt-one"),
            test_zcash_message(b"two", b"pczt-two"),
        ]);

        validate_zcash_batch(&batch).unwrap();
    }

    #[test]
    fn test_validate_zcash_batch_fingerprint_binds_reviewed_bytes() {
        let batch = test_zcash_batch(vec![
            test_zcash_message(b"one", b"pczt-one"),
            test_zcash_message(b"two", b"pczt-two"),
        ]);
        let fingerprint = validate_zcash_batch(&batch).unwrap();

        assert_eq!(
            validate_zcash_batch(&batch).unwrap(),
            fingerprint,
            "identical batch bytes must produce the reviewed fingerprint"
        );

        // The signing gate itself: refuse with nothing reviewed, sign only the
        // reviewed batch, follow re-reviews, and disarm on clear. One test
        // exercises the whole lifecycle because the static is shared state.
        let other = validate_zcash_batch(&test_zcash_batch(vec![test_zcash_message(
            b"one",
            b"pczt-other",
        )]))
        .unwrap();
        *REVIEWED_BATCH_FINGERPRINT.lock() = None;
        assert!(
            !confirm_batch_reviewed(&fingerprint),
            "no review armed: refuse"
        );
        *REVIEWED_BATCH_FINGERPRINT.lock() = Some(fingerprint);
        assert!(
            confirm_batch_reviewed(&fingerprint),
            "the reviewed batch signs"
        );
        assert!(
            !confirm_batch_reviewed(&other),
            "a different batch is refused while another is armed"
        );
        *REVIEWED_BATCH_FINGERPRINT.lock() = Some(other);
        assert!(
            !confirm_batch_reviewed(&fingerprint),
            "a new review replaces the armed fingerprint"
        );
        *REVIEWED_BATCH_FINGERPRINT.lock() = None;
        assert!(
            !confirm_batch_reviewed(&other),
            "a cleared review disarms signing"
        );

        let changed_payload = test_zcash_batch(vec![
            test_zcash_message(b"one", b"pczt-one"),
            test_zcash_message(b"two", b"pczt-2wo"),
        ]);
        assert_ne!(
            validate_zcash_batch(&changed_payload).unwrap(),
            fingerprint,
            "a changed payload is a different batch"
        );

        let changed_id = test_zcash_batch(vec![
            test_zcash_message(b"one", b"pczt-one"),
            test_zcash_message(b"2wo", b"pczt-two"),
        ]);
        assert_ne!(
            validate_zcash_batch(&changed_id).unwrap(),
            fingerprint,
            "a changed message id is a different batch"
        );

        let reordered = test_zcash_batch(vec![
            test_zcash_message(b"two", b"pczt-two"),
            test_zcash_message(b"one", b"pczt-one"),
        ]);
        assert_ne!(
            validate_zcash_batch(&reordered).unwrap(),
            fingerprint,
            "reordered messages are a different batch"
        );

        let changed_request_id = ZcashSignBatch::new(
            ZCASH_SIGN_BATCH_VERSION,
            b"other-request".to_vec(),
            ZCASH_SIGN_BATCH_NETWORK_MAINNET,
            vec![
                test_zcash_message(b"one", b"pczt-one"),
                test_zcash_message(b"two", b"pczt-two"),
            ],
            Some(true),
        );
        assert_ne!(
            validate_zcash_batch(&changed_request_id).unwrap(),
            fingerprint,
            "a changed request id is a different batch"
        );
    }

    #[test]
    fn test_validate_zcash_batch_accepts_missing_atomic_as_default() {
        let batch = ZcashSignBatch::new(
            ZCASH_SIGN_BATCH_VERSION,
            b"test-request".to_vec(),
            ZCASH_SIGN_BATCH_NETWORK_MAINNET,
            vec![test_zcash_message(b"one", b"pczt-one")],
            None,
        );

        validate_zcash_batch(&batch).unwrap();
    }

    #[test]
    fn test_validate_zcash_batch_accepts_max_messages() {
        let batch = test_zcash_batch(
            (0..ZCASH_BATCH_MAX_MESSAGES)
                .map(|index| {
                    test_zcash_message(
                        format!("id-{index}").as_bytes(),
                        format!("pczt-{index}").as_bytes(),
                    )
                })
                .collect(),
        );

        validate_zcash_batch(&batch).unwrap();
    }

    #[test]
    fn test_validate_zcash_batch_rejects_version_network_and_atomic_policy() {
        let message = test_zcash_message(b"one", b"pczt-one");

        let wrong_version = ZcashSignBatch::new(
            ZCASH_SIGN_BATCH_VERSION + 1,
            b"test-request".to_vec(),
            ZCASH_SIGN_BATCH_NETWORK_MAINNET,
            vec![message.clone()],
            Some(true),
        );
        assert!(matches!(
            validate_zcash_batch(&wrong_version),
            Err(RustCError::UnsupportedTransaction(message))
                if message.contains("unsupported Zcash batch version")
        ));

        let wrong_network = ZcashSignBatch::new(
            ZCASH_SIGN_BATCH_VERSION,
            b"test-request".to_vec(),
            ZCASH_SIGN_BATCH_NETWORK_MAINNET + 1,
            vec![message.clone()],
            Some(true),
        );
        assert!(matches!(
            validate_zcash_batch(&wrong_network),
            Err(RustCError::UnsupportedTransaction(message))
                if message.contains("only Zcash mainnet")
        ));

        let non_atomic = ZcashSignBatch::new(
            ZCASH_SIGN_BATCH_VERSION,
            b"test-request".to_vec(),
            ZCASH_SIGN_BATCH_NETWORK_MAINNET,
            vec![message],
            Some(false),
        );
        assert!(matches!(
            validate_zcash_batch(&non_atomic),
            Err(RustCError::UnsupportedTransaction(message))
                if message.contains("atomic=true")
        ));
    }

    #[test]
    fn test_validate_zcash_batch_rejects_empty_request_id_and_messages() {
        let empty_request_id = ZcashSignBatch::new(
            ZCASH_SIGN_BATCH_VERSION,
            vec![],
            ZCASH_SIGN_BATCH_NETWORK_MAINNET,
            vec![test_zcash_message(b"one", b"pczt-one")],
            Some(true),
        );
        assert_eq!(
            validate_zcash_batch(&empty_request_id).unwrap_err(),
            RustCError::InvalidData("Zcash batch has no request id".to_string())
        );

        let empty_messages = test_zcash_batch(vec![]);
        assert_eq!(
            validate_zcash_batch(&empty_messages).unwrap_err(),
            RustCError::InvalidData("Zcash batch has no messages".to_string())
        );
    }

    #[test]
    fn test_validate_zcash_batch_rejects_invalid_message_fields() {
        let unsupported_kind = ZcashSignMessage::new(
            b"one".to_vec(),
            ZCASH_SIGN_MESSAGE_KIND_PCZT_V1 + 1,
            b"pczt-one".to_vec(),
            Some(sha256(b"pczt-one").to_vec()),
        );
        assert!(matches!(
            validate_zcash_batch(&test_zcash_batch(vec![unsupported_kind])),
            Err(RustCError::UnsupportedTransaction(message))
                if message.contains("unsupported Zcash batch message kind")
        ));

        let empty_message_id = test_zcash_message(b"", b"pczt-one");
        assert_eq!(
            validate_zcash_batch(&test_zcash_batch(vec![empty_message_id])).unwrap_err(),
            RustCError::InvalidData("Zcash batch message 0 has no id".to_string())
        );

        let empty_payload = test_zcash_message(b"one", b"");
        assert_eq!(
            validate_zcash_batch(&test_zcash_batch(vec![empty_payload])).unwrap_err(),
            RustCError::InvalidData("Zcash batch message 0 has no payload".to_string())
        );
    }

    #[test]
    fn test_validate_zcash_batch_rejects_too_many_messages() {
        let batch = test_zcash_batch(
            (0..=ZCASH_BATCH_MAX_MESSAGES)
                .map(|index| {
                    test_zcash_message(
                        format!("id-{index}").as_bytes(),
                        format!("pczt-{index}").as_bytes(),
                    )
                })
                .collect(),
        );

        assert!(matches!(
            validate_zcash_batch(&batch),
            Err(RustCError::UnsupportedTransaction(message))
                if message.contains("supports at most")
        ));
    }

    #[test]
    fn test_validate_zcash_batch_rejects_duplicate_ids_and_payloads() {
        let duplicate_ids = test_zcash_batch(vec![
            test_zcash_message(b"same", b"pczt-one"),
            test_zcash_message(b"same", b"pczt-two"),
        ]);
        assert_eq!(
            validate_zcash_batch(&duplicate_ids).unwrap_err(),
            RustCError::InvalidData("Zcash batch contains duplicate message ids".to_string())
        );

        let duplicate_payloads = test_zcash_batch(vec![
            test_zcash_message(b"one", b"pczt"),
            test_zcash_message(b"two", b"pczt"),
        ]);
        assert_eq!(
            validate_zcash_batch(&duplicate_payloads).unwrap_err(),
            RustCError::InvalidData("Zcash batch contains duplicate payloads".to_string())
        );
    }

    #[test]
    fn test_validate_zcash_batch_rejects_payload_digest_mismatch() {
        let message = ZcashSignMessage::new(
            b"one".to_vec(),
            ZCASH_SIGN_MESSAGE_KIND_PCZT_V1,
            b"pczt-one".to_vec(),
            Some(sha256(b"different-payload").to_vec()),
        );
        let batch = test_zcash_batch(vec![message]);

        assert_eq!(
            validate_zcash_batch(&batch).unwrap_err(),
            RustCError::InvalidData("Zcash batch message 0 payload digest mismatch".to_string())
        );
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
