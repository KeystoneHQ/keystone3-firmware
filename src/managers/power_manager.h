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
