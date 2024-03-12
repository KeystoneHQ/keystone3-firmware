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
#include "screen_manager.h"
#include "gui_passphrase_widgets.h"
#include "gui_keyboard_hintbox.h"
#include "gui_firmware_update_widgets.h"
#include "background_task.h"
#include "gui_forget_pass_widgets.h"
#include "gui_usb_connection_widgets.h"
#include "gui_pop_message_box.h"
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
            const char *title;
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

    char tempBuf[BUFFER_SIZE_32] = {0};
    char showString[BUFFER_SIZE_64] = {0};
    GetSerialNumber(tempBuf);
    snprintf_s(showString, BUFFER_SIZE_64, "SN:%s", tempBuf);
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
#ifdef BTC_ONLY
        ShowWallPaper(false);
#endif
    }
    lv_obj_clear_flag(g_pageWidget->page, LV_OBJ_FLAG_HIDDEN);
    // g_lockView.isActive = true;
    lv_obj_set_parent(g_pageWidget->page, lv_scr_act());
    if (GetKeyboardWidgetMode() != g_verifyLock->mode % 2) {
        PassWordPinSwitch(g_verifyLock);
    }
    GuiUpdateEnterPasscodeParam(g_verifyLock, single);
    GuilockScreenRefresh();
}

void GuiLockScreenTurnOff(void)
{
    static uint16_t single = SIG_LOCK_VIEW_VERIFY_PIN;
    lv_obj_add_flag(g_pageWidget->page, LV_OBJ_FLAG_HIDDEN);
    GuiModeGetWalletDesc();
    GuiEnterPassCodeStatus(g_verifyLock, true);

    if (g_oldWalletIndex == 0xFF) {
        g_oldWalletIndex = GetCurrentAccountIndex();
    }

    if ((GetCurrentAccountIndex() != g_oldWalletIndex) ||
            GuiIsForgetPass()) {
        g_oldWalletIndex = GetCurrentAccountIndex();
        GuiCloseToTargetView(&g_homeView);
    } else {
        GuiEmitSignal(GUI_EVENT_REFRESH, &single, sizeof(single));
        GuiFirmwareUpdateWidgetRefresh();
    }
    HardwareInitAfterWake();
    // g_lockView.isActive = false;
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
    HardwareInitAfterWake();
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
        if (g_oldWalletIndex == 0xFF) {
            g_oldWalletIndex = GetCurrentAccountIndex();
        }
        if (IsUpdateSuccess()) {
            lv_obj_add_flag(g_pageWidget->page, LV_OBJ_FLAG_HIDDEN);
            GuiModeGetWalletDesc();
            GuiEnterPassCodeStatus(g_verifyLock, true);
            GuiFrameOpenView(&g_homeView);
            GuiFrameOpenView(&g_updateSuccessView);
        } else if (ModelGetPassphraseQuickAccess()) {
            lv_obj_add_flag(g_pageWidget->page, LV_OBJ_FLAG_HIDDEN);
            GuiModeGetWalletDesc();
            GuiEnterPassCodeStatus(g_verifyLock, true);
            GuiFrameOpenView(&g_passphraseView);
        } else if (g_homeView.isActive) {
            GuiLockScreenTurnOff();
        } else if (g_forgetPassView.isActive) {
            GuiLockScreenTurnOff();
        } else {
            lv_obj_add_flag(g_pageWidget->page, LV_OBJ_FLAG_HIDDEN);
            SetNavBarMidBtn(g_pageWidget->navBarWidget, NVS_MID_BUTTON_BUTT, NULL, NULL);
            GuiFrameOpenView(&g_homeView);
            HardwareInitAfterWake();
        }
        // Close the loading page after closing the lock screen page
        GuiCloseGenerateXPubLoading();
    }
}

void GuiLockScreenErrorCount(void *param)
{
    PasswordVerifyResult_t *passwordVerifyResult = (PasswordVerifyResult_t *)param;
    printf("GuiLockScreenErrorCount  errorcount is %d\n", passwordVerifyResult->errorCount);
    if (g_verifyLock != NULL) {
        char hint[BUFFER_SIZE_128];
        char tempBuf[BUFFER_SIZE_128];
        int leftCount = 0;

        if (*(uint16_t *)passwordVerifyResult->signal == SIG_LOCK_VIEW_VERIFY_PIN
                || *(uint16_t *)passwordVerifyResult->signal == SIG_LOCK_VIEW_SCREEN_GO_HOME_PASS) {
            leftCount = MAX_LOGIN_PASSWORD_ERROR_COUNT - passwordVerifyResult->errorCount;
            ASSERT(leftCount >= 0);
            if (leftCount > 1) {
                snprintf_s(hint, BUFFER_SIZE_128, _("unlock_device_attempts_left_plural_times_fmt"), leftCount - 4);
            } else {
                snprintf_s(hint, BUFFER_SIZE_128, _("unlock_device_attempts_left_singular_times_fmt"), leftCount - 4);
            }
            snprintf_s(tempBuf, BUFFER_SIZE_128, "#F55831 %s #", hint);
            lv_label_set_text(g_verifyLock->errLabel, tempBuf);
            GuiPassowrdToLockTimePage(leftCount);
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

static const char *GuiJudgeTitle(void)
{
    char *title;
    //no fingerprint presents
    if (g_fpErrorCount >= FINGERPRINT_EN_SING_ERR_TIMES || (GetRegisteredFingerNum() < 1) || GuiLockScreenIsFirstUnlock()) {
        if (g_verifyLock->mode == ENTER_PASSCODE_LOCK_VERIFY_PIN || g_verifyLock->mode == ENTER_PASSCODE_VERIFY_PIN) {
            title = (char *)_("unlock_device_use_pin");
        } else {
            title = (char *)_("unlock_device_use_password");
        }
        return title;
    }
    if (GetFingerUnlockFlag() && g_purpose == LOCK_SCREEN_PURPOSE_UNLOCK) {
        title = SRAM_MALLOC(BUFFER_SIZE_64);
        memset_s(title, BUFFER_SIZE_64, 0, BUFFER_SIZE_64);
        if (g_verifyLock->mode == ENTER_PASSCODE_LOCK_VERIFY_PIN || g_verifyLock->mode == ENTER_PASSCODE_VERIFY_PIN) {
            snprintf_s(title, BUFFER_SIZE_64, _("unlock_device_title_fmt"), _("unlock_device_use_pin"), _("unlock_device_use_fingerprint"));
        } else {
            snprintf_s(title, BUFFER_SIZE_64, _("unlock_device_title_fmt"), _("unlock_device_use_password"), _("unlock_device_use_fingerprint"));
        }
        return title;
    }
    if (g_verifyLock->mode == ENTER_PASSCODE_LOCK_VERIFY_PIN || g_verifyLock->mode == ENTER_PASSCODE_VERIFY_PIN) {
        title = (char *)_("unlock_device_use_pin");
    } else {
        title = (char *)_("unlock_device_use_password");
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
    if (GuiLockScreenIsTop() == false) {
        return;
    }
    if (GuiPassphraseLoadingIsTop() == true) {
        return;
    }

    if (g_LoadingView != NULL && lv_obj_is_valid(g_LoadingView)) {
        lv_obj_del(g_LoadingView);
        g_LoadingView = NULL;
    }

    SetPageLockScreen(false);
    FpCancelCurOperate();
    g_canDismissLoading = false;

    g_LoadingView = GuiCreateContainer(lv_obj_get_width(lv_scr_act()), lv_obj_get_height(lv_scr_act()));
    lv_obj_add_flag(g_LoadingView, LV_OBJ_FLAG_HIDDEN);
    GuiCreateCircleAroundAnimation(g_LoadingView, -51);
    lv_obj_add_flag(g_LoadingView, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_size(g_LoadingView, lv_obj_get_width(lv_scr_act()), lv_obj_get_height(lv_scr_act()));

    lv_obj_t *label = GuiCreateTextLabel(g_LoadingView, _("prepare_wallet_hint"));
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 403);

    lv_obj_t *hintLabel = GuiCreateNoticeLabel(g_LoadingView, _("prepare_wallet_second_step"));
    lv_obj_set_style_text_align(hintLabel, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_width(hintLabel, 408);
    lv_obj_align(hintLabel, LV_ALIGN_TOP_MID, 0, 457);

    lv_obj_align(g_LoadingView, LV_ALIGN_DEFAULT, 0, 0);
    lv_obj_clear_flag(g_LoadingView, LV_OBJ_FLAG_HIDDEN);
    lv_obj_invalidate(g_LoadingView);

    g_isShowLoading = true;
    g_countDown = 0;
    g_countDownTimer = lv_timer_create(CountDownTimerChangeLabelTextHandler, 1000, hintLabel);
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
#if (USB_POP_WINDOW_ENABLE == 1)
    if (GetUSBSwitch() == true && GetUsbDetectState()) {
        GuiApiEmitSignalWithValue(SIG_INIT_USB_CONNECTION, 1);
    }
#endif
}