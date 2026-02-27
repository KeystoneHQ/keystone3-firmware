/************************ (C) COPYRIGHT Megahuntmicro *************************
 * File Name            : usbh_hcs.h
 * Author               : Megahuntmicro
 * Version              : V1.0.0
 * Date                 : 21-October-2014
 * Description          : Header file for usbh_hcs.c.
 *****************************************************************************/


#ifndef __USBH_HCS_H
#define __USBH_HCS_H


#ifdef __cplusplus
extern "C" {
#endif

/* Include ------------------------------------------------------------------*/
#include "usbh_core.h"
/* Exported types -----------------------------------------------------------*/
/* Exported constants -------------------------------------------------------*/
/* Exported macro -----------------------------------------------------------*/
/* Exported functions -------------------------------------------------------*/
/* Exported variables -------------------------------------------------------*/
/** @addtogroup USBH_LIB
  * @{
  */

/** @addtogroup USBH_LIB_CORE
* @{
*/

/** @defgroup USBH_HCS
  * @brief This file is the header file for usbh_hcs.c
  * @{
  */

/** @defgroup USBH_HCS_Exported_Defines
  * @{
  */
#define HC_MAX           8

#define HC_OK            0x0000
#define HC_USED          0x8000
#define HC_ERROR         0xFFFF
#define HC_USED_MASK     0x7FFF
/**
  * @}
  */

/** @defgroup USBH_HCS_Exported_Types
  * @{
  */
/**
  * @}
  */


/** @defgroup USBH_HCS_Exported_Macros
  * @{
  */
/**
  * @}
  */

/** @defgroup USBH_HCS_Exported_Variables
  * @{
  */
/**
  * @}
  */

/** @defgroup USBH_HCS_Exported_FunctionsPrototype
  * @{
  */

uint8_t USBH_Alloc_Channel(USB_OTG_CORE_HANDLE *pdev, uint8_t ep_addr);

uint8_t USBH_Free_Channel(USB_OTG_CORE_HANDLE *pdev, uint8_t idx);

uint8_t USBH_DeAllocate_AllChannel(USB_OTG_CORE_HANDLE *pdev);

uint8_t USBH_Open_Channel(USB_OTG_CORE_HANDLE *pdev,
                          uint8_t ch_num,
                          uint8_t dev_address,
                          uint8_t speed,
                          uint8_t ep_type,
                          uint16_t mps);

uint8_t USBH_Modify_Channel(USB_OTG_CORE_HANDLE *pdev,
                            uint8_t hc_num,
                            uint8_t dev_address,
                            uint8_t speed,
                            uint8_t ep_type,
                            uint16_t mps);
/**
  * @}
  */

#ifdef __cplusplus
}
#endif

#endif  /* __USBH_HCS_H */

/************************ (C) COPYRIGHT 2014 Megahuntmicro ****END OF FILE****/
