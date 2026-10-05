#include "qrdecode_task.h"
#include "stdint.h"
#include "stdbool.h"
#include "stdio.h"
#include "cmsis_os.h"
#include "drv_qrdecode.h"
#include "user_memory.h"
#include "user_msg.h"
#include "ui_display_task.h"
#include "log_print.h"
#include "drv_ds28s60.h"
#include "drv_trng.h"
#include "librust_c.h"
#include "gui_api.h"
#include "gui_views.h"
#include "gui_chain.h"
#include "user_delay.h"
#include "touchpad_task.h"
#include "gui_analyze.h"
#include "gui_web_auth_result_widgets.h"
#include "qrdecode_task.h"
#include "gui_resolve_ur.h"
#include "mpu_sandbox_task.h"
#include "mpu_sandbox_ur_validator.h"
#ifdef WEB3_VERSION
#include "gui_key_derivation_request_widgets.h"
#endif

#define QUIT_AREA_X_START                   10
#define QUIT_AREA_Y_START                   64
#define TOUCH_AREA_OFFSET                   64

typedef struct {
    GuiPosition_t start;
    GuiPosition_t end;
    void (*func)(void);
} QrDecodeViewTouchArea_t;

const static QrDecodeViewTouchArea_t g_qrDecodeObjects[2] = {
    {{QUIT_AREA_X_START, QUIT_AREA_Y_START}, {QUIT_AREA_X_START + TOUCH_AREA_OFFSET, QUIT_AREA_Y_START + TOUCH_AREA_OFFSET}, LvglCloseCurrentView},
};

static void QrDecodeTask(void *argument);
static void QrDecodeMinuteTimerFunc(void *argument);
static bool IsUrQrCode(const char *qrString, size_t length);
static bool ValidateQrUrInSandbox(const char *qrString, size_t length);

osThreadId_t g_qrDecodeTaskHandle;
static volatile QrDecodeStateType g_qrDecodeState;
osTimerId_t g_qrDecodeMinuteTimer;

void CreateQrDecodeTask(void)
{
    const osThreadAttr_t qrDecodeTaskAttributes = {
        .name = "qrDecodeTask",
        .stack_size = 1024 * 20,
        .priority = (osPriority_t)osPriorityHigh,
    };
    g_qrDecodeTaskHandle = osThreadNew(QrDecodeTask, NULL, &qrDecodeTaskAttributes);
    g_qrDecodeMinuteTimer = osTimerNew(QrDecodeMinuteTimerFunc, osTimerPeriodic, NULL, NULL);
}

static void QrDecodeTask(void *argument)
{
    int32_t ret;
    osStatus_t osRet;
    Message_t rcvMsg;
    uint32_t waitTime;
    uint32_t count = 0;
    g_qrDecodeState = QR_DECODE_STATE_OFF;

    while (1) {
        waitTime = g_qrDecodeState == QR_DECODE_STATE_ON ? 1 : 1000;
        osRet = osMessageQueueGet(g_qrDecodeQueue, &rcvMsg, NULL, waitTime);
        if (osRet == osOK) {
            switch (rcvMsg.id) {
            case QRDECODE_MSG_START: {
                if (g_qrDecodeState == QR_DECODE_STATE_OFF) {
                    SetLvglHandlerAndSnapShot(false);
                    ret = QrDecodeInit(GetLvglGramAddr());
                    printf("decode init ret=%d\r\n", (int)ret);
                    if (ret == DecodeInitSuccess) {
                        printf("start qrdecode\r\n");
                        g_qrDecodeState = QR_DECODE_STATE_ON;
                        osTimerStart(g_qrDecodeMinuteTimer, 60000);
                    } else {
                        SetLvglHandlerAndSnapShot(true);
                    }
                } else {
                    printf("already started\r\n");
                }
            }
            break;
            case QRDECODE_MSG_STOP: {
                if (g_qrDecodeState == QR_DECODE_STATE_ON) {
                    osTimerStop(g_qrDecodeMinuteTimer);
                    QrDecodeDeinit();
                    g_qrDecodeState = QR_DECODE_STATE_OFF;
                    SetLvglHandlerAndSnapShot(true);
                    printf("stop qrdecode\r\n");
                } else {
                    printf("already stoped\r\n");
                }
            }
            break;
            case QRDECODE_MSG_MINUTE: {
                // printf("cam fps=%f\r\n", count / 60.0);
                // printf("cam tick=%d\r\n", QrDecodeGetCamTick() / count);
                // printf("view tick=%d\r\n", QrDecodeGetViewTick() / count);
                // printf("decode tick=%d\r\n", QrDecodeGetDecodeTick() / count);
                count = 0;
            }
            break;
            default:
                break;
            }
        }

        if (g_qrDecodeState == QR_DECODE_STATE_ON) {
            ProcessQr(count);
        }
    }
}

void ProcessQr(uint32_t count)
{
    char qrString[QR_DECODE_STRING_LEN] = {0};
    static uint8_t testProgress = 0;
    static bool firstQrFlag = true;
    static PtrDecoder decoder = NULL;
    static UrViewType_t urViewType = {0, 0};
    static struct URParseResult *urResult = NULL;

    uint32_t retFromRust = 0;
    int32_t ret = QrDecodeProcess(qrString, QR_DECODE_STRING_LEN, testProgress);
    if (ret > 0) {
        size_t qrLength = strnlen_s(qrString, QR_DECODE_STRING_LEN);
        if (qrLength >= QR_DECODE_STRING_LEN) {
            retFromRust = URDecodeError;
        } else if (firstQrFlag == true) {
            if (IsUrQrCode(qrString, qrLength)) {
                if (ValidateQrUrInSandbox(qrString, qrLength)) {
                    urResult = parse_ur(qrString);
                } else {
                    retFromRust = URDecodeError;
                }
            } else {
                urResult = parse_qrcode_text(qrString);
            }
            if ((retFromRust == 0U) && (urResult->error_code == 0)) {
                if (urResult->is_multi_part == 0) {
                    // single qr code
                    firstQrFlag = true;
                    urViewType.viewType = urResult->t;
                    urViewType.urType = urResult->ur_type;
                    handleURResult(urResult, NULL, urViewType, false);
                    urResult = NULL;
                    testProgress = 0;
                } else {
                    // first qr code
                    firstQrFlag = false;
                    decoder = urResult->decoder;
                    testProgress = urResult->progress;
                }
            } else if (retFromRust == 0U) {
                retFromRust = urResult->error_code;
            }

        } else {
            // follow qrcode
            if (!ValidateQrUrInSandbox(qrString, qrLength)) {
                retFromRust = URDecodeError;
            } else {
                struct URParseMultiResult *MultiurResult = receive(qrString, decoder);
                if (MultiurResult->error_code == 0) {
                    testProgress = MultiurResult->progress;
                    if (MultiurResult->is_complete) {
                        firstQrFlag = true;
                        urViewType.viewType = MultiurResult->t;
                        urViewType.urType = MultiurResult->ur_type;
                        if (urResult != NULL) {
                            free_ur_parse_result(urResult);
                            urResult = NULL;
                        }
                        handleURResult(NULL, MultiurResult, urViewType, true);
                        testProgress = 0;
                    }
                } else {
                    retFromRust = MultiurResult->error_code;
                    printf("error code: %d\r\n", (int)MultiurResult->error_code);
                    printf("error message: %s\r\n", MultiurResult->error_message);
                    if (urResult != NULL) {
                        free_ur_parse_result(urResult);
                        urResult = NULL;
                    }
                }
                if (!(MultiurResult->is_complete)) {
                    free_ur_parse_multi_result(MultiurResult);
                }
            }
        }

        if (retFromRust != 0) {
            if (urResult != NULL) {
                free_ur_parse_result(urResult);
                urResult = NULL;
            }
            firstQrFlag = true;
            decoder = NULL;
            testProgress = 0;
            StopQrDecode();
            UserDelay(500);
            urViewType.viewType = retFromRust;
            urViewType.urType = retFromRust;
            GuiApiEmitSignal(SIG_QRCODE_VIEW_SCAN_FAIL, &urViewType, sizeof(urViewType));
        }
    } else if (ret < 0) {
        printf("decode err=%d\r\n", (int)ret);
    }
    count++;
}

static bool IsUrQrCode(const char *qrString, size_t length)
{
    return (length >= 3U) &&
           ((qrString[0] == 'u') || (qrString[0] == 'U')) &&
           ((qrString[1] == 'r') || (qrString[1] == 'R')) &&
           (qrString[2] == ':');
}

static bool ValidateQrUrInSandbox(const char *qrString, size_t length)
{
    uint32_t status = MPU_SANDBOX_UR_NOT_CHECKED;

    if (!MpuSandboxValidateUr((const uint8_t *)qrString, length, &status)) {
        printf("QR UR sandbox failed closed\r\n");
        __DSB();
        NVIC_SystemReset();
        for (;;) {
        }
    }
    if (status != MPU_SANDBOX_UR_OK) {
        printf("QR UR sandbox rejected status=%lu\r\n", (unsigned long)status);
        return false;
    }
    return true;
}

void StartQrDecode(void)
{
    PubValueMsg(QRDECODE_MSG_START, 0);
}

void StopQrDecode(void)
{
    PubValueMsg(QRDECODE_MSG_STOP, 0);
}

static void QrDecodeMinuteTimerFunc(void *argument)
{
    PubValueMsg(QRDECODE_MSG_MINUTE, 0);
}

bool IsQrDecodeRunning(void)
{
    return (g_qrDecodeState == QR_DECODE_STATE_ON);
}

void QrDecodeTouchQuit(void)
{
    static bool quitArea = false;
    TouchStatus_t *pStatus;
    if (g_qrDecodeState == QR_DECODE_STATE_OFF) {
        return;
    }
    pStatus = GetLatestTouchStatus();
    if (pStatus->touch) {
        quitArea = true;
    } else {
        if (quitArea) {
            for (int i = 0; i < 1; i++) {
                if (pStatus->x >= g_qrDecodeObjects[i].start.x && pStatus->x <= g_qrDecodeObjects[i].end.x &&
                        pStatus->y >= g_qrDecodeObjects[i].start.y && pStatus->y <= g_qrDecodeObjects[i].end.y) {
                    osKernelLock();
                    StopQrDecode();
                    g_qrDecodeObjects[i].func();
                    ClearTouchBuffer();
                    osKernelUnlock();
                    quitArea = false;
                    return;
                }
            }
            quitArea = false;
        }
    }
}
