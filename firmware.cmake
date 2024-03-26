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

if(NOT BUILD_PRODUCTION)
    aux_source_directory(test/ TEST_CMD)
    list(APPEND INCLUDE_DIR test)
endif()

set_property(SOURCE external/mh1903_lib/Device/MegaHunt/mhscpu/Source/GCC/startup_mhscpu.s PROPERTY LANGUAGE C)
set_property(SOURCE src/cm_backtrace/fault_handler/gcc/cmb_fault.s PROPERTY LANGUAGE C)
aux_source_directory(external/mh1903_lib/MHSCPU_Driver/src/ MH1903_DRIVER)
aux_source_directory(src/driver/ DRIVER)
aux_source_directory(src/cm_backtrace/ CM_BACKTRACE)
aux_source_directory(src/tasks/ TASKS)
aux_source_directory(src/crypto CRYPTO)
aux_source_directory(src/crypto/bips BIPS)
aux_source_directory(src/crypto/checksum CHECKSUM)
aux_source_directory(src/crypto/utils CRYPTO_UTILS)
file(GLOB_RECURSE WEBUSB_PROTOCOL WEBUSB_PROTOCOL/*.c)
list(FILTER all_files EXCLUDE REGEX "some_directory/general/.*")

file(GLOB_RECURSE USB_DRIVER
    "src/driver/usb/*.c"
    "external/mh1903_lib/SCPU_USB_Lib/SCPU_USB_Device_Library/Core/src/*.c"
    "external/mh1903_lib/SCPU_USB_Lib/SCPU_USB_Device_Library/Class/msc/src/*.c"
    "external/mh1903_lib/SCPU_USB_Lib/SCPU_USB_Device_Library/Class/cdc/src/*.c"
    "external/mh1903_lib/SCPU_USB_Lib/SCPU_USB_OTG_Driver/src/*.c"
    "external/mh1903_lib/USB-ESL/CircularBuffer.c"    
)

set(TASKS
    src/tasks/ui_display_task.c
    src/tasks/background_task.c
    src/tasks/touchpad_task.c
    src/tasks/usb_task.c
    src/tasks/fingerprint_task.c
    src/tasks/background_app.c
)

file(GLOB_RECURSE SRC
    "src/*.c"
    "src/utils/*.c"
    "src/utils/log/*.c"
    "src/managers/*.c"
    "src/error_codes/*.c"
    "src/firmware/*.c"
    "src/msg/*.c"
    "src/ram/*.c"
    "src/hardware_interface/*.c"    
    "src/config/*"
)
    
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

set(CMAKE_EXE_LINKER_FLAGS " -T ${LINKER_SCRIPT} -Wl,-Map=mh1903.map -lm -mcpu=${MCU} --specs=nano.specs -specs=nosys.specs -nostartfiles -Wl,-Map=${PROJECT_BINARY_DIR}/${PROJECT_NAME}.map,--cref -Wl,--gc-sections ")
