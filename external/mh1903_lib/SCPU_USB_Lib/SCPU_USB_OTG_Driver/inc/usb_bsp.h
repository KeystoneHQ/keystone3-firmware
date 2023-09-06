/************************ (C) COPYRIGHT Megahuntmicro *************************
 * File Name            : usb_bsp.h
 * Author               : Megahuntmicro
 * Version              : V1.0.0
 * Date                 : 21-October-2014
 * Description          : Specific api's relative to the used hardware platform.
 *****************************************************************************/


#ifndef __USB_BSP_H__
#define __USB_BSP_H__


#ifdef __cplusplus
extern "C" {
#endif

/* Include ------------------------------------------------------------------*/
#include "usb_core.h"
#include "usb_conf.h"
/* Exported types -----------------------------------------------------------*/
/* Exported constants -------------------------------------------------------*/
/* Exported macro -----------------------------------------------------------*/
/* Exported functions -------------------------------------------------------*/
/* Exported variables -------------------------------------------------------*/
/** @addtogroup USB_OTG_DRIVER
  * @{
  */

/** @defgroup USB_BSP
  * @brief This file is the
  * @{
  */


/** @defgroup USB_BSP_Exported_Defines
  * @{
  */
/**
  * @}
  */


/** @defgroup USB_BSP_Exported_Types
  * @{
  */
/**
  * @}
  */


/** @defgroup USB_BSP_Exported_Macros
  * @{
  */
/**
  * @}
  */

/** @defgroup USB_BSP_Exported_Variables
  * @{
  */
/**
  * @}
  */

/** @defgroup USB_BSP_Exported_FunctionsPrototype
  * @{
  */
void BSP_Init(void);

void USB_OTG_BSP_Init(USB_OTG_CORE_HANDLE *pdev);
void USB_OTG_BSP_uDelay(const uint32_t usec);
void USB_OTG_BSP_mDelay(const uint32_t msec);
void USB_OTG_BSP_EnableInterrupt(USB_OTG_CORE_HANDLE *pdev);
#ifdef USE_HOST_MODE
void USB_OTG_BSP_ConfigVBUS(USB_OTG_CORE_HANDLE *pdev);
void USB_OTG_BSP_DriveVBUS(USB_OTG_CORE_HANDLE *pdev, uint8_t state);
void USBID_detc_CMD(FunctionalState NewState);
uint8_t USBID_status(void);
void USB_vbus_CMD(USB_OTG_CORE_HANDLE *pdev, FunctionalState NewState);
uint8_t Get_Vbus_status(USB_OTG_CORE_HANDLE *pdev);

#endif
/**
  * @}
  */

#ifdef __cplusplus
}
#endif

#endif  /* __USB_BSP_H__ */

/************************ (C) COPYRIGHT 2014 Megahuntmicro ****END OF FILE****/
