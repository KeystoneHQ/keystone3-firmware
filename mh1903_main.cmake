add_definitions(-DUSE_STDPERIPH_DRIVER -DUSE_FULL_ASSERT -DLV_CONF_INCLUDE_SIMPLE -DLV_LVGL_H_INCLUDE_SIMPLE -DARM_MATH_CM4 -D__FPU_PRESENT="1" -D__TARGET_FPU_VFP)

include_directories(
    Inc 
    core 
    drv 
    cm_backtrace
    mh1903_lib/MHSCPU_Driver/inc 
    mh1903_lib/Device/MegaHunt/mhscpu/Include 
    mh1903_lib/CMSIS/Include 
    mh1903_lib/MHSCPU_Driver/inc/cryptlib 
    mh1903_lib/MHSCPU_Driver/inc/emvlib 
    mid/utils
    ${CMAKE_CURRENT_BINARY_DIR}
)

file(GLOB_RECURSE SRC_DIR_LIST "mh1903_lib/MHSCPU_Driver/src/*.c" "drv/*.c" "core/*.c")
message(STATUS ${SRC_DIR_LIST})

set(mh1903_driver 
mh1903_lib/MHSCPU_Driver/src/misc.c
mh1903_lib/MHSCPU_Driver/src/mhscpu_gpio.c
mh1903_lib/MHSCPU_Driver/src/mhscpu_uart.c
mh1903_lib/MHSCPU_Driver/src/mhscpu_sysctrl.c
mh1903_lib/Device/MegaHunt/mhscpu/Source/GCC/startup_mhscpu.s
)
