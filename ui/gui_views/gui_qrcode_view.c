/*
 * Copyright (c) keyst.one. 2020-2025. All rights reserved.
 * @FilePath: \project-pillar-firmware\ui\gui_views\gui_qrcode_view.c
 * @Description:
 * @Author: stone wang
 * @LastEditTime: 2023-04-14 13:27:47
 */
#include "gui.h"
#include "gui_obj.h"
#include "gui_model.h"
#include "gui_views.h"
#include "gui_qrcode_widgets.h"
#include "screen_manager.h"
#include "gui_lock_widgets.h"

static int32_t GuiQrCodeViewInit(void)
{
    SetPageLockScreen(false);
    GuiQrCodeScreenInit();
    return SUCCESS_CODE;
}

static int32_t GuiQrCodeViewDeInit(void)
{
    GuiQrCodeDeInit();
    SetPageLockScreen(true);
    return SUCCESS_CODE;
}

int32_t GuiQrCodeViewEventProcess(void *self, uint16_t usEvent, void *param, uint16_t usLen)
{
    switch (usEvent) {
    case GUI_EVENT_OBJ_INIT:
        return GuiQrCodeViewInit();
    case GUI_EVENT_OBJ_DEINIT:
        return GuiQrCodeViewDeInit();
    case GUI_EVENT_REFRESH:
        GuiQrCodeRefresh();
        break;
    case SIG_QRCODE_VIEW_SCAN_FAIL:
        GuiQrCodeScanResult(false, param);
        break;
    case SIG_QRCODE_VIEW_SCAN_PASS:
        GuiQrCodeScanResult(true, param);
        break;
    case SIG_VERIFY_PASSWORD_PASS:
        if (param != NULL) {
            uint16_t sig = *(uint16_t *)param;
            if (sig == SIG_LOCK_VIEW_SCREEN_GO_HOME_PASS) {
                GuiLockScreenToHome();
                return SUCCESS_CODE;
            }
        }
        GuiQrCodeVerifyPasswordResult(true);
        break;
    case SIG_VERIFY_PASSWORD_FAIL:
        if (param != NULL) {
            PasswordVerifyResult_t *passwordVerifyResult = (PasswordVerifyResult_t *)param;
            uint16_t sig = *(uint16_t *) passwordVerifyResult->signal;
            if (sig == SIG_LOCK_VIEW_SCREEN_GO_HOME_PASS) {
                GuiLockScreenPassCode(false);
                GuiLockScreenErrorCount(param);
                return SUCCESS_CODE;
            }
        }
        GuiQrCodeVerifyPasswordResult(false);
        GuiLockScreenPassCode(false);
        GuiQrCodeVerifyPasswordErrorCount(param);
        break;
    case SIG_FINGER_RECOGNIZE_RESPONSE:
        GuiQrCodeDealFingerRecognize(param);
        break;
    default:
        return ERR_GUI_UNHANDLED;
    }
    return SUCCESS_CODE;
}

GUI_VIEW g_qrCodeView = {
    .id = SCREEN_QRCODE,
    .previous = NULL,
    .isActive = false,
    .optimization = false,
    .pEvtHandler = GuiQrCodeViewEventProcess,
};

