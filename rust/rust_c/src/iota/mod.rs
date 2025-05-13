use alloc::{format, slice};
use alloc::{
    string::{String, ToString},
    vec,
};
use alloc::vec::Vec;
use bitcoin::bip32::DerivationPath;
use core::str::FromStr;
use cty::c_char;
use crate::common::structs::SimpleResponse;
use crate::common::types::{PtrString};
use crate::common::utils::{convert_c_char, recover_c_char};
use crate::extract_ptr_with_type;

#[no_mangle]
pub extern "C" fn iota_get_address_from_xpub(
    xpub: PtrString,
) -> *mut SimpleResponse<c_char> {
    let xpub = recover_c_char(xpub);
    match app_iota::address::get_address_from_xpub(xpub) {
        Ok(result) => SimpleResponse::success(convert_c_char(result)).simple_c_ptr(),
        Err(e) => SimpleResponse::from(e).simple_c_ptr(),
    }
}

#[cfg(test)]
mod tests {
    use super::*;
}
