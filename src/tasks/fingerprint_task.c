#include "fingerprint_task.h"
#include "fingerprint_process.h"
#include "cmsis_os.h"
#include "general_msg.h"
#include "user_msg.h"
#include "mhscpu.h"
#include "user_memory.h"
#include "keystore.h"
#include "event_groups.h"
#include "timers.h"

extern EventGroupHandle_t g_fpEventGroup;
extern char g_intrRecvBuffer[RCV_MSG_MAX_LEN];
static void FingerprintTask(void *pvParameter);
osThreadId_t g_fingerprintTaskId;
osTimerId_t g_fingerInitTimer = NULL;
osTimerId_t g_fpTimeoutTimer = NULL;
void FpTimeoutHandle(void *argument);
void FpResponseHandleStop(void);
bool GuiNeedFpRecognize(void);

void CreateFingerprintTask(void)
{
    const osThreadAttr_t fingerprintTaskAttributes = {
        .name = "FingerprintTask",
        .priority = osPriorityHigh,
        .stack_size = 1024 * 10,
    };
    g_fingerprintTaskId = osThreadNew(FingerprintTask, NULL, &fingerprintTaskAttributes);
}

void FingerPrintGroupSetBit(uint32_t uxBitsToSet)
{
    xEventGroupSetBits(g_fpEventGroup, uxBitsToSet);
}

void FpGetAesStateHandle(void *argument)
{
    SearchFpFwVersion();
}

void FpRecognizeHandle(void *argument)
{
    printf("%s %d\n", __func__, __LINE__);
    if (GuiNeedFpRecognize()) {
        FpRecognize(RECOGNIZE_UNLOCK);
    }
}

static void FingerprintTask(void *pvParameter)
{
    uint8_t len = 0;

    FingerprintRestart();
    g_fpTimeoutTimer = osTimerNew(FpTimeoutHandle, osTimerPeriodic, NULL, NULL);
    osTimerId_t getAesKeyTimer = osTimerNew(FpGetAesStateHandle, osTimerOnce, NULL, NULL);
    osTimerStart(getAesKeyTimer, 100);
    while (1) {
        EventBits_t bit = xEventGroupWaitBits(g_fpEventGroup, FINGER_PRINT_ALL_EVENT, pdTRUE, pdFALSE, portMAX_DELAY);
        switch (bit) {
        case FINGER_PRINT_EVENT_UART_RECV:
            len = g_intrRecvBuffer[2] * 256 + g_intrRecvBuffer[1];
            FingerprintRcvMsgHandle(g_intrRecvBuffer, len + 3);
            memset(g_intrRecvBuffer, 0, sizeof(g_intrRecvBuffer));
            break;
        case FINGER_PRINT_EVENT_RESTART:
            FpResponseHandleStop();
            FingerprintRestart();
            getAesKeyTimer = osTimerNew(FpRecognizeHandle, osTimerOnce, NULL, NULL);
            osTimerStart(getAesKeyTimer, 150);
            break;
        case FINGER_PRINT_EVENT_LOW_POWER:
            SetFpLowPowerMode();
            break;
        }
    }
}

void CloseFingerInitTimer(void)
{
    osTimerStop(g_fingerInitTimer);
}
