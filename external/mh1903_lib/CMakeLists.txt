file(GLOB driver_src MHSCPU_Driver/src/*.*)

include_directories(MHSCPU_Driver/inc)
add_library(mh1903_driver STATIC ${driver_src})
