/*********************************************************************
 * Copyright (c) keyst.one. 2020-2025. All rights reserved.
 * name       : drv_gd25qxx.h
 * Description: gd25qxx spi flash driver
 * author     : stone wang
 * data       : 2022-12-07 11:54
**********************************************************************/

#ifndef _DRV_GD25QXX_H
#define _DRV_GD25QXX_H

#define GD25QXX_FLASH_BASE                (0x0UL)
#define GD25QXX_FLASH_SIZE                (0x1000000)
#define GD25QXX_PAGE_SIZE                 (0x100)
#define GD25QXX_SECTOR_SIZE               (0x1000)
#define GD25QXX_PAGE_NUM                  (GD25QXX_FLASH_SIZE / GD25QXX_PAGE_SIZE)
#define GD25QXX_SECTOR_NUM                (GD25QXX_FLASH_SIZE / GD25QXX_SECTOR_SIZE)

#define GD25QXX_CMD_WRITE_ENABLE          0x06
#define GD25QXX_CMD_WRITE_DISABLE         0x04
#define GD25QXX_CMD_READ_IDENTIFICATION   0x9F
#define GD25QXX_CMD_READ_STATUS           0x05
#define GD25QXX_CMD_READ_DATA             0x03
#define GD25QXX_CMD_FAST_READ_DATA        0x0B
#define GD25QXX_CMD_PAGE_PROGRAM          0x02
#define GD25QXX_CMD_CHIP_ERASE            0xC7
#define GD25QXX_CMD_SECTOR_ERASE          0x20
#define GD25QXX_CMD_BLOCK_ERASE           0xD8
#define GD25QXX_CMD_POWER_DOWN            0xB9
#define GD25QXX_CMD_RELEASE_POWER_DOWN    0xAB

#define Dummy_Byte                        0xA5

#define GD25QXX_FLASH_STATUS_WIP          (1UL << 0)
#define GD25QXX_FLASH_STATUS_WEL          (1UL << 1)

typedef enum {
    GD25_FLASH_ERASE = 0,
    GD25_FLASH_READ,
    GD25_FLASH_WRITE,

    GD25_FLASH_OPEARTE_BUTT,
} Gd25Flash_Operate;

void Gd25FlashInit(void);
void Gd25FlashOpen(void);
uint32_t Gd25FlashReadID(void);
int32_t Gd25FlashSectorErase(uint32_t addr);
int32_t Gd25FlashChipErase(void);
int32_t Gd25FlashBlockErase(uint32_t addr);
int32_t Gd25FlashReadBuffer(uint32_t addr, uint8_t *buffer, uint32_t size);
int32_t Gd25FlashWriteBuffer(uint32_t addr, const uint8_t *buffer, uint32_t size);

#endif /* _DRV_GD25QXX_H */

