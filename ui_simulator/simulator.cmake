set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR})
add_compile_definitions(LV_LVGL_H_INCLUDE_SIMPLE)
add_compile_definitions(LV_CONF_INCLUDE_SIMPLE)
add_compile_definitions(COMPILE_SIMULATOR)
if(CMAKE_HOST_SYSTEM_NAME MATCHES "Darwin")
    add_compile_definitions(COMPILE_MAC_SIMULATOR)
endif()
add_compile_definitions(HASH_AND_SALT_TEST_MODE)

file(GLOB_RECURSE INCLUDES "ui_simulator/lv_drivers/*.h" "external/lvgl/*.h")
file(GLOB_RECURSE SOURCES  "ui_simulator/lv_drivers/*.c")
file(GLOB_RECURSE CRYPTO
    "src/crypto/secret_cache.c"
    "src/crypto/account_public_info.c"
    "src/crypto/utils/hash_and_salt.c"
    "src/crypto/utils/hmac.c"
    "src/crypto/utils/hkdf.c"
    "src/crypto/utils/pbkdf2.c"
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
    list(APPEND GUI_INCLUDE_PATH src/ui/gui_widgets/btc_only src/ui/gui_widgets/general)
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
    "src/managers/se_manager.c"
)

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/build) 
SET(CMAKE_CXX_FLAGS "-O3")

include_directories(
    external/lvgl/src
    ui_simulator
    ui_simulator/lib/rust-builds
    ${SRC_INCLUDE_PATH}
    ${EXTERNAL_INCLUDE_PATH}
    ${GUI_INCLUDE_PATH}
    ${CRYPTO_INCLUDE_PATH}
)

if(CMAKE_HOST_SYSTEM_NAME MATCHES "Windows")
    link_directories(ui_simulator/lib/windows)
endif()

if(CMAKE_HOST_SYSTEM_NAME MATCHES "Darwin")
    SET(SDL2_INCLUDE_DIRS /opt/homebrew/include)
    link_directories(/opt/homebrew/opt/sdl2/lib)
endif()

#store simulator json files
set(ASSETS_PATH ui_simulator/assets)
if(NOT EXISTS ${ASSETS_PATH})
    file(MAKE_DIRECTORY ${ASSETS_PATH})
endif()

set(QRCODE_FILE ui_simulator/assets/qrcode_data.txt)
if(NOT EXISTS ${QRCODE_FILE})
    file(TOUCH ${QRCODE_FILE})
endif()

include_directories(${SDL2_INCLUDE_DIRS})
