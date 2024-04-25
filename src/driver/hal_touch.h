#ifndef _HAL_TOUCH_H
#define _HAL_TOUCH_H

#include "stdint.h"
#include "stdbool.h"

#define TOUCH_PAD_RES_X                 480
#define TOUCH_PAD_RES_Y                 800

typedef void (*TouchPadIntCallbackFunc_t)(void);

typedef struct {
    bool touch;
    bool continueReading;
    uint16_t x;
    uint16_t y;
} TouchStatus_t;

typedef struct {
    void (*Init)(void);
    void (*Open)(void);
    int32_t (*GetStatus)(TouchStatus_t *status);
} HalTouchOpt_t;

void TouchInit(TouchPadIntCallbackFunc_t func);
void TouchOpen(void);
void TouchClose(void);
int32_t TouchGetStatus(TouchStatus_t *status);

void TouchPadIntHandler(void);

void TouchPadTest(int argc, char *argv[]);

#endif
