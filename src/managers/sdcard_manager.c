#include "sdcard_manager.h"
#include "user_memory.h"

#ifndef COMPILE_SIMULATOR
#include "user_fatfs.h"
#include "safe_str_lib.h"
#define SD_ROOT "0:/"
#else
#include "simulator_model.h"
#include "simulator_mock_define.h"

#define SD_ROOT "C:/assets/sd/"
#endif

#define MAX_FILENAME_LEN 128

bool FileExists(char *filename)
{
    char *target = SRAM_MALLOC(MAX_FILENAME_LEN);
    snprintf_s(target, MAX_FILENAME_LEN, "%s%s", SD_ROOT, filename);
    return FatfsFileExist(target);
}

int WriteFile(const char *filename, const uint8_t *content, uint32_t len)
{
    char *path = SRAM_MALLOC(MAX_FILENAME_LEN);
    snprintf_s(path, MAX_FILENAME_LEN, "%s%s", SD_ROOT, filename);
    int ret = FatfsFileWrite(path, content, len);
    return ret;
}
