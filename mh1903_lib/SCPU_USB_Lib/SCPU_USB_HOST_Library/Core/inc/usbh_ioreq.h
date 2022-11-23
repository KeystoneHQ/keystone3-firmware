/************************ (C) COPYRIGHT Megahuntmicro *************************
 * File Name            : usbh_ioreq.h
 * Author               : Megahuntmicro
 * Version              : V1.0.0
 * Date                 : 21-October-2014
 * Description          : Header file for usbh_ioreq.c.
 *****************************************************************************/


#ifndef __USBH_IOREQ_H
#define __USBH_IOREQ_H


#ifdef __cplusplus
extern "C" {
#endif

/* Include ------------------------------------------------------------------*/
#include "usb_conf.h"
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

/** @defgroup USBH_IOREQ
  * @brief This file is the header file for usbh_ioreq.c
  * @{
  */


/** @defgroup USBH_IOREQ_Exported_Defines
  * @{
  */
#define USBH_SETUP_PKT_SIZE   8
#define USBH_EP0_EP_NUM       0
#define USBH_MAX_PACKET_SIZE  0x40
/**
  * @}
  */


/** @defgroup USBH_IOREQ_Exported_Types
  * @{
  */
/**
  * @}
  */


/** @defgroup USBH_IOREQ_Exported_Macros
  * @{
  */
/**
  * @}
  */

/** @defgroup USBH_IOREQ_Exported_Variables
  * @{
  */
/**
  * @}
  */

/** @defgroup USBH_IOREQ_Exported_FunctionsPrototype
  * @{
  */
USBH_Status USBH_CtlSendSetup(USB_OTG_CORE_HANDLE *pdev,
                              uint8_t *buff,
                              uint8_t hc_num);

USBH_Status USBH_CtlSendData(USB_OTG_CORE_HANDLE *pdev,
                             uint8_t *buff,
                             uint16_t length,
                             uint8_t hc_num);

USBH_Status USBH_CtlReceiveData(USB_OTG_CORE_HANDLE *pdev,
                                uint8_t *buff,
                                uint16_t length,
                                uint8_t hc_num);

USBH_Status USBH_BulkReceiveData(USB_OTG_CORE_HANDLE *pdev,
                                 uint8_t *buff,
                                 uint16_t length,
                                 uint8_t hc_num);

USBH_Status USBH_BulkSendData(USB_OTG_CORE_HANDLE *pdev,
                              uint8_t *buff,
                              uint16_t length,
                              uint8_t hc_num);

USBH_Status USBH_InterruptReceiveData(USB_OTG_CORE_HANDLE *pdev,
                                      uint8_t             *buff,
                                      uint8_t             length,
                                      uint8_t             hc_num);

USBH_Status USBH_InterruptSendData(USB_OTG_CORE_HANDLE *pdev,
                                   uint8_t *buff,
                                   uint8_t length,
                                   uint8_t hc_num);

USBH_Status USBH_CtlReq(USB_OTG_CORE_HANDLE *pdev,
                        USBH_HOST *phost,
                        uint8_t             *buff,
                        uint16_t            length);

USBH_Status USBH_IsocReceiveData(USB_OTG_CORE_HANDLE *pdev,
                                 uint8_t *buff,
                                 uint32_t length,
                                 uint8_t hc_num);


USBH_Status USBH_IsocSendData(USB_OTG_CORE_HANDLE *pdev,
                              uint8_t *buff,
                              uint32_t length,
                              uint8_t hc_num);
/**
  * @}
  */

#ifdef __cplusplus
}
#endif

#endif  /* __USBH_IOREQ_H */
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
