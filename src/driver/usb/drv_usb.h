#ifndef _DRV_USB_H
#define _DRV_USB_H

#include "mhscpu.h"
#include "usb_bsp.h"
#include "usb_dcd_int.h"
#include "usb_otg.h"
#include "usbd_core.h"
#include "usbd_desc.h"

#if CONFIG_USB_DEVICE_MSC
#include "usbd_msc_core.h"
#undef DeviceCallback

#include "usbd_cdc_core.h"

//#define DeviceCallback &USBD_MSC_cb
#define DeviceCallback &USBCompositeCb
//#define DeviceCallback &USBD_CDC_cb
#endif

void UsbInit(void);
void UsbLoop(void);
void UsbDeInit(void);
bool UsbInitState(void);
void UsbSetIRQ(bool enable);
void ConnectUsbMutexRestrict(void);

#endif /* _DRV_USB_H */

