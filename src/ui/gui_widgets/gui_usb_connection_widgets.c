#include "gui_usb_connection_widgets.h"
#include "gui.h"
#include "gui_pop_message_box.h"
#include "gui_hintbox.h"
#include "usb_task.h"
#include "gui_views.h"
#include "gui_api.h"
#include "device_setting.h"
#include "drv_aw32001.h"

static void GuiUsbConnectionInit(void);
static void GuiUsbConnectionDeInit(void);
static void NotNowHandler(lv_event_t *e);
static void ConnectUsbHandler(lv_event_t *e);

static lv_obj_t *g_usbConnectionHintBox = NULL;

const GuiMsgBox_t g_guiMsgBoxUsbConnection = {
    GuiUsbConnectionInit,
    GuiUsbConnectionDeInit,
    GUI_USB_CONNECTION_PRIORITY,
};

void GuiUsbConnectionInit(void)
{
    if (g_usbConnectionHintBox == NULL) {
        g_usbConnectionHintBox = GuiCreateGeneralHintBox(&imgUsbConnection, _("firmware_update_usb_connect_info_title"), _("firmware_update_usb_connect_info_desc"), NULL, _("not_now"), DARK_GRAY_COLOR, _("Approve"), RED_COLOR);
        lv_obj_t *leftBtn = GuiGetHintBoxLeftBtn(g_usbConnectionHintBox);
        lv_obj_add_event_cb(leftBtn, NotNowHandler, LV_EVENT_CLICKED, g_usbConnectionHintBox);

        lv_obj_t *rightBtn = GuiGetHintBoxRightBtn(g_usbConnectionHintBox);
        lv_obj_add_event_cb(rightBtn, ConnectUsbHandler, LV_EVENT_CLICKED, g_usbConnectionHintBox);
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
    CloseMsgBox(&g_guiMsgBoxUsbConnection);
    CloseUsb();
}

static void ConnectUsbHandler(lv_event_t *e)
{
#ifndef COMPILE_SIMULATOR
    if (GetUSBSwitch() && GetUsbDetectState()) {
        OpenUsb();
    }
#endif
    CloseMsgBox(&g_guiMsgBoxUsbConnection);
}
