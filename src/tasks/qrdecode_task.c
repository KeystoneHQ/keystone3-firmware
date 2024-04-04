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
#include "assert.h"
#include "qrdecode_task.h"
#include "gui_resolve_ur.h"
#ifndef BTC_ONLY
#include "gui_key_derivation_request_widgets.h"
#endif

static void QrDecodeTask(void *argument);
static void QrDecodeMinuteTimerFunc(void *argument);

osThreadId_t g_qrDecodeTaskHandle;
static volatile QrDecodeStateType g_qrDecodeState;
osTimerId_t g_qrDecodeMinuteTimer;

void CreateQrDecodeTask(void)
{
    const osThreadAttr_t qrDecodeTaskAttributes = {
        .name = "qrDecodeTask",
        .stack_size = 1024 * 8,
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
                    printf("decode init ret=%d\r\n", ret);
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
    static char qrString[QR_DECODE_STRING_LEN] = {0};
    static uint8_t testProgress = 0;
    static bool firstQrFlag = true;
    static PtrDecoder decoder = NULL;
    static UrViewType_t urViewType = {0, 0};
    static struct URParseResult *urResult;

    uint32_t retFromRust = 0;
    int32_t ret = QrDecodeProcess(qrString, QR_DECODE_STRING_LEN, testProgress);

    if (ret > 0) {
        if (firstQrFlag == true) {
            assert(strnlen_s(qrString, QR_DECODE_STRING_LEN) < QR_DECODE_STRING_LEN);
            urResult = parse_ur(qrString);
            if (urResult->error_code == 0) {
                if (urResult->is_multi_part == 0) {
                    // single qr code
                    firstQrFlag = true;
                    urViewType.viewType = urResult->t;
                    urViewType.urType = urResult->ur_type;
                    handleURResult(urResult, NULL, urViewType, false);
                    testProgress = 0;
                } else {
                    // first qr code
                    firstQrFlag = false;
                    decoder = urResult->decoder;
                    testProgress = urResult->progress;
                }
            } else {
                retFromRust = urResult->error_code;
            }
        } else {
            // follow qrcode
            struct URParseMultiResult *MultiurResult = receive(qrString, decoder);
            if (MultiurResult->error_code == 0) {
                testProgress = MultiurResult->progress;
                if (MultiurResult->is_complete) {
                    firstQrFlag = true;
                    urViewType.viewType = MultiurResult->t;
                    urViewType.urType = MultiurResult->ur_type;
                    handleURResult(urResult, MultiurResult, urViewType, true);
                    testProgress = 0;
                }
            } else {
                retFromRust = MultiurResult->error_code;
                printf("error code: %d\r\n", MultiurResult->error_code);
                printf("error message: %s\r\n", MultiurResult->error_message);
            }
            if (!(MultiurResult->is_complete)) {
                free_ur_parse_multi_result(MultiurResult);
            }
        }

        if (retFromRust != 0) {
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
        printf("decode err=%d\r\n", ret);
    }
    count++;
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

#define QUIT_AREA_X_START 10
#define QUIT_AREA_Y_START 64
#define QUIT_AREA_X_END QUIT_AREA_X_START + 64
#define QUIT_AREA_Y_END QUIT_AREA_Y_START + 64

void QrDecodeTouchQuit(void)
{
    static bool quitArea = false;
    TouchStatus_t *pStatus;
    if (g_qrDecodeState == QR_DECODE_STATE_OFF) {
        return;
    }
    pStatus = GetLatestTouchStatus();
    if (pStatus->touch) {
        if (pStatus->x >= QUIT_AREA_X_START && pStatus->x <= QUIT_AREA_X_END &&
                pStatus->y >= QUIT_AREA_Y_START && pStatus->y <= QUIT_AREA_Y_END) {
            quitArea = true;
        } else {
            quitArea = false;
        }
    } else {
        if (quitArea) {
            quitArea = false;
            osKernelLock();
            StopQrDecode();
            LvglCloseCurrentView();
            ClearTouchBuffer();
            osKernelUnlock();
        }
        quitArea = false;
    }
}
