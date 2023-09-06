set(CMAKE_SYSTEM_NAME Generic)
cmake_minimum_required(VERSION 3.10)
project(mh1903)

option(COMPILE_BOOT "compile on BOOT target" OFF)
option(COMPILE_MAIN "compile on MAIN target" ON)

set(CROSS_COMPILE_PREFIX arm-none-eabi)
set(CMAKE_C_COMPILER ${CROSS_COMPILE_PREFIX}-gcc)
set(CMAKE_CXX_COMPILER ${CROSS_COMPILE_PREFIX}-g++)
set(CMAKE_ASM_COMPILER ${CROSS_COMPILE_PREFIX}-gcc)
set(CMAKE_OBJCOPY ${CROSS_COMPILE_PREFIX}-objcopy)
set(CMAKE_OBJDUMP ${CROSS_COMPILE_PREFIX}-objdump)
set(CMAKE_SIZE ${CROSS_COMPILE_PREFIX}-size)
set(MCU cortex-m4)
set(LINKER_SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/mh1903b.ld)
set(ARCH_FLAGS "-mcpu=${MCU} -mthumb -mlittle-endian")
set(MCU_FLAGS "${ARCH_FLAGS} -Os -mfloat-abi=hard -mfpu=fpv4-sp-d16")
set(CMAKE_C_FLAGS "${MCU_FLAGS} -Wall -Wno-unknown-pragmas -Wno-format -g")
set(CMAKE_CXX_FLAGS "${MCU_FLAGS} -Wall -Wno-unknown-pragmas -Wno-format -g")
set(CMAKE_C_FLAGS_DEBUG "-O0 -g2 -ggdb")
set(CMAKE_C_FLAGS_RELEASE "-O3")

if(CMAKE_HOST_SYSTEM_NAME MATCHES "Linux")
    set(HOST_ARCH LINUX_ARCH)
elseif(CMAKE_HOST_SYSTEM_NAME MATCHES "Windows")
    set(HOST_ARCH WINDOWS_ARCH)
else()
    set(HOST_ARCH DARWIN_ARCH)
endif()

if(CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "aarch64")
    message("this is aarch64 cpu")
elseif(CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "x86_64")
    message("this is x86_64 cpu")
endif()

function (print_firmware_info)
    message(STATUS "\n\n=============================================")
    message(STATUS "            - General -")
    message(STATUS "Firmware version:       ${FIRMWARE_VERSION_FULL}")
    message(STATUS "Firmware v. (git):      ${GIT_FIRMWARE_VERSION_STRING}")
    message(STATUS "Firmware-btc version:   ${FIRMWARE_BTC_ONLY_VERSION_FULL}")
    message(STATUS "Firmware-btc v. (git):  ${GIT_FIRMWARE_BTC_ONLY_VERSION_STRING}")
    message(STATUS "Bootloader version:     ${BOOTLOADER_VERSION_FULL}")
    message(STATUS "Bootloader v. (git):    ${GIT_BOOTLOADER_VERSION_STRING}")
    message(STATUS "CMake version:          ${CMAKE_VERSION}")
    message(STATUS "System:                 ${CMAKE_SYSTEM}")
    message(STATUS "Processor:              ${CMAKE_SYSTEM_PROCESSOR}")
    message(STATUS "             - Build -")
    message(STATUS "Compiler version:       ${CMAKE_C_COMPILER_ID} ${C_COMPILER_VERSION}")
    message(STATUS "Compiler:               ${CMAKE_C_COMPILER}")
    message(STATUS "Linker:                 ${CMAKE_LINKER}")
    message(STATUS "Archiver:               ${CMAKE_AR}")
    message(STATUS "Default CFLAGS:         ${CMAKE_C_FLAGS} ${CMAKE_C_FLAGS_${CMAKE_BUILD_TYPE}}")
    message(STATUS "Default EXE LDFLAGS:    ${CMAKE_EXE_LINKER_FLAGS}")
    message(STATUS "            - Options -")
    message(STATUS "Build type:             ${CMAKE_BUILD_TYPE}")
    message(STATUS "Hardware version:       ${HARDWARE}")
    message(STATUS "Verbose:                ${CMAKE_VERBOSE_MAKEFILE}")
    message(STATUS "Coverage flags:         ${COVERAGE}")
    message(STATUS "\n=============================================\n\n")
endfunction()

if(COMPILE_MAIN)
    set(FIRMWARE_VERSION "0.0.1")
    message(STATUS "compile main firmware")
    add_subdirectory(rust)
    add_definitions(
    -DUSE_STDPERIPH_DRIVER 
    -DUSE_FULL_ASSERT 
    -DLV_CONF_INCLUDE_SIMPLE 
    -DLV_LVGL_H_INCLUDE_SIMPLE 
    -DARM_MATH_CM4 
    -D__TARGET_FPU_VFP 
    -DNDEBUG -DATCA_HAL_I2C
    -DCONFIG_USB_DEVICE_MSC
#    -DRUST_MEMORY_DEBUG
    )

    if(BUILD_PRODUCTION)
        add_compile_definitions(BUILD_PRODUCTION)
    endif()

    print_firmware_info()
    #include(mh1903_main.cmake)

    include_directories(
        Inc
        core
        tasks
        test
        driver
        driver/usb
        mh1903_lib/SCPU_USB_Lib/SCPU_USB_Device_Library/Class/msc/inc
        mh1903_lib/SCPU_USB_Lib/SCPU_USB_Device_Library/Class/cdc/inc
        mh1903_lib/SCPU_USB_Lib/SCPU_USB_OTG_Driver/inc
        mh1903_lib/SCPU_USB_Lib/SCPU_USB_Device_Library/Core/inc
        mh1903_lib/USB-ESL
        cm_backtrace
        FreeRTOS/Source/include
        FreeRTOS
        FreeRTOS/Source/portable/GCC/ARM_CM4F
        FreeRTOS/CMSIS_RTOS_V2
        crypto
        crypto/bips
        crypto/checksum
        crypto/utils
        crypto/slip39
        crypto/slip39/trezor-crypto
        mh1903_lib/MHSCPU_Driver/inc 
        mh1903_lib/Device/MegaHunt/mhscpu/Include 
        mh1903_lib/CMSIS/Include 
        mh1903_lib/MHSCPU_Driver/inc/cryptlib 
        mh1903_lib/MHSCPU_Driver/inc/emvlib
        business
        business/utils
        business/utils/log
        business/managers 
        business/error_codes 
        business/firmware 
        business/msg 
        business/ram 
        hardware_interface
        lvgl
        lvgl/src
        lvgl/src/core
        lvgl/src/draw
        lvgl/src/extra
        lvgl/src/font
        lvgl/src/hal
        lvgl/src/misc
        lvgl/src/widgets
        lvgl/src/core
        ui/lv_demos
        ui/pillar_demos
        ui/gui_views
        ui/gui_assets
        ui/gui_assets/abi
        ui/gui_frame
        ui/gui_widgets
        ui/lv_i18n
        ui/gui_model
        ui/gui_analyze
        ui/gui_chain
        ui/gui_components
        ui/wallet
        utils
        utils/log
        protocol
        protocol/services
        external
        external/ctaes
        external/fatfs
        external/ccan
        external/cjson
        external/quicklz
        external/sqlite3
        external/cryptoauthlib/lib
        build/rust-builds
        ${CMAKE_CURRENT_BINARY_DIR}
    )

    set_property(SOURCE mh1903_lib/Device/MegaHunt/mhscpu/Source/GCC/startup_mhscpu.s PROPERTY LANGUAGE C)
    set_property(SOURCE cm_backtrace/fault_handler/gcc/cmb_fault.s PROPERTY LANGUAGE C)
    aux_source_directory(mh1903_lib/MHSCPU_Driver/src/ mh1903_driver)
    aux_source_directory(driver/ driver)
    aux_source_directory(test/ test)
    aux_source_directory(cm_backtrace/ cm_backtrace)
    aux_source_directory(tasks/ tasks)
    aux_source_directory(crypto crypto)
    aux_source_directory(crypto/bips bips)
    aux_source_directory(crypto/checksum checksum)
    aux_source_directory(crypto/utils crypto_utils)
    aux_source_directory(core/ core)
    aux_source_directory(protocol/ protocol)
    aux_source_directory(protocol/services protocol_services)
    
    file (GLOB_RECURSE sqlite3
        "components/sqlite3/*.c"
    )

    file(GLOB_RECURSE components
        "components/utils/*.c"
        "components/ringbuff/*.c"
        ${sqlite3}
    )
    
    file(GLOB_RECURSE usb
        "driver/usb/*.c"
        "mh1903_lib/SCPU_USB_Lib/SCPU_USB_Device_Library/Core/src/*.c"
        "mh1903_lib/SCPU_USB_Lib/SCPU_USB_Device_Library/Class/msc/src/*.c"
        "mh1903_lib/SCPU_USB_Lib/SCPU_USB_Device_Library/Class/cdc/src/*.c"
        "mh1903_lib/SCPU_USB_Lib/SCPU_USB_OTG_Driver/src/*.c"
        "mh1903_lib/USB-ESL/CircularBuffer.c"    
    )
    
    file(GLOB_RECURSE cryptoslip39
        "crypto/slip39/gf256.c"
        "crypto/slip39/rs1024.c"
        "crypto/slip39/slip39.c"
        "crypto/slip39/trezor-crypto/memzero.c"
        "crypto/slip39/trezor-crypto/sha2.c"
        "crypto/slip39/trezor-crypto/rand.c"
        "crypto/slip39/trezor-crypto/pbkdf2.c"
    )
    
    file(GLOB_RECURSE freertos
        FreeRTOS/Source/croutine.c
        FreeRTOS/Source/event_groups.c
        FreeRTOS/Source/list.c
        FreeRTOS/Source/queue.c
        FreeRTOS/Source/stream_buffer.c
        FreeRTOS/Source/tasks.c
        FreeRTOS/Source/timers.c
        FreeRTOS/Source/portable/GCC/ARM_CM4F/port.c
        FreeRTOS/CMSIS_RTOS_V2/cmsis_os2.c
        FreeRTOS/Source/portable/MemMang/heap_4.c
        FreeRTOS/rtos_expand.c
    )

    file(GLOB_RECURSE lvgl
        "lvgl/src/core/*.c"
        "lvgl/src/draw/*.c"
        "lvgl/src/hal/*.c"
        "lvgl/src/misc/*.c"
        "lvgl/src/widgets/*.c"
        "lvgl/src/extra/*.c"
        "lvgl/src/font/*.c"
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

    file(GLOB_RECURSE external
        "external/fatfs/*.c"
        "external/cjson/*.c"
        "external/quicklz/*.c"
        "external/sqlite3/*.c"
        ${ccan}
        ${cryptoauthlib}
        ${ctaes}
    )

    file(GLOB_RECURSE gui
        "ui/gui_assets/*.c"
        "ui/gui_assets/font/*.c"
        "ui/gui_assets/abi/*.c"
        "ui/gui_frame/*.c"
        "ui/gui_views/*.c"
        "ui/gui_widgets/*.c"
        "ui/pillar_demos/qrcode_sign/*.c"
        "ui/lv_i18n/*.c"
        "ui/gui_model/*.c"
		"ui/gui_analyze/gui_analyze.c"
		"ui/gui_chain/*"
		"ui/gui_components/*"
		"ui/wallet/*"
    )

    file(GLOB_RECURSE hardware_interface
        "hardware_interface/*.c"
    )

    file(GLOB_RECURSE business
        "business/*.c"
        "business/utils/*.c"
        "business/utils/log/*.c"
        "business/managers/*.c"
        "business/error_codes/*.c"
        "business/firmware/*.c"
        "business/msg/*.c"
        "business/ram/*.c"
    )

    set(objsrc ${mh1903_driver} 
        ${business}
        ${hardware_interface}
        ${freertos} 
        ${driver} 
        ${usb}
        ${external}
        ${cm_backtrace} 
        ${tasks} 
        ${test}
        ${crypto}
        ${cryptoslip39}
        ${bips}
        ${checksum}
        ${crypto_utils}
        ${lvgl}
        ${gui}
        ${core}
        ${protocol}
        ${protocol_services}
        mh1903_lib/Device/MegaHunt/mhscpu/Source/GCC/startup_mhscpu.s
        cm_backtrace/fault_handler/gcc/cmb_fault.s)

    set(CMAKE_EXE_LINKER_FLAGS " -T ${LINKER_SCRIPT} -Wl,-Map=mh1903.map -lm -mcpu=${MCU} --specs=nano.specs -specs=nosys.specs -nostartfiles -Wl,-Map=${PROJECT_BINARY_DIR}/${PROJECT_NAME}.map,--cref -Wl,--gc-sections ")

    add_executable(${PROJECT_NAME}.elf ${objsrc})
    add_dependencies(${PROJECT_NAME}.elf rust_c)

    target_link_libraries(${PROJECT_NAME}.elf 
        ${CMAKE_CURRENT_SOURCE_DIR}/mh1903_lib/MHSCPU_Driver/lib/MH1903_QRDecodeLib.a
        m
        ${CMAKE_BINARY_DIR}/rust-builds/librust_c.a
    )

    set(ELF_FILE ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.elf)
    set(HEX_FILE ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.hex)
    set(BIN_FILE ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.bin)

    add_custom_command(TARGET "${PROJECT_NAME}.elf" POST_BUILD
        COMMAND ${CMAKE_OBJCOPY} -Obinary ${ELF_FILE} ${BIN_FILE}
        COMMAND ${CMAKE_OBJCOPY} -Oihex  ${ELF_FILE} ${HEX_FILE}
        COMMENT "Building ${PROJECT_NAME}.bin and ${PROJECT_NAME}.hex"

        COMMAND ${CMAKE_COMMAND} -E copy ${HEX_FILE} "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.hex"
        COMMAND ${CMAKE_COMMAND} -E copy ${BIN_FILE} "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.bin"

        COMMAND ${CMAKE_SIZE} --format=berkeley ${PROJECT_NAME}.elf ${PROJECT_NAME}.hex
        COMMENT "Invoking: Cross ARM GNU Print Size"
    )
elseif(COMPILE_BOOT)
    set(FIRMWARE_BOOT_VERSION "0.0.1")
    message(FATAL_ERROR "Unknow..")
endif(COMPILE_MAIN)
