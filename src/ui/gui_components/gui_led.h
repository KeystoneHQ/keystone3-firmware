/*********************************************************************
 * Copyright (c) keyst.one. 2020-2025. All rights reserved.
 * name       : gui_led.h
 * Description:
 * author     : stone wang
 * data       : 2023-02-01 15:58
**********************************************************************/

#ifndef _GUI_LED_H
#define _GUI_LED_H

#include "gui.h"

typedef enum {
    PASSCODE_LED_ON,
    PASSCODE_LED_OFF,
    PASSCODE_LED_ERR,

    PASSCODE_LED_BUTT,
} PASSCODE_LED_STATUS_ENUM;

void *GuiCreateLed(lv_obj_t *parent);
void GuiSetLedStatus(lv_obj_t *led, PASSCODE_LED_STATUS_ENUM status);

#endif /* _GUI_LED_H */

