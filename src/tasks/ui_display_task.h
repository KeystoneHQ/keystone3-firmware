/**************************************************************************************************
 * Copyright (c) keyst.one. 2020-2025. All rights reserved.
 * Description: UI显示任务
 * Author: leon sun
 * Create: 2022-11-28
 ************************************************************************************************/

#ifndef _UI_DISPLAY_TASK_H
#define _UI_DISPLAY_TASK_H

#include "stdint.h"
#include "stdbool.h"

void CreateUiDisplayTask(void);
void SetLvglHandlerAndSnapShot(bool enable);
void LvglCloseCurrentView(void);
uint8_t *GetLvglGramAddr(void);
uint32_t GetLvglGramSize(void);
void ActivateUiTaskLoop(void);

extern bool g_reboot;
#endif

