#include "test_task.h"
#include "stdio.h"
#include "cmsis_os.h"
#include "user_msg.h"
#include "hal_lcd.h"


static void TestTask(void *argument);
static void MeasurementTimerFunc(void *argument);

osThreadId_t g_testTaskHandle;
osTimerId_t g_measurementTimer;

void CreateTestTask(void)
{
    const osThreadAttr_t testtTask_attributes = {
        .name = "testTask",
        .stack_size = 1024 * 4,
        .priority = (osPriority_t) osPriorityNormal,
    };
    g_testTaskHandle = osThreadNew(TestTask, NULL, &testtTask_attributes);
    g_measurementTimer = osTimerNew(MeasurementTimerFunc, osTimerPeriodic, NULL, NULL);
}


void TestTask(void *argument)
{
    Message_t rcvMsg;
    osStatus_t ret;

    while (1) {
        ret = osMessageQueueGet(g_testQueue, &rcvMsg, NULL, 100000);
        if (ret != osOK) {
            continue;
        }
        switch (rcvMsg.id) {
        case TEST_MSG_MEASUREMENT: {
            //printf("measurement in test task\r\n");
        }
        break;
        //case TEST_MSG_ASYNPRINT: {
        //    EmptyPrintBuffer();
        //}
        //break;
        default:
            break;
        }
    }
}

static void MeasurementTimerFunc(void *argument)
{
    PubValueMsg(TEST_MSG_MEASUREMENT, 0);
}
