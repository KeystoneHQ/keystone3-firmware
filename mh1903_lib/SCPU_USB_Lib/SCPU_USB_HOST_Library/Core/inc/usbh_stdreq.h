/************************ (C) COPYRIGHT Megahuntmicro *************************
 * File Name            : usbh_stdreq.h
 * Author               : Megahuntmicro
 * Version              : V1.0.0
 * Date                 : 21-October-2014
 * Description          : Header file for usbh_stdreq.c.
 *****************************************************************************/


#ifndef __USBH_STDREQ_H
#define __USBH_STDREQ_H


#ifdef __cplusplus
extern "C" {
#endif

/* Include ------------------------------------------------------------------*/
#include "usb_conf.h"
#include "usb_hcd.h"
#include "usbh_core.h"
#include "usbh_def.h"
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

/** @defgroup USBH_STDREQ
  * @brief This file is the
  * @{
  */


/** @defgroup USBH_STDREQ_Exported_Defines
  * @{
  */
/*Standard Feature Selector for clear feature command*/
#define FEATURE_SELECTOR_ENDPOINT         0X00
#define FEATURE_SELECTOR_DEVICE           0X01


#define INTERFACE_DESC_TYPE               0x04
#define ENDPOINT_DESC_TYPE                0x05
#define INTERFACE_DESC_SIZE               0x09


#define USBH_HID_CLASS                    0x03

/**
  * @}
  */


/** @defgroup USBH_STDREQ_Exported_Types
  * @{
  */
/**
  * @}
  */


/** @defgroup USBH_STDREQ_Exported_Macros
  * @{
  */
/**
  * @}
  */

/** @defgroup USBH_STDREQ_Exported_Variables
  * @{
  */
extern uint8_t USBH_CfgDesc[512];
/**
  * @}
  */

/** @defgroup USBH_STDREQ_Exported_FunctionsPrototype
  * @{
  */
USBH_Status USBH_GetDescriptor(USB_OTG_CORE_HANDLE *pdev,
                               USBH_HOST           *phost,
                               uint8_t  req_type,
                               uint16_t value_idx,
                               uint8_t* buff,
                               uint16_t length);

USBH_Status USBH_Get_DevDesc(USB_OTG_CORE_HANDLE *pdev,
                             USBH_HOST *phost,
                             uint8_t length);

USBH_Status USBH_Get_StringDesc(USB_OTG_CORE_HANDLE *pdev,
                                USBH_HOST           *phost,
                                uint8_t string_index,
                                uint8_t *buff,
                                uint16_t length);

USBH_Status USBH_SetCfg(USB_OTG_CORE_HANDLE *pdev,
                        USBH_HOST *phost,
                        uint16_t configuration_value);

USBH_Status USBH_Get_CfgDesc(USB_OTG_CORE_HANDLE *pdev,
                             USBH_HOST           *phost,
                             uint16_t length);

USBH_Status USBH_SetAddress(USB_OTG_CORE_HANDLE *pdev,
                            USBH_HOST           *phost,
                            uint8_t DeviceAddress);

USBH_Status USBH_ClrFeature(USB_OTG_CORE_HANDLE *pdev,
                            USBH_HOST           *phost,
                            uint8_t ep_num, uint8_t hc_num);

USBH_Status USBH_SetInterface(USB_OTG_CORE_HANDLE *pdev,
                              USBH_HOST *phost,
                              uint8_t ep_num, uint8_t altSetting);

USBH_Status USBH_Issue_ClrFeature(USB_OTG_CORE_HANDLE *pdev,
                                  USBH_HOST           *phost,
                                  uint8_t ep_num);

USBH_DescHeader_t      *USBH_GetNextDesc(uint8_t   *pbuf,
        uint16_t  *ptr);
/**
  * @}
  */

#ifdef __cplusplus
}
#endif

#endif  /* __USBH_STDREQ_H */
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
