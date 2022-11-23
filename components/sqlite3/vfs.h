
#ifndef __VFS_H__
#define __VFS_H__

#include "vfs_int.h"

// ---------------------------------------------------------------------------
// file functions
//

// vfs_read - read data from file
//   fd: file descriptor
//   ptr: destination data buffer
//   len: requested length
//   Returns: Number of bytes read, or VFS_RES_ERR in case of error
int32_t vfs_read(vfs_file *fd, void *ptr, size_t len);

// vfs_write - write data to file
//   fd: file descriptor
//   ptr: source data buffer
//   len: requested length
//   Returns: Number of bytes written, or VFS_RES_ERR in case of error
int32_t vfs_write(vfs_file *fd, const void *ptr, size_t len);

// vfs_lseek - move read/write pointer
//   fd: file descriptor
//   off: offset
//   whence: VFS_SEEK_SET - set pointer to off
//           VFS_SEEK_CUR - set pointer to current position + off
//           VFS_SEEK_END - set pointer to end of file + off
//   Returns: New position, or VFS_RES_ERR in case of error
int32_t vfs_lseek(vfs_file *fd, int32_t off);

// vfs_flush - flush write cache to file
//   fd: file descriptor
//   Returns: VFS_RES_OK, or VFS_RES_ERR in case of error
int32_t vfs_flush(vfs_file *fd);

// vfs_size - get current file size
//   fd: file descriptor
//   Returns: File size
uint32_t vfs_size(vfs_file *fd);

// ---------------------------------------------------------------------------
// dir functions
//


// vfs_open - open file
//   name: file name
//   mode: open mode
//   Returns: File descriptor, or NULL in case of error
vfs_file *vfs_open(const char *name, const char *mode);

// vfs_stat - stat file or directory
//   name: file or directory name
//   buf:  pre-allocated structure to be filled in
//   Returns: VFS_RES_OK, or VFS_RES_ERR in case of error
int32_t vfs_stat(const char *name, struct vfs_stat *buf);

// vfs_remove - remove file or directory
//   name: file or directory name
//   Returns: VFS_RES_OK, or VFS_RES_ERR in case of error
int32_t vfs_remove(const char *name);

// vfs_set_spiffs_file_obj - sets the file object before opening SPIFFS file
//    in_filep - pointer to existing file object
void vfs_set_spiffs_file_obj(void *in_filep);

#endif
