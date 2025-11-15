#include "sdcard_manager.h"
#include "user_memory.h"

#ifndef COMPILE_SIMULATOR
#include "user_fatfs.h"
#define SD_ROOT "0:/"
#else
#include "simulator_model.h"
#define SD_ROOT "C:/assets/sd/"
#endif

#define MAX_FILENAME_LEN 128

int FileWrite(const char *filename, const uint8_t *content, uint32_t len)
{
    char path[MAX_FILENAME_LEN] = {0};
    snprintf_s(path, MAX_FILENAME_LEN, "%s%s", SD_ROOT, filename);
    return FatfsFileWrite((const char*)path, content, len);
}
