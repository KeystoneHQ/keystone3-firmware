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
#include "gui_page.h"
#ifndef COMPILE_SIMULATOR
#include "drv_usb.h"
#endif

static lv_obj_t *g_cont;
static lv_obj_t *usbConnectionSw;
static PageWidget_t *g_pageWidget;


static void GuiConnectionNVSBarInit(void);
static void GuiConnectionEntranceWidget(lv_obj_t *parent);

static void UsbConnectionHandler(lv_event_t *e);
static void UsbConnectionSwitchHandler(lv_event_t * e);

void GuiConnectionWidgetsInit()
{
    g_pageWidget = CreatePageWidget();
    lv_obj_t *cont = g_pageWidget->contentZone;
    g_cont = cont;
    GuiConnectionEntranceWidget(cont);
}

void GuiConnectionWidgetsDeInit()
{
    if (g_cont != NULL) {
        lv_obj_del(g_cont);
        g_cont = NULL;
    }
    if (g_pageWidget != NULL) {
        DestroyPageWidget(g_pageWidget);
        g_pageWidget = NULL;
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
    SetNavBarLeftBtn(g_pageWidget->navBarWidget, NVS_BAR_RETURN, CloseCurrentViewHandler, NULL);
    SetMidBtnLabel(g_pageWidget->navBarWidget, NVS_BAR_MID_LABEL, _("usb_connection_title"));
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

    lv_obj_t *titlelabel = GuiCreateTextLabel(parent, _("usb_connection_subtitle"));

    lv_obj_t *contentLabel = lv_label_create(parent);
    lv_label_set_text(contentLabel, _("usb_connection_desc"));
    lv_obj_set_style_text_font(contentLabel, g_defIllustrateFont, LV_PART_MAIN);
    lv_obj_set_style_text_color(contentLabel, WHITE_COLOR, LV_PART_MAIN);
    lv_obj_set_style_text_opa(contentLabel, LV_OPA_56, LV_PART_MAIN);
    lv_label_set_long_mode(contentLabel, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(contentLabel, 346);


    GuiButton_t table[] = {
        {
            .obj = titlelabel,
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