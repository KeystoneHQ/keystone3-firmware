#include "stdlib.h"
#include "stdio.h"
#include "vfs.h"
#include "sdfat_fns.h"
#include "user_fatfs.h"
#include "err_code.h"

#define LDRV_TRAVERSAL 0

vfs_file *vfs_open(const char *name, const char *mode)
{
    vfs_file *ret = NULL;
    if (memcmp(name, "0:", 2) == 0) {
        ret = sdfat_open(name, mode);
    }
    return ret;
}


int32_t vfs_stat(const char *name, struct vfs_stat *buf)
{
    strcpy(buf->name, name);
    if (memcmp(name, "0:", 2) == 0) {
        return sdfat_exists(name);
    }
    return ERR_SQLITE3_FAILED;
}

int32_t vfs_remove(const char *name)
{
    if (memcmp(name, "0:", 2) == 0) {
        return sdfat_remove(name);
    }
    return ERR_SQLITE3_FAILED;
}

int32_t vfs_read(vfs_file *fd, void *ptr, size_t len)
{
    if (fd->fs_type == VFS_FS_FATFS) {
        return sdfat_read(fd, ptr, len);
    }
    return ERR_SQLITE3_FAILED;
}

int32_t vfs_write(vfs_file *fd, const void *ptr, size_t len)
{
    if (fd->fs_type == VFS_FS_FATFS) {
        return sdfat_write(fd, ptr, len);
    }
    return ERR_SQLITE3_FAILED;
}

int32_t vfs_lseek(vfs_file *fd, int32_t off)
{
    if (fd->fs_type == VFS_FS_FATFS) {
        return sdfat_lseek(fd, off);
    }
    return ERR_SQLITE3_FAILED;
}

int32_t vfs_flush(vfs_file *fd)
{
    if (fd->fs_type == VFS_FS_FATFS) {
        return sdfat_flush(fd);
    }
    return ERR_SQLITE3_FAILED;
}

uint32_t vfs_size(vfs_file *fd)
{
    if (fd->fs_type == VFS_FS_FATFS) {
        return sdfat_size(fd);
    }
    return 0;
}
