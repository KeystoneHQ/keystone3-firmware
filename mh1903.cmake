

# search for program/library/include in the build host directories
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

# Target-specific flags
#型号
set(MCU_FAMILY cortex-m4)
#布局文件
set(LINKER_SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/gcc/mh1903b.ld)
#内核相关
set(CPU "-mcpu=cortex-m4")
set(FPU "")
set(FLOAT_ABI "")

add_definitions(-DUSE_STDPERIPH_DRIVER -DUSE_FULL_ASSERT -DLV_CONF_INCLUDE_SIMPLE -DLV_LVGL_H_INCLUDE_SIMPLE -DARM_MATH_CM4 -D__FPU_PRESENT="1" -D__TARGET_FPU_VFP)

# generate flags from user variables
if(CMAKE_BUILD_TYPE MATCHES Debug)
set(DBG_FLAGS "-g3 -gdwarf-2 -O0")
elseif(CMAKE_BUILD_TYPE MATCHES Release)
set(DBG_FLAGS "-O3")
endif()

set(MCU_FLAGS "-mcpu=cortex-m0plus -mthumb ${FPU} ${FLOAT_ABI}")
# compiler: language specific flags CFLAGS
set(CMAKE_C_FLAGS "${MCU_FLAGS} -std=gnu99 -Wall -fdata-sections -ffunction-sections ${DBG_FLAGS} " CACHE INTERNAL "C compiler flags")
#CPP 
set(CMAKE_CXX_FLAGS "${MCU_FLAGS} -fno-rtti -fno-exceptions -fno-builtin -Wall -fdata-sections -ffunction-sections ${DBG_FLAGS} " CACHE INTERNAL "Cxx compiler flags")
#ASFLAGS
set(CMAKE_ASM_FLAGS "${MCU_FLAGS} -x assembler-with-cpp ${DBG_FLAGS} " CACHE INTERNAL "ASM compiler flags")
#LDFLAGS -mcpu=cortex-m0plus -mthumb
set(CMAKE_EXE_LINKER_FLAGS "-Wl,-Map=$(OUTPUT).map -lm $(MCU) -mthumb -Wl,--gc-sections -nostartfiles -std=gnu99 --specs=nosys.specs -mfloat-abi=hard -mfpu=fpv4-sp-d16")
#要链接的库 对应makefile的 LIBS
#set(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS "" CACHE INTERNAL "Shared linker flags")
