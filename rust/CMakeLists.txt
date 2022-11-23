message(STATUS "compile rust")

set(CARGO_ARM_TARGET thumbv7em-none-eabihf)
set(RUST_DIR ${CMAKE_SOURCE_DIR}/rust)

find_program(RUST_CARGO_EXECUTABLE cargo)
find_program(BINDGEN_EXE bindgen)
find_program(CBINDGEN_EXE cbindgen)
set(TARGET_PATH	${CMAKE_BINARY_DIR}/rust-builds)

option(LIB_RUST_C "Set to switch to Compile the RUST_C library" OFF)

if(NOT EXISTS ${TARGET_PATH})
    file(MAKE_DIRECTORY ${TARGET_PATH})
endif()

add_subdirectory(rust_c)
