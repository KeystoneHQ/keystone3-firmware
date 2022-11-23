/**************************************************************************************************
 * Copyright (c) keyst.one. 2020-2025. All rights reserved.
 * Description: Touch pad task.
 * Author: leon sun
 * Create: 2023-1-17
 ************************************************************************************************/

#include "touchpad_task.h"
#include "stdio.h"
#include "string.h"
#include "cmsis_os.h"
#include "qrdecode_task.h"
#include "ui_display_task.h"
#include "screen_manager.h"
#include "screenshot.h"

//#define TOUCH_TEST_ON_LCD
#define TOUCH_RING_BUFFER_SIZE          10

#ifdef TOUCH_TEST_ON_LCD
#include "hal_lcd.h"
#endif


static void TouchPadTask(void *argument);
static void TouchIntHandler(void);

osThreadId_t g_touchPadTaskHandle;
osSemaphoreId_t g_touchPadSem = NULL;
TouchStatus_t g_touchStatus[TOUCH_RING_BUFFER_SIZE];
volatile bool g_touchPress;
static uint8_t g_touchWriteIndex, g_touchReadIndex;

void CreateTouchPadTask(void)
{
    const osThreadAttr_t touchPadTask_attributes = {
        .name = "TouchPadTask",
        .stack_size = 1024 * 1,
        .priority = (osPriority_t) osPriorityHigh7,
    };
    g_touchPadTaskHandle = osThreadNew(TouchPadTask, NULL, &touchPadTask_attributes);
}


static void TouchPadTask(void *argument)
{
    TouchStatus_t touchStatus = {0};
    uint32_t waitTime;
    g_touchPadSem = osSemaphoreNew(20, 0, NULL);
    bool lastTouch = false;

    TouchInit(TouchIntHandler);
    osDelay(50);
    while (1) {
        waitTime = touchStatus.touch ? 1000 : osWaitForever;
        osSemaphoreAcquire(g_touchPadSem, waitTime);
        osKernelLock();
        ClearLockScreenTime();
        TouchGetStatus(&touchStatus);
#ifdef TOUCH_TEST_ON_LCD
        uint16_t testColor = 0xFFFF;
        LcdDraw(touchStatus.x, touchStatus.y, touchStatus.x, touchStatus.y, &testColor);
#else
        if (lastTouch) {
            g_touchWriteIndex++;
            if (g_touchWriteIndex >= TOUCH_RING_BUFFER_SIZE) {
                g_touchWriteIndex = 0;
            }
            memcpy(&g_touchStatus[g_touchWriteIndex], &touchStatus, sizeof(TouchStatus_t));
        }
        lastTouch = touchStatus.touch;
#endif
        osKernelUnlock();
        ActivateUiTaskLoop();
        QrDecodeTouchQuit();
#ifndef BUILD_PRODUCTION
        // ScreenShotTouch();
#endif
    }
}

TouchStatus_t *GetTouchStatus(void)
{
    uint8_t index;

    index = g_touchReadIndex;
    //printf("index=%d\r\n", index);
    if (g_touchReadIndex != g_touchWriteIndex) {
        g_touchReadIndex++;
        if (g_touchReadIndex >= TOUCH_RING_BUFFER_SIZE) {
            g_touchReadIndex = 0;
        }
        g_touchStatus[index].continueReading = true;
    } else {
        g_touchStatus[index].continueReading = false;
    }
    return &g_touchStatus[index];
}


bool GetTouchPress(void)
{
    return g_touchPress;
}


static void TouchIntHandler(void)
{
    osSemaphoreRelease(g_touchPadSem);
}


