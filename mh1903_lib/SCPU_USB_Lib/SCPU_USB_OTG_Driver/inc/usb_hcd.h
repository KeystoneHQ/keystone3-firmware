/************************ (C) COPYRIGHT Megahuntmicro *************************
 * File Name            : usb_hcd.h
 * Author               : Megahuntmicro
 * Version              : V1.0.0
 * Date                 : 21-October-2014
 * Description          : Host layer Header file.
 *****************************************************************************/


#ifndef __USB_HCD_H__
#define __USB_HCD_H__


#ifdef __cplusplus
extern "C" {
#endif

/* Include ------------------------------------------------------------------*/
#include "usb_core.h"
#include "usb_regs.h"
/* Exported types -----------------------------------------------------------*/
/* Exported constants -------------------------------------------------------*/
/* Exported macro -----------------------------------------------------------*/
/* Exported functions -------------------------------------------------------*/
/* Exported variables -------------------------------------------------------*/
/** @addtogroup USB_OTG_DRIVER
  * @{
  */

/** @defgroup USB_HCD
  * @brief This file is the
  * @{
  */


/** @defgroup USB_HCD_Exported_Defines
  * @{
  */
/**
  * @}
  */


/** @defgroup USB_HCD_Exported_Types
  * @{
  */
/**
  * @}
  */


/** @defgroup USB_HCD_Exported_Macros
  * @{
  */
/**
  * @}
  */

/** @defgroup USB_HCD_Exported_Variables
  * @{
  */
/**
  * @}
  */

/** @defgroup USB_HCD_Exported_FunctionsPrototype
  * @{
  */
uint32_t  HCD_Init(USB_OTG_CORE_HANDLE *pdev,
                   USB_OTG_CORE_ID_TypeDef coreID);
uint32_t  HCD_HC_Init(USB_OTG_CORE_HANDLE *pdev,
                      uint8_t hc_num);
uint32_t  HCD_SubmitRequest(USB_OTG_CORE_HANDLE *pdev,
                            uint8_t hc_num) ;
uint32_t  HCD_GetCurrentSpeed(USB_OTG_CORE_HANDLE *pdev);
uint32_t  HCD_ResetPort(USB_OTG_CORE_HANDLE *pdev);
uint32_t  HCD_IsDeviceConnected(USB_OTG_CORE_HANDLE *pdev);
uint16_t  HCD_GetCurrentFrame(USB_OTG_CORE_HANDLE *pdev) ;
URB_STATE HCD_GetURB_State(USB_OTG_CORE_HANDLE *pdev,  uint8_t ch_num);
uint32_t  HCD_GetXferCnt(USB_OTG_CORE_HANDLE *pdev,  uint8_t ch_num);
HC_STATUS HCD_GetHCState(USB_OTG_CORE_HANDLE *pdev,  uint8_t ch_num) ;
/**
  * @}
  */
#ifdef __cplusplus
}
#endif

#endif  /* __USB_HCD_H__ */

/************************ (C) COPYRIGHT 2014 Megahuntmicro ****END OF FILE****/
