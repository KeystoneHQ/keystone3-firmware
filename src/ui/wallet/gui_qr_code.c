#include "gui_qr_code.h"
#include "safe_str_lib.h"

static lv_timer_t *g_timer;
static UREncodeResult *g_result;
static lv_obj_t *g_qr;
static UpdateFunc g_updateFunc;

static bool g_qrcodeSuspend = false;

void QRCodeControl(bool suspend)
{
    g_qrcodeSuspend = suspend;
}

static void QRTimerHandler(lv_timer_t *timer);
static void QRTimerHandler(lv_timer_t *timer)
{
#ifndef COMPILE_SIMULATOR
    if (g_result) {
        if (!g_qrcodeSuspend) {
            PtrEncoder encoder = g_result->encoder;
            UREncodeMultiResult *multiResult = get_next_part(encoder);
            if (multiResult->error_code == 0) {
                char *data = multiResult->data;
                uint16_t dataLen = strnlen_s(data, SIMPLERESPONSE_C_CHAR_MAX_LEN);
                g_updateFunc(g_qr, data, dataLen);
                free_ur_encode_muilt_result(multiResult);
            }
        }
    }
#endif
}

void UpdateQrCode(GetUR func, lv_obj_t *qr, UpdateFunc updateFunc)
{
    if (updateFunc == NULL) {
        g_updateFunc = lv_qrcode_update;
    } else {
        g_updateFunc = updateFunc;
    }
    g_result = func();
    if (g_result -> error_code == 0) {
        if (g_result->is_multi_part) {
            char *data = g_result->data;
            uint16_t dataLen = strnlen_s(data, SIMPLERESPONSE_C_CHAR_MAX_LEN);
            g_updateFunc(qr, data, dataLen);
            g_qr = qr;
            g_timer = lv_timer_create(QRTimerHandler, 200, NULL);
        } else {
            char *data = g_result->data;
            uint16_t dataLen = strnlen_s(data, SIMPLERESPONSE_C_CHAR_MAX_LEN);
            updateFunc(qr, data, dataLen);
            CloseQRTimer();
        }
    }
}

void ShowQrCode(GetUR func, lv_obj_t *qr)
{
    UpdateQrCode(func, qr, lv_qrcode_update);
}

void CloseQRTimer(void)
{
    if (g_timer) {
        lv_timer_del(g_timer);
        g_timer = NULL;
    }
    if (g_result) {
#ifndef COMPILE_SIMULATOR
        free_ur_encode_result(g_result);
#endif
        g_result = NULL;
    }
    if (g_qr) {
        g_qr = NULL;
    }
}