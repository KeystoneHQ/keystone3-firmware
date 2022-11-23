/**************************************************************************************************
 * Copyright (c) keyst.one. 2020-2025. All rights reserved.
 * Description: virtual touch driven by uart.
 * Author: leon sun
 * Create: 2022-11-30
 ************************************************************************************************/

#ifndef _DRV_VIRTUAL_TOUCH_H
#define _DRV_VIRTUAL_TOUCH_H


#include "stdint.h"
#include "stdbool.h"


void SetVirtualTouchCoord(uint16_t x, uint16_t y);
void SetVirtualTouchState(bool pressed);
bool GetVirtualTouch(uint16_t *x, uint16_t *y);

#endif

