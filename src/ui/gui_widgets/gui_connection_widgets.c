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
#include "gui_api.h"
#include "user_fatfs.h"
#ifndef COMPILE_SIMULATOR
#include "drv_usb.h"
#endif

static lv_obj_t *g_cont;
static lv_obj_t *usbConnectionSw;
static PageWidget_t *g_pageWidget;
static lv_obj_t *g_noticeWindow;

static void GuiConnectionNVSBarInit(void);
static void GuiConnectionEntranceWidget(lv_obj_t *parent);

static void UsbConnectionHandler(lv_event_t *e);
static void UsbConnectionSwitchHandler(lv_event_t * e);
static void FormatMicroSDWindowHandler(lv_event_t *e);

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
{

}

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
    if (!GetUSBSwitch()) {
        lv_obj_add_state(usbConnectionSw, LV_STATE_CHECKED);
    } else {
        lv_obj_clear_state(usbConnectionSw, LV_STATE_CHECKED);
    }

    lv_obj_t *titlelabel = GuiCreateTextLabel(parent, _("usb_connection_subtitle"));
    lv_obj_t *contentLabel = GuiCreateIllustrateLabel(parent, _("usb_connection_desc"));
    lv_obj_set_style_text_opa(contentLabel, LV_OPA_80, LV_PART_MAIN);
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

    lv_obj_t *button = GuiCreateButton(parent, 456, 178, table, NUMBER_OF_ARRAYS(table),
                                       UsbConnectionHandler, NULL);
    lv_obj_align(button, LV_ALIGN_DEFAULT, 12, 0);

    lv_obj_t *line = GuiCreateDividerLine(parent);
    lv_obj_align_to(line, button, LV_ALIGN_OUT_BOTTOM_MID, 0, 12);

    table[0].obj = GuiCreateTextLabel(parent, _("sdcard_format_subtitle"));
    table[0].align = LV_ALIGN_LEFT_MID;
    table[0].position.x = 24;
    table[0].position.y = 0;
    button = GuiCreateButton(parent, 456, 84, table, 1, FormatMicroSDWindowHandler, NULL);
    lv_obj_align_to(button, line, LV_ALIGN_OUT_BOTTOM_MID, 0, 12);
}

void FormatMicroHandleResult(bool en)
{
    GUI_DEL_OBJ(g_noticeWindow)
    if (en) {
        g_noticeWindow = GuiCreateConfirmHintBox(lv_scr_act(), &imgSuccess, _("sdcard_format_success_title"), _("sdcard_format_success_desc"), NULL, _("Done"), ORANGE_COLOR);
        lv_obj_t *btn = GuiGetHintBoxRightBtn(g_noticeWindow);
        lv_obj_add_event_cb(btn, CloseHintBoxHandler, LV_EVENT_CLICKED, &g_noticeWindow);
    } else {
        g_noticeWindow = GuiCreateConfirmHintBox(lv_scr_act(), &imgFailed, _("sdcard_format_failed_title"), _("sdcard_format_failed_desc"), NULL, _("Done"), ORANGE_COLOR);
        lv_obj_t *btn = GuiGetHintBoxRightBtn(g_noticeWindow);
        lv_obj_add_event_cb(btn, CloseHintBoxHandler, LV_EVENT_CLICKED, &g_noticeWindow);
    }
}

static void FormatMicroSDHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        GUI_DEL_OBJ(g_noticeWindow)
        if (SdCardInsert()) {
            g_noticeWindow = GuiCreateAnimHintBox(lv_scr_act(), 480, 278, 82);
            lv_obj_t *title = GuiCreateTextLabel(g_noticeWindow, _("sdcard_formating"));
            lv_obj_align(title, LV_ALIGN_BOTTOM_MID, 0, -76);
            GuiModelFormatMicroSd();
        } else {
            g_noticeWindow = GuiCreateErrorCodeHintbox(ERR_UPDATE_SDCARD_NOT_DETECTED, &g_noticeWindow);
        }
    }
}

static void FormatMicroSDWindowHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        g_noticeWindow = GuiCreateGeneralHintBox(lv_scr_act(), &imgWarn, _("sdcard_format_subtitle"), _("sdcard_format_desc"), NULL,
                         _("Cancel"), WHITE_COLOR_OPA20, _("sdcard_format_confirm"), DEEP_ORANGE_COLOR);
        lv_obj_t *leftBtn = GuiGetHintBoxLeftBtn(g_noticeWindow);
        lv_obj_add_event_cb(leftBtn, CloseHintBoxHandler, LV_EVENT_CLICKED, &g_noticeWindow);
        lv_obj_t *rightBtn = GuiGetHintBoxRightBtn(g_noticeWindow);
        lv_obj_add_event_cb(rightBtn, FormatMicroSDHandler, LV_EVENT_CLICKED, NULL);
    }
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
            printf("air gap on...\n");
            SetUSBSwitch(0);
            CloseUsb();
        } else {
            SetUsbState(true);
            SetUSBSwitch(1);
            printf("air gap off...\n");
            GuiApiEmitSignalWithValue(SIG_INIT_USB_CONNECTION, 1);
        }
        SaveDeviceSettings();
    }
}