#ifndef __USBTARGET_H__
#define __USBTARGET_H__

#include "USBESL.h"

extern void USBTargetDelayUs(uint32_t us);

extern void USBTargetEnablePhy(uint32_t usbBase, bool isEnable);

extern void USBTargetEnableInterrupt(uint32_t usbBase, bool isEnable);

extern void USBTargetEnableModule(uint32_t usbBase, bool isEnable);

extern void USBTargetResetModule(uint32_t usbBase);

#endif
