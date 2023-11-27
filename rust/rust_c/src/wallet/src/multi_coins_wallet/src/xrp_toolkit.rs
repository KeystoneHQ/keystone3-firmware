use app_wallets::xrp_toolkit::generate_sync_ur;
use common_rust_c::types::PtrString;
use common_rust_c::ur::{UREncodeResult, FRAGMENT_MAX_LENGTH_DEFAULT};
use common_rust_c::utils::recover_c_char;
use third_party::ur_registry::bytes::Bytes;
use third_party::ur_registry::traits::RegistryItem;

#[no_mangle]
pub extern "C" fn get_connect_xrp_toolkit_ur(
    hd_path: PtrString,
    root_x_pub: PtrString,
    root_path: PtrString,
) -> *mut UREncodeResult {
    let hd_path = recover_c_char(hd_path);
    let root_x_pub = recover_c_char(root_x_pub);
    let root_path = recover_c_char(root_path);
    let result = generate_sync_ur(hd_path.as_str(), root_x_pub.as_str(), root_path.as_str());
    match result.map(|v| v.try_into()) {
        Ok(v) => match v {
            Ok(data) => UREncodeResult::encode(
                data,
                Bytes::get_registry_type().get_type(),
                FRAGMENT_MAX_LENGTH_DEFAULT.clone(),
            )
            .c_ptr(),
            Err(e) => UREncodeResult::from(e).c_ptr(),
        },
        Err(e) => UREncodeResult::from(e).c_ptr(),
    }
}
