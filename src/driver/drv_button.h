#ifndef _DRV_BUTTON_IO_H
#define _DRV_BUTTON_IO_H

#include "stdint.h"
#include "stdbool.h"

typedef void (*ButtonEventCallbackFunc_t)(void);

typedef enum {
    BUTTON_EVENT_SHORT_PRESS,
    BUTTON_EVENT_LONG_PRESS,
    BUTTON_EVENT_RELEASE,
} ButtonEventType;

void ButtonInit(void);
void RegisterButtonEvent(ButtonEventType event, ButtonEventCallbackFunc_t func);
void ButtonIntHandler(void);
bool ButtonPress(void);

#endif
