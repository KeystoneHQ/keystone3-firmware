/************************ (C) COPYRIGHT Megahuntmicro *************************
 * File Name            : usbd_ioreq.h
 * Author               : Megahuntmicro
 * Version              : V1.0.0
 * Date                 : 21-October-2014
 * Description          : header file for the usbd_ioreq.c file.
 *****************************************************************************/

#ifndef __USBD_IOREQ_H_
#define __USBD_IOREQ_H_


#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include  "usbd_def.h"
#include  "usbd_core.h"

/** @defgroup USBD_IOREQ_Exported_FunctionsPrototype
  * @{
  */

USBD_Status  USBD_CtlSendData(USB_OTG_CORE_HANDLE  *pdev,
                              uint8_t *buf,
                              uint16_t len);

USBD_Status  USBD_CtlContinueSendData(USB_OTG_CORE_HANDLE  *pdev,
                                      uint8_t *pbuf,
                                      uint16_t len);

USBD_Status USBD_CtlPrepareRx(USB_OTG_CORE_HANDLE  *pdev,
                              uint8_t *pbuf,
                              uint16_t len);

USBD_Status  USBD_CtlContinueRx(USB_OTG_CORE_HANDLE  *pdev,
                                uint8_t *pbuf,
                                uint16_t len);

USBD_Status  USBD_CtlSendStatus(USB_OTG_CORE_HANDLE  *pdev);

USBD_Status  USBD_CtlReceiveStatus(USB_OTG_CORE_HANDLE  *pdev);

uint16_t  USBD_GetRxCount(USB_OTG_CORE_HANDLE  *pdev,
                          uint8_t epnum);

/**
  * @}
  */

#endif /* __USBD_IOREQ_H_ */

/************************ (C) COPYRIGHT 2014 Megahuntmicro ****END OF FILE****/
