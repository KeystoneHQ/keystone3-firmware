use crate::common::types::{Ptr, PtrBytes, PtrString};
use crate::common::ur::UREncodeResult;
use crate::common::utils::recover_c_char;
use crate::extract_array;
use alloc::format;
use alloc::string::ToString;
use app_wallets::metamask::ETHAccountTypeApp::Bip44Standard;
use cty::uint32_t;
use ur_registry::crypto_hd_key::CryptoHDKey;
use ur_registry::error::URError;
use ur_registry::traits::RegistryItem;

//only support export bip44standard eth account to imToken, rewrite this func if imToken supports other chains
#[no_mangle]
pub unsafe extern "C" fn get_connect_imtoken_ur(
    master_fingerprint: PtrBytes,
    master_fingerprint_length: uint32_t,
    xpub: PtrString,
    wallet_name: PtrString,
) -> Ptr<UREncodeResult> {
    if master_fingerprint_length != 4 {
        return UREncodeResult::from(URError::UrEncodeError(format!(
            "master fingerprint length must be 4, current is {master_fingerprint_length}"
        )))
        .c_ptr();
    }
    let mfp = extract_array!(master_fingerprint, u8, master_fingerprint_length);
    let mfp = match <&[u8; 4]>::try_from(mfp) {
        Ok(mfp) => mfp,
        Err(e) => return UREncodeResult::from(URError::UrEncodeError(e.to_string())).c_ptr(),
    };
    let wallet_name = recover_c_char(wallet_name);
    let result = app_wallets::metamask::generate_standard_legacy_hd_key(
        mfp,
        &recover_c_char(xpub),
        Bip44Standard,
        Some(wallet_name),
    );
    match result.map(|v| v.try_into()) {
        Ok(v) => match v {
            Ok(data) => {
                UREncodeResult::encode(data, CryptoHDKey::get_registry_type().get_type(), 240)
                    .c_ptr()
            }
            Err(e) => UREncodeResult::from(e).c_ptr(),
        },
        Err(e) => UREncodeResult::from(e).c_ptr(),
    }
}
