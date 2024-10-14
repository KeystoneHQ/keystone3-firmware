include(rust_c.cmake)

if(NOT BUILD_TYPE STREQUAL "Simulator")
    if(BTC_ONLY)
        set(CARGO_FLAG --manifest-path ${CARGO_TOML_PATH} --release --target ${CARGO_ARM_TARGET} --no-default-features --features btc-only)
    else()
        set(CARGO_FLAG --manifest-path ${CARGO_TOML_PATH} --release --target ${CARGO_ARM_TARGET})
    endif()


    if(DEFINED DEBUG_MEMORY)
        message(STATUS "build with DEBUG_MEMORY")
        set(CARGO_FLAG ${CARGO_FLAG} --features debug-memory)
    endif()

    message(STATUS "cargo build command: " ${CARGO_FLAG})
    message(STATUS ${TARGET_PATH})

    set(COPY_COMMAND  cp ${RUST_C_DIR}/target/${CARGO_ARM_TARGET}/release/${LIB_NAME} ${TARGET_PATH})

    add_custom_target(rust_c ALL
        COMMAND ${CBINDGEN_EXE} ${CBINDGEN_FLAG}
        COMMAND rustup run nightly-2023-12-01 ${RUST_CARGO_EXECUTABLE} build ${CARGO_FLAG}
        COMMAND ${COPY_COMMAND}
    )
else()
    if(BTC_ONLY)
        set(CARGO_FLAG --manifest-path ${CARGO_TOML_PATH} --release --no-default-features --features simulator_btc_only)
    else()
        set(CARGO_FLAG --manifest-path ${CARGO_TOML_PATH} --release --no-default-features --features simulator)
    endif()

    if(DEFINED DEBUG_MEMORY)
        message(STATUS "build with DEBUG_MEMORY")
        set(CARGO_FLAG ${CARGO_FLAG} --features debug-memory)
    endif()

    message(STATUS "Cargo Flags: " ${CARGO_FLAG})

    set(COPY_COMMAND  cp ${RUST_C_DIR}/target/release/${LIB_NAME} ${TARGET_PATH})

    add_custom_target(rust_c ALL
        COMMAND ${CBINDGEN_EXE} ${CBINDGEN_FLAG}
        COMMAND rustup run nightly-2023-12-01 ${RUST_CARGO_EXECUTABLE} build ${CARGO_FLAG}
        COMMAND ${COPY_COMMAND}
    )
endif()