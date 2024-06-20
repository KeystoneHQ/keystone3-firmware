use alloc::string::ToString;
use app_wallets::tonkeeper::{generate_sync_ur, PathInfo};
use common_rust_c::{
    extract_array,
    types::{Ptr, PtrBytes, PtrString},
    ur::{UREncodeResult, FRAGMENT_MAX_LENGTH_DEFAULT},
    utils::recover_c_char,
};
use third_party::{
    hex,
    ur_registry::{crypto_hd_key::CryptoHDKey, error::URError, traits::RegistryItem},
};

#[no_mangle]
pub extern "C" fn get_tonkeeper_wallet_ur(
    public_key: PtrString,
    wallet_name: PtrString,

    master_fingerprint: PtrBytes,
    master_fingerprint_length: u32,
    path: PtrString,
) -> Ptr<UREncodeResult> {
    let path_info = match master_fingerprint_length {
        4 => {
            let mfp = extract_array!(master_fingerprint, u8, master_fingerprint_length);
            let mfp = match <[u8; 4]>::try_from(mfp) {
                Ok(mfp) => mfp,
                Err(e) => {
                    return UREncodeResult::from(URError::UrEncodeError(e.to_string())).c_ptr()
                }
            };
            let path = recover_c_char(path);
            Some(PathInfo {
                mfp,
                path: path.to_string(),
            })
        }
        _ => None,
    };
    let pubkey = recover_c_char(public_key);
    let pubkey = match hex::decode(pubkey) {
        Ok(pubkey) => pubkey,
        Err(_e) => return UREncodeResult::from(URError::UrEncodeError(_e.to_string())).c_ptr(),
    };
    let wallet_name = recover_c_char(wallet_name);
    match generate_sync_ur(pubkey, wallet_name, path_info) {
        Ok(data) => match data.try_into() {
            Ok(_v) => UREncodeResult::encode(
                _v,
                CryptoHDKey::get_registry_type().get_type(),
                FRAGMENT_MAX_LENGTH_DEFAULT.clone(),
            )
            .c_ptr(),
            Err(_e) => UREncodeResult::from(_e).c_ptr(),
        },
        Err(_e) => UREncodeResult::from(_e).c_ptr(),
    }
}
