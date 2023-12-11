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

