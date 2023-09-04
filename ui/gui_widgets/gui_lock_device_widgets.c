#include "gui.h"
#include "gui_obj.h"
#include "gui_views.h"
#include "gui_status_bar.h"
#include "gui_lock_device_widgets.h"
#include "gui_model.h"
#include "keystore.h"
#include "gui_lock_widgets.h"
#include "screen_manager.h"

#ifdef COMPILE_MAC_SIMULATOR
#include "simulator_model.h"
#endif

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

static uint32_t startTime;
static uint32_t needLockTime;

static void *pageParam = NULL;

static bool g_resetSuccessful = false;

void OpenForgetPasswordHandler(lv_event_t *e);

static bool IsLockTimePage()
{
    return pageParam != NULL;
}
void GuiLockDeviceInit(void *param)
{
    pageParam = param;

    lv_obj_t *cont = GuiCreateContainer(lv_obj_get_width(lv_scr_act()), lv_obj_get_height(lv_scr_act()) -
                                        GUI_MAIN_AREA_OFFSET);
    lv_obj_align(cont, LV_ALIGN_DEFAULT, 0, GUI_MAIN_AREA_OFFSET);
    lv_obj_add_flag(cont, LV_OBJ_FLAG_CLICKABLE);
    g_cont = cont;
    lv_obj_t *img = GuiCreateImg(cont, &imgLockedDevice);
    lv_obj_align(img, LV_ALIGN_TOP_MID, 0, 139 - 96);

    char lockHint[128];

    if (!IsLockTimePage()) {
        sprintf(lockHint, "%s", "Device Locked");
    } else {
        sprintf(lockHint, "%s", "Device Unavailable");
    }

    lv_obj_t *label =  GuiCreateLabelWithFont(cont, lockHint, &openSansEnLittleTitle);
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 236 - 96);

    if (!IsLockTimePage()) {
        sprintf(lockHint, "%s", "Please wipe your device. All the data on this device will be erased after wiped");
    } else {
        uint16_t time = GuiGetLockTimeByLeftErrorCount(*(uint16_t*)pageParam) / 60;
        if (time == 1) {
            sprintf(lockHint, "Please unlock your device in #F55831 %d# minute", time);
        } else {
            sprintf(lockHint, "Please unlock your device in #F55831 %d# minutes", time);
        }
    }

    label =  GuiCreateLabelWithFont(cont, lockHint, &openSans_20);
    lv_label_set_recolor(label, true);
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 288 - 96);
    lv_obj_set_width(label, 408);
    lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, 0);

    if (!IsLockTimePage()) {
        lv_obj_set_style_text_opa(label, LV_OPA_56, LV_PART_MAIN);
        lv_obj_t *btn = GuiCreateBtn(cont, "Wipe Device Now (15)");
        lv_obj_set_size(btn, 302, 66);
        lv_obj_align(btn, LV_ALIGN_TOP_MID, 0, 622 - 96);
        lv_obj_set_style_bg_opa(btn, LV_OPA_TRANSP, LV_STATE_DEFAULT);
        lv_obj_set_style_text_align(btn, LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_set_style_text_color(lv_obj_get_child(btn, 0), DEEP_ORANGE_COLOR, LV_PART_MAIN);

        lv_obj_add_event_cb(btn, WipeDeviceHandler, LV_EVENT_CLICKED, NULL);
        g_countDownTimer = lv_timer_create(CountDownTimerWipeDeviceHandler, 1000, btn);
    } else {
        lv_obj_set_style_text_color(label, lv_color_hex(0xc4c4c4), LV_PART_MAIN);

        lv_obj_t *btn = GuiCreateBtn(cont, "Forget Password?");
        lv_obj_set_size(btn, 302, 66);
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
    GuiNvsBarSetRightCb(NVS_RIGHT_BUTTON_BUTT, NULL, NULL);
    GuiNvsBarSetLeftCb(NVS_LEFT_BUTTON_BUTT, NULL, NULL);
    GuiNvsBarSetMidCb(NVS_MID_BUTTON_BUTT, NULL, NULL);

    uint32_t currentTime = GetCurrentStampTime();
    if (currentTime - startTime >= needLockTime) {
        GuiLockedDeviceCountDownDestruct(NULL, NULL);
        GuiModelWriteLastLockDeviceTime(0);
        GuiCLoseCurrentWorkingView();
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
    if (g_cont != NULL) {
        lv_obj_del(g_cont);
        g_cont = NULL;
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
    char buf[32] = {0};
    --countDown;
    if (countDown > 0) {
        sprintf(buf, "Wipe Device Now (%d)", countDown);
    } else {
        strcpy(buf, "Wipe Device Now");
    }
    lv_label_set_text(lv_obj_get_child(obj, 0), buf);
    if (countDown <= 0) {
        WipeDevice();
        GuiLockedDeviceCountDownDestruct(NULL, NULL);
    }
}

static void CountDownTimerLockTimeHandler(lv_timer_t *timer)
{
    uint32_t currentTime = GetCurrentStampTime();
    if (currentTime - startTime >= needLockTime) {
        if (GuiCheckIfTopView(&g_lockDeviceView)) {
            GuiCLoseCurrentWorkingView();
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
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
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
    SetPageLockScreen(false);
    g_cont = GuiCreateContainer(lv_obj_get_width(lv_scr_act()), lv_obj_get_height(lv_scr_act()) -
                                GUI_STATUS_BAR_HEIGHT);
    lv_obj_align(g_cont, LV_ALIGN_DEFAULT, 0, GUI_STATUS_BAR_HEIGHT);
    lv_obj_add_flag(g_cont, LV_OBJ_FLAG_CLICKABLE);
    GuiCreateCircleAroundAnimation(g_cont, -84);
    lv_obj_set_size(g_cont, lv_obj_get_width(lv_scr_act()), lv_obj_get_height(lv_scr_act()) -
                    GUI_STATUS_BAR_HEIGHT);

    lv_obj_t *label = GuiCreateTextLabel(g_cont, "Resetting Device");
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 355);

    label = GuiCreateNoticeLabel(g_cont, "Erasing Secure Element...");
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 410);

    label = GuiCreateNoticeLabel(g_cont, "Do not power off your device while the installation process is underway");
    lv_obj_set_width(label, 408);
    lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 612);

    GuiModelLockedDeviceDelAllWalletDesc();
}

void GuiDelALLWalletSetup(void)
{
    GuiStopCircleAroundAnimation();
    GuiCloseToTargetView(&g_setupView);
    GuiNvsBarSetRightCb(NVS_RIGHT_BUTTON_BUTT, NULL, NULL);
    GuiNvsBarSetLeftCb(NVS_LEFT_BUTTON_BUTT, NULL, NULL);
    GuiNvsBarSetMidBtnLabel(NVS_BAR_MID_LABEL, "");
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
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_CLICKED) {
        OpenForgetPasswordHandler(e);
        SetLockDeviceAlive(true);
    }
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