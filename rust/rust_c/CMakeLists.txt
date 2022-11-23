set(LIB_NAME librust_c.a)
set(RUST_C_DIR ${RUST_DIR}/rust_c)
set(CBINDGEN_CONFIG_PATH ${RUST_C_DIR}/cbindgen.toml)
set(CARGO_TOML_PATH ${RUST_C_DIR}/Cargo.toml)
set(CARGO_FALG --manifest-path ${CARGO_TOML_PATH} --release --target ${CARGO_ARM_TARGET})
set(CBINDGEN_FLAG ${RUST_C_DIR} --config ${CBINDGEN_CONFIG_PATH} --output ${TARGET_PATH}/librust_c.h --lang c)

message(STATUS ${TARGET_PATH})

set(COPY_COMMAND  cp ${RUST_C_DIR}/target/${CARGO_ARM_TARGET}/release/${LIB_NAME} ${TARGET_PATH})

add_custom_target(rust_c ALL
    COMMAND rustup run nightly-2023-06-26 ${RUST_CARGO_EXECUTABLE} build ${CARGO_FALG}
    COMMAND ${CBINDGEN_EXE} ${CBINDGEN_FLAG}
    COMMAND ${COPY_COMMAND}
)

