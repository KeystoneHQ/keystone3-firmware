#include "gui_usb_connection_widgets.h"
#include "gui.h"
#include "gui_pop_message_box.h"
#include "gui_hintbox.h"
#include "usb_task.h"
#include "gui_views.h"
#include "gui_api.h"


static void GuiUsbConnectionInit(void);
static void GuiUsbConnectionDeInit(void);
static void NotNowHandler(lv_event_t *e);
static void ConnectUsbHandler(lv_event_t *e);
void ConnectUsbMutexRelease(void);

static lv_obj_t *g_usbConnectionHintBox = NULL;

const GuiMsgBox_t g_guiMsgBoxUsbConnection = {
    GuiUsbConnectionInit,
    GuiUsbConnectionDeInit,
    GUI_USB_CONNECTION_PRIORITY,
};

static void GuiUsbConnectionInit(void)
{
    lv_obj_t *arc, *img, *label, *button;
    if (g_usbConnectionHintBox == NULL) {
        g_usbConnectionHintBox = GuiCreateHintBox(lv_scr_act(), 480, 416, false);
        //&imgUsbConnection
        arc = lv_arc_create(g_usbConnectionHintBox);
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
        img = GuiCreateImg(g_usbConnectionHintBox, &imgUsbConnection);
        lv_obj_align(img, LV_ALIGN_BOTTOM_LEFT, 54, -314);

        label = GuiCreateLittleTitleLabel(g_usbConnectionHintBox, _("firmware_update_usb_connect_info_title"));
        lv_obj_align(label, LV_ALIGN_BOTTOM_LEFT, 36, -232);
        label = GuiCreateIllustrateLabel(g_usbConnectionHintBox, _("firmware_update_usb_connect_info_desc"));
        lv_obj_align(label, LV_ALIGN_BOTTOM_LEFT, 36, -130);

        button = GuiCreateBtnWithFont(g_usbConnectionHintBox, _("not_now"), g_defTextFont);
        lv_obj_align(button, LV_ALIGN_BOTTOM_LEFT, 36, -24);
        lv_obj_set_size(button, 192, 66);
        lv_obj_set_style_bg_color(button, DARK_GRAY_COLOR, LV_PART_MAIN);
        lv_obj_add_event_cb(button, NotNowHandler, LV_EVENT_CLICKED, NULL);
        button = GuiCreateBtnWithFont(g_usbConnectionHintBox, _("Approve"), g_defTextFont);
        lv_obj_align(button, LV_ALIGN_BOTTOM_RIGHT, -36, -24);
        lv_obj_set_size(button, 192, 66);
        lv_obj_set_style_bg_color(button, ORANGE_COLOR, LV_PART_MAIN);
        lv_obj_add_event_cb(button, ConnectUsbHandler, LV_EVENT_CLICKED, NULL);
    }
}


static void GuiUsbConnectionDeInit(void)
{
    if (g_usbConnectionHintBox) {
        lv_obj_del(g_usbConnectionHintBox);
        g_usbConnectionHintBox = NULL;
    }
}


static void NotNowHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        CloseMsgBox(&g_guiMsgBoxUsbConnection);
        CloseUsb();
    }
}


static void ConnectUsbHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
#ifndef COMPILE_SIMULATOR
        OpenUsb();
        ConnectUsbMutexRelease();
#endif
        CloseMsgBox(&g_guiMsgBoxUsbConnection);
    }
}

