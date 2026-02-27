#ifndef _SCREEN_MANAGER_H
#define _SCREEN_MANAGER_H

#include "stdint.h"
#include "stdbool.h"
#include "err_code.h"

void ScreenManagerInit(void);
void SetLockScreen(bool enable);
void SetPageLockScreen(bool enable);
void SetLockTimeState(bool enable);
bool IsPreviousLockScreenEnable(void);
void ClearLockScreenTime(void);
void SetLockTimeOut(uint32_t timeOut);
void SetLockDeviceAlive(bool alive);

#endif
