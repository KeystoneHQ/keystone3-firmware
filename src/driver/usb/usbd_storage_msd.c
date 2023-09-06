/************************ (C) COPYRIGHT Megahuntmicro *************************
 * File Name            : usbd_storage_msd.c
 * Author               : Megahuntmicro
 * Version              : V1.0.0
 * Date                 : 21-October-2014
 * Description          : This file provides the disk operations functions.
 *****************************************************************************/
#include "usbd_msc_mem.h"
#include "drv_gd25qxx.h"

#define STORAGE_LUN_NBR 1

/* USB Mass storage Standard Inquiry Data */
const int8_t STORAGE_Inquirydata[] = {
    0x00, 0x80, 0x02, 0x02, 0x1F, 0x00, 0x00, 0x00,       //
    'K',  'e',  'y',  'S',  't',  'o',  'n',  'e',        //
    'D',  'i',  's',  'k',              //
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //
    '1',  '.',  '0',  0x00,
};

static int8_t STORAGE_Init(uint8_t lun);
static int8_t STORAGE_GetCapacity(uint8_t lun, uint32_t* block_num, uint32_t* block_size);
static int8_t STORAGE_IsReady(uint8_t lun);
static int8_t STORAGE_IsWriteProtected(uint8_t lun);
static int8_t STORAGE_Read(uint8_t lun, uint8_t* buf, uint32_t blk_addr, uint16_t blk_len);
static int8_t STORAGE_Write(uint8_t lun, uint8_t* buf, uint32_t blk_addr, uint16_t blk_len);
static int8_t STORAGE_GetMaxLun(void);

USBD_STORAGE_cb_TypeDef USBD_VirtualFatFS_fops = {
    STORAGE_Init,        //
    STORAGE_GetCapacity, //
    STORAGE_IsReady,     //
    STORAGE_IsWriteProtected,
    STORAGE_Read,
    STORAGE_Write,
    STORAGE_GetMaxLun,
    (int8_t*)STORAGE_Inquirydata,
};

USBD_STORAGE_cb_TypeDef* USBD_STORAGE_fops = &USBD_VirtualFatFS_fops;

/**
 * @brief  Initialize the storage medium
 * @param  lun : logical unit number
 * @retval Status
 */
int8_t STORAGE_Init(uint8_t lun)
{
//    vfs_init("MH-USB-Disk", MB(8));
    return 0;
}

/**
 * @brief  return medium capacity and block size
 * @param  lun : logical unit number
 * @param  block_num :  number of physical block
 * @param  block_size : size of a physical block
 * @retval Status
 */
int8_t STORAGE_GetCapacity(uint8_t lun, uint32_t* block_num, uint32_t* block_size)
{
    *block_size = MSC_MEDIA_PACKET;
    *block_num  = GD25QXX_SECTOR_NUM / 2;
    return (0);
}

/**
 * @brief  check whether the medium is ready
 * @param  lun : logical unit number
 * @retval Status
 */
int8_t STORAGE_IsReady(uint8_t lun)
{
    return (0);
}

/**
 * @brief  check whether the medium is write-protected
 * @param  lun : logical unit number
 * @retval Status
 */
int8_t STORAGE_IsWriteProtected(uint8_t lun)
{
    return 0;
}

/**
 * @brief  Read data from the medium
 * @param  lun : logical unit number
 * @param  buf : Pointer to the buffer to save data
 * @param  blk_addr :  address of 1st block to be read
 * @param  blk_len : nmber of blocks to be read
 * @retval Status
 */

int8_t STORAGE_Read(uint8_t lun, uint8_t* buffer, uint32_t block_number, uint16_t count)
{
    Gd25FlashReadBuffer(block_number * MSC_MEDIA_PACKET, buffer, MSC_MEDIA_PACKET * count);
    return 0;
}

/**
 * @brief  Write data to the medium
 * @param  lun : logical unit number
 * @param  buf : Pointer to the buffer to write from
 * @param  blk_addr :  address of 1st block to be written
 * @param  blk_len : nmber of blocks to be read
 * @retval Status
 */
int8_t STORAGE_Write(uint8_t lun, uint8_t* buffer, uint32_t block_number, uint16_t count)
{
    Gd25FlashSectorErase(block_number * MSC_MEDIA_PACKET);
    Gd25FlashWriteBuffer(block_number * MSC_MEDIA_PACKET, buffer, count * MSC_MEDIA_PACKET);
    return 0;
}

/**
 * @brief  Return number of supported logical unit
 * @param  None
 * @retval number of logical unit
 */
int8_t STORAGE_GetMaxLun(void)
{
    return (STORAGE_LUN_NBR - 1);
}
