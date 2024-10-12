use common_rust_c::utils::convert_c_char;
use cty::c_char;
use sim_qr_reader::*;

#[no_mangle]
pub extern "C" fn read_qr_code_from_screen(
    on_qr_detected: extern "C" fn(*const c_char) -> bool,
) {
    let _ = continuously_read_qr_code_from_screen(|qr_code: &str| {
        on_qr_detected(convert_c_char(qr_code.to_string()))
    });
}
