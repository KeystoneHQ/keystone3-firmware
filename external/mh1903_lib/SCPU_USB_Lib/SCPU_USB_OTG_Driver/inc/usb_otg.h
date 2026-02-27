/************************ (C) COPYRIGHT Megahuntmicro *************************
 * File Name            : usb_otg.h
 * Author               : Megahuntmicro
 * Version              : V1.0.0
 * Date                 : 21-October-2014
 * Description          : OTG Core Header.
 *****************************************************************************/


#ifndef __USB_OTG_H__
#define __USB_OTG_H__


#ifdef __cplusplus
extern "C" {
#endif

/* Include ------------------------------------------------------------------*/
#include "usb_regs.h"
#include "usb_core.h"
/* Exported types -----------------------------------------------------------*/
/* Exported constants -------------------------------------------------------*/
/* Exported macro -----------------------------------------------------------*/
/* Exported functions -------------------------------------------------------*/
/* Exported variables -------------------------------------------------------*/
/** @addtogroup USB_OTG_DRIVER
  * @{
  */

/** @defgroup USB_OTG
  * @brief This file is the
  * @{
  */


/** @defgroup USB_OTG_Exported_Defines
  * @{
  */


void USB_OTG_InitiateSRP(USB_OTG_CORE_HANDLE *pdev);
void USB_OTG_InitiateHNP(USB_OTG_CORE_HANDLE *pdev, uint8_t state, uint8_t mode);
void USB_OTG_Switchback(USB_OTG_CORE_HANDLE *pdev);
uint32_t  USB_OTG_GetCurrentState(USB_OTG_CORE_HANDLE *pdev);

/**
  * @}
  */


/** @defgroup USB_OTG_Exported_Types
  * @{
  */
/**
  * @}
  */


/** @defgroup USB_OTG_Exported_Macros
  * @{
  */
/**
  * @}
  */

/** @defgroup USB_OTG_Exported_Variables
  * @{
  */
/**
  * @}
  */

/** @defgroup USB_OTG_Exported_FunctionsPrototype
  * @{
  */
/**
  * @}
  */
#ifdef __cplusplus
}
#endif

#endif  /* __USB_OTG_H__ */

/************************ (C) COPYRIGHT 2014 Megahuntmicro ****END OF FILE****/
