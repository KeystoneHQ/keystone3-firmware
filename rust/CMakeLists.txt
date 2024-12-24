message(STATUS "compile rust")
set(LIB_NAME librust_c.a)

# set variables
find_program(RUST_CARGO_EXECUTABLE cargo)
find_program(BINDGEN_EXE bindgen)
find_program(CBINDGEN_EXE cbindgen)

# BUILD_ENVIRONMENT: PRODUCTION, DEBUG, SIMULATOR
# BUILD_VARIANT: BTC_ONLY, MULTI_COINS

set(RUST_DIR ${CMAKE_SOURCE_DIR}/rust)
set(RUST_C_DIR ${RUST_DIR}/rust_c)

if(BUILD_ENVIRONMENT STREQUAL "SIMULATOR")
    set(FEATURE_ENVIRONMENT "simulator")
    set(CBIDGEN_FOLDER "simulator")
    set(TARGET_PATH	${CMAKE_SOURCE_DIR}/ui_simulator/lib/rust-builds)
elseif(BUILD_ENVIRONMENT STREQUAL "PRODUCTION")
    set(FEATURE_ENVIRONMENT "production")
    set(CBIDGEN_FOLDER "firmware")
    set(TARGET_PATH	${CMAKE_BINARY_DIR}/rust-builds)
    set(CARGO_ARM_TARGET thumbv7em-none-eabihf)
else()
    set(FEATURE_ENVIRONMENT "debug")
    set(CBIDGEN_FOLDER "firmware")
    set(TARGET_PATH	${CMAKE_BINARY_DIR}/rust-builds)
    set(CARGO_ARM_TARGET thumbv7em-none-eabihf)
endif()

message(STATUS "build target path: " ${TARGET_PATH})
if(NOT EXISTS ${TARGET_PATH})
    file(MAKE_DIRECTORY ${TARGET_PATH})
endif()

if(BUILD_VARIANT STREQUAL "BTC_ONLY")
    set(FEATURE_VARIANT "btc-only")
    set(CBIDGEN_CONFIG_FILE "btc_only.toml")
else()
    set(FEATURE_VARIANT "multi-coins")
    set(CBIDGEN_CONFIG_FILE "multi_coin.toml")
endif()

set(FEATURE ${FEATURE_ENVIRONMENT}-${FEATURE_VARIANT})

set(CBINDGEN_CONFIG_PATH ${RUST_C_DIR}/cbindgens/${CBIDGEN_FOLDER}/${CBIDGEN_CONFIG_FILE})
set(CBINDGEN_FLAG ${RUST_C_DIR} --config ${CBINDGEN_CONFIG_PATH} --output ${TARGET_PATH}/librust_c.h --lang c)

# compile rust lib
set(CARGO_FLAG --release -p rust_c --no-default-features)
set(CARGO_FLAG ${CARGO_FLAG} --features ${FEATURE})
if(CARGO_ARM_TARGET)
    set(CARGO_FLAG ${CARGO_FLAG} --target ${CARGO_ARM_TARGET})
    set(COPY_COMMAND  cp ${RUST_DIR}/target/${CARGO_ARM_TARGET}/release/${LIB_NAME} ${TARGET_PATH})
else()
    set(COPY_COMMAND  cp ${RUST_DIR}/target/release/${LIB_NAME} ${TARGET_PATH})
endif()

message(STATUS "cargo build command: " ${CARGO_FLAG})
#TODO: add debug-momery if need

#run build
add_custom_target(rust_c ALL
    COMMAND ${CBINDGEN_EXE} ${CBINDGEN_FLAG}
    COMMAND rustup run nightly-2024-01-31 ${RUST_CARGO_EXECUTABLE} build ${CARGO_FLAG}
    COMMAND ${COPY_COMMAND}
    WORKING_DIRECTORY ${RUST_DIR}
)
