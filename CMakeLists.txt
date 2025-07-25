set(CMAKE_SYSTEM_NAME Generic)
cmake_minimum_required(VERSION 3.10)

option(COMPILE_KEYSTONE "compile on MAIN target" ON)

set(BUILD_ENVIRONMENT "DEBUG")
set(BUILD_VARIANT "MULTI_COINS")

if(CYPHERPUNK)
    message(STATUS "Building for CYPHERPUNK")
    add_compile_definitions(CYPHERPUNK_VERSION)
    set(WIDGET_PATH "multi")
    set(WIDGET_SUBPATH "multi/cypherpunk")
    set(BUILD_VARIANT "CYPHERPUNK")
elseif(BTC_ONLY)
    message(STATUS "Building for BTC_ONLY")
    set(WIDGET_PATH "btc_only")
    set(WIDGET_SUBPATH "btc_only/multi_sig")
    set(BUILD_VARIANT "BTC_ONLY")
    add_compile_definitions(BTC_ONLY)
    add_compile_definitions(EXCLUDE_RUSTC)
else()
    message(STATUS "Building for WEB3_VERSION")
    add_compile_definitions(WEB3_VERSION)
    set(WIDGET_PATH "multi")
    set(WIDGET_SUBPATH "multi/web3")
    set(BUILD_VARIANT "MULTI_COINS")
endif()
if(BUILD_TYPE STREQUAL "Simulator")
    set(COMPILE_KEYSTONE OFF CACHE BOOL "compile on MAIN target" FORCE)
    set(COMPILE_SIMULATOR ON CACHE BOOL "compile on Simulator target" FORCE)
    set(BUILD_ENVIRONMENT "SIMULATOR")
endif()

if(BUILD_PRODUCTION)
    set(BUILD_ENVIRONMENT "PRODUCTION")
endif()

set(CMAKE_C_FLAGS_DEBUG "-O0 -g2 -ggdb")
set(CMAKE_C_FLAGS_RELEASE "-O3")

add_subdirectory(rust)

# add definition
function(add_definition_with_message OPTION_NAME)
    if(${OPTION_NAME})
        message(STATUS "Enabled option: ${OPTION_NAME}")
        add_compile_definitions(${OPTION_NAME})
    endif()
endfunction()

add_definition_with_message(ENABLE_SCREEN_SHOT)

add_definition_with_message(DEBUG_MEMORY)

add_definition_with_message(BUILD_PRODUCTION)

get_cmake_property(_variableNames VARIABLES)
list(SORT _variableNames)

foreach(_variableName ${_variableNames})
    if(_variableName MATCHES "KT_FEATURE_*")
        add_definition_with_message(${_variableName})
    endif()
endforeach()


set(COMMON_DEFINITIONS
    USE_STDPERIPH_DRIVER
    USE_FULL_ASSERT
    ARM_MATH_CM4
    __TARGET_FPU_VFP
    NDEBUG
    ATCA_HAL_I2C
    CONFIG_USB_DEVICE_MSC
)

# add include path
set(CRYPTO_INCLUDE_PATH
    src/crypto
    src/crypto/bips
    src/crypto/checksum
    src/crypto/utils
    src/crypto/slip39
    src/crypto/slip39/trezor-crypto
)

set(GUI_CUSTOM_INCLUDE_PATH
    src/ui/gui_widgets/${WIDGET_SUBPATH}
    src/ui/gui_widgets/${WIDGET_PATH}
    src/ui/gui_chain/${WIDGET_SUBPATH}
    src/ui/gui_chain/${WIDGET_PATH}
    src/ui/gui_wallet/${WIDGET_SUBPATH}
    src/ui/gui_wallet/${WIDGET_PATH}
)

set(GUI_INCLUDE_PATH
    src/ui
    src/ui/gui_views
    src/ui/gui_assets
    src/ui/gui_assets/abi
    src/ui/gui_frame
    ${GUI_CUSTOM_INCLUDE_PATH}
    src/ui/gui_widgets
    src/ui/gui_widgets/setting
    src/ui/lv_i18n
    src/ui/gui_model
    src/ui/gui_analyze
    src/ui/gui_chain
    src/ui/gui_components
    src/ui/gui_wallet
)

set(EXTERNAL_INCLUDE_PATH
    external
    external/ctaes
    external/fatfs
    external/ccan
    external/cjson
    external/quicklz
    external/sqlite3
    external/circular_buffer
    external/cryptoauthlib/lib
    external/FreeRTOS
    external/FreeRTOS/Source/include
    external/FreeRTOS/Source/portable/GCC/ARM_CM4F
    external/FreeRTOS/CMSIS_RTOS_V2
    external/lvgl
    external/lvgl/src
    external/lvgl/src/core
    external/lvgl/src/draw
    external/lvgl/src/extra
    external/lvgl/src/font
    external/lvgl/src/hal
    external/lvgl/src/misc
    external/lvgl/src/widgets
    external/lvgl/src/core
    external/safec/include
    external/safec/src
    external/mh1903_lib/MHSCPU_Driver/inc
    external/mh1903_lib/Device/MegaHunt/mhscpu/Include
    external/mh1903_lib/CMSIS/Include
    external/mh1903_lib/MHSCPU_Driver/inc/cryptlib
    external/mh1903_lib/MHSCPU_Driver/inc/emvlib
    external/mh1903_lib/SCPU_USB_Lib/SCPU_USB_Device_Library/Class/msc/inc
    external/mh1903_lib/SCPU_USB_Lib/SCPU_USB_Device_Library/Class/cdc/inc
    external/mh1903_lib/SCPU_USB_Lib/SCPU_USB_OTG_Driver/inc
    external/mh1903_lib/SCPU_USB_Lib/SCPU_USB_Device_Library/Core/inc
    external/mh1903_lib/USB-ESL
)

set(SRC_INCLUDE_PATH
    src
    src/config
    src/tasks
    src/driver
    src/driver/usb
    src/cm_backtrace
    src/utils
    src/utils/log
    src/managers
    src/managers/btc_only
    src/error_codes
    src/firmware
    src/msg
    src/ram
    src/hardware_interface
    src/webusb_protocol
    src/webusb_protocol/services
    src/webusb_protocol/general
)

set(INCLUDE_DIR
    build/rust-builds
    ${SRC_INCLUDE_PATH}
    ${CRYPTO_INCLUDE_PATH}
    ${GUI_INCLUDE_PATH}
    ${EXTERNAL_INCLUDE_PATH}
    ${CMAKE_CURRENT_BINARY_DIR}
)

if(NOT BTC_ONLY)
    list(APPEND INCLUDE_DIR src/webusb_protocol/general src/webusb_protocol/general/eapdu_services)
endif()

if(NOT BUILD_PRODUCTION)
    list(APPEND INCLUDE_DIR test)
    set(TEST_CMD test)
endif()
    
file(GLOB GUI_CUSTOM_VIEW_WIDGETS
    src/ui/gui_views/*.c
    src/ui/gui_widgets/*.c
    src/ui/gui_wallet/*.c
    src/ui/gui_chain/*.c
    src/ui/gui_analyze/*.c
    src/ui/gui_widgets/${WIDGET_SUBPATH}/*.c
    src/ui/gui_widgets/${WIDGET_PATH}/*.c
    src/ui/gui_views/${WIDGET_SUBPATH}/*.c
    src/ui/gui_views/${WIDGET_PATH}/*.c
    src/ui/gui_chain/${WIDGET_SUBPATH}/*.c
    src/ui/gui_chain/${WIDGET_PATH}/*.c
    src/ui/gui_wallet/${WIDGET_SUBPATH}/*.c
    src/ui/gui_wallet/${WIDGET_PATH}/*.c
    src/ui/gui_analyze/${WIDGET_SUBPATH}/*.c
    src/ui/gui_analyze/${WIDGET_PATH}/*.c
)

file(GLOB_RECURSE GUI
    ${GUI_CUSTOM_VIEW_WIDGETS}
    "src/ui/gui_assets/*.c"
    "src/ui/gui_assets/font/*.c"
    "src/ui/gui_assets/abi/*.c"
    "src/ui/gui_frame/*.c"
    "src/ui/gui_widgets/setting/*.c"
    "src/ui/lv_i18n/*.c"
    "src/ui/gui_model/*.c"
    "src/ui/gui_components/*.c"
)

file(GLOB_RECURSE LVGL
    "external/lvgl/src/core/*.c"
    "external/lvgl/src/draw/*.c"
    "external/lvgl/src/hal/*.c"
    "external/lvgl/src/misc/*.c"
    "external/lvgl/src/widgets/*.c"
    "external/lvgl/src/extra/*.c"
    "external/lvgl/src/font/*.c"
)

file(GLOB_RECURSE CRYPTO_SLIP39
    "src/crypto/slip39/gf256.c"
    "src/crypto/slip39/rs1024.c"
    "src/crypto/slip39/slip39.c"
    "src/crypto/slip39/trezor-crypto/memzero.c"
    "src/crypto/slip39/trezor-crypto/sha2.c"
    "src/crypto/slip39/trezor-crypto/rand.c"
    "src/crypto/slip39/trezor-crypto/pbkdf2.c"
)

if(COMPILE_KEYSTONE)
    message(STATUS "Compiling for MAIN target")
    project(mh1903)
    include(firmware.cmake)

    file(GLOB_RECURSE SAFE_C
        "external/safec/src/*.c"
        "external/safec/src/extmem/*.c"
        "external/safec/src/extstr/*.c"
        "external/safec/src/io/*.c"
        "external/safec/src/mem/*.c"
        "external/safec/src/misc/*.c"
        "external/safec/src/str/*.c"
    )

    file(GLOB_RECURSE ctaes
        "external/ctaes/ctaes.c"
    )

    file(GLOB_RECURSE ccan
        "external/ccan/*.c"
    )

    file(GLOB_RECURSE cryptoauthlib
        "external/cryptoauthlib/lib/atca_basic.c"
        "external/cryptoauthlib/lib/atca_cfgs.c"
        "external/cryptoauthlib/lib/atca_debug.c"
        "external/cryptoauthlib/lib/atca_device.c"
        "external/cryptoauthlib/lib/atca_helpers.c"
        "external/cryptoauthlib/lib/atca_iface.c"
        "external/cryptoauthlib/lib/atca_utils_sizes.c"
        "external/cryptoauthlib/lib/calib/*.c"
        "external/cryptoauthlib/lib/host/atca_host.c"
        "external/cryptoauthlib/lib/crypto/*.c"
        "external/cryptoauthlib/lib/crypto/hashes/*.c"
        "external/cryptoauthlib/lib/hal/*.c"
    )

    file(GLOB_RECURSE EXTERNAL
        "external/fatfs/*.c"
        "external/cjson/*.c"
        "external/quicklz/*.c"
        "external/sqlite3/*.c"
        "external/circular_buffer/*.c"
        ${ccan}
        ${cryptoauthlib}
        ${ctaes}
    )

    set(objsrc ${MH1903_DRIVER}
        ${SRC}
        ${FREERTOS}
        ${DRIVER}
        ${USB_DRIVER}
        ${EXTERNAL}
        ${CM_BACKTRACE}
        ${TASKS}
        ${TEST_CMD}
        ${CRYPTO}
        ${CRYPTO_SLIP39}
        ${BIPS}
        ${CHECKSUM}
        ${CRYPTO_UTILS}
        ${LVGL}
        ${GUI}
        ${WEBUSB_PROTOCOL}
        ${SAFE_C}
        external/mh1903_lib/Device/MegaHunt/mhscpu/Source/GCC/startup_mhscpu.s
        src/cm_backtrace/fault_handler/gcc/cmb_fault.s
    )

    foreach(directory ${excluded_directories})
        file(GLOB_RECURSE excluded_files "${directory}/*")
        list(REMOVE_ITEM objsrc ${excluded_files})
    endforeach()

    add_executable(${PROJECT_NAME}.elf ${objsrc})
    target_compile_definitions(${PROJECT_NAME}.elf PRIVATE ${COMMON_DEFINITIONS})
    target_include_directories(${PROJECT_NAME}.elf PRIVATE ${INCLUDE_DIR})
    add_dependencies(${PROJECT_NAME}.elf rust_c)

    target_link_libraries(${PROJECT_NAME}.elf
        ${CMAKE_CURRENT_SOURCE_DIR}/external/mh1903_lib/MHSCPU_Driver/lib/MH1903_QRDecodeLib.a
        m
        ${CMAKE_BINARY_DIR}/rust-builds/librust_c.a
    )

    set(ELF_FILE ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.elf)
    set(HEX_FILE ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.hex)
    set(BIN_FILE ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.bin)

    add_custom_command(TARGET "${PROJECT_NAME}.elf" POST_BUILD
        COMMAND ${CMAKE_OBJCOPY} -Obinary ${ELF_FILE} ${BIN_FILE}
        COMMAND ${CMAKE_OBJCOPY} -Oihex ${ELF_FILE} ${HEX_FILE}
        COMMENT "Building ${PROJECT_NAME}.bin and ${PROJECT_NAME}.hex"

        COMMAND ${CMAKE_COMMAND} -E copy ${HEX_FILE} "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.hex"
        COMMAND ${CMAKE_COMMAND} -E copy ${BIN_FILE} "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.bin"

        COMMAND ${CMAKE_SIZE} --format=berkeley ${PROJECT_NAME}.elf ${PROJECT_NAME}.hex
        COMMENT "Invoking: Cross ARM GNU Print Size"
    )
else()
    project(simulator)
    include(ui_simulator/simulator.cmake)

    if(BTC_ONLY)
        set(excluded_directories
            src/ui/gui_chain/others
            src/ui/gui_widgets/general
            src/ui/gui_views/general
            src/webusb_protocol/general)
        SET(INCLUDES src/managers/btc_only/multi_sig_wallet_manager.c src/managers/sdcard_manager.c)
    else()
        set(excluded_directories
            src/ui/gui_views/btc_only
            src/ui/gui_widgets/btc_only)
    endif()

    foreach(directory ${excluded_directories})
        file(GLOB_RECURSE excluded_files "${directory}/*")
        list(REMOVE_ITEM objsrc ${excluded_files})
    endforeach()

    add_executable(${PROJECT_NAME}
        ${SIMULATOR_SRC}
        ${EXTERNAL}
        ${SOURCES}
        ${INCLUDES}
        ${GUI}
        ${LVGL}
        ${CRYPTO}
        ${UTILS}
        ${CRYPTO_SLIP39}
    )
    add_dependencies(${PROJECT_NAME} rust_c)
    target_link_libraries(${PROJECT_NAME} PRIVATE SDL2 ${CMAKE_SOURCE_DIR}/ui_simulator/lib/rust-builds/librust_c.a)
    if(CMAKE_HOST_SYSTEM_NAME MATCHES "Linux")
      target_link_libraries(${PROJECT_NAME} PRIVATE m dl pthread )
    endif()

    if(CMAKE_HOST_SYSTEM_NAME MATCHES "Windows")
        target_link_libraries(${PROJECT_NAME} PRIVATE windows ntdll)
    endif()

    if(CMAKE_HOST_SYSTEM_NAME MATCHES "Darwin")
        # add framework for MacOS simulator to scan QR code
        target_link_libraries(${PROJECT_NAME} PRIVATE "-framework CoreFoundation")
        target_link_libraries(${PROJECT_NAME} PRIVATE "-framework CoreGraphics")
        
        set(CODESIGN_IDENTITY "keystone simulator")
        set(ENTITLEMENTS_FILE "${CMAKE_SOURCE_DIR}/entitlements.plist")
        set(CODESIGN_COMMAND codesign --force --options runtime --entitlements ${ENTITLEMENTS_FILE} --sign "${CODESIGN_IDENTITY}")
        add_custom_command(TARGET ${PROJECT_NAME}
            POST_BUILD
            COMMAND ${CODESIGN_COMMAND} ${PROJECT_NAME}
            COMMENT "Signing the executable"
        )
    endif()
endif()