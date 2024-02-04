#include "cmd_task.h"
#include "cmsis_os.h"
#include "general_msg.h"
#include "user_msg.h"
#include "test_cmd.h"
#include "mhscpu.h"


#define TEST_CMD_MAX_LENGTH     3072

static void CmdTask(void *pvParameter);


uint8_t g_testCmdRcvBuffer[TEST_CMD_MAX_LENGTH];
uint32_t g_testCmdRcvCount = 0;
osThreadId_t g_cmdTaskId;

void CreateCmdTask(void)
{
    const osThreadAttr_t cmdTaskAttributes = {
        .name = "CmdTask",
        .priority = osPriorityRealtime7,
        .stack_size = 1024 * 20,
    };
    g_cmdTaskId = osThreadNew(CmdTask, NULL, &cmdTaskAttributes);
}


static void CmdTask(void *pvParameter)
{
    Message_t rcvMsg;
    osStatus_t ret;
    while (1) {
        ret = osMessageQueueGet(g_cmdQueue, &rcvMsg, NULL, 10000);
        if (ret != osOK) {
            continue;
        }
        switch (rcvMsg.id) {
        case MSG_TEST_CMD_FRAME:
            g_testCmdRcvBuffer[rcvMsg.value - 3] = 0;
#ifndef BUILD_PRODUCTION
            CompareAndRunTestCmd((char *)g_testCmdRcvBuffer + 1);
#endif
            break;
        default:
            break;
        }
        if (rcvMsg.buffer != NULL) {
            SramFree(rcvMsg.buffer);
        }
    }
}


void __inline CmdIsrRcvByte(uint8_t byte)
{
    static uint32_t lastTick = 0;
    uint32_t tick;
    static uint32_t rxF8Count = 0;

    if (osKernelGetState() < osKernelRunning) {
        return;
    }
    tick = osKernelGetTickCount();
    if (g_testCmdRcvCount != 0) {
        if (tick - lastTick > 200) {
            g_testCmdRcvCount = 0;
            rxF8Count = 0;
        }
    }
    lastTick = tick;
    if (byte == 0xF8) {
        if (rxF8Count++ > 10) {
            NVIC_SystemReset();
        }
    } else {
        rxF8Count = 0;
    }

    if (g_testCmdRcvCount == 0) {
        if (byte == '#') {
            g_testCmdRcvBuffer[g_testCmdRcvCount] = byte;
            g_testCmdRcvCount++;
        }
    } else if (byte == '\n' && g_testCmdRcvBuffer[g_testCmdRcvCount - 1] == '\r') {
        g_testCmdRcvBuffer[g_testCmdRcvCount] = byte;
        g_testCmdRcvBuffer[g_testCmdRcvCount + 1] = 0;
        PubValueMsg(MSG_TEST_CMD_FRAME, g_testCmdRcvCount + 2);
        g_testCmdRcvCount = 0;
    } else if (g_testCmdRcvCount >= TEST_CMD_MAX_LENGTH - 1) {
        g_testCmdRcvCount = 0;
    } else {
        g_testCmdRcvBuffer[g_testCmdRcvCount] = byte;
        g_testCmdRcvCount++;
    }
}

