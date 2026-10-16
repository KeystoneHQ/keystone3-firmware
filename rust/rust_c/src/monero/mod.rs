#![no_std]

extern crate alloc;
use crate::common::errors::RustCError;
use crate::common::structs::{SimpleResponse, TransactionCheckResult, TransactionParseResult};
use crate::common::ur::{UREncodeResult, FRAGMENT_MAX_LENGTH_DEFAULT};
use alloc::boxed::Box;
use alloc::string::ToString;
use app_monero::address::Address;
use app_monero::key::{PrivateKey, PublicKey};
use app_monero::structs::{AddressType, Network};
use app_monero::utils::constants::{OUTPUT_EXPORT_MAGIC, PRVKEY_LEH, UNSIGNED_TX_PREFIX};
// use crate::free::Free;
use crate::common::free::Free;
use crate::common::types::{PtrBytes, PtrString, PtrT, PtrUR};
use crate::common::utils::{convert_c_char, recover_c_char};
use crate::extract_array;
use crate::extract_ptr_with_type;
use core::ffi::c_void;
use cty::c_char;
use rand_core::OsRng;
use structs::{DisplayMoneroOutput, DisplayMoneroUnsignedTx};
use ur_registry::monero::xmr_keyimage::XmrKeyImage;
use ur_registry::monero::xmr_output::XmrOutput;
use ur_registry::monero::xmr_txsigned::XmrTxSigned;
use ur_registry::monero::xmr_txunsigned::XmrTxUnsigned;
use ur_registry::traits::{RegistryItem, To};
use zeroize::{Zeroize, Zeroizing};

mod structs;

#[no_mangle]
pub unsafe extern "C" fn monero_get_address(
    pub_spend_key: PtrString,
    private_view_key: PtrString,
    major: u32,
    minor: u32,
    is_subaddress: bool,
) -> *mut SimpleResponse<c_char> {
    if pub_spend_key.is_null() {
        return SimpleResponse::from(RustCError::InvalidXPub).simple_c_ptr();
    }
    let spend_key = cstr_core::CStr::from_ptr(pub_spend_key).to_bytes();
    let pvk_bytes = match parse_view_key(private_view_key) {
        Ok(key) => key,
        Err(e) => return SimpleResponse::from(e).simple_c_ptr(),
    };
    let pvk = match PrivateKey::try_from_slice(&*pvk_bytes) {
        Ok(key) => key,
        Err(_) => {
            return SimpleResponse::from(RustCError::InvalidData("invalid view key".into()))
                .simple_c_ptr()
        }
    };
    let public_spend = match hex::decode(spend_key)
        .ok()
        .and_then(|bytes| PublicKey::from_bytes(&bytes).ok())
    {
        Some(key) => key,
        _ => {
            return SimpleResponse::from(RustCError::InvalidData("invalid data".to_string()))
                .simple_c_ptr()
        }
    };

    match app_monero::address::generate_address(&public_spend, &pvk, major, minor, is_subaddress) {
        Ok(address) => SimpleResponse::success(convert_c_char(address.to_string()) as *mut c_char)
            .simple_c_ptr(),
        _ => {
            SimpleResponse::from(RustCError::InvalidData("invalid data".to_string())).simple_c_ptr()
        }
    }
}

unsafe fn safe_parse_key(decrypt_key: PtrBytes) -> Result<Zeroizing<[u8; PRVKEY_LEH]>, RustCError> {
    if decrypt_key.is_null() {
        return Err(RustCError::InvalidMasterFingerprint);
    }
    let bytes = extract_array!(decrypt_key, u8, PRVKEY_LEH);
    Ok(Zeroizing::new(
        bytes
            .try_into()
            .map_err(|_| RustCError::InvalidMasterFingerprint)?,
    ))
}

unsafe fn parse_view_key(ptr: PtrString) -> Result<Zeroizing<[u8; PRVKEY_LEH]>, RustCError> {
    if ptr.is_null() {
        return Err(RustCError::InvalidMasterFingerprint);
    }
    let mut key = Zeroizing::new([0; PRVKEY_LEH]);
    let text = cstr_core::CStr::from_ptr(ptr).to_bytes();
    hex::decode_to_slice(text, &mut *key).map_err(|_| RustCError::InvalidMasterFingerprint)?;
    PrivateKey::try_from_slice(&*key).map_err(|_| RustCError::InvalidMasterFingerprint)?;
    Ok(key)
}

#[no_mangle]
pub unsafe extern "C" fn monero_output_request_check(
    ptr: PtrUR,
    decrypt_key: PtrBytes,
    pvk: PtrString,
) -> PtrT<TransactionCheckResult> {
    if ptr.is_null() {
        return TransactionCheckResult::from(RustCError::InvalidData(
            "missing request data".into(),
        ))
        .c_ptr();
    }
    let request = extract_ptr_with_type!(ptr, XmrOutput);
    let decrypt_key = match safe_parse_key(decrypt_key) {
        Ok(decrypt_key) => decrypt_key,
        _ => return TransactionCheckResult::from(RustCError::InvalidMasterFingerprint).c_ptr(),
    };
    let pvk = match parse_view_key(pvk) {
        Ok(pvk) => pvk,
        _ => return TransactionCheckResult::from(RustCError::InvalidMasterFingerprint).c_ptr(),
    };

    let payload = request.get_payload();

    match app_monero::utils::decrypt_data_with_decrypt_key(
        *decrypt_key,
        *pvk,
        payload,
        OUTPUT_EXPORT_MAGIC,
    ) {
        Ok(_) => TransactionCheckResult::new().c_ptr(),
        _ => TransactionCheckResult::from(RustCError::InvalidMasterFingerprint).c_ptr(),
    }
}

#[no_mangle]
pub unsafe extern "C" fn monero_unsigned_request_check(
    ptr: PtrUR,
    decrypt_key: PtrBytes,
    pvk: PtrString,
    pub_spend_key: PtrString,
) -> PtrT<TransactionCheckResult> {
    clear_approved_unsigned_tx();
    if ptr.is_null() {
        return TransactionCheckResult::from(RustCError::InvalidData(
            "missing request data".into(),
        ))
        .c_ptr();
    }
    let request = extract_ptr_with_type!(ptr, XmrTxUnsigned);
    let decrypt_key = match safe_parse_key(decrypt_key) {
        Ok(key) => key,
        Err(_) => {
            return TransactionCheckResult::from(RustCError::InvalidMasterFingerprint).c_ptr()
        }
    };
    if pvk.is_null() || pub_spend_key.is_null() {
        return TransactionCheckResult::from(RustCError::InvalidData(
            "missing wallet keys".to_string(),
        ))
        .c_ptr();
    }
    let pvk = match parse_view_key(pvk) {
        Ok(key) => key,
        Err(_) => {
            return TransactionCheckResult::from(RustCError::InvalidMasterFingerprint).c_ptr()
        }
    };
    let public_spend: [u8; 32] = match hex::decode(recover_c_char(pub_spend_key))
        .ok()
        .and_then(|v| v.try_into().ok())
    {
        Some(key) => key,
        None => return TransactionCheckResult::from(RustCError::InvalidXPub).c_ptr(),
    };
    match app_monero::transfer::check_unsigned(
        request.get_payload(),
        *decrypt_key,
        *pvk,
        public_spend,
    ) {
        Ok(_) => TransactionCheckResult::new().c_ptr(),
        Err(app_monero::errors::MoneroError::MismatchedMfp) => {
            TransactionCheckResult::from(RustCError::InvalidMasterFingerprint).c_ptr()
        }
        Err(e) => TransactionCheckResult::from(RustCError::InvalidData(e.to_string())).c_ptr(),
    }
}

#[no_mangle]
pub unsafe extern "C" fn get_monero_pvk_by_seed(
    seed: PtrBytes,
    seed_len: u32,
) -> *mut SimpleResponse<c_char> {
    let seed = extract_array!(seed, u8, seed_len as usize);

    match app_monero::key::generate_keypair(seed, 0) {
        Ok(keypair) => {
            let pvk = keypair.view.to_bytes();
            SimpleResponse::success(convert_c_char(hex::encode(pvk)) as *mut c_char).simple_c_ptr()
        }
        _ => {
            SimpleResponse::from(RustCError::InvalidData("invalid seed".to_string())).simple_c_ptr()
        }
    }
}

#[no_mangle]
pub unsafe extern "C" fn monero_parse_output(
    ptr: PtrUR,
    decrypt_key: PtrBytes,
    pvk: PtrString,
) -> PtrT<TransactionParseResult<DisplayMoneroOutput>> {
    if ptr.is_null() {
        return TransactionParseResult::from(RustCError::InvalidData(
            "missing request data".into(),
        ))
        .c_ptr();
    }
    let request = extract_ptr_with_type!(ptr, XmrOutput);
    let decrypt_key = match safe_parse_key(decrypt_key) {
        Ok(decrypt_key) => decrypt_key,
        _ => return TransactionParseResult::from(RustCError::InvalidMasterFingerprint).c_ptr(),
    };
    let pvk = match parse_view_key(pvk) {
        Ok(key) => key,
        Err(e) => return TransactionParseResult::from(e).c_ptr(),
    };

    match app_monero::outputs::parse_display_info(&request.get_payload(), *decrypt_key, *pvk) {
        Ok(display) => TransactionParseResult::success(Box::into_raw(Box::new(
            DisplayMoneroOutput::from(display),
        )))
        .c_ptr(),
        Err(error) => {
            TransactionParseResult::from(RustCError::InvalidData(error.to_string())).c_ptr()
        }
    }
}

struct ApprovalState {
    locked: core::sync::atomic::AtomicBool,
    digest: core::cell::UnsafeCell<(u64, Option<[u8; 32]>)>,
}
unsafe impl Sync for ApprovalState {}
static APPROVED_UNSIGNED_TX: ApprovalState = ApprovalState {
    locked: core::sync::atomic::AtomicBool::new(false),
    digest: core::cell::UnsafeCell::new((0, None)),
};

fn with_approval_state<T>(f: impl FnOnce(&mut (u64, Option<[u8; 32]>)) -> T) -> T {
    use core::sync::atomic::Ordering;
    #[cfg(target_arch = "arm")]
    let interrupt_mask: u32;
    #[cfg(target_arch = "arm")]
    unsafe {
        core::arch::asm!("mrs {}, PRIMASK", "cpsid i", out(reg) interrupt_mask);
    }
    while APPROVED_UNSIGNED_TX
        .locked
        .compare_exchange_weak(false, true, Ordering::Acquire, Ordering::Relaxed)
        .is_err()
    {
        core::hint::spin_loop();
    }
    struct Guard {
        #[cfg(target_arch = "arm")]
        interrupt_mask: u32,
    }
    impl Drop for Guard {
        fn drop(&mut self) {
            APPROVED_UNSIGNED_TX.locked.store(false, Ordering::Release);
            #[cfg(target_arch = "arm")]
            unsafe {
                core::arch::asm!("msr PRIMASK, {}", in(reg) self.interrupt_mask);
            }
        }
    }
    let _guard = Guard {
        #[cfg(target_arch = "arm")]
        interrupt_mask,
    };
    f(unsafe { &mut *APPROVED_UNSIGNED_TX.digest.get() })
}

fn remember_approved_unsigned_tx(payload: &[u8], generation: u64) -> bool {
    let digest = app_monero::transfer::payload_digest(payload);
    with_approval_state(|state| {
        if state.0 != generation {
            return false;
        }
        state.1 = Some(digest);
        true
    })
}

#[cfg(test)]
fn approved_unsigned_tx_matches(payload: &[u8]) -> bool {
    let digest = app_monero::transfer::payload_digest(payload);
    with_approval_state(|state| state.1 == Some(digest))
}

fn take_approved_unsigned_tx(payload: &[u8]) -> bool {
    let digest = app_monero::transfer::payload_digest(payload);
    with_approval_state(|state| {
        state.0 = state.0.wrapping_add(1);
        state.1.take() == Some(digest)
    })
}

fn clear_approved_unsigned_tx() -> u64 {
    with_approval_state(|state| {
        state.0 = state.0.wrapping_add(1);
        state.1 = None;
        state.0
    })
}

#[no_mangle]
pub extern "C" fn monero_clear_transaction_state() {
    clear_approved_unsigned_tx();
}

#[no_mangle]
pub unsafe extern "C" fn monero_parse_unsigned_tx(
    ptr: PtrUR,
    decrypt_key: PtrBytes,
    pvk: PtrString,
    pub_spend_key: PtrString,
) -> PtrT<TransactionParseResult<DisplayMoneroUnsignedTx>> {
    let generation = clear_approved_unsigned_tx();
    if pvk.is_null() || pub_spend_key.is_null() {
        return TransactionParseResult::from(RustCError::InvalidData("missing wallet keys".into()))
            .c_ptr();
    }
    if ptr.is_null() {
        return TransactionParseResult::from(RustCError::InvalidData(
            "missing request data".into(),
        ))
        .c_ptr();
    }
    let request = extract_ptr_with_type!(ptr, XmrTxUnsigned);
    let decrypt_key = match safe_parse_key(decrypt_key) {
        Ok(decrypt_key) => decrypt_key,
        _ => return TransactionParseResult::from(RustCError::InvalidMasterFingerprint).c_ptr(),
    };
    let pvk = match parse_view_key(pvk) {
        Ok(pvk) => pvk,
        _ => return TransactionParseResult::from(RustCError::InvalidMasterFingerprint).c_ptr(),
    };
    let public_spend = match hex::decode(recover_c_char(pub_spend_key)) {
        Ok(key) if key.len() == PRVKEY_LEH => key,
        _ => return TransactionParseResult::from(RustCError::InvalidXPub).c_ptr(),
    };

    let payload = request.get_payload();

    match app_monero::transfer::parse_unsigned(
        payload.clone(),
        *decrypt_key,
        *pvk,
        public_spend.try_into().unwrap(),
    ) {
        Ok(display) => {
            if !remember_approved_unsigned_tx(&payload, generation) {
                return TransactionParseResult::from(RustCError::InvalidData(
                    "transaction session ended".into(),
                ))
                .c_ptr();
            }
            TransactionParseResult::success(Box::into_raw(Box::new(DisplayMoneroUnsignedTx::from(
                display,
            ))))
            .c_ptr()
        }
        Err(error) => {
            clear_approved_unsigned_tx();
            TransactionParseResult::from(RustCError::InvalidData(error.to_string())).c_ptr()
        }
    }
}

#[no_mangle]
pub unsafe extern "C" fn monero_generate_keyimage(
    ptr: PtrUR,
    seed: PtrBytes,
    seed_len: u32,
    major: u32,
) -> PtrT<UREncodeResult> {
    if ptr.is_null() || seed.is_null() || seed_len == 0 {
        return UREncodeResult::from(RustCError::InvalidData("missing request data".into()))
            .c_ptr();
    }
    let request = extract_ptr_with_type!(ptr, XmrOutput);
    let seed = extract_array!(seed, u8, seed_len as usize);
    let keypair = match app_monero::key::generate_keypair(seed, major) {
        Ok(keypair) => keypair,
        _ => {
            return UREncodeResult::from(RustCError::InvalidData("invalid seed".to_string()))
                .c_ptr()
        }
    };
    match app_monero::key_images::generate_export_ur_data(keypair, request.get_payload(), OsRng) {
        Ok(data) => {
            let data = XmrKeyImage::new(data);

            UREncodeResult::encode(
                data.try_into().unwrap(),
                XmrKeyImage::get_registry_type().get_type(),
                FRAGMENT_MAX_LENGTH_DEFAULT,
            )
            .c_ptr()
        }
        Err(_) => UREncodeResult::from(RustCError::InvalidData("invalid data".to_string())).c_ptr(),
    }
}

#[no_mangle]
pub unsafe extern "C" fn monero_generate_signature(
    ptr: PtrUR,
    seed: PtrBytes,
    seed_len: u32,
    major: u32,
) -> PtrT<UREncodeResult> {
    if ptr.is_null() || seed.is_null() || seed_len == 0 {
        clear_approved_unsigned_tx();
        return UREncodeResult::from(RustCError::InvalidData("missing request data".into()))
            .c_ptr();
    }
    let request = extract_ptr_with_type!(ptr, XmrTxUnsigned);
    let seed = extract_array!(seed, u8, seed_len as usize);
    let payload = request.get_payload();
    let matches = take_approved_unsigned_tx(&payload);
    if !matches {
        return UREncodeResult::from(RustCError::InvalidData(
            "unapproved transaction".to_string(),
        ))
        .c_ptr();
    }

    let keypair = match app_monero::key::generate_keypair(seed, major) {
        Ok(keypair) => keypair,
        _ => {
            return UREncodeResult::from(RustCError::InvalidData("invalid seed".to_string()))
                .c_ptr()
        }
    };
    let result = app_monero::transfer::sign_tx(keypair, payload);
    clear_approved_unsigned_tx();

    match result {
        Ok(data) => {
            let data = XmrTxSigned::new(data);

            UREncodeResult::encode(
                data.try_into().unwrap(),
                XmrTxSigned::get_registry_type().get_type(),
                FRAGMENT_MAX_LENGTH_DEFAULT,
            )
            .c_ptr()
        }
        Err(_) => UREncodeResult::from(RustCError::InvalidData("invalid data".to_string())).c_ptr(),
    }
}

#[no_mangle]
pub unsafe extern "C" fn monero_generate_decrypt_key(pvk: PtrString) -> *mut SimpleResponse<u8> {
    if pvk.is_null() {
        return SimpleResponse::from(RustCError::InvalidMasterFingerprint).simple_c_ptr();
    }
    let pvk = match parse_view_key(pvk) {
        Ok(pvk) => pvk,
        _ => return SimpleResponse::from(RustCError::InvalidMasterFingerprint).simple_c_ptr(),
    };

    let key = app_monero::utils::generate_decrypt_key(*pvk);

    SimpleResponse::success(Box::into_raw(Box::new(key)) as *mut u8).simple_c_ptr()
}

#[no_mangle]
pub unsafe extern "C" fn monero_free_decrypt_key_response(response: *mut c_void) {
    if response.is_null() {
        return;
    }

    let response = response as *mut SimpleResponse<u8>;
    let data = (*response).data;
    if !data.is_null() {
        let mut key = Box::from_raw(data as *mut [u8; PRVKEY_LEH]);
        key.zeroize();
    }
    (*response).data = core::ptr::null_mut();
    crate::common::free::free_simple_response_u8(response);
}

#[no_mangle]
pub unsafe extern "C" fn get_extended_monero_pubkeys_by_seed(
    seed: PtrBytes,
    seed_len: u32,
    path: PtrString,
) -> *mut SimpleResponse<c_char> {
    let path = recover_c_char(path);
    let major = path
        .split('/')
        .nth(3)
        .unwrap()
        .replace("'", "")
        .parse::<u32>()
        .unwrap();
    let seed = extract_array!(seed, u8, seed_len as usize);
    let keypair = app_monero::key::generate_keypair(seed, major).unwrap();
    let public_spend_key = keypair.spend.get_public_key();
    let result = hex::encode(public_spend_key.as_bytes());

    SimpleResponse::success(convert_c_char(result.to_string())).simple_c_ptr()
}

#[cfg(test)]
mod request_boundary_tests {
    use super::*;

    #[test]
    fn receive_address_ffi_key_boundaries() {
        unsafe {
            let view = cstr_core::CString::new(alloc::format!("01{}", "00".repeat(31))).unwrap();
            for bytes in [
                alloc::vec![b'f'; 64],
                alloc::vec![b'0'; 64],
                alloc::format!("01{}", "00".repeat(31)).into_bytes(),
                alloc::vec![0xff; 64],
            ] {
                let spend = cstr_core::CString::new(bytes).unwrap();
                for subaddress in [false, true] {
                    let result = monero_get_address(
                        spend.as_ptr() as PtrString,
                        view.as_ptr() as PtrString,
                        0,
                        1,
                        subaddress,
                    );
                    assert!(!result.is_null());
                    assert!((*result).data.is_null());
                    crate::common::free::free_simple_response_c_char(result);
                }
            }
            let spend = cstr_core::CString::new(alloc::format!("58{}", "66".repeat(31))).unwrap();
            for subaddress in [false, true] {
                let result = monero_get_address(
                    spend.as_ptr() as PtrString,
                    view.as_ptr() as PtrString,
                    0,
                    1,
                    subaddress,
                );
                assert!(!(*result).data.is_null());
                crate::common::free::free_simple_response_c_char(result);
            }
        }
    }

    #[test]
    fn view_key_text_boundaries() {
        unsafe {
            let invalid = cstr_core::CString::new(alloc::vec![0xff; 64]).unwrap();
            assert!(parse_view_key(invalid.as_ptr() as PtrString).is_err());
            let zero = cstr_core::CString::new("00".repeat(32)).unwrap();
            assert!(parse_view_key(zero.as_ptr() as PtrString).is_err());
            let valid = cstr_core::CString::new(alloc::format!("01{}", "00".repeat(31))).unwrap();
            let key = parse_view_key(valid.as_ptr() as PtrString).unwrap();
            assert_eq!(key[0], 1);
        }
    }

    #[test]
    fn missing_keys_clear_parse_state() {
        unsafe {
            let generation = clear_approved_unsigned_tx();
            assert!(remember_approved_unsigned_tx(b"request", generation));
            assert!(!take_approved_unsigned_tx(b"other"));
            assert!(!approved_unsigned_tx_matches(b"request"));
            let generation = clear_approved_unsigned_tx();
            monero_clear_transaction_state();
            assert!(!remember_approved_unsigned_tx(b"request", generation));
            let generation = clear_approved_unsigned_tx();
            assert!(remember_approved_unsigned_tx(b"request", generation));
            assert!(take_approved_unsigned_tx(b"request"));
            assert!(!take_approved_unsigned_tx(b"request"));
            let generation = clear_approved_unsigned_tx();
            assert!(remember_approved_unsigned_tx(b"request", generation));
            let result = monero_parse_unsigned_tx(
                core::ptr::null_mut(),
                core::ptr::null_mut(),
                core::ptr::null_mut(),
                core::ptr::null_mut(),
            );
            assert!(!approved_unsigned_tx_matches(b"request"));
            (*result).free();
            drop(Box::from_raw(result));
            for result in [
                monero_output_request_check(
                    core::ptr::null_mut(),
                    core::ptr::null_mut(),
                    core::ptr::null_mut(),
                ),
                monero_unsigned_request_check(
                    core::ptr::null_mut(),
                    core::ptr::null_mut(),
                    core::ptr::null_mut(),
                    core::ptr::null_mut(),
                ),
            ] {
                assert!(!result.is_null());
                (*result).free();
                drop(Box::from_raw(result));
            }
            let result = monero_parse_output(
                core::ptr::null_mut(),
                core::ptr::null_mut(),
                core::ptr::null_mut(),
            );
            (*result).free();
            drop(Box::from_raw(result));
            for result in [
                monero_generate_keyimage(core::ptr::null_mut(), core::ptr::null_mut(), 0, 0),
                monero_generate_signature(core::ptr::null_mut(), core::ptr::null_mut(), 0, 0),
            ] {
                assert!(!result.is_null());
                (*result).free();
                drop(Box::from_raw(result));
            }
            let result = monero_generate_decrypt_key(core::ptr::null_mut());
            assert!(!result.is_null());
            monero_free_decrypt_key_response(result.cast());
        }
    }
}
