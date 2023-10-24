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
#include "device_setting.h"
#include "gui_page.h"
#include "account_manager.h"
#include "user_memory.h"
#include "presetting.h"
#include "assert.h"

#ifdef COMPILE_SIMULATOR
#define FINGERPRINT_EN_SING_ERR_TIMES           (5)
#define FINGERPRINT_SING_ERR_TIMES              (3)
#define FINGERPRINT_SING_DISABLE_ERR_TIMES      (15)
#else
#include "drv_usb.h"
#endif

#ifdef COMPILE_MAC_SIMULATOR
#include "simulator_model.h"
#endif

static bool ModelGetPassphraseQuickAccess(void);
static void GuiPassowrdToLockTimePage(uint16_t errorCount);
void GuiLockScreenClearModal(lv_obj_t *cont);
static char* GuiJudgeTitle();
static void CountDownTimerChangeLabelTextHandler(lv_timer_t *timer);

static GuiEnterPasscodeItem_t *g_verifyLock = NULL;
static lv_obj_t *g_lockScreenCont;
static bool g_firstUnlock = true;
static uint8_t g_fpErrorCount = 0;
static LOCK_SCREEN_PURPOSE_ENUM g_purpose = LOCK_SCREEN_PURPOSE_UNLOCK;
static PageWidget_t *g_pageWidget;

static lv_obj_t *g_LoadingView = NULL;
static lv_timer_t *g_countDownTimer;
static int8_t g_countDown = 0;
static bool g_canDismissLoading = false;

void GuiLockScreenUpdatePurpose(LOCK_SCREEN_PURPOSE_ENUM purpose)
{
    g_purpose = purpose;
}

bool GuiNeedFpRecognize(void)
{
    if (g_fpErrorCount < FINGERPRINT_EN_SING_ERR_TIMES) {
        return true;
    } else {
        return false;
    }
}

void GuiFpRecognizeResult(bool en)
{
    if (en) {
        g_fpErrorCount = 0;
        SetMidBtnLabel(g_pageWidget->navBarWidget, NVS_BAR_MID_LABEL, "");
        ClearCurrentPasswordErrorCount();
    } else {
        g_fpErrorCount++;
        if (g_fpErrorCount < FINGERPRINT_EN_SING_ERR_TIMES) {
            SetMidBtnLabel(g_pageWidget->navBarWidget, NVS_BAR_MID_LABEL, _("try_again"));
            FpRecognize(RECOGNIZE_UNLOCK);
        } else {
            char* title;
            if (g_verifyLock->mode == ENTER_PASSCODE_LOCK_VERIFY_PIN || g_verifyLock->mode == ENTER_PASSCODE_VERIFY_PIN) {
                title = _("unlock_device_use_pin");
            } else {
                title = _("unlock_device_use_password");
            }
            SetMidBtnLabel(g_pageWidget->navBarWidget, NVS_BAR_MID_LABEL, title);
        }
    }
    GuiEnterPassCodeStatus(g_verifyLock, true);
    printf("GuiFpRecognizeResult g_fpErrorCount is %d....\n", g_fpErrorCount);
    GuiFingerPrintStatus(g_verifyLock, en, g_fpErrorCount);
}

void GuiLockScreenFpRecognize(void)
{
    if (g_fpErrorCount < FINGERPRINT_EN_SING_ERR_TIMES) {
        FpRecognize(RECOGNIZE_UNLOCK);
    }
}

void GuiLockScreenClearFirstUnlock(void)
{
    g_firstUnlock = false;
}

void GuiLockScreenSetFirstUnlock(void)
{
    g_firstUnlock = true;
}

bool GuiLockScreenIsFirstUnlock(void)
{
    return g_firstUnlock;
}

static void LockViewWipeDeviceHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        lv_obj_t *cont = GuiCreateContainer(lv_obj_get_width(lv_scr_act()), lv_obj_get_height(lv_scr_act()) -
                                            GUI_STATUS_BAR_HEIGHT);
        lv_obj_add_flag(cont, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_align(cont, LV_ALIGN_DEFAULT, 0, GUI_STATUS_BAR_HEIGHT);
        GuiCreateCircleAroundAnimation(lv_scr_act(), -40);

        lv_obj_t *label = GuiCreateTextLabel(cont, _("system_settings_wipe_device_generating_title"));
        lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 355);

        label = GuiCreateNoticeLabel(cont, _("system_settings_wipe_device_generating_desc1"));
        lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 410);

        label = GuiCreateNoticeLabel(cont, _("system_settings_wipe_device_generating_desc2"));
        lv_obj_set_width(label, 408);
        lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 612);
        GuiModelLockedDeviceDelAllWalletDesc();
    }
}

void GuiLockScreenWipeDevice(void)
{
    lv_obj_t *cont = GuiCreateContainer(lv_obj_get_width(lv_scr_act()), lv_obj_get_height(lv_scr_act()) -
                                        GUI_STATUS_BAR_HEIGHT);
    lv_obj_add_flag(cont, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_align(cont, LV_ALIGN_DEFAULT, 0, GUI_STATUS_BAR_HEIGHT);

    lv_obj_t *img = GuiCreateImg(cont, &imgWarn);
    lv_obj_align(img, LV_ALIGN_TOP_MID, 0, 180 - GUI_STATUS_BAR_HEIGHT);

    lv_obj_t *label = GuiCreateTextLabel(cont, _("error_unknown_error"));
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 284 - GUI_STATUS_BAR_HEIGHT);

    lv_obj_t *desc = GuiCreateNoticeLabel(cont, _("error_unknown_error_desc"));
    lv_obj_set_style_text_align(desc, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    lv_obj_align(desc, LV_ALIGN_TOP_MID, 0, 336 - GUI_STATUS_BAR_HEIGHT);

    label = GuiCreateIllustrateLabel(cont, _("support_link"));
    lv_obj_set_style_text_color(label, LIGHT_BLUE_COLOR, LV_PART_MAIN);
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 570 - GUI_STATUS_BAR_HEIGHT);

    char tempBuf[32] = {0};
    char showString[64] = {0};
    GetSerialNumber(tempBuf);
    sprintf(showString, "SN:%s", tempBuf);
    label = GuiCreateNoticeLabel(cont, showString);
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 620 - GUI_STATUS_BAR_HEIGHT);

    lv_obj_t *btn = GuiCreateBtn(cont, _("wipe_device"));
    lv_obj_set_style_bg_color(btn, RED_COLOR, LV_PART_MAIN);
    lv_obj_set_size(btn, 408, 66);
    lv_obj_align(btn, LV_ALIGN_TOP_MID, 0, 690 - GUI_STATUS_BAR_HEIGHT);
    lv_obj_add_event_cb(btn, LockViewWipeDeviceHandler, LV_EVENT_CLICKED, NULL);
}

bool GuiLockScreenIsTop(void)
{
    if (g_pageWidget != NULL && g_pageWidget->page != NULL) {
        return !lv_obj_has_flag(g_pageWidget->page, LV_OBJ_FLAG_HIDDEN);
    }
    return false;
}

void GuiLockScreenHidden(void)
{
    if (g_pageWidget->page != NULL) {
        lv_obj_add_flag(g_pageWidget->page, LV_OBJ_FLAG_HIDDEN);
    }
}

void OpenForgetPasswordHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_CLICKED) {
        GUI_VIEW *view = (GUI_VIEW *)lv_event_get_user_data(e);
        FpCancelCurOperate();
        lv_obj_add_flag(g_pageWidget->page, LV_OBJ_FLAG_HIDDEN);
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
    lv_obj_clear_flag(g_pageWidget->page, LV_OBJ_FLAG_HIDDEN);
    // g_lockView.isActive = true;
    lv_obj_set_parent(g_pageWidget->page, lv_scr_act());
    GuiUpdateEnterPasscodeParam(g_verifyLock, single);
    GuilockScreenRefresh();
}

static uint8_t g_oldWalletIndex = 0xFF;
void GuiLockScreenTurnOff(void)
{
    static uint16_t single = SIG_LOCK_VIEW_VERIFY_PIN;
    lv_obj_add_flag(g_pageWidget->page, LV_OBJ_FLAG_HIDDEN);
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

    // Close the loading page after closing the lock screen page
    if (g_canDismissLoading) {
        if (g_LoadingView != NULL && lv_obj_is_valid(g_LoadingView)) {
            GuiStopCircleAroundAnimation();
            lv_obj_del(g_LoadingView);
            g_LoadingView = NULL;
        }
    }
    
}

void GuiUpdateOldAccountIndex(void)
{
    g_oldWalletIndex = GetCurrentAccountIndex();
}

void GuiLockScreenToHome(void)
{
    lv_obj_add_flag(g_pageWidget->page, LV_OBJ_FLAG_HIDDEN);
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
        if (ModelGetPassphraseQuickAccess()) {
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
            lv_obj_add_flag(g_pageWidget->page, LV_OBJ_FLAG_HIDDEN);
            SetNavBarMidBtn(g_pageWidget->navBarWidget, NVS_MID_BUTTON_BUTT, NULL, NULL);
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
        int leftCount = 0;

        if (*(uint16_t *)passwordVerifyResult->signal == SIG_LOCK_VIEW_VERIFY_PIN
                || *(uint16_t *)passwordVerifyResult->signal == SIG_LOCK_VIEW_SCREEN_GO_HOME_PASS) {
            leftCount = MAX_LOGIN_PASSWORD_ERROR_COUNT - passwordVerifyResult->errorCount;
            ASSERT(leftCount >= 0);
            sprintf(hint, _("unlock_device_attempts_left_times_fmt"), (MAX_LOGIN_PASSWORD_ERROR_COUNT - passwordVerifyResult->errorCount));
            lv_label_set_text(g_verifyLock->errLabel, hint);
            GuiPassowrdToLockTimePage(MAX_LOGIN_PASSWORD_ERROR_COUNT - passwordVerifyResult->errorCount);
            if (passwordVerifyResult->errorCount == MAX_LOGIN_PASSWORD_ERROR_COUNT) {
                GuiFrameOpenView(&g_lockDeviceView);
                FpCancelCurOperate();
            }
        }
    }
}

static void GuiPassowrdToLockTimePage(uint16_t leftErrorCount)
{
    static uint16_t staticLeftErrorCount;
    if (leftErrorCount < 5 && leftErrorCount > 0) {
        staticLeftErrorCount = leftErrorCount;
        GuiFrameOpenViewWithParam(&g_lockDeviceView, &staticLeftErrorCount, sizeof(staticLeftErrorCount));
        GuiModelWriteLastLockDeviceTime(GetCurrentStampTime());
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
        SetNavBarLeftBtn(g_pageWidget->navBarWidget, NVS_BAR_CLOSE, GuiLockScreenTurnOffHandler, NULL);
    }
}


void GuiLockScreenInit(void *param)
{
    g_pageWidget = CreatePageWidget();
    lv_obj_t *cont = g_pageWidget->contentZone;

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
    SetMidBtnLabel(g_pageWidget->navBarWidget, NVS_BAR_MID_LABEL, GuiJudgeTitle());
}

static char* GuiJudgeTitle()
{
    char* title;
    //no fingerprint presents
    if (g_fpErrorCount >= FINGERPRINT_EN_SING_ERR_TIMES || (GetRegisteredFingerNum() < 1) || GuiLockScreenIsFirstUnlock()) {
        if (g_verifyLock->mode == ENTER_PASSCODE_LOCK_VERIFY_PIN || g_verifyLock->mode == ENTER_PASSCODE_VERIFY_PIN) {
            title = _("unlock_device_use_pin");
        } else {
            title = _("unlock_device_use_password");
        }
        return title;
    }
    if (GetFingerUnlockFlag() && g_purpose == LOCK_SCREEN_PURPOSE_UNLOCK) {
        title = SRAM_MALLOC(64);
        memset(title, 0, 64);
        if (g_verifyLock->mode == ENTER_PASSCODE_LOCK_VERIFY_PIN || g_verifyLock->mode == ENTER_PASSCODE_VERIFY_PIN) {
            sprintf(title, _("unlock_device_title_fmt"), _("unlock_device_use_pin"), _("unlock_device_use_fingerprint"));
        } else {
            sprintf(title, _("unlock_device_title_fmt"), _("unlock_device_use_password"), _("unlock_device_use_fingerprint"));
        }
        return title;
    }
    if (g_verifyLock->mode == ENTER_PASSCODE_LOCK_VERIFY_PIN || g_verifyLock->mode == ENTER_PASSCODE_VERIFY_PIN) {
        title = _("unlock_device_use_pin");
    } else {
        title = _("unlock_device_use_password");
    }
    return title;
}

void GuilockScreenRefresh(void)
{
    SetNavBarRightBtn(g_pageWidget->navBarWidget, NVS_RIGHT_BUTTON_BUTT, NULL, NULL);
    SetNavBarLeftBtn(g_pageWidget->navBarWidget, NVS_LEFT_BUTTON_BUTT, NULL, NULL);
    GuiFingerPrintStatus(g_verifyLock, true, g_fpErrorCount);
    if (GuiCheckIfTopView(&g_lockDeviceView)) {
        SetNavBarMidBtn(g_pageWidget->navBarWidget, NVS_MID_BUTTON_BUTT, NULL, NULL);
        SetMidBtnLabel(g_pageWidget->navBarWidget, NVS_BAR_MID_LABEL, "");
    } else {
        SetMidBtnLabel(g_pageWidget->navBarWidget, NVS_BAR_MID_LABEL, GuiJudgeTitle());
    }
    if (g_pageWidget->page != NULL) {
        lv_obj_clear_flag(g_pageWidget->page, LV_OBJ_FLAG_HIDDEN);
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

static void CountDownTimerChangeLabelTextHandler(lv_timer_t *timer)
{
    lv_obj_t *obj = (lv_obj_t *)timer->user_data;
    ++g_countDown;
    if (g_countDown == 3) {
        lv_label_set_text(obj, "Setting up support for new coins...");
    } else  if (g_countDown == 6) {
        lv_label_set_text(obj, "Generating extended public key...");
        if (g_countDownTimer != NULL) {
            g_countDown = 0;
            lv_timer_del(g_countDownTimer);
            g_countDownTimer = NULL;
            UNUSED(g_countDownTimer);
        }
    }
}

void GuiShowGenerateXPubLoading(void)
{
    if (GuiLockScreenIsTop() == false) {
        return;
    }
    
    if (g_LoadingView != NULL && lv_obj_is_valid(g_LoadingView)) {
        lv_obj_del(g_LoadingView);
        g_LoadingView = NULL;
    }
    
    SetPageLockScreen(false);
    g_canDismissLoading = false;

    g_LoadingView = GuiCreateContainer(lv_obj_get_width(lv_scr_act()), lv_obj_get_height(lv_scr_act()));
    GuiCreateCircleAroundAnimation(g_LoadingView, -51);
    lv_obj_add_flag(g_LoadingView, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_size(g_LoadingView, lv_obj_get_width(lv_scr_act()), lv_obj_get_height(lv_scr_act()));

    lv_obj_t *label = GuiCreateTextLabel(g_LoadingView, "Preparing Wallet");
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 403);

    lv_obj_t *hintLabel = GuiCreateNoticeLabel(g_LoadingView, "New chains support detected");
    lv_obj_set_style_text_align(hintLabel, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_width(hintLabel, 408);
    lv_obj_align(hintLabel, LV_ALIGN_TOP_MID, 0, 457);

    lv_obj_align(g_LoadingView, LV_ALIGN_DEFAULT, 0, 0);

    g_countDown = 0;
    g_countDownTimer = lv_timer_create(CountDownTimerChangeLabelTextHandler, 1000, hintLabel);
}

void GuiHideGenerateXPubLoading(void)
{   
    if (GuiLockScreenIsTop() == false) {
        return;
    }
    g_canDismissLoading = true;
    SetPageLockScreen(true);
}