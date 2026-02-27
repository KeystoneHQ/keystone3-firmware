#ifndef _DRV_VIRTUAL_TOUCH_H
#define _DRV_VIRTUAL_TOUCH_H

#include "stdint.h"
#include "stdbool.h"

void SetVirtualTouchCoord(uint16_t x, uint16_t y);
void SetVirtualTouchState(bool pressed);
bool GetVirtualTouch(uint16_t *x, uint16_t *y);

#endif
