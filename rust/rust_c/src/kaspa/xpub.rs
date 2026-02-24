// FFI interface for Kaspa Extended Public Key operations

use alloc::string::ToString;
use alloc::vec::Vec;
use app_kaspa::{get_account_xpub, get_extended_pubkey, get_extended_pubkey_bytes, get_master_fingerprint};
use crate::common::structs::SimpleResponse;
use crate::common::types::{PtrBytes, PtrString};
use crate::common::utils::convert_c_char;
use crate::extract_array;
use crate::utils::recover_c_char;
use core::slice;

/// Get Kaspa extended public key (xpub) from seed
/// 
/// # Safety
/// This function is unsafe because it dereferences raw pointers
/// 
/// # Arguments
/// * `seed` - Pointer to seed bytes
/// * `seed_len` - Length of seed (typically 64)
/// * `path` - C string derivation path (e.g., "m/44'/111111'/0'")
/// 
/// # Returns
/// * Pointer to SimpleResponse containing xpub string (Base58 format)
// kaspa_get_extended_pubkey removed — unused FFI in repository.
// If needed in future, reintroduce with appropriate caller updates.

/// Get Kaspa extended public key bytes (78 bytes encoded format)
/// 
/// # Safety
/// This function is unsafe because it dereferences raw pointers
/// 
/// # Arguments
/// * `seed` - Pointer to seed bytes
/// * `seed_len` - Length of seed
/// * `path` - C string derivation path
/// 
/// # Returns
/// * Pointer to SimpleResponse containing hex-encoded xpub bytes
#[no_mangle]
pub unsafe extern "C" fn kaspa_get_extended_pubkey_bytes(
    seed: PtrBytes,
    seed_len: u32,
    path: PtrString,
) -> *mut SimpleResponse<cty::c_char> {
    let seed = extract_array!(seed, u8, seed_len as usize);
    let path = recover_c_char(path);
    
    match get_extended_pubkey_bytes(seed, &path) {
        Ok(bytes) => SimpleResponse::success(convert_c_char(hex::encode(bytes))).simple_c_ptr(),
        Err(e) => SimpleResponse::from(e).simple_c_ptr(),
    }
}

/// Get Kaspa account extended public key
/// Uses standard path: m/44'/111111'/account_index'
/// 
/// # Safety
/// This function is unsafe because it dereferences raw pointers
/// 
/// # Arguments
/// * `seed` - Pointer to seed bytes
/// * `seed_len` - Length of seed
/// * `account_index` - Account index (usually 0)
/// 
/// # Returns
/// * Pointer to SimpleResponse containing xpub string
#[no_mangle]
pub unsafe extern "C" fn kaspa_get_account_xpub(
    seed: PtrBytes,
    seed_len: u32,
    account_index: u32,
) -> *mut SimpleResponse<cty::c_char> {
    let seed = extract_array!(seed, u8, seed_len as usize);
    
    match get_account_xpub(seed, account_index) {
        Ok(xpub) => SimpleResponse::success(convert_c_char(xpub)).simple_c_ptr(),
        Err(e) => SimpleResponse::from(e).simple_c_ptr(),
    }
}

/// Get Kaspa master fingerprint (4 bytes)
/// Used for PSKT signing to identify the signing key
/// 
/// # Safety
/// This function is unsafe because it dereferences raw pointers
/// 
/// # Arguments
/// * `seed` - Pointer to seed bytes
/// * `seed_len` - Length of seed
/// 
/// # Returns
/// * Pointer to SimpleResponse containing hex-encoded fingerprint (8 hex chars)
#[no_mangle]
pub unsafe extern "C" fn kaspa_get_master_fingerprint(
    seed: PtrBytes,
    seed_len: u32,
) -> *mut SimpleResponse<cty::c_char> {
    let seed = extract_array!(seed, u8, seed_len as usize);
    
    match get_master_fingerprint(seed) {
        Ok(fingerprint) => SimpleResponse::success(convert_c_char(hex::encode(fingerprint))).simple_c_ptr(),
        Err(e) => SimpleResponse::from(e).simple_c_ptr(),
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    
    #[test]
    fn test_kaspa_get_extended_pubkey() {
        let seed = hex::decode(
            "5eb00bbddcf069084889a8ab9155568165f5c453ccb85e70811aaed6f6da5fc19a5ac40b389cd370d086206dec8aa6c43daea6690f20ad3d8d48b2d2ce9e38e4"
        ).unwrap();
        
        let path = "m/44'/111111'/0'";
        let path_cstr = std::ffi::CString::new(path).unwrap();
        
        unsafe {
            let result = kaspa_get_extended_pubkey(
                seed.as_ptr(),
                seed.len() as u32,
                path_cstr.as_ptr(),
            );
            
            assert!(!result.is_null());
            // Cleanup
            let _ = Box::from_raw(result);
        }
    }
}
