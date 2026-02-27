/************************ (C) COPYRIGHT Megahuntmicro *************************
 * File Name            : usbd_msc_mem.h
 * Author               : Megahuntmicro
 * Version              : V1.0.0
 * Date                 : 21-October-2014
 * Description          : header for the STORAGE DISK file file
 *****************************************************************************/


/* Define to prevent recursive inclusion -------------------------------------*/

#ifndef __USBD_MEM_H
#define __USBD_MEM_H
/* Includes ------------------------------------------------------------------*/
#include "usbd_def.h"
#include "usbd_msc_core.h"

/** @defgroup USBD_MEM
  * @brief header file for the storage disk file
  * @{
  */

/** @defgroup USBD_MEM_Exported_Defines
  * @{
  */
#define USBD_STD_INQUIRY_LENGTH     36
/**
  * @}
  */


/** @defgroup USBD_MEM_Exported_TypesDefinitions
  * @{
  */

typedef struct _USBD_STORAGE {
    int8_t (* Init)(uint8_t lun);
    int8_t (* GetCapacity)(uint8_t lun, uint32_t *block_num, uint32_t *block_size);
    int8_t (* IsReady)(uint8_t lun);
    int8_t (* IsWriteProtected)(uint8_t lun);
    int8_t (* Read)(uint8_t lun, uint8_t *buf, uint32_t blk_addr, uint16_t blk_len);
    int8_t (* Write)(uint8_t lun, uint8_t *buf, uint32_t blk_addr, uint16_t blk_len);
    int8_t (* GetMaxLun)(void);
    int8_t *pInquiry;

} USBD_STORAGE_cb_TypeDef;
/**
  * @}
  */



/** @defgroup USBD_MEM_Exported_Macros
  * @{
  */

/**
  * @}
  */

/** @defgroup USBD_MEM_Exported_Variables
  * @{
  */

/**
  * @}
  */

/** @defgroup USBD_MEM_Exported_FunctionsPrototype
  * @{
  */
extern USBD_STORAGE_cb_TypeDef *USBD_STORAGE_fops;
/**
  * @}
  */

#endif /* __USBD_MEM_H */
/**
  * @}
  */

/**
  * @}
  */

/**
* @}
*/

/************************ (C) COPYRIGHT 2014 Megahuntmicro ****END OF FILE****/
