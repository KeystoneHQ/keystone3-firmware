#ifndef _DRV_SYS_H
#define _DRV_SYS_H

#include "stdint.h"
#include "stdbool.h"
#include "user_delay.h"

#define DelayMs(ms)     UserDelay(ms)

void SystemClockInit(void);
void NvicInit(void);
void PrintSystemInfo(void);

#endif
