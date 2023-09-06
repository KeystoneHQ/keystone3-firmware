/************************ (C) COPYRIGHT Megahuntmicro *************************
 * File Name            : usbd_conf.h
 * Author               : Megahuntmicro
 * Version              : V1.0.0
 * Date                 : 21-October-2014
 * Description          : USB Device configuration file
 *****************************************************************************/

#ifndef __USBD_CONF__H__
#define __USBD_CONF__H__

#include "usb_conf.h"

/* USB Device Config Start */
#define USBD_CFG_MAX_NUM 1

// May be smaller than USB_OTG_MAX_EP_COUNT, define it as needed
#define USBD_ITF_MAX_NUM USB_OTG_MAX_EP_COUNT

#define USB_MAX_STR_DESC_SIZ 64

#define USBD_SELF_POWERED
/* USB Device Config End */


#if CONFIG_USB_DEVICE_MSC
/* MSC Config Start */
#if USB_OTG_MAX_EP_COUNT > 4
#define MSC_IN_EP  0x84
#define MSC_OUT_EP 0x04
#else
#define MSC_EP_NUM 0x01
#define MSC_IN_EP  0x81
#define MSC_OUT_EP 0x01
#endif

#define MSC_MAX_PACKET 64

#define MSC_MEDIA_PACKET 4096
/* MSC Config End */
#endif // CONFIG_USB_DEVICE_MSC




#endif //__USBD_CONF__H__

/************************ (C) COPYRIGHT 2014 Megahuntmicro ****END OF FILE****/
