/**************************************************************************************************
 * Copyright (c) keyst.one. 2020-2025. All rights reserved.
 * Description: FLASH address define.
 * Author: leon sun
 * Create: 2023-4-11
 ************************************************************************************************/

#ifndef _FLASH_ADDRESS_H
#define _FLASH_ADDRESS_H


//SPI FLASH ADDR

//USB FATFS, 8MB
#define SPI_FLASH_ADDR_USB_FATFS                0x00000000
#define SPI_FLASH_SIZE_USB_FATFS                0x800000

//LOG, 500KB
#define SPI_FLASH_ADDR_LOG                      0x00800000
#define SPI_FLASH_SIZE_LOG                      0x80000

//USER IMAGE, 480KB
#define SPI_FLASH_ADDR_USER_IMAGE               0x00F3A000
#define SPI_FLASH_SIZE_USER_IMAGE               0x78000

#define SPI_FLASH_ADDR_EACH_SIZE                0x19000

//USER1 DATA, 100KB
#define SPI_FLASH_ADDR_USER1_DATA               0x00FB2000
#define SPI_FLASH_SIZE_USER1_DATA               0x14000

#define SPI_FLASH_ADDR_USER1_MUTABLE_DATA       0x00FC6000
#define SPI_FLASH_SIZE_USER1_MUTABLE_DATA       0x5000

//USER2 DATA, 100KB
#define SPI_FLASH_ADDR_USER2_DATA               0x00FCB000
#define SPI_FLASH_SIZE_USER2_DATA               0x14000

#define SPI_FLASH_ADDR_USER2_MUTABLE_DATA       0x00FDF000
#define SPI_FLASH_SIZE_USER2_MUTABLE_DATA       0x5000

//USER3 DATA, 100KB
#define SPI_FLASH_ADDR_USER3_DATA               0x00FE4000
#define SPI_FLASH_SIZE_USER3_DATA               0x14000

#define SPI_FLASH_ADDR_USER3_MUTABLE_DATA       0x00FF8000
#define SPI_FLASH_SIZE_USER3_MUTABLE_DATA       0x5000

//BATTERY INFO, 4KB
#define SPI_FLASH_ADDR_BATTERY_INFO             0x00FFD000
#define SPI_FLASH_SIZE_BATTERY_INFO             0x1000

//NORMAL PARAM, 4KB
#define SPI_FLASH_ADDR_NORMAL_PARAM             0x00FFE000
#define SPI_FLASH_SIZE_NORMAL_PARAM             0x1000

//PROTECT PARAM, 4KB
#define SPI_FLASH_ADDR_PROTECT_PARAM            0x00FFF000
#define SPI_FLASH_SIZE_PROTECT_PARAM            0x1000


#endif

