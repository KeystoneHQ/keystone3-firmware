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

static lv_obj_t *g_cont;
static lv_obj_t *g_wipeDeviceHintBox = NULL;
static lv_timer_t *g_countDownTimer;

static void GuiWipeDeviceNVSBarInit();
static void GuiWipeDeviceEntranceWidget(lv_obj_t *parent);
// static void UnHandler(lv_event_t *e);
static void WipeDeviceHandler(lv_event_t *e);
static void NotNowHandler(lv_event_t *e);
static void GuiShowWipeDeviceHintBox(void);
static void ExecWipeDeviceHandler(lv_event_t *e);
static void WipeDevice(void);
static void CountDownTimerHandler(lv_timer_t *timer);
static void GuiCountDownDestruct(void *obj, void* param);


void GuiWipeDeviceWidgetsInit()
{
    GuiWipeDeviceNVSBarInit();

    lv_obj_t *cont = GuiCreateContainer(lv_obj_get_width(lv_scr_act()), lv_obj_get_height(lv_scr_act()) -
                                        GUI_MAIN_AREA_OFFSET);
    lv_obj_align(cont, LV_ALIGN_DEFAULT, 0, GUI_MAIN_AREA_OFFSET);
    lv_obj_add_flag(cont, LV_OBJ_FLAG_CLICKABLE);

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
}

void GuiWipeDeviceWidgetsRefresh()
{
    GuiWipeDeviceNVSBarInit();
}


void GuiWipeDeviceWidgetsRestart()
{}


static void GuiWipeDeviceNVSBarInit()
{
    GuiNvsBarSetLeftCb(NVS_BAR_RETURN, CloseCurrentViewHandler, NULL);
    GuiNvsBarSetMidCb(NVS_MID_BUTTON_BUTT, NULL, NULL);
    GuiNvsBarSetRightCb(NVS_RIGHT_BUTTON_BUTT, NULL, NULL);
}


void GuiWipeDeviceEntranceWidget(lv_obj_t *parent)
{
    lv_obj_t *label = GuiCreateLittleTitleLabel(parent, "Wipe Device");
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 140);

    label = GuiCreateNoticeLabel(parent, "All data stored on this device, including all of your wallets, will be permanently deleted.");
    lv_obj_set_width(label, 406);
    lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 192);

    label = GuiCreateTextLabel(parent, "Wipe Device Now");
    lv_obj_set_style_text_color(label, lv_color_hex(0xf55831), LV_PART_MAIN);

    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 541);
    lv_obj_add_flag(label, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(label, WipeDeviceHandler, LV_EVENT_CLICKED, NULL);


}

// static void UnHandler(lv_event_t *e)
// {
//     lv_event_code_t code = lv_event_get_code(e);
//     if (code == LV_EVENT_CLICKED) {
//     }
// }

static void WipeDeviceHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        GuiShowWipeDeviceHintBox();
    }

}


static void GuiShowWipeDeviceHintBox(void)
{
    lv_obj_t *arc, *img, *label, *button;
    if (g_wipeDeviceHintBox == NULL) {
        g_wipeDeviceHintBox = GuiCreateHintBox(lv_scr_act(), 480, 416, false);
        arc = lv_arc_create(g_wipeDeviceHintBox);
        lv_obj_set_size(arc, 72, 72);
        lv_obj_align(arc, LV_ALIGN_BOTTOM_LEFT, 36, -296);
        lv_arc_set_bg_angles(arc, 0, 360);
        lv_obj_remove_style(arc, NULL, LV_PART_KNOB);   /*Be sure the knob is not displayed*/
        lv_obj_clear_flag(arc, LV_OBJ_FLAG_CLICKABLE);  /*To not allow adjusting by click*/
        lv_arc_set_value(arc, 0);
        lv_obj_set_style_arc_width(arc, 2, LV_PART_MAIN);
        lv_obj_set_style_arc_color(arc, WHITE_COLOR, LV_PART_MAIN);
        lv_obj_set_style_arc_opa(arc, LV_OPA_30, LV_PART_MAIN);
        lv_obj_align(arc, LV_ALIGN_BOTTOM_LEFT, 36, -296);
        img = GuiCreateImg(g_wipeDeviceHintBox, &imgWarn);
        lv_obj_align(img, LV_ALIGN_BOTTOM_LEFT, 36, -296);

        label = GuiCreateLittleTitleLabel(g_wipeDeviceHintBox, "Wipe Device");
        lv_obj_align(label, LV_ALIGN_BOTTOM_LEFT, 36, -232);
        label = GuiCreateIllustrateLabel(g_wipeDeviceHintBox, "Please double confirm that by continue all data stored on this device, including all of your wallets, will be permanently deleted.");
        lv_obj_align(label, LV_ALIGN_BOTTOM_LEFT, 36, -130);

        button = GuiCreateBtnWithFont(g_wipeDeviceHintBox, "Not Now", g_defTextFont);
        lv_obj_align(button, LV_ALIGN_BOTTOM_LEFT, 36, -24);
        lv_obj_set_size(button, 192, 66);
        lv_obj_set_style_bg_color(button, DARK_GRAY_COLOR, LV_PART_MAIN);
        lv_obj_add_event_cb(button, NotNowHandler, LV_EVENT_CLICKED, NULL);
        button = GuiCreateBtnWithFont(g_wipeDeviceHintBox, "Wipe(5)", g_defTextFont);
        lv_obj_align(button, LV_ALIGN_BOTTOM_RIGHT, -36, -24);
        lv_obj_set_size(button, 192, 66);
        lv_obj_set_style_bg_color(button, ORANGE_COLOR, LV_PART_MAIN);
        lv_obj_add_event_cb(button, ExecWipeDeviceHandler, LV_EVENT_CLICKED, NULL);
        lv_obj_clear_flag(button, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_set_style_bg_opa(button, LV_OPA_50, LV_STATE_DEFAULT);
        g_countDownTimer = lv_timer_create(CountDownTimerHandler, 1000, button);

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
        WipeDevice();
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


static void CountDownTimerHandler(lv_timer_t *timer)
{
    lv_obj_t *obj = (lv_obj_t *)timer->user_data;
    static int8_t countDown = 5;
    char buf[16] = {0};
    --countDown;
    if (countDown > 0) {
        sprintf(buf, "Wipe(%d)", countDown);
    } else {
        strcpy(buf, "Wipe");
    }
    // lv_label_set_text(obj, buf);
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