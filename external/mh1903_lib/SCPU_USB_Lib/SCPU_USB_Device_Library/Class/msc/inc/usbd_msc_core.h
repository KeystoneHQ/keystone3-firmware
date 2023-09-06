/************************ (C) COPYRIGHT Megahuntmicro *************************
 * File Name            : usbd_msc_core.h
 * Author               : Megahuntmicro
 * Version              : V1.0.0
 * Date                 : 21-October-2014
 * Description          : Header file for usbd_msc_core.c
 *****************************************************************************/


/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef _USB_MSC_CORE_H_
#define _USB_MSC_CORE_H_

#include  "usbd_ioreq.h"

/** @addtogroup USBD_MSC_BOT
  * @{
  */

/** @defgroup USBD_MSC
  * @brief This file is the Header file for USBD_msc.c
  * @{
  */


/** @defgroup USBD_BOT_Exported_Defines
  * @{
  */

#ifndef CONFIG_USB_DEVICE_MSC
/* MSC default Config Start */
#define MSC_IN_EP  0x81
#define MSC_OUT_EP 0x01
#define MSC_MAX_PACKET 64
#define MSC_MEDIA_PACKET 512
/* MSC default Config End */
#endif


#define BOT_GET_MAX_LUN              0xFE
#define BOT_RESET                    0xFF
#define USB_MSC_CONFIG_DESC_SIZ      32

#define MSC_EPIN_SIZE                MSC_MAX_PACKET
#define MSC_EPOUT_SIZE               MSC_MAX_PACKET


extern USBD_Class_cb_TypeDef  USBD_MSC_cb;

uint8_t  USBD_MSC_Init(void  *pdev, uint8_t cfgidx);
uint8_t  USBD_MSC_DeInit(void  *pdev, uint8_t cfgidx);
uint8_t  USBD_MSC_Setup(void  *pdev, USB_SETUP_REQ *req);
uint8_t  USBD_MSC_DataIn(void  *pdev, uint8_t epnum);
uint8_t  USBD_MSC_DataOut(void  *pdev, uint8_t epnum);
uint8_t  *USBD_MSC_GetCfgDesc(uint8_t speed, uint16_t *length);


#endif


/************************ (C) COPYRIGHT Megahuntmicro *****END OF FILE****/
