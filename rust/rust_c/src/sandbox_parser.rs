#[no_mangle]
#[cfg_attr(target_os = "none", link_section = ".sandbox_parser_entry")]
pub unsafe extern "C" fn mpu_sandbox_validate_ur(input: *const u8, input_len: usize) -> u32 {
    if input.is_null() {
        return sandbox_parser::ValidationStatus::InvalidInput as u32;
    }

    let input = core::slice::from_raw_parts(input, input_len);
    sandbox_parser::validate_ur(input) as u32
}

#[no_mangle]
#[cfg_attr(target_os = "none", link_section = ".sandbox_parser_entry")]
pub unsafe extern "C" fn mpu_sandbox_validate_cbor(input: *const u8, input_len: usize) -> u32 {
    if input.is_null() {
        return sandbox_parser::ValidationStatus::InvalidInput as u32;
    }

    let input = core::slice::from_raw_parts(input, input_len);
    sandbox_parser::validate_cbor(input) as u32
}

#[no_mangle]
#[cfg_attr(target_os = "none", link_section = ".sandbox_parser_entry")]
pub unsafe extern "C" fn mpu_sandbox_validate_eip712_json(input: *mut u8, input_len: usize) -> u32 {
    if input.is_null() {
        return sandbox_parser::ValidationStatus::InvalidInput as u32;
    }

    let input = core::slice::from_raw_parts_mut(input, input_len);
    sandbox_parser::validate_eip712_json(input) as u32
}
