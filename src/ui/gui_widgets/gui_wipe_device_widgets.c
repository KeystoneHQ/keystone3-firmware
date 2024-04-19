#include "gui.h"
#include "gui_views.h"
#include "gui_status_bar.h"
#include "gui_keyboard.h"
#include "gui_button.h"
#include "gui_hintbox.h"
#include "gui_model.h"
#include "user_memory.h"
#include "presetting.h"
#include "gui_wipe_device_widgets.h"
#include "screen_manager.h"
#include "gui_page.h"

static lv_obj_t *g_cont;
static lv_obj_t *g_wipeDeviceHintBox = NULL;
static lv_timer_t *g_countDownTimer;
static PageWidget_t *g_pageWidget;

static void GuiWipeDeviceNVSBarInit();
static void GuiWipeDeviceEntranceWidget(lv_obj_t *parent);
static void WipeDeviceHandler(lv_event_t *e);
static void NotNowHandler(lv_event_t *e);
static void GuiShowWipeDeviceHintBox(void);
static void ExecWipeDeviceHandler(lv_event_t *e);
static void WipeDeviceDeal(void);
static void CountDownTimerHandler(lv_timer_t *timer);
static void GuiCountDownDestruct(void *obj, void* param);

void GuiWipeDeviceWidgetsInit()
{
    g_pageWidget = CreatePageWidget();
    lv_obj_t *cont = g_pageWidget->contentZone;
    g_cont = cont;
    lv_obj_t *img = GuiCreateImg(cont, &imgWipeDevice);
    lv_obj_align(img, LV_ALIGN_TOP_MID, 0, 36);

    GuiWipeDeviceEntranceWidget(cont);
}

void GuiWipeDeviceWidgetsDeInit()
{
    GUI_DEL_OBJ(g_wipeDeviceHintBox);
    if (g_cont != NULL) {
        lv_obj_del(g_cont);
        g_cont = NULL;
    }
    if (g_pageWidget != NULL) {
        DestroyPageWidget(g_pageWidget);
        g_pageWidget = NULL;
    }
}

void GuiWipeDeviceWidgetsRefresh()
{
    GuiWipeDeviceNVSBarInit();
}

void GuiWipeDeviceWidgetsRestart()
{}

static void GuiWipeDeviceNVSBarInit()
{
    SetNavBarLeftBtn(g_pageWidget->navBarWidget, NVS_BAR_RETURN, CloseCurrentViewHandler, NULL);
}

void GuiWipeDeviceEntranceWidget(lv_obj_t *parent)
{
    lv_obj_t *label = GuiCreateLittleTitleLabel(parent, _("system_settings_wipe_device_wipe_title"));
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 140);

    label = GuiCreateNoticeLabel(parent, _("system_settings_wipe_device_wipe_desc"));
    lv_obj_set_width(label, 406);
    lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 192);

    label = GuiCreateTextLabel(parent, _("system_settings_wipe_device_wipe_button"));
    lv_obj_set_style_text_color(label, lv_color_hex(0xf55831), LV_PART_MAIN);

    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 541);
    lv_obj_add_flag(label, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(label, WipeDeviceHandler, LV_EVENT_CLICKED, NULL);

}

static void WipeDeviceHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        GuiShowWipeDeviceHintBox();
    }
}

static void GuiShowWipeDeviceHintBox(void)
{
    if (g_wipeDeviceHintBox == NULL) {
        g_wipeDeviceHintBox = GuiCreateGeneralHintBox(&imgWarn, _("wipe_device"), _("system_settings_wipe_device_wipe_alert_desc"), NULL,
                              _("not_now"), WHITE_COLOR_OPA20, _("system_settings_wipe_device_wipe_start_text"), ORANGE_COLOR);
        lv_obj_t *leftBtn = GuiGetHintBoxLeftBtn(g_wipeDeviceHintBox);
        lv_obj_add_event_cb(leftBtn, NotNowHandler, LV_EVENT_CLICKED, NULL);
        lv_obj_t *rightBtn = GuiGetHintBoxRightBtn(g_wipeDeviceHintBox);
        lv_obj_add_event_cb(rightBtn, ExecWipeDeviceHandler, LV_EVENT_CLICKED, NULL);
        lv_obj_clear_flag(rightBtn, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_set_style_bg_opa(rightBtn, LV_OPA_50, LV_STATE_DEFAULT);
        g_countDownTimer = lv_timer_create(CountDownTimerHandler, 1000, rightBtn);
    }
}

static void NotNowHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        GuiCountDownDestruct(NULL, NULL);
        GUI_DEL_OBJ(g_wipeDeviceHintBox);
    }
}

static void ExecWipeDeviceHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        GUI_DEL_OBJ(g_wipeDeviceHintBox);
        WipeDeviceDeal();
    }
}

static void WipeDeviceDeal(void)
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

static void CountDownTimerHandler(lv_timer_t *timer)
{
    lv_obj_t *obj = (lv_obj_t *)timer->user_data;
    static int8_t countDown = 5;
    char buf[BUFFER_SIZE_16] = {0};
    --countDown;
    if (countDown > 0) {
        snprintf_s(buf, BUFFER_SIZE_16, _("system_settings_wipe_device_wipe_fmt"), countDown);
    } else {
        strcpy_s(buf, BUFFER_SIZE_16, _("system_settings_wipe_device_wipe_end_text"));
    }
    lv_label_set_text(lv_obj_get_child(obj, 0), buf);

    if (countDown <= 0) {
        lv_obj_set_style_bg_opa(obj, LV_OPA_100, LV_PART_MAIN);
        lv_obj_add_flag(obj, LV_OBJ_FLAG_CLICKABLE);
        lv_timer_del(timer);
        countDown = 5;
        g_countDownTimer = NULL;
        UNUSED(g_countDownTimer);
    }
}

static void GuiCountDownDestruct(void *obj, void* param)
{
    if (g_countDownTimer != NULL) {
        lv_timer_del(g_countDownTimer);
        g_countDownTimer = NULL;
    }
}