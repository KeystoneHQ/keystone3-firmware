message(STATUS "compile rust")
set(LIB_NAME librust_c.a)

# set variables
find_program(RUST_CARGO_EXECUTABLE cargo)
find_program(BINDGEN_EXE bindgen)
find_program(CBINDGEN_EXE cbindgen)

# BUILD_ENVIRONMENT: PRODUCTION, DEBUG, SIMULATOR
# BUILD_VARIANT: BTC_ONLY, MULTI_COINS, CYPHERPUNK

set(RUST_DIR ${CMAKE_SOURCE_DIR}/rust)
set(RUST_C_DIR ${RUST_DIR}/rust_c)

if(BUILD_ENVIRONMENT STREQUAL "SIMULATOR")
    set(FEATURE_ENVIRONMENT "simulator")
    set(TARGET_PATH	${CMAKE_SOURCE_DIR}/ui_simulator/lib/rust-builds)
elseif(BUILD_ENVIRONMENT STREQUAL "PRODUCTION")
    set(FEATURE_ENVIRONMENT "production")
    set(TARGET_PATH	${CMAKE_BINARY_DIR}/rust-builds)
    set(CARGO_ARM_TARGET thumbv7em-none-eabihf)
else()
    set(FEATURE_ENVIRONMENT "debug")
    set(TARGET_PATH	${CMAKE_BINARY_DIR}/rust-builds)
    set(CARGO_ARM_TARGET thumbv7em-none-eabihf)
endif()

message(STATUS "build environment: " ${BUILD_ENVIRONMENT})
message(STATUS "build variant: " ${BUILD_VARIANT})

message(STATUS "build target path: " ${TARGET_PATH})
if(NOT EXISTS ${TARGET_PATH})
    file(MAKE_DIRECTORY ${TARGET_PATH})
endif()

if(BUILD_VARIANT STREQUAL "BTC_ONLY")
    set(FEATURE_VARIANT "btc-only")
elseif(BUILD_VARIANT STREQUAL "CYPHERPUNK")
    set(FEATURE_VARIANT "cypherpunk")
else()
    set(FEATURE_VARIANT "multi-coins")
endif()

set(FEATURE ${FEATURE_ENVIRONMENT}-${FEATURE_VARIANT})

set(CBINDGEN_BINDINGS_TARGET ${RUST_C_DIR}/bindings/${FEATURE}/librust_c.h)
set(COPY_BINDINGS_TARGET cp ${CBINDGEN_BINDINGS_TARGET} ${TARGET_PATH})

# compile rust lib
set(CARGO_FLAG --release -p rust_c --no-default-features)
set(CARGO_FLAG ${CARGO_FLAG} --features ${FEATURE})
if(CARGO_ARM_TARGET)
    set(CARGO_FLAG ${CARGO_FLAG} --target ${CARGO_ARM_TARGET})
    set(COPY_BUILD_TARGET  cp ${RUST_DIR}/target/${CARGO_ARM_TARGET}/release/${LIB_NAME} ${TARGET_PATH})
else()
    set(COPY_BUILD_TARGET  cp ${RUST_DIR}/target/release/${LIB_NAME} ${TARGET_PATH})
endif()

message(STATUS "cargo build command: " ${CARGO_FLAG})
#TODO: add debug-momery if need

#run build
add_custom_target(rust_c ALL
    COMMAND rustup run nightly-2024-10-09 ${RUST_CARGO_EXECUTABLE} build ${CARGO_FLAG}
    COMMAND ${COPY_BUILD_TARGET}
    COMMAND ${COPY_BINDINGS_TARGET}
    WORKING_DIRECTORY ${RUST_DIR}
)
