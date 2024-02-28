message(STATUS "compile rust")

set(CARGO_ARM_TARGET thumbv7em-none-eabihf)
set(RUST_DIR ${CMAKE_SOURCE_DIR}/rust)

find_program(RUST_CARGO_EXECUTABLE cargo)
find_program(BINDGEN_EXE bindgen)
find_program(CBINDGEN_EXE cbindgen)

if(NOT CMAKE_BUILD_TYPE STREQUAL "Simulator")
    set(TARGET_PATH	${CMAKE_BINARY_DIR}/rust-builds)
else()
    set(TARGET_PATH	${CMAKE_SOURCE_DIR}/ui_simulator/lib/rust-builds)
endif()

message(STATUS "build target path: " ${TARGET_PATH})

option(LIB_RUST_C "Set to switch to Compile the RUST_C library" OFF)

if(NOT EXISTS ${TARGET_PATH})
    file(MAKE_DIRECTORY ${TARGET_PATH})
endif()

add_subdirectory(rust_c)
