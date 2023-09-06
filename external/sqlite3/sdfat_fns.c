#include "user_memory.h"
#include "err_code.h"
#include "sdfat_fns.h"
#include "user_fatfs.h"
#include "vfs_int.h"

static FIL g_sdFatFs;

vfs_vol *sdfat_mount(const char *name, int num)
{
    return NULL;
}

vfs_file *sdfat_open(const char *name, const char *mode)
{
    vfs_file *ret = NULL;

    ret = (vfs_file *)SRAM_MALLOC(sizeof(vfs_file));
    ret->fs_type = VFS_FS_FATFS;
    ret->file_obj = NULL;
    FRESULT res = f_open(&g_sdFatFs, name, FA_READ);
    if (res) {
        return NULL;
    }
    ret->file_obj = SRAM_MALLOC(sizeof(g_sdFatFs));
    memcpy(ret->file_obj, &g_sdFatFs, sizeof(g_sdFatFs));
    return ret;
}

int32_t sdfat_remove(const char *name)
{
    return ERR_SQLITE3_FAILED;
}

int32_t sdfat_exists(const char *name)
{
    return ERR_SQLITE3_FAILED;
}

int32_t sdfat_close(vfs_file *fd)
{
    SRAM_FREE(fd->file_obj);
    SRAM_FREE(fd);
    if (FR_OK == f_close(&g_sdFatFs)) {
        return SUCCESS_CODE;
    }

    return ERR_SQLITE3_FAILED;
}

int32_t sdfat_read(vfs_file *fd, void *ptr, size_t len)
{
    uint32_t readBytes = 0;
    FRESULT res = f_read(&g_sdFatFs, (void*)ptr, len, &readBytes);
    if (res) {
        return ERR_SQLITE3_FAILED;
    }
    return readBytes;
}

int32_t sdfat_write(vfs_file *fd, const void *ptr, size_t len)
{
    return 0;
}

int32_t sdfat_lseek(vfs_file *fd, int32_t off)
{
    FRESULT res = f_lseek(&g_sdFatFs, off);
    if (res) {
        return ERR_SQLITE3_FAILED;
    }

    return SUCCESS_CODE;
}

int32_t sdfat_flush(vfs_file *fd)
{
    return f_sync(&g_sdFatFs);
}

uint32_t sdfat_size(vfs_file *fd)
{
    uint32_t fileSize = f_size(&g_sdFatFs);
    return fileSize;
}
