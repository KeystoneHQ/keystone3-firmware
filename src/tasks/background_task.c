#include "background_task.h"
#include "background_app.h"
#include "drv_aw32001.h"
#include "drv_battery.h"
#include "drv_tamper.h"
#include "stdio.h"
#include "string.h"
#include "cmsis_os.h"
#include "user_memory.h"
#include "user_msg.h"
#include "user_fatfs.h"
#include "user_delay.h"
#include "err_code.h"
#include "gui_api.h"
#include "gui_views.h"
#include "usb_task.h"
#include "anti_tamper.h"

static void BackgroundTask(void *argument);

osThreadId_t g_backgroundTaskHandle;

void CreateBackgroundTask(void)
{
    const osThreadAttr_t backgroundTask_attributes = {
        .name = "BackgroundTask",
        .stack_size = 1024 * 16,
        .priority = (osPriority_t)osPriorityBelowNormal,
    };
    g_backgroundTaskHandle = osThreadNew(BackgroundTask, NULL, &backgroundTask_attributes);
}

/// @brief Notify the background task to execute a function.
/// @param[in] func The function which would be executed in background task.
/// @param[in] inData.
/// @param[in] inDataLen.
/// @return err code.
int32_t AsyncExecute(BackgroundAsyncFunc_t func, const void *inData, uint32_t inDataLen)
{
    BackgroundAsync_t async = {0};
    async.func = func;
    if (inDataLen > 0) {
        async.inData = SRAM_MALLOC(inDataLen);
        memcpy(async.inData, inData, inDataLen);
        async.inDataLen = inDataLen;
    }
    PubBufferMsg(BACKGROUND_MSG_EXECUTE, &async, sizeof(BackgroundAsync_t));
    return SUCCESS_CODE;
}

int32_t AsyncExecuteWithPtr(BackgroundAsyncFunc_t func, const void *inData)
{
    BackgroundAsync_t async = {0};
    async.func = func;
    async.inData = (void *)inData;
    async.inDataLen = 4;
    PubBufferMsg(BACKGROUND_MSG_EXECUTE, &async, sizeof(BackgroundAsync_t));
    return SUCCESS_CODE;
}

/// @brief Notify the background task to execute a function after a specified time delay.
/// @param[in] func The function which would be executed in background task.
/// @param[in] inData.
/// @param[in] inDataLen.
/// @param[in] delay millisecond.
/// @return err code.
int32_t AsyncDelayExecute(BackgroundAsyncFunc_t func, const void *inData, uint32_t inDataLen, uint32_t delay)
{
    BackgroundAsync_t async = {0};
    async.func = func;
    async.delay = delay;
    if (inDataLen > 0) {
        async.inData = SRAM_MALLOC(inDataLen);
        memcpy(async.inData, inData, inDataLen);
        async.inDataLen = inDataLen;
    }
    PubBufferMsg(BACKGROUND_MSG_EXECUTE, &async, sizeof(BackgroundAsync_t));
    return SUCCESS_CODE;
}

int32_t AsyncExecuteRunnable(BackgroundAsyncFuncWithRunnable_t func, const void *inData, uint32_t inDataLen, BackgroundAsyncRunnable_t runnable)
{
    BackgroundRunnable_t async = {0};
    async.func = func;
    if (inDataLen > 0) {
        async.inData = SRAM_MALLOC(inDataLen);
        memcpy(async.inData, inData, inDataLen);
        async.inDataLen = inDataLen;
    }
    async.runnable = runnable;
    PubBufferMsg(BACKGROUND_MSG_EXECUTE_RUNNABLE, &async, sizeof(BackgroundRunnable_t));
    return SUCCESS_CODE;
}

/// @brief Notify the background task to reboot the system.
void SystemReboot(void)
{
    PubValueMsg(BACKGROUND_MSG_RESET, SYSTEM_RESET_TYPE_REBOOT);
}

/// @brief Notify the background task to poweroff the system.
void SystemPoweroff(void)
{
    PubValueMsg(BACKGROUND_MSG_RESET, SYSTEM_RESET_TYPE_POWEROFF);
}

static void BackgroundTask(void *argument)
{
    Message_t rcvMsg;
    osStatus_t ret;
    BackgroundAsync_t *async;
    uint16_t battState;
    bool sdCardState = GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_7);
    BackGroundAppInit();
    while (1) {
        ret = osMessageQueueGet(g_backgroundQueue, &rcvMsg, NULL, 10000);
        if (ret != osOK) {
            continue;
        }
        switch (rcvMsg.id) {
        case BACKGROUND_MSG_EXECUTE: {
            if (rcvMsg.buffer == NULL || rcvMsg.length != sizeof(BackgroundAsync_t)) {
                printf("rcv BACKGROUND_MSG_EXECUTE err,rcvMsg.buffer=0x%08X,rcvMsg.length=%d\r\n", rcvMsg.buffer, rcvMsg.length);
                break;
            }
            async = (BackgroundAsync_t *)rcvMsg.buffer;
            if (async->delay > 0) {
                osDelay(async->delay);
            }
            if (async->func) {
                async->func(async->inData, async->inDataLen);
            }
            if (async->inData) {
                SRAM_FREE(async->inData);
            }
        }
        break;
        case BACKGROUND_MSG_EXECUTE_RUNNABLE: {
            if (rcvMsg.buffer == NULL || rcvMsg.length != sizeof(BackgroundRunnable_t)) {
                printf("rcv BACKGROUND_MSG_EXECUTE_RUNNABLE err,rcvMsg.buffer=0x%08X,rcvMsg.length=%d\r\n", rcvMsg.buffer, rcvMsg.length);
                break;
            }
            BackgroundRunnable_t *async_r = (BackgroundRunnable_t *)rcvMsg.buffer;
            // compiler will optimize the argument inData and inDataLen if they are not used in some cases;
            bool shouldClean = async_r->inDataLen > 0 && async_r->inData != NULL;
            if (async_r->func) {
                async_r->func(async_r->inData, async_r->inDataLen, async_r->runnable);
            }
            if (shouldClean) {
                SRAM_FREE(async_r->inData);
            }
        }
        break;
        case BACKGROUND_MSG_MINUTE: {
            // battery measurement, etc.
            if (GetChargeState() != CHARGE_STATE_NOT_CHARGING) {
                // printf("charger insert minute @bg_task\r\n");
                ChangerRefreshState();
            }
        }
        break;
        case BACKGROUND_MSG_CHANGER_INSERT: {
            osDelay(200);
            // printf("charger insert @bg_task\r\n");
            ChangerRefreshState();
            battState = GetBatterPercent();
            if (GetUsbPowerState() == USB_POWER_STATE_CONNECT) {
                battState |= 0x8000;
            }
            if (GetUsbDetectState() == false) {
                SetUsbState(false);
                GuiApiEmitSignalWithValue(SIG_INIT_USB_CONNECTION, 0);
            }
            printf("send battState=0x%04X\r\n", battState);
            GuiApiEmitSignal(SIG_INIT_BATTERY, &battState, sizeof(battState));
        }
        break;
        case BACKGROUND_MSG_RESET: {
            printf("reset device type=%d\r\n", rcvMsg.value);
            ExecuteSystemReset((SystemResetType)rcvMsg.value);
        }
        break;
        case BACKGROUND_MSG_BATTERY_INTERVAL: {
            if (rcvMsg.value != 0 || BatteryIntervalHandler()) {
                battState = GetBatterPercent();
                if (GetUsbPowerState() == USB_POWER_STATE_CONNECT) {
                    battState |= 0x8000;
                }
                printf("send battState=0x%04X\r\n", battState);
                GuiApiEmitSignal(SIG_INIT_BATTERY, &battState, sizeof(battState));
            }
        }
        break;
        case BACKGROUND_MSG_TAMPER: {
            TamperBackgroundHandler();
        }
        break;
        case BACKGROUND_MSG_SD_CARD_CHANGE: {
            UserDelay(100);
            if (sdCardState == GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_7)) {
                break;
            } else {
                sdCardState = GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_7);
                if (sdCardState == false) {
                    ret = MountSdFatfs();
                    uint32_t freeSize = FatfsGetSize("0:");
                    if (freeSize > 0) {
                        GuiApiEmitSignalWithValue(SIG_INIT_SDCARD_CHANGE, sdCardState);
                    }
                } else {
                    UnMountSdFatfs();
                    GuiApiEmitSignalWithValue(SIG_INIT_SDCARD_CHANGE, sdCardState);
                }
            }
        }
        break;
        default:
            break;
        }
        if (rcvMsg.buffer != NULL) {
            SRAM_FREE(rcvMsg.buffer);
        }
    }
}
