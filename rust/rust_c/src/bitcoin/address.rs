use app_bitcoin;
use app_bitcoin::addresses::xyzpub;
use bitcoin::secp256k1::ffi::types::c_char;
use common_rust_c::structs::SimpleResponse;
use common_rust_c::types::PtrString;
use common_rust_c::utils::{convert_c_char, recover_c_char};
use core::str::FromStr;

#[no_mangle]
pub extern "C" fn utxo_get_address(
    hd_path: PtrString,
    x_pub: PtrString,
) -> *mut SimpleResponse<c_char> {
    let x_pub = recover_c_char(x_pub);
    let hd_path = recover_c_char(hd_path);
    let address = app_bitcoin::get_address(hd_path, &x_pub);
    match address {
        Ok(result) => SimpleResponse::success(convert_c_char(result) as *mut c_char).simple_c_ptr(),
        Err(e) => SimpleResponse::from(e).simple_c_ptr(),
    }
}

#[no_mangle]
pub extern "C" fn xpub_convert_version(
    x_pub: PtrString,
    target: PtrString,
) -> *mut SimpleResponse<c_char> {
    let x_pub = recover_c_char(x_pub);
    let target = recover_c_char(target);
    let ver = xyzpub::Version::from_str(target.as_str());
    match ver {
        Ok(v) => match xyzpub::convert_version(x_pub, &v) {
            Ok(result) => {
                SimpleResponse::success(convert_c_char(result) as *mut c_char).simple_c_ptr()
            }
            Err(e) => SimpleResponse::from(e).simple_c_ptr(),
        },
        Err(e) => SimpleResponse::from(e).simple_c_ptr(),
    }
}
