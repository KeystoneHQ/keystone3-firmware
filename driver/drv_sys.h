/**************************************************************************************************
 * Copyright (c) keyst.one 2020-2025. All rights reserved.
 * Description: 时钟/中断之类的初始化.
 * Author: leon sun
 * Create: 2022-11-8
 ************************************************************************************************/

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
