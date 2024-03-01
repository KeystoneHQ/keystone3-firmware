#ifndef _TOUCH_PAD_TASK_H
#define _TOUCH_PAD_TASK_H

#include "stdint.h"
#include "stdbool.h"
#include "hal_touch.h"

#include "drv_ft6336.h"

void CreateTouchPadTask(void);
TouchStatus_t *GetTouchStatus(void);
TouchStatus_t *GetLatestTouchStatus(void);
bool GetTouchPress(void);
void ClearTouchBuffer(void);

#endif

