/*********************************************************************
 * Copyright (c) keyst.one. 2020-2025. All rights reserved.
 * name       : user_fatfs.h
 * Description:
 * author     : stone wang
 * data       : 2022-12-16 11:06
**********************************************************************/

#ifndef _USER_FATFS_H
#define _USER_FATFS_H

#include "diskio.h"
#include "ff.h"

int MMC_disk_initialize(void);
int MMC_disk_write(
    const BYTE *buff,   /* Data to be written */
    LBA_t sector,       /* Start sector in LBA */
    UINT count          /* Number of sectors to write */
);
int MMC_disk_read(
    BYTE *buff,     /* Data buffer to store read data */
    LBA_t sector,   /* Start sector in LBA */
    UINT count      /* Number of sectors to read */
);

int USB_disk_initialize(void);
int USB_disk_read(
    BYTE *buff,     /* Data buffer to store read data */
    LBA_t sector,   /* Start sector in LBA */
    UINT count      /* Number of sectors to read */
);
int USB_disk_write(
    const BYTE *buff, /* Data to be written */
    LBA_t sector,       /* Start sector in LBA */
    UINT count          /* Number of sectors to write */
);

int FatfsMount(void);
int MountUsbFatfs(void);
int MountSdFatfs(void);
int UnMountSdFatfs(void);
void FatfsShowVolumeStatus(char *ptr);
void FatfsDirectoryListing(char *ptr);
void CopyToFlash(void);
int FatfsCatFile(const TCHAR* path);
int FatfsFileMd5(const TCHAR* path);
int FatfsFileWrite(const TCHAR* path, const uint8_t *data, uint32_t len);
int FatfsFileCreate(const TCHAR* path);
int FatfsFileAppend(const TCHAR* path, const uint8_t *data, uint32_t len);
int FatfsFileDelete(const TCHAR* path);
int FatfsFileCopy(const TCHAR* source, const TCHAR* dest);
uint32_t FatfsFileGetSize(const TCHAR *path);
void FatfsError(FRESULT errNum);
uint32_t FatfsGetSize(const char *path);

#endif /* _USER_FATFS_H */

