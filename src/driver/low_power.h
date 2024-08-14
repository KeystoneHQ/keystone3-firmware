#ifndef _LOW_POWER_H
#define _LOW_POWER_H

#include "stdint.h"
#include "stdbool.h"
#include "mhscpu.h"

typedef enum {
    LOW_POWER_STATE_WORKING,
    LOW_POWER_STATE_DEEP_SLEEP,
} LowPowerState;

typedef enum {
    WAKE_UP_BY_BUTTON,
    WAKE_UP_BY_FINGER,
} WakeUpMethod;

void LowPowerTest(int argc, char *argv[]);
uint32_t EnterLowPower(void);
void RecoverFromLowPower(void);
void EnterDeepSleep(void);
void RecoverFromDeepSleep(void);
void EnterCpuSleep(void);
void SetAllGpioLow(void);
void DisableAllHardware(void);
LowPowerState GetLowPowerState(void);
void SetGpioFloat(GPIO_TypeDef *GpioX, uint32_t pin);
void SetGpioPullUp(GPIO_TypeDef *GpioX, uint32_t pin);
void SetGpioLow(GPIO_TypeDef *GpioX, uint32_t pin);
void SetGpioHigh(GPIO_TypeDef *GpioX, uint32_t pin);
void LowerPowerTimerStart(void);

#endif
