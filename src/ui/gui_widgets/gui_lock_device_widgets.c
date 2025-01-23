#include "define.h"
#include "gui.h"
#include "gui_obj.h"
#include "gui_views.h"
#include "gui_status_bar.h"
#include "gui_lock_device_widgets.h"
#include "gui_model.h"
#include "keystore.h"
#include "gui_lock_widgets.h"
#include "screen_manager.h"
#include "gui_page.h"
#include "account_manager.h"
#include "gui_hintbox.h"
#include "user_memory.h"

typedef struct {
    uint16_t leftErrorCode;
    uint16_t lockTime;
} ErrorCountLockTimeMap_t;

const static ErrorCountLockTimeMap_t LOCK_TIME_MAPS[] = {
    {4, 1 * 60},
    {3, 5 * 60},
    {2, 15 * 60},
    {1, 60 * 60}
};

static void CountDownTimerWipeDeviceHandler(lv_timer_t *timer);
static void CountDownTimerLockTimeHandler(lv_timer_t *timer);

static void GuiLockedDeviceCountDownDestruct(void *obj, void* param);
static void WipeDeviceHandler(lv_event_t *e);
static void WipeDevice(void);
static uint32_t CalculateLockDeiceTime(void);
static void ForgetHandler(lv_event_t *e);

static lv_obj_t *g_cont;
static lv_timer_t *g_countDownTimer;
static int8_t countDown = 15;
static lv_obj_t *g_hintBox = NULL;

static uint32_t startTime;
static uint32_t needLockTime;

static void *pageParam = NULL;

static bool g_resetSuccessful = false;
static PageWidget_t *g_pageWidget;

void OpenForgetPasswordHandler(lv_event_t *e);

static bool IsLockTimePage()
{
    return pageParam != NULL;
}
void GuiLockDeviceInit(void *param)
{
    pageParam = param;
    if (g_pageWidget != NULL) {
        DestroyPageWidget(g_pageWidget);
        g_pageWidget = NULL;
    }
    g_pageWidget = CreatePageWidget();
    lv_obj_t *cont = g_pageWidget->contentZone;
    g_cont = cont;
    lv_obj_t *img = GuiCreateImg(cont, &imgLockedDevice);
    lv_obj_align(img, LV_ALIGN_TOP_MID, 0, 139 - 96);

    char lockHint[BUFFER_SIZE_128];

    if (!IsLockTimePage()) {
        strcpy_s(lockHint, BUFFER_SIZE_128, _("unlock_device_fingerprint_pin_device_locked_title"));
    } else {
        strcpy_s(lockHint, BUFFER_SIZE_128, _("unlock_device_time_limited_error_max_title"));
    }

    lv_obj_t *label =  GuiCreateLittleTitleLabel(cont, lockHint);
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 236 - 96);

    if (!IsLockTimePage()) {
        strcpy_s(lockHint, BUFFER_SIZE_128, _("unlock_device_fingerprint_pin_device_locked_desc"));
    } else {
        uint16_t time = GuiGetLockTimeByLeftErrorCount(*(uint16_t*)pageParam) / 60;
        if (time == 1) {
            strcpy_s(lockHint, BUFFER_SIZE_128, _("unlock_device_time_limited_error_max_desc"));
        } else if (time == 60) {
            snprintf_s(lockHint, BUFFER_SIZE_128, _("unlock_device_time_limited_error_max_warning_fmt"), time);
        } else {
            snprintf_s(lockHint, BUFFER_SIZE_128, _("unlock_device_time_limited_error_max_desc_fmt"), time);
        }
    }

    label =  GuiCreateLabelWithFont(cont, lockHint, g_defIllustrateFont);
    lv_label_set_recolor(label, true);
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 288 - 96);
    lv_obj_set_width(label, 408);
    lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, 0);

    if (!IsLockTimePage()) {
        lv_obj_set_style_text_opa(label, LV_OPA_80, LV_PART_MAIN);
        lv_obj_t *btn = GuiCreateTextBtn(cont, _("unlock_device_fingerprint_pin_device_locked_btn_start_text"));
        lv_obj_set_size(btn, 302, 66);
        lv_obj_align(btn, LV_ALIGN_TOP_MID, 0, 622 - 96);
        lv_obj_set_style_bg_opa(btn, LV_OPA_TRANSP, LV_STATE_DEFAULT);
        lv_obj_set_style_text_align(btn, LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_set_style_text_color(lv_obj_get_child(btn, 0), DEEP_ORANGE_COLOR, LV_PART_MAIN);

        lv_obj_add_event_cb(btn, WipeDeviceHandler, LV_EVENT_CLICKED, NULL);
        g_countDownTimer = lv_timer_create(CountDownTimerWipeDeviceHandler, 1000, btn);
    } else {
        lv_obj_set_style_text_color(label, lv_color_hex(0xc4c4c4), LV_PART_MAIN);
        lv_obj_t *btn = GuiCreateTextBtn(cont, _("forgot_password_reset_passcode_intro_text"));
        lv_obj_align(btn, LV_ALIGN_TOP_MID, 0, 622 - 96);
        lv_obj_set_style_bg_opa(btn, LV_OPA_TRANSP, LV_STATE_DEFAULT);
        lv_obj_set_style_text_align(btn, LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_add_event_cb(btn, ForgetHandler, LV_EVENT_CLICKED, &g_lockDeviceView);

        needLockTime = CalculateLockDeiceTime();
        startTime = GetCurrentStampTime();
        g_countDownTimer = lv_timer_create(CountDownTimerLockTimeHandler, 1000, NULL);
        SetLockTimeState(true);
    }
}

void GuiLockDeviceRefresh(void)
{
    uint32_t currentTime = GetCurrentStampTime();
    if (currentTime - startTime >= needLockTime) {
        GuiLockedDeviceCountDownDestruct(NULL, NULL);
        GuiModelWriteLastLockDeviceTime(0);
        GuiCloseCurrentWorkingView();
        SetLockDeviceAlive(false);
        if (!g_resetSuccessful) {
            static uint16_t single = SIG_LOCK_VIEW_VERIFY_PIN;
            GuiEmitSignal(SIG_LOCK_VIEW_SCREEN_ON_VERIFY, &single, sizeof(single));
        }
    }
}

void GuiLockDeviceDeInit(void)
{
    g_resetSuccessful = false;
    GUI_DEL_OBJ(g_hintBox)
    if (g_cont != NULL) {
        lv_obj_del(g_cont);
        g_cont = NULL;
    }
    if (g_pageWidget != NULL) {
        DestroyPageWidget(g_pageWidget);
        g_pageWidget = NULL;
    }
    SetLockTimeState(false);
}

static void GuiLockedDeviceCountDownDestruct(void *obj, void* param)
{
    if (g_countDownTimer != NULL) {
        countDown = 15;
        lv_timer_del(g_countDownTimer);
        g_countDownTimer = NULL;
        UNUSED(g_countDownTimer);

    }
}

static void CountDownTimerWipeDeviceHandler(lv_timer_t *timer)
{
    lv_obj_t *obj = (lv_obj_t *)timer->user_data;
    char buf[BUFFER_SIZE_64] = {0};
    --countDown;
    if (countDown > 0) {
        snprintf_s(buf, BUFFER_SIZE_64, _("unlock_device_fingerprint_pin_device_locked_btn_fmt"), countDown);
    } else {
        strcpy_s(buf, BUFFER_SIZE_64, ("system_settings_wipe_device_wipe_button"));
    }
    lv_label_set_text(lv_obj_get_child(obj, 0), buf);
    if (countDown <= 0) {
        if (CHECK_BATTERY_LOW_POWER()) {
            g_hintBox = GuiCreateErrorCodeWindow(ERR_KEYSTORE_SAVE_LOW_POWER, &g_hintBox, NULL);
        } else {
            WipeDevice();
        }
        GuiLockedDeviceCountDownDestruct(NULL, NULL);
    }
}

static void CountDownTimerLockTimeHandler(lv_timer_t *timer)
{
    uint32_t currentTime = GetCurrentStampTime();
    if (currentTime - startTime >= needLockTime) {
        if (GuiCheckIfTopView(&g_lockDeviceView)) {
            GuiCloseCurrentWorkingView();
            if (!g_resetSuccessful) {
                static uint16_t single = SIG_LOCK_VIEW_VERIFY_PIN;
                GuiEmitSignal(SIG_LOCK_VIEW_SCREEN_ON_VERIFY, &single, sizeof(single));
            }
        }
        GuiLockedDeviceCountDownDestruct(NULL, NULL);
        GuiModelWriteLastLockDeviceTime(0);
        SetLockDeviceAlive(false);
    }
}

static void WipeDeviceHandler(lv_event_t *e)
{
    if (CHECK_BATTERY_LOW_POWER()) {
        g_hintBox = GuiCreateErrorCodeWindow(ERR_KEYSTORE_SAVE_LOW_POWER, &g_hintBox, NULL);
    } else {
        WipeDevice();
        GuiLockedDeviceCountDownDestruct(NULL, NULL);
    }
}

static void WipeDevice(void)
{
    if (g_cont != NULL) {
        lv_obj_del(g_cont);
        g_cont = NULL;
    }
    if (g_pageWidget != NULL) {
        DestroyPageWidget(g_pageWidget);
        g_pageWidget = NULL;
    }
    SetPageLockScreen(false);
    g_cont = GuiCreateContainer(lv_obj_get_width(lv_scr_act()), lv_obj_get_height(lv_scr_act()) -
                                GUI_STATUS_BAR_HEIGHT);
    lv_obj_align(g_cont, LV_ALIGN_DEFAULT, 0, GUI_STATUS_BAR_HEIGHT);
    lv_obj_add_flag(g_cont, LV_OBJ_FLAG_CLICKABLE);
    GuiCreateCircleAroundAnimation(g_cont, -84);
    lv_obj_set_size(g_cont, lv_obj_get_width(lv_scr_act()), lv_obj_get_height(lv_scr_act()) -
                    GUI_STATUS_BAR_HEIGHT);

    lv_obj_t *label = GuiCreateTextLabel(g_cont, _("system_settings_wipe_device_generating_title"));
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 355);

    label = GuiCreateNoticeLabel(g_cont, _("system_settings_wipe_device_generating_desc1"));
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 410);

    label = GuiCreateNoticeLabel(g_cont, _("system_settings_wipe_device_generating_desc2"));
    lv_obj_set_width(label, 408);
    lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 612);

    GuiModelLockedDeviceDelAllWalletDesc();
}

void GuiDelALLWalletSetup(void)
{
    GuiStopCircleAroundAnimation();
    GuiCloseToTargetView(&g_setupView);
}

uint16_t GuiGetLockTimeByLeftErrorCount(uint16_t leftErrorCount)
{
    for (size_t i = 0; i < NUMBER_OF_ARRAYS(LOCK_TIME_MAPS); i++) {
        ErrorCountLockTimeMap_t map = LOCK_TIME_MAPS[i];
        if (leftErrorCount == map.leftErrorCode) {
            return map.lockTime;
        }
    }
    return 0;
}

static uint32_t CalculateLockDeiceTime(void)
{
    uint32_t originLockTime = GuiGetLockTimeByLeftErrorCount(*(uint16_t*)pageParam);
    uint32_t lastLockDeviceTime = GetLastLockDeviceTime();
    uint32_t stillNeedLockTime = originLockTime - (GetCurrentStampTime() - lastLockDeviceTime);
    if (stillNeedLockTime < originLockTime && stillNeedLockTime > 0) {
        printf("still need to lock %ds\n", stillNeedLockTime);
        return stillNeedLockTime;
    }
    printf("originLockTime need to lock %ds\n", originLockTime);
    return originLockTime;
}

static void ForgetHandler(lv_event_t *e)
{
    OpenForgetPasswordHandler(e);
    SetLockDeviceAlive(true);
}

void ResetSuccess(void)
{
    g_resetSuccessful = true;
    needLockTime = 0;
}

void GuiClearAllTop(void)
{
    GuiCloseToTargetView(&g_lockDeviceView);
}