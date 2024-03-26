#include "gui.h"
#include "gui_obj.h"
#include "gui_views.h"
#include "gui_enter_passcode.h"
#include "gui_model.h"
#include "gui_lock_widgets.h"
#include "gui_hintbox.h"
#include "gui_api.h"
#include "keystore.h"
#include "fingerprint_process.h"
#include "device_setting.h"
#include "gui_page.h"
#include "account_manager.h"
#include "user_memory.h"
#include "presetting.h"
#include "assert.h"
#include "screen_manager.h"

#include "background_task.h"

#include "usb_task.h"

#ifdef COMPILE_SIMULATOR
#include "assert.h"
#define FINGERPRINT_EN_SING_ERR_TIMES           (5)
#define FINGERPRINT_SING_ERR_TIMES              (3)
#define FINGERPRINT_SING_DISABLE_ERR_TIMES      (15)
#else
#include "drv_aw32001.h"
#include "drv_usb.h"
#include "device_setting.h"
#endif

#ifdef COMPILE_MAC_SIMULATOR
#include "simulator_model.h"
#endif

static void GuiPassowrdToLockTimePage(uint16_t errorCount);
void GuiLockScreenClearModal(lv_obj_t *cont);
static const char *GuiJudgeTitle();
static void CountDownTimerChangeLabelTextHandler(lv_timer_t *timer);
static void GuiCloseGenerateXPubLoading(void);
static void HardwareInitAfterWake(void);
int32_t InitSdCardAfterWakeup(const void *inData, uint32_t inDataLen);

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
static bool g_isShowLoading = false;
static uint8_t g_oldWalletIndex = 0xFF;

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
    }
}

void GuiLockScreenTurnOn(void *param)
{
}

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
    static uint16_t staticLeftErrorCount;
    if (leftErrorCount < 5 && leftErrorCount > 0) {
        staticLeftErrorCount = leftErrorCount;
        GuiFrameOpenViewWithParam(&g_lockDeviceView, &staticLeftErrorCount, sizeof(staticLeftErrorCount));
        GuiModelWriteLastLockDeviceTime(GetCurrentStampTime());
    }
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

static const char *GuiJudgeTitle(void)
{
}

void GuilockScreenRefresh(void)
{
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

static void CountDownTimerChangeLabelTextHandler(lv_timer_t *timer)
{
    lv_obj_t *obj = (lv_obj_t *)timer->user_data;
    ++g_countDown;
    if (g_countDown == 3) {
        lv_label_set_text(obj, _("prepare_wallet_third_step"));
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
}

void GuiHideGenerateXPubLoading(void)
{
    if (!g_isShowLoading) {
        return;
    }
    if (GuiLockScreenIsTop() == false) {
        return;
    }

    g_canDismissLoading = true;
    SetPageLockScreen(true);
}

static void GuiCloseGenerateXPubLoading(void)
{
    if (g_canDismissLoading) {
        if (g_LoadingView != NULL && lv_obj_is_valid(g_LoadingView)) {
            GuiStopCircleAroundAnimation();
            lv_obj_del(g_LoadingView);
            g_LoadingView = NULL;
        }
        g_canDismissLoading = false;
    }
}

static void HardwareInitAfterWake(void)
{
    AsyncExecute(InitSdCardAfterWakeup, NULL, 0);
}