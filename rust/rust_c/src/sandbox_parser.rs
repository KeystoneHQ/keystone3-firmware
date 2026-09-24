use sandbox_parser::{CborStreamValidator, ValidationStatus};

#[no_mangle]
#[cfg_attr(target_os = "none", link_section = ".sandbox_parser_entry")]
pub unsafe extern "C" fn mpu_sandbox_validate_ur(input: *const u8, input_len: usize) -> u32 {
    if input.is_null() {
        return ValidationStatus::InvalidInput as u32;
    }

    let input = core::slice::from_raw_parts(input, input_len);
    sandbox_parser::validate_ur(input) as u32
}

#[no_mangle]
#[cfg_attr(target_os = "none", link_section = ".sandbox_parser_entry")]
pub unsafe extern "C" fn mpu_sandbox_validate_cbor(input: *const u8, input_len: usize) -> u32 {
    if input.is_null() {
        return ValidationStatus::InvalidInput as u32;
    }

    let input = core::slice::from_raw_parts(input, input_len);
    sandbox_parser::validate_cbor(input) as u32
}

#[no_mangle]
#[cfg_attr(target_os = "none", link_section = ".sandbox_parser_entry")]
pub unsafe extern "C" fn mpu_sandbox_validate_eip712_json(input: *mut u8, input_len: usize) -> u32 {
    if input.is_null() {
        return ValidationStatus::InvalidInput as u32;
    }

    let input = core::slice::from_raw_parts_mut(input, input_len);
    sandbox_parser::validate_eip712_json(input) as u32
}

// Streaming CBOR validation. The sandbox runtime owns `state` (a
// MPU_SANDBOX_CBOR_STREAM_STATE_SIZE-byte, 8-byte-aligned buffer in the
// sandbox RW region) and feeds the reassembled UR chunk by chunk, so that no
// copy of the payload is ever held inside the sandbox. `push` never reports a
// verdict: failures are sticky and surface from `finish`, so that the chunk
// protocol keeps returning NOT_CHECKED until the final chunk.
//
// `init` is the only entry point that accepts uninitialized memory; `push`
// and `finish` require the state that `init` produced. The runtime upholds
// this by calling `init` on a payload's first chunk and `push`/`finish` only
// while that state is active (mpu_sandbox_runtime.c,
// MpuSandboxProcessCborStreamChunk).

/// Initializes validator state in `capacity` bytes at `state`. Returns
/// MPU_SANDBOX_UR_OK, or MPU_SANDBOX_UR_INVALID_INPUT if the memory is null,
/// smaller than the validator, or not 8-byte aligned.
///
/// # Safety
/// `state` must be valid for writes of `capacity` bytes.
#[no_mangle]
#[cfg_attr(target_os = "none", link_section = ".sandbox_parser_entry")]
pub unsafe extern "C" fn mpu_sandbox_cbor_stream_init(state: *mut u8, capacity: usize) -> u32 {
    if CborStreamValidator::init_in_raw(state, capacity) {
        ValidationStatus::Ok as u32
    } else {
        ValidationStatus::InvalidInput as u32
    }
}

/// Feeds `input_len` bytes of the reassembled UR into the validator. Returns
/// MPU_SANDBOX_UR_OK when the bytes were accepted (a verdict comes only from
/// `finish`), or MPU_SANDBOX_UR_INVALID_INPUT for a null pointer or a state
/// buffer that does not fit a validator.
///
/// # Safety
/// `state` and `capacity` must describe memory that
/// `mpu_sandbox_cbor_stream_init` initialized with the same `capacity` and
/// that has since been modified only by these `mpu_sandbox_cbor_stream_*`
/// functions; it must be valid for reads and writes of `capacity` bytes and
/// not accessed otherwise during the call. `input` must be valid for reads of
/// `input_len` bytes.
#[no_mangle]
#[cfg_attr(target_os = "none", link_section = ".sandbox_parser_entry")]
pub unsafe extern "C" fn mpu_sandbox_cbor_stream_push(
    state: *mut u8,
    capacity: usize,
    input: *const u8,
    input_len: usize,
) -> u32 {
    let Some(validator) = CborStreamValidator::from_raw(state, capacity) else {
        return ValidationStatus::InvalidInput as u32;
    };
    if input.is_null() {
        return ValidationStatus::InvalidInput as u32;
    }
    validator.push(core::slice::from_raw_parts(input, input_len));
    ValidationStatus::Ok as u32
}

/// Ends the payload and returns its validation status (an
/// MpuSandboxUrValidationStatus_t value), or MPU_SANDBOX_UR_INVALID_INPUT for
/// a null pointer or a state buffer that does not fit a validator.
///
/// # Safety
/// As for `mpu_sandbox_cbor_stream_push`: `state` and `capacity` must
/// describe memory that `mpu_sandbox_cbor_stream_init` initialized with the
/// same `capacity` and that has since been modified only by these
/// `mpu_sandbox_cbor_stream_*` functions, valid for reads and writes of
/// `capacity` bytes and not accessed otherwise during the call.
#[no_mangle]
#[cfg_attr(target_os = "none", link_section = ".sandbox_parser_entry")]
pub unsafe extern "C" fn mpu_sandbox_cbor_stream_finish(state: *mut u8, capacity: usize) -> u32 {
    match CborStreamValidator::from_raw(state, capacity) {
        Some(validator) => validator.finish() as u32,
        None => ValidationStatus::InvalidInput as u32,
    }
}
