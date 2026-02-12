use crate::common::structs::SimpleResponse;
use crate::common::types::PtrString;
use crate::common::utils::{convert_c_char, recover_c_char};
use alloc::boxed::Box;
use bitcoin::secp256k1::ffi::types::c_char;

/// Get Kaspa address for given xpub and derivation path
/// Path format: m/44'/111111'/0'/0/x
#[no_mangle]
pub unsafe extern "C" fn kaspa_get_address(
    xpub: PtrString,
    path: PtrString,
) -> *mut SimpleResponse<c_char> {
    let xpub = recover_c_char(xpub);
    let path = recover_c_char(path);
    
    match app_kaspa::get_address(&xpub, &path) {
        Ok(address) => SimpleResponse::success(convert_c_char(address) as *mut c_char).simple_c_ptr(),
        Err(e) => SimpleResponse::from(e).simple_c_ptr(),
    }
}

/// Generate Kaspa derivation path
/// Returns path in format: m/44'/111111'/account'/change/index
#[no_mangle]
pub unsafe extern "C" fn kaspa_get_derivation_path(
    account: u32,
    change: u32,
    index: u32,
) -> *mut SimpleResponse<c_char> {
    let path = app_kaspa::addresses::get_kaspa_derivation_path(account, change, index);
    SimpleResponse::success(convert_c_char(path) as *mut c_char).simple_c_ptr()
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_kaspa_derivation_path() {
        unsafe {
            let result = kaspa_get_derivation_path(0, 0, 0);
            assert!(!result.is_null());
            // Clean up
            let _ = Box::from_raw(result);
        }
    }
}
