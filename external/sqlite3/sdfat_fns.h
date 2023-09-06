#ifndef __SDFAT_FNS_H__
#define __SDFAT_FNS_H__

#include "vfs_int.h"

vfs_vol *sdfat_mount(const char *name, int num);
vfs_file *sdfat_open(const char *name, const char *mode);
int32_t sdfat_remove(const char *name);
int32_t sdfat_exists(const char *name);
int32_t sdfat_close(vfs_file *fd);
int32_t sdfat_read(vfs_file *fd, void *ptr, size_t len);
int32_t sdfat_write(vfs_file *fd, const void *ptr, size_t len);
int32_t sdfat_lseek(vfs_file *fd, int32_t off);
int32_t sdfat_flush(vfs_file *fd);
uint32_t sdfat_size(vfs_file *fd);

#endif
