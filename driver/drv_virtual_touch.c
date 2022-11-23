/**************************************************************************************************
 * Copyright (c) keyst.one. 2020-2025. All rights reserved.
 * Description: virtual touch driven by uart.
 * Author: leon sun
 * Create: 2022-11-30
 ************************************************************************************************/

#include "drv_virtual_touch.h"

static volatile uint16_t g_virtualTouchX = 0;
static volatile uint16_t g_virtualTouchY = 0;
static volatile bool g_virtualPressed = false;

void SetVirtualTouchCoord(uint16_t x, uint16_t y)
{
    g_virtualTouchX = x;
    g_virtualTouchY = y;
}


void SetVirtualTouchState(bool pressed)
{
    g_virtualPressed = pressed;
}

bool GetVirtualTouch(uint16_t *x, uint16_t *y)
{
    *x = g_virtualTouchX;
    *y = g_virtualTouchY;
    return g_virtualPressed;
}
