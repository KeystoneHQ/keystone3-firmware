/************************ (C) COPYRIGHT Megahuntmicro *************************
 * File Name            : usbd_core.h
 * Author               : Megahuntmicro
 * Version              : V1.0.0
 * Date                 : 21-October-2014
 * Description          : Header file for usbd_core.c.
 *****************************************************************************/


#ifndef __USBD_CORE_H
#define __USBD_CORE_H



#ifdef __cplusplus
extern "C" {
#endif


/* Includes ------------------------------------------------------------------*/
#include "usb_dcd.h"
#include "usbd_def.h"
#include "usbd_conf.h"


/** @defgroup USBD_CORE
  * @brief This file is the Header file for usbd_core.c file
  * @{
  */


/** @defgroup USBD_CORE_Exported_Defines
  * @{
  */
typedef enum {
    USBD_OK   = 0,
    USBD_BUSY,
    USBD_FAIL,
} USBD_Status;
/**
  * @}
  */

/** @defgroup USBD_CORE_Exported_FunctionsPrototype
  * @{
  */
void USBD_Init(USB_OTG_CORE_HANDLE *pdev,
               USB_OTG_CORE_ID_TypeDef coreID,
               USBD_DEVICE *pDevice,
               USBD_Class_cb_TypeDef *class_cb,
               USBD_Usr_cb_TypeDef *usr_cb);

USBD_Status USBD_DeInit(USB_OTG_CORE_HANDLE *pdev);

USBD_Status USBD_ClrCfg(USB_OTG_CORE_HANDLE  *pdev, uint8_t cfgidx);

USBD_Status USBD_SetCfg(USB_OTG_CORE_HANDLE  *pdev, uint8_t cfgidx);

/**
  * @}
  */

#endif /* __USBD_CORE_H */

/************************ (C) COPYRIGHT 2014 Megahuntmicro ****END OF FILE****/
