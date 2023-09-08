/*
 * Copyright (c) keyst.one. 2020-2025. All rights reserved.
 * @FilePath: \project-pillar-firmware\ui\gui_widgets\gui_lock_widgets.c
 * @Description:
 * @Author: stone wang
 * @LastEditTime: 2023-04-04 16:53:20
 */
#include "gui.h"
#include "gui_obj.h"
#include "gui_views.h"
#include "gui_enter_passcode.h"
#include "gui_status_bar.h"
#include "gui_model.h"
#include "gui_lock_widgets.h"
#include "gui_status_bar.h"
#include "gui_hintbox.h"
#include "gui_api.h"
#include "keystore.h"
#include "gui_lock_device_widgets.h"
#include "fingerprint_process.h"
#ifdef COMPILE_SIMULATOR
#define FINGERPRINT_EN_SING_ERR_TIMES           (5)
#define FINGERPRINT_SING_ERR_TIMES              (3)
#define FINGERPRINT_SING_DISABLE_ERR_TIMES      (15)
#else
#include "drv_usb.h"
#endif

static bool ModelGetPassphraseQuickAccess(void);
static void GuiShowPasswordErrorHintBox(void);
static void UnlockDeviceHandler(lv_event_t *e);
static void CountDownTimerWipeDeviceHandler(lv_timer_t *timer);
static void GuiHintBoxToLockSreen(void);
static void GuiCountDownDestruct(void *obj, void *param);
static void GuiPassowrdToLockTimePage(uint16_t errorCount);
void GuiLockScreenClearModal(lv_obj_t *cont);
static char* GuiJudgeTitle();

static GuiEnterPasscodeItem_t *g_verifyLock = NULL;
static lv_obj_t *g_lockScreenCont;
static lv_obj_t *g_errorHintBox;
static lv_timer_t *g_countDownTimer;
static int8_t countDown = 5;
static bool g_firstUnlock = true;
static uint8_t g_fpErrorCount = 0;
static LOCK_SCREEN_PURPOSE_ENUM g_purpose = LOCK_SCREEN_PURPOSE_UNLOCK;

void GuiLockScreenUpdatePurpose(LOCK_SCREEN_PURPOSE_ENUM purpose)
{
    g_purpose = purpose;
}

void GuiFpRecognizeResult(bool en)
{
    if (en) {
        g_fpErrorCount = 0;
        GuiNvsBarSetMidBtnLabel(NVS_BAR_MID_LABEL, "");
        ClearCurrentPasswordErrorCount();
    } else {
        g_fpErrorCount++;
        if (g_fpErrorCount < FINGERPRINT_EN_SING_ERR_TIMES) {
            GuiNvsBarSetMidBtnLabel(NVS_BAR_MID_LABEL, "Try Again");
            FpRecognize(RECOGNIZE_UNLOCK);
        } else {
            char* title;
            if (g_verifyLock->mode == ENTER_PASSCODE_LOCK_VERIFY_PIN || g_verifyLock->mode == ENTER_PASSCODE_VERIFY_PIN) {
                title = "Use PIN";
            } else {
                title = "Use Password";
            }
            GuiNvsBarSetMidBtnLabel(NVS_BAR_MID_LABEL, title);
        }
    }
    GuiEnterPassCodeStatus(g_verifyLock, true);
    printf("GuiFpRecognizeResult g_fpErrorCount is %d....\n", g_fpErrorCount);
    GuiFingerPrintStatus(g_verifyLock, en, g_fpErrorCount);
}

void GuiLockScreenClearFirstUnlock(void)
{
    g_firstUnlock = false;
}

void GuiLockScreenFpRecognize(void)
{
    if (g_fpErrorCount < FINGERPRINT_EN_SING_ERR_TIMES) {
        FpRecognize(RECOGNIZE_UNLOCK);
    }
}

bool GuiLockScreenIsFirstUnlock(void)
{
    return g_firstUnlock;
}

bool GuiLockScreenIsTop(void)
{
    if (g_lockScreenCont != NULL) {
        return !lv_obj_has_flag(g_lockScreenCont, LV_OBJ_FLAG_HIDDEN);
    }
    return false;
}

void GuiLockScreenHidden(void)
{
    if (g_lockScreenCont != NULL) {
        lv_obj_add_flag(g_lockScreenCont, LV_OBJ_FLAG_HIDDEN);
    }
}

void OpenForgetPasswordHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_CLICKED) {
        GUI_VIEW *view = (GUI_VIEW *)lv_event_get_user_data(e);
        FpCancelCurOperate();
        lv_obj_add_flag(g_lockScreenCont, LV_OBJ_FLAG_HIDDEN);
        GuiFrameOpenViewWithParam(&g_forgetPassView, view, sizeof(view));
    }
}

void GuiLockScreenTurnOn(void *param)
{
    uint16_t *single = param;
    if (*single == SIG_LOCK_VIEW_VERIFY_PIN || *single == SIG_LOCK_VIEW_SCREEN_GO_HOME_PASS) {
        GuiNvsBarSetWalletIcon(NULL);
        GuiNvsBarSetWalletName("");
    }
    lv_obj_clear_flag(g_lockScreenCont, LV_OBJ_FLAG_HIDDEN);
    // g_lockView.isActive = true;
    lv_obj_set_parent(g_lockScreenCont, lv_scr_act());
    GuiUpdateEnterPasscodeParam(g_verifyLock, single);
    GuilockScreenRefresh();
}

static uint8_t g_oldWalletIndex = 0xFF;
void GuiLockScreenTurnOff(void)
{
    static uint16_t single = SIG_LOCK_VIEW_VERIFY_PIN;
    lv_obj_add_flag(g_lockScreenCont, LV_OBJ_FLAG_HIDDEN);
    GuiModeGetWalletDesc();
    GuiEnterPassCodeStatus(g_verifyLock, true);

    if (g_oldWalletIndex == 0xFF) {
        g_oldWalletIndex = GetCurrentAccountIndex();
    }

    printf("oldWalletIndex = %d\n", g_oldWalletIndex);
    printf("GetCurrentAccountIndex() = %d\n", GetCurrentAccountIndex());
    if (GetCurrentAccountIndex() != g_oldWalletIndex) {
        g_oldWalletIndex = GetCurrentAccountIndex();
        GuiCloseToTargetView(&g_homeView);
    } else {
        GuiEmitSignal(GUI_EVENT_REFRESH, &single, sizeof(single));
    }
    // g_lockView.isActive = false;
}

void GuiUpdateOldAccountIndex(void)
{
    g_oldWalletIndex = GetCurrentAccountIndex();
}

void GuiLockScreenToHome(void)
{
    lv_obj_add_flag(g_lockScreenCont, LV_OBJ_FLAG_HIDDEN);
    GuiModeGetWalletDesc();
    GuiEnterPassCodeStatus(g_verifyLock, true);
    GuiCloseToTargetView(&g_homeView);
}

void GuiLockScreenTurnOffHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        GuiLockScreenTurnOff();
    }
}

void GuiLockScreenPassCode(bool en)
{
    GuiEnterPassCodeStatus(g_verifyLock, en);
    if (en) {
        g_fpErrorCount = 0;
        FpCancelCurOperate();
        // if (ModelGetPassphraseQuickAccess()) {
        if (0) {
            if (g_passphraseView.isActive) {
                GuiLockScreenTurnOff();
            } else {
                GuiFrameOpenView(&g_passphraseView);
            }
            printf("passphrase quick access\r\n");
        } else if (g_homeView.isActive) {
            printf("g_homeView.isActive\r\n");
            GuiLockScreenTurnOff();
        } else if (g_forgetPassView.isActive) {
            printf("g_forgetPassView.isActive\r\n");
            GuiLockScreenTurnOff();
        } else {
            lv_obj_add_flag(g_lockScreenCont, LV_OBJ_FLAG_HIDDEN);
            GuiNvsBarSetMidCb(NVS_MID_BUTTON_BUTT, NULL, NULL);
            GuiFrameOpenView(&g_homeView);
            if (g_oldWalletIndex == 0xFF) {
                g_oldWalletIndex = GetCurrentAccountIndex();
            }
            printf("%s %d\n", __func__, __LINE__);
        }
        UsbInit();
    }
}

void GuiLockScreenErrorCount(void *param)
{
    PasswordVerifyResult_t *passwordVerifyResult = (PasswordVerifyResult_t *)param;
    printf("GuiLockScreenErrorCount  errorcount is %d\n", passwordVerifyResult->errorCount);
    if (g_verifyLock != NULL) {
        char hint[128];

        if (*(uint16_t *)passwordVerifyResult->signal == SIG_LOCK_VIEW_VERIFY_PIN
                || *(uint16_t *)passwordVerifyResult->signal == SIG_LOCK_VIEW_SCREEN_GO_HOME_PASS) {
            sprintf(hint, "Incorrect password, you have #F55831 %d# chances left", (MAX_LOGIN_PASSWORD_ERROR_COUNT - passwordVerifyResult->errorCount));
            lv_label_set_text(g_verifyLock->errLabel, hint);
            GuiPassowrdToLockTimePage(MAX_LOGIN_PASSWORD_ERROR_COUNT - passwordVerifyResult->errorCount);
            if (passwordVerifyResult->errorCount == MAX_LOGIN_PASSWORD_ERROR_COUNT) {
                // GuiCLoseCurrentWorkingView();
                GuiFrameOpenView(&g_lockDeviceView);
                FpCancelCurOperate();
            }
        } else {
            sprintf(hint, "Incorrect password, you have #F55831 %d# chances left", (MAX_CURRENT_PASSWORD_ERROR_COUNT_SHOW_HINTBOX - passwordVerifyResult->errorCount));
            lv_label_set_text(g_verifyLock->errLabel, hint);
            printf("GuiLockScreenErrorCount  errorcount is %d sig is %d \n", passwordVerifyResult->errorCount, *(uint16_t *)passwordVerifyResult->signal);
            if (passwordVerifyResult->errorCount == MAX_CURRENT_PASSWORD_ERROR_COUNT_SHOW_HINTBOX) {
                GuiShowPasswordErrorHintBox();
            }
        }
    }
}

static void GuiPassowrdToLockTimePage(uint16_t leftErrorCount)
{
    static uint16_t staticLeftErrorCount;
    if (leftErrorCount < 5 && leftErrorCount > 0) {
        staticLeftErrorCount = leftErrorCount;
        // GuiCLoseCurrentWorkingView();
        GuiFrameOpenViewWithParam(&g_lockDeviceView, &staticLeftErrorCount, sizeof(staticLeftErrorCount));
        GuiModelWriteLastLockDeviceTime(GetCurrentStampTime());
    }
}

static void GuiShowPasswordErrorHintBox(void)
{
    if (g_errorHintBox == NULL) {
        g_errorHintBox = GuiCreateResultHintbox(lv_scr_act(), 386, &imgFailed,
                                                "Attempt Limit Exceeded", "Device lock imminent. Please unlock to access the device.",
                                                NULL, DARK_GRAY_COLOR, "Unlock Device (5s)", DARK_GRAY_COLOR);
    }

    if (g_errorHintBox != NULL) {
        lv_obj_set_parent(g_errorHintBox, lv_scr_act());
        if (lv_obj_has_flag(g_errorHintBox, LV_OBJ_FLAG_HIDDEN)) {
            lv_obj_clear_flag(g_errorHintBox, LV_OBJ_FLAG_HIDDEN);
        }
        lv_obj_t *btn = GuiGetHintBoxRightBtn(g_errorHintBox);
        lv_label_set_text(lv_obj_get_child(btn, 0), "Unlock Device (5s)");

        lv_obj_remove_event_cb(btn, UnlockDeviceHandler);
        lv_obj_add_event_cb(btn, UnlockDeviceHandler, LV_EVENT_CLICKED, NULL);
        g_countDownTimer = lv_timer_create(CountDownTimerWipeDeviceHandler, 1000, btn);
    }
}

static void CountDownTimerWipeDeviceHandler(lv_timer_t *timer)
{
    lv_obj_t *obj = (lv_obj_t *)timer->user_data;
    char buf[32] = {0};
    --countDown;
    if (countDown > 0) {
        sprintf(buf, "Unlock Device (%ds)", countDown);
    } else {
        strcpy(buf, "Unlock Device");
    }
    lv_label_set_text(lv_obj_get_child(obj, 0), buf);
    if (countDown <= 0) {
        GuiHintBoxToLockSreen();
        GuiCountDownDestruct(NULL, NULL);
    }
}

static void UnlockDeviceHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        GuiHintBoxToLockSreen();
        GuiCountDownDestruct(NULL, NULL);
    }
}

static void GuiHintBoxToLockSreen(void)
{
    static uint16_t sig = SIG_LOCK_VIEW_SCREEN_GO_HOME_PASS;
    GuiLockScreenUpdatePurpose(LOCK_SCREEN_PURPOSE_UNLOCK);
    GuiEmitSignal(SIG_LOCK_VIEW_SCREEN_ON_VERIFY, &sig, sizeof(sig));
    if (g_verifyLock != NULL) {
        if (!lv_obj_has_flag(g_verifyLock->errLabel, LV_OBJ_FLAG_HIDDEN)) {
            lv_obj_add_flag(g_verifyLock->errLabel, LV_OBJ_FLAG_HIDDEN);
        }
    }
    if (g_errorHintBox != NULL) {
        if (!lv_obj_has_flag(g_errorHintBox, LV_OBJ_FLAG_HIDDEN)) {
            lv_obj_add_flag(g_errorHintBox, LV_OBJ_FLAG_HIDDEN);
        }
    }
}

static void GuiCountDownDestruct(void *obj, void *param)
{
    if (g_countDownTimer != NULL) {
        countDown = 5;
        lv_timer_del(g_countDownTimer);
        g_countDownTimer = NULL;
        UNUSED(g_countDownTimer);
    }
}

void GuiLockScreenTurnOnHandler(lv_event_t *e)
{
    static uint16_t single = 0;
    lv_event_code_t code = lv_event_get_code(e);
    GuiLockScreenUpdatePurpose(LOCK_SCREEN_PURPOSE_VERIFY);
    if (code == LV_EVENT_CLICKED) {
        uint16_t *walletSetIndex = lv_event_get_user_data(e);
        single = *walletSetIndex;
        GuiEmitSignal(SIG_LOCK_VIEW_SCREEN_ON_VERIFY, &single, sizeof(single));
        GuiNvsBarSetLeftCb(NVS_BAR_CLOSE, GuiLockScreenTurnOffHandler, NULL);
    }
}

void GuiLockScreenInit(void *param)
{
    lv_obj_t *cont = GuiCreateContainer(lv_obj_get_width(lv_scr_act()), lv_obj_get_height(lv_scr_act()) -
                                        GUI_MAIN_AREA_OFFSET);
    lv_obj_align(cont, LV_ALIGN_DEFAULT, 0, GUI_STATUS_BAR_HEIGHT + GUI_NAV_BAR_HEIGHT);
    g_lockScreenCont = cont;
    GuiEnterPasscodeItem_t *item = GuiCreateEnterPasscode(cont, NULL, param, ENTER_PASSCODE_VERIFY_PIN);
    g_verifyLock = item;

    GuiEmitSignal(SIG_LOCK_VIEW_SCREEN_GO_LOCK_DEVICE_PASS, NULL, 0);
}


void GuiJumpToLockDevicePage(void)
{
    uint8_t errorCount = GetLoginPasswordErrorCount();
    printf("GuiJumpToLockDevicePage errorCount is  %d\n", errorCount);

    if (errorCount > 5) {
        static uint16_t staticLeftErrorCount;
        uint32_t lastLockDeviceTime = GetLastLockDeviceTime();
        staticLeftErrorCount = MAX_LOGIN_PASSWORD_ERROR_COUNT    - errorCount;
        uint32_t currentTime = GetCurrentStampTime();
        uint32_t diffTime = currentTime - lastLockDeviceTime;
        printf("diffTime is  %ds\n", diffTime);
        printf("lastLockDeviceTime is  %ds\n", lastLockDeviceTime);
        if (diffTime < GuiGetLockTimeByLeftErrorCount(staticLeftErrorCount)) {
            printf("continue lock time %ds\n", diffTime);
            GuiFrameOpenViewWithParam(&g_lockDeviceView, &staticLeftErrorCount, sizeof(staticLeftErrorCount));
        }
    }
}

void GuiLockScreenPasscodeSwitch(bool isPin)
{
    GuiNvsBarSetMidBtnLabel(NVS_BAR_MID_LABEL, GuiJudgeTitle());
}

static char* GuiJudgeTitle()
{
    char* title;
    //no fingerprint presents
    if (g_fpErrorCount >= FINGERPRINT_EN_SING_ERR_TIMES || (GetRegisteredFingerNum() < 1) || GuiLockScreenIsFirstUnlock()) {
        if (g_verifyLock->mode == ENTER_PASSCODE_LOCK_VERIFY_PIN || g_verifyLock->mode == ENTER_PASSCODE_VERIFY_PIN) {
            title = "Use PIN";
        } else {
            title = "Use Password";
        }
        return title;
    }
    if (GetFingerUnlockFlag() && g_purpose == LOCK_SCREEN_PURPOSE_UNLOCK) {
        if (g_verifyLock->mode == ENTER_PASSCODE_LOCK_VERIFY_PIN || g_verifyLock->mode == ENTER_PASSCODE_VERIFY_PIN) {
            title = "Use PIN or Use Fingerprint";
        } else {
            title = "Use Password or Use Fingerprint";
        }
        return title;
    }
    if (g_verifyLock->mode == ENTER_PASSCODE_LOCK_VERIFY_PIN || g_verifyLock->mode == ENTER_PASSCODE_VERIFY_PIN) {
        title = "Use PIN";
    } else {
        title = "Use Password";
    }
    return title;
}

void GuilockScreenRefresh(void)
{
    GuiNvsBarSetRightCb(NVS_RIGHT_BUTTON_BUTT, NULL, NULL);
    GuiNvsBarSetLeftCb(NVS_LEFT_BUTTON_BUTT, NULL, NULL);
    GuiFingerPrintStatus(g_verifyLock, true, g_fpErrorCount);
    if (GuiCheckIfTopView(&g_lockDeviceView)) {
        GuiNvsBarSetMidCb(NVS_MID_BUTTON_BUTT, NULL, NULL);
        GuiNvsBarSetMidBtnLabel(NVS_BAR_MID_LABEL, "");
    } else {
        GuiNvsBarSetMidBtnLabel(NVS_BAR_MID_LABEL, GuiJudgeTitle());
    }
    if (g_lockScreenCont != NULL) {
        lv_obj_clear_flag(g_lockScreenCont, LV_OBJ_FLAG_HIDDEN);
    }
}

void GuiLockScreenUpdatePassCode(void)
{
    static uint16_t single = SIG_LOCK_VIEW_VERIFY_PIN;
    if (g_verifyLock != NULL) {
        GuiUpdateEnterPasscodeParam(g_verifyLock, &single);
    }
    GuilockScreenRefresh();
}

#ifndef COMPILE_SIMULATOR
#include "keystore.h"
#endif


static bool ModelGetPassphraseQuickAccess(void)
{
#ifdef COMPILE_SIMULATOR
    return false;
#else
    if (PassphraseExist(GetCurrentAccountIndex()) == false && GetPassphraseQuickAccess() == true && GetPassphraseMark() == true) {
        return true;
    } else {
        return false;
    }
#endif
}

