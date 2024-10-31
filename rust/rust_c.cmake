set(LIB_NAME librust_c.a)

find_program(RUST_CARGO_EXECUTABLE cargo)
find_program(BINDGEN_EXE bindgen)
find_program(CBINDGEN_EXE cbindgen)

if(NOT BUILD_TYPE STREQUAL "Simulator")
    set(TARGET_PATH	${CMAKE_BINARY_DIR}/rust-builds)
else()
    set(TARGET_PATH	${CMAKE_SOURCE_DIR}/ui_simulator/lib/rust-builds)
endif()

message(STATUS "build target path: " ${TARGET_PATH})

option(LIB_RUST_C "Set to switch to Compile the RUST_C library" OFF)

if(NOT EXISTS ${TARGET_PATH})
    file(MAKE_DIRECTORY ${TARGET_PATH})
endif()

set(RUST_DIR ${CMAKE_SOURCE_DIR}/rust)
set(RUST_C_DIR ${RUST_DIR}/rust_c)
set(CARGO_ARM_TARGET thumbv7em-none-eabihf)

if(COMPILE_SIMULATOR)
    if(BTC_ONLY)
        set(CBINDGEN_CONFIG_PATH ${RUST_C_DIR}/cbindgens/simulator/btc_only.toml)
    else()
        set(CBINDGEN_CONFIG_PATH ${RUST_C_DIR}/cbindgens/simulator/multi_coin.toml)
    endif()
else()
    if(BTC_ONLY)
        set(CBINDGEN_CONFIG_PATH ${RUST_C_DIR}/cbindgens/release/btc_only.toml)
    else()
        set(CBINDGEN_CONFIG_PATH ${RUST_C_DIR}/cbindgens/release/multi_coin.toml)
    endif()
endif()
set(CBINDGEN_FLAG ${RUST_C_DIR} --config ${CBINDGEN_CONFIG_PATH} --output ${TARGET_PATH}/librust_c.h --lang c)