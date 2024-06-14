#include "fetch_sensitive_data_task.h"
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
#include "drv_usb.h"
#include "usb_task.h"
#include "anti_tamper.h"
#include "device_setting.h"

static void FetchSensitiveDataTask(void *argument);

osThreadId_t g_sensitiveDataTaskHandle;

void CreateFetchSensitiveDataTask(void)
{
    const osThreadAttr_t SensitiveDataTask_attributes = {
        .name = "SensitiveDataTask",
        .stack_size = 1024 * 35,
        .priority = (osPriority_t)osPriorityBelowNormal,
    };
    g_sensitiveDataTaskHandle = osThreadNew(FetchSensitiveDataTask, NULL, &SensitiveDataTask_attributes);
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
    PubBufferMsg(SENSITIVE_MSG_EXECUTE, &async, sizeof(BackgroundAsync_t));
    return SUCCESS_CODE;
}

int32_t AsyncExecuteWithPtr(BackgroundAsyncFunc_t func, const void *inData)
{
    BackgroundAsync_t async = {0};
    async.func = func;
    async.inData = (void *)inData;
    async.inDataLen = 4;
    PubBufferMsg(SENSITIVE_MSG_EXECUTE, &async, sizeof(BackgroundAsync_t));
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
    PubBufferMsg(SENSITIVE_MSG_EXECUTE, &async, sizeof(BackgroundAsync_t));
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
    PubBufferMsg(SENSITIVE_MSG_EXECUTE_RUNNABLE, &async, sizeof(BackgroundRunnable_t));
    return SUCCESS_CODE;
}

static void FetchSensitiveDataTask(void *argument)
{
    Message_t rcvMsg;
    osStatus_t ret;
    BackgroundAsync_t *async;
    while (1) {
        ret = osMessageQueueGet(g_sensitiveQueue, &rcvMsg, NULL, 10000);
        if (ret != osOK) {
            continue;
        }
        switch (rcvMsg.id) {
        case SENSITIVE_MSG_EXECUTE: {
            if (rcvMsg.buffer == NULL || rcvMsg.length != sizeof(BackgroundAsync_t)) {
                printf("rcv SENSITIVE_MSG_EXECUTE err,rcvMsg.buffer=0x%08X,rcvMsg.length=%d\r\n", rcvMsg.buffer, rcvMsg.length);
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
        case SENSITIVE_MSG_EXECUTE_RUNNABLE: {
            if (rcvMsg.buffer == NULL || rcvMsg.length != sizeof(BackgroundRunnable_t)) {
                printf("rcv SENSITIVE_MSG_EXECUTE_RUNNABLE err,rcvMsg.buffer=0x%08X,rcvMsg.length=%d\r\n", rcvMsg.buffer, rcvMsg.length);
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
        default:
            break;
        }
        if (rcvMsg.buffer != NULL) {
            SRAM_FREE(rcvMsg.buffer);
        }
    }
}