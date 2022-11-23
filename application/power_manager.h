/**************************************************************************************************
 * Copyright (c) keyst.one. 2020-2025. All rights reserved.
 * Description: Power manager.
 * Author: leon sun
 * Create: 2023-5-25
 ************************************************************************************************/


#ifndef _POWER_MANAGER_H
#define _POWER_MANAGER_H

#include "stdint.h"
#include "stdbool.h"
#include "err_code.h"

void PowerManagerInit(void);
void SetShutdownTimeOut(uint32_t timeOut);
void AutoShutdownHandler(uint32_t time);
void ClearShutdownTime(void);
void SetShowPowerOffPage(bool isShow);
#endif
