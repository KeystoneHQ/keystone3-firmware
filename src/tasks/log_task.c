#include "log_task.h"
#include "stdio.h"
#include "string.h"
#include "cmsis_os.h"
#include "user_memory.h"
#include "user_msg.h"
#include "err_code.h"
#include "log.h"
#include "user_utils.h"

static void LogTask(void *argument);

static osThreadId_t g_logTaskHandle;
static bool g_logLoopStart = false;

void CreateLogTask(void)
{
    const osThreadAttr_t logTaskAttributes = {
        .name = "LogTask",
        .stack_size = 1024 * 8,
        .priority = (osPriority_t) osPriorityLow,
    };
    g_logTaskHandle = osThreadNew(LogTask, NULL, &logTaskAttributes);
}



static void LogTask(void *argument)
{
    Message_t rcvMsg;
    osStatus_t ret;

    g_logLoopStart = true;
    while (1) {
        ret = osMessageQueueGet(g_logQueue, &rcvMsg, NULL, 10000);
        if (ret == osOK) {
            switch (rcvMsg.id) {
            case LOG_MSG_WRITE: {
                WriteLogDataToFlash(rcvMsg.buffer, rcvMsg.length);
            }
            break;
            case LOG_MSG_ERASE: {
                LogEraseSync();
            }
            break;
            case LOG_MSG_EXPORT: {
                LogExportSync();
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
}


/// @brief Get the status of whether the log task loop has been started.
/// @param
/// @return True if the log task loop has been started, False otherwise.
bool LogLoopStart(void)
{
    return g_logLoopStart;
}

