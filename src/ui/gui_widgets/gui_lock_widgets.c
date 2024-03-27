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

#include "fingerprint_process.h"
#include "device_setting.h"
#include "gui_page.h"
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

static GuiEnterPasscodeItem_t *g_verifyLock = NULL;
static lv_obj_t *g_lockScreenCont;
static bool g_firstUnlock = true;
static uint8_t g_fpErrorCount = 0;
static LOCK_SCREEN_PURPOSE_ENUM g_purpose = LOCK_SCREEN_PURPOSE_UNLOCK;
static PageWidget_t *g_pageWidget;

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
    // lv_event_code_t code = lv_event_get_code(e);
    // if (code == LV_EVENT_CLICKED) {
    //     lv_obj_t *cont = GuiCreateContainer(lv_obj_get_width(lv_scr_act()), lv_obj_get_height(lv_scr_act()) -
    //                                         GUI_STATUS_BAR_HEIGHT);
    //     lv_obj_add_flag(cont, LV_OBJ_FLAG_CLICKABLE);
    //     lv_obj_align(cont, LV_ALIGN_DEFAULT, 0, GUI_STATUS_BAR_HEIGHT);
    //     GuiCreateCircleAroundAnimation(lv_scr_act(), -40);

    //     lv_obj_t *label = GuiCreateTextLabel(cont, "Resetting Device");
    //     lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 355);

    //     label = GuiCreateNoticeLabel(cont, "Erasing Secure Element...");
    //     lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 410);

    //     label = GuiCreateNoticeLabel(cont, "Do not power off your device while the installation process is underway");
    //     lv_obj_set_width(label, 408);
    //     lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, 0);
    //     lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 612);
    // }
}

void GuiLockScreenWipeDevice(void)
{
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

}

void GuiLockScreenTurnOn(void *param)
{
}

static uint8_t g_oldWalletIndex = 0xFF;
void GuiLockScreenTurnOff(void)
{
}

void GuiUpdateOldAccountIndex(void)
{
    g_oldWalletIndex = GetCurrentAccountIndex();
}

void GuiLockScreenToHome(void)
{

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

}

void GuiLockScreenErrorCount(void *param)
{

}

static void GuiPassowrdToLockTimePage(uint16_t leftErrorCount)
{
}

void GuiLockScreenTurnOnHandler(lv_event_t *e)
{
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

}

void GuiLockScreenPasscodeSwitch(bool isPin)
{
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
    // SetNavBarRightBtn(g_pageWidget->navBarWidget, NVS_RIGHT_BUTTON_BUTT, NULL, NULL);
    // SetNavBarLeftBtn(g_pageWidget->navBarWidget, NVS_LEFT_BUTTON_BUTT, NULL, NULL);
    // GuiFingerPrintStatus(g_verifyLock, true, g_fpErrorCount);
    // if (GuiCheckIfTopView(&g_lockDeviceView)) {
    //     SetNavBarMidBtn(g_pageWidget->navBarWidget, NVS_MID_BUTTON_BUTT, NULL, NULL);
    //     SetMidBtnLabel(g_pageWidget->navBarWidget, NVS_BAR_MID_LABEL, "");
    // } else {
    //     SetMidBtnLabel(g_pageWidget->navBarWidget, NVS_BAR_MID_LABEL, GuiJudgeTitle());
    // }
    // if (g_pageWidget->page != NULL) {
    //     lv_obj_clear_flag(g_pageWidget->page, LV_OBJ_FLAG_HIDDEN);
    // }
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

