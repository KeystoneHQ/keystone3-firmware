/**************************************************************************************************
 * Copyright (c) Larrie studio. 2020-2023. All rights reserved.
 * Description: rtos扩展功能
 * Author: leon sun
 * Create: 2022-6-24
 ************************************************************************************************/


#ifndef _RTOS_EXPAND_H
#define _RTOS_EXPAND_H

#include "stdint.h"

void PrintTasksStatus(void);
void ClrRunTimeStats(void);
uint32_t GetRunTimeCounter(void);
void ConfigureTimerForRunTimeStats(void);

#endif
