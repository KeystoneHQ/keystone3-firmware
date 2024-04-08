#ifndef _DRV_AW32001_H
#define _DRV_AW32001_H

#include "stdint.h"
#include "stdbool.h"

typedef enum {
    CHARGE_STATE_NOT_CHARGING,
    CHARGE_STATE_PRE_CHARGE,
    CHARGE_STATE_CHARGE,
    CHARGE_STATE_CHARGE_DONE,
} ChargeState;

typedef enum {
    USB_POWER_STATE_DISCONNECT,
    USB_POWER_STATE_CONNECT,
} UsbPowerState;

typedef void (*ChangerInsertIntCallbackFunc_t)(void);

void Aw32001Init(void);
void RegisterChangerInsertCallback(ChangerInsertIntCallbackFunc_t func);
int32_t Aw32001PowerOff(void);
int32_t Aw32001RefreshState(void);
ChargeState GetChargeState(void);
UsbPowerState GetUsbPowerState(void);
bool GetUsbDetectState(void);
void ChangerInsertHandler(void);
void Aw32001Test(int argc, char *argv[]);

#endif
