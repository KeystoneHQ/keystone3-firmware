set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR})
add_compile_definitions(LV_LVGL_H_INCLUDE_SIMPLE)
add_compile_definitions(LV_CONF_INCLUDE_SIMPLE)
add_compile_definitions(COMPILE_SIMULATOR)
add_compile_definitions(HASH_AND_SALT_TEST_MODE)

file(GLOB_RECURSE INCLUDES "ui_simulator/lv_drivers/*.h" "external/lvgl/*.h")
file(GLOB_RECURSE SOURCES  "ui_simulator/lv_drivers/*.c")
file(GLOB_RECURSE CRYPTO
    "src/crypto/secret_cache.c"
    "src/crypto/account_public_info.c"
    "src/crypto/utils/hash_and_salt.c"
    "src/crypto/utils/hmac.c"
    "src/crypto/utils/hkdf.c"
    "src/crypto/bip39.c"
    "src/crypto/wordlist.c"
    "src/crypto/bips/mnemonic.c"
    "src/device_settings.c"
)
file(GLOB_RECURSE EXTERNAL
    "external/cjson/*.c"
    "external/ccan/*.c"
    "external/ctaes/ctaes.c"
)
file(GLOB_RECURSE SIMULATOR_SRC
    ui_simulator/*.c
)

if(BTC_ONLY)
    list(APPEND GUI_INCLUDE_PATH src/ui/gui_widgets/btc_only)
else()
    list(APPEND GUI_INCLUDE_PATH src/ui/gui_chain/others src/ui/gui_widgets/general  src/webusb_protocol/general src/webusb_protocol/general/eapdu_services)
    list(FILTER GUI EXCLUDE REGEX src/ui/gui_widgets/btc_only)
endif()

file(GLOB_RECURSE UTILS
    "src/utils/user_utils.c"
    "src/utils/log/log_print.c"
    "src/error_codes/*.c"
    "src/config/*.c"
    "src/managers/keystore.c"
    "src/managers/account_manager.c"
)

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/build) 
SET(CMAKE_CXX_FLAGS "-O3")

link_directories(ui_simulator/lib)
include_directories(
    external/lvgl/src
    ui_simulator
    ${SRC_INCLUDE_PATH}
    ${EXTERNAL_INCLUDE_PATH}
    ${GUI_INCLUDE_PATH}
    ${CRYPTO_INCLUDE_PATH}
)
include_directories(${SDL2_INCLUDE_DIRS})
