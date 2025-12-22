set(CMAKE_SYSTEM_NAME Generic)
cmake_minimum_required(VERSION 3.10)

set(BUILD_ENVIRONMENT "DEBUG")
set(CMAKE_C_FLAGS_DEBUG "-O0 -g2 -ggdb")
set(CMAKE_C_FLAGS_RELEASE "-O3")

# Must include firmware.cmake before project() to set up compiler
include(firmware.cmake)

project(mh1903)

set(COMMON_DEFINITIONS
    USE_STDPERIPH_DRIVER
    USE_FULL_ASSERT
    ARM_MATH_CM4
    __TARGET_FPU_VFP
    NDEBUG
)

# Include paths
set(EXTERNAL_INCLUDE_PATH
    external
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
    external/mh1903_lib/MHSCPU_Driver/inc
    external/mh1903_lib/Device/MegaHunt/mhscpu/Include
    external/mh1903_lib/CMSIS/Include
)

set(SRC_INCLUDE_PATH
    src
    src/config
    src/tasks
    src/driver
    src/ram
    src/utils
)

set(INCLUDE_DIR
    ${SRC_INCLUDE_PATH}
    ${EXTERNAL_INCLUDE_PATH}
)

# LVGL source files
file(GLOB_RECURSE LVGL
    "external/lvgl/src/core/*.c"
    "external/lvgl/src/draw/*.c"
    "external/lvgl/src/hal/*.c"
    "external/lvgl/src/misc/*.c"
    "external/lvgl/src/widgets/*.c"
    "external/lvgl/src/extra/*.c"
    "external/lvgl/src/font/*.c"
)

# FreeRTOS source files
file(GLOB_RECURSE FREERTOS
    "external/FreeRTOS/Source/croutine.c"
    "external/FreeRTOS/Source/event_groups.c"
    "external/FreeRTOS/Source/list.c"
    "external/FreeRTOS/Source/queue.c"
    "external/FreeRTOS/Source/stream_buffer.c"
    "external/FreeRTOS/Source/tasks.c"
    "external/FreeRTOS/Source/timers.c"
    "external/FreeRTOS/Source/portable/GCC/ARM_CM4F/port.c"
    "external/FreeRTOS/CMSIS_RTOS_V2/cmsis_os2.c"
    "external/FreeRTOS/Source/portable/MemMang/heap_4.c"
    "external/FreeRTOS/rtos_expand.c"
)

# MH1903 driver source files
aux_source_directory(external/mh1903_lib/MHSCPU_Driver/src/ MH1903_DRIVER)

# Application source files
set(APP_SRC
    src/main.c
    src/tasks/helloworld_task.c
    src/driver/drv_sys.c
    src/driver/drv_uart.c
    src/driver/drv_psram.c
    src/driver/drv_lcd_bright.c
    src/driver/drv_parallel8080.c
    src/driver/drv_ili9806.c
    src/driver/drv_nt35510.c
    src/driver/hal_lcd.c
    src/ram/psram_heap_4.c
    src/ram/user_memory.c
    src/utils/assert.c
    src/utils/user_delay.c
    src/utils/user_utils.c
    src/config/version.c
    src/driver/drv_power.c
)

set(objsrc 
    ${MH1903_DRIVER}
    ${APP_SRC}
    ${FREERTOS}
    ${LVGL}
    external/mh1903_lib/Device/MegaHunt/mhscpu/Source/GCC/startup_mhscpu.s
)

add_executable(${PROJECT_NAME}.elf ${objsrc})
target_compile_definitions(${PROJECT_NAME}.elf PRIVATE ${COMMON_DEFINITIONS})
target_include_directories(${PROJECT_NAME}.elf PRIVATE ${INCLUDE_DIR})

target_link_libraries(${PROJECT_NAME}.elf m)

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