/************************ (C) COPYRIGHT Megahuntmicro *************************
 * File Name            : usbd_req.h
 * Author               : Megahuntmicro
 * Version              : V1.0.0
 * Date                 : 21-October-2014
 * Description          : header file for the usbd_req.c file.
 *****************************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/

#ifndef __USB_REQUEST_H_
#define __USB_REQUEST_H_


#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include  "usbd_def.h"
#include  "usbd_core.h"
#include  "usbd_conf.h"
#include  "usbd_ioreq.h"

/** @defgroup USBD_REQ_Exported_FunctionsPrototype
  * @{
  */

USBD_Status  USBD_StdDevReq(USB_OTG_CORE_HANDLE  *pdev, USB_SETUP_REQ  *req);
USBD_Status  USBD_StdItfReq(USB_OTG_CORE_HANDLE  *pdev, USB_SETUP_REQ  *req);
USBD_Status  USBD_StdEPReq(USB_OTG_CORE_HANDLE  *pdev, USB_SETUP_REQ  *req);
void USBD_ParseSetupRequest(USB_OTG_CORE_HANDLE  *pdev,
                            USB_SETUP_REQ *req);

void USBD_CtlError(USB_OTG_CORE_HANDLE  *pdev,
                   USB_SETUP_REQ *req);

void USBD_GetString(uint8_t *desc, uint8_t *unicode, uint16_t *len);
/**
  * @}
  */

#endif /* __USB_REQUEST_H_ */

/************************ (C) COPYRIGHT 2014 Megahuntmicro ****END OF FILE****/
