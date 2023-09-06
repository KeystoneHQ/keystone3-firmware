#include "gui.h"
#include "gui_views.h"
#include "gui_status_bar.h"
#include "gui_keyboard.h"
#include "gui_button.h"
#include "gui_hintbox.h"
#include "gui_model.h"
#include "user_memory.h"
#include "presetting.h"
#include "gui_connection_widgets.h"
#include "device_setting.h"
#include "usb_task.h"
#ifndef COMPILE_SIMULATOR
#include "drv_usb.h"
#endif

static lv_obj_t *g_cont;
static lv_obj_t *usbConnectionSw;


static void GuiConnectionNVSBarInit(void);
static void GuiConnectionEntranceWidget(lv_obj_t *parent);

static void UsbConnectionHandler(lv_event_t *e);
static void UsbConnectionSwitchHandler(lv_event_t * e);

void GuiConnectionWidgetsInit()
{
    GuiConnectionNVSBarInit();

    lv_obj_t *cont = GuiCreateContainer(lv_obj_get_width(lv_scr_act()), lv_obj_get_height(lv_scr_act()) -
                                        GUI_MAIN_AREA_OFFSET);
    lv_obj_align(cont, LV_ALIGN_DEFAULT, 0, GUI_MAIN_AREA_OFFSET);
    lv_obj_add_flag(cont, LV_OBJ_FLAG_CLICKABLE);
    g_cont = cont;
    GuiConnectionEntranceWidget(cont);
}

void GuiConnectionWidgetsDeInit()
{
    if (g_cont != NULL) {
        lv_obj_del(g_cont);
        g_cont = NULL;
    }
}

void GuiConnectionWidgetsRefresh()
{
    GuiConnectionNVSBarInit();
}


void GuiConnectionWidgetsRestart()
{}


static void GuiConnectionNVSBarInit()
{
    GuiNvsBarSetLeftCb(NVS_BAR_RETURN, CloseCurrentViewHandler, NULL);
    GuiNvsBarSetMidBtnLabel(NVS_BAR_MID_LABEL, "Connection");
    GuiNvsBarSetRightCb(NVS_RIGHT_BUTTON_BUTT, NULL, NULL);
}




void GuiConnectionEntranceWidget(lv_obj_t *parent)
{

    usbConnectionSw = lv_switch_create(parent);
    lv_obj_add_event_cb(usbConnectionSw, UsbConnectionSwitchHandler, LV_EVENT_ALL, NULL);
    lv_obj_set_style_bg_color(usbConnectionSw, ORANGE_COLOR, LV_STATE_CHECKED | LV_PART_INDICATOR);
    lv_obj_set_style_bg_color(usbConnectionSw, WHITE_COLOR, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(usbConnectionSw, LV_OPA_30, LV_PART_MAIN);
    if (GetUSBSwitch()) {
        lv_obj_add_state(usbConnectionSw, LV_STATE_CHECKED);
    } else {
        lv_obj_clear_state(usbConnectionSw, LV_STATE_CHECKED);
    }

    lv_obj_t *tittlelabel = GuiCreateTextLabel(parent, "Data transfer with USB");

    lv_obj_t *contentLabel = lv_label_create(parent);
    lv_label_set_text(contentLabel, "When disabled, the usb can only be used for charging battery");
    lv_obj_set_style_text_font(contentLabel, g_defIllustrateFont, LV_PART_MAIN);
    lv_obj_set_style_text_color(contentLabel, WHITE_COLOR, LV_PART_MAIN);
    lv_obj_set_style_text_opa(contentLabel, LV_OPA_56, LV_PART_MAIN);
    lv_label_set_long_mode(contentLabel, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(contentLabel, 346);


    GuiButton_t table[] = {
        {
            .obj = tittlelabel,
            .align = LV_ALIGN_DEFAULT,
            .position = {24, 24},
        },
        {
            .obj = usbConnectionSw,
            .align = LV_ALIGN_DEFAULT,
            .position = {376, 24},
        },
        {
            .obj = contentLabel,
            .align = LV_ALIGN_DEFAULT,
            .position = {24, 64},
        },
    };

    lv_obj_t *button = GuiCreateButton(parent, 456, 148, table, NUMBER_OF_ARRAYS(table),
                                       UsbConnectionHandler, NULL);
    // lv_obj_clear_flag(button, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_align(button, LV_ALIGN_DEFAULT, 12, 0);

}

static void UsbConnectionHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        if (lv_obj_has_state(usbConnectionSw, LV_STATE_CHECKED)) {
            lv_obj_clear_state(usbConnectionSw, LV_STATE_CHECKED);
        } else {
            lv_obj_add_state(usbConnectionSw, LV_STATE_CHECKED);
        }
        lv_event_send(usbConnectionSw, LV_EVENT_VALUE_CHANGED, NULL);
    }

}

static void UsbConnectionSwitchHandler(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * obj = lv_event_get_target(e);

    if (code == LV_EVENT_VALUE_CHANGED) {
        if (lv_obj_has_state(obj, LV_STATE_CHECKED)) {
            printf("usb switch on\n");
            SetUSBSwitch(1);
            CloseUsb();
            OpenUsb();
        } else {
            SetUSBSwitch(0);
            SetUsbState(false);
            CloseUsb();
            printf("usb switch off\n");
        }
        SaveDeviceSettings();
    }
}