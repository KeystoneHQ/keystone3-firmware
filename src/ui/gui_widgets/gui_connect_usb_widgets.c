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

static void GuiConnectUsbNVSBarInit(void);
static void GuiConnectUsbEntranceWidget(lv_obj_t *parent);
static void GuiConnectUsbCreateImg(lv_obj_t *parent);
static void RejectButtonHandler(lv_event_t *e);
static void ApproveButtonHandler(lv_event_t *e);

static PageWidget_t *g_pageWidget;
static lv_obj_t *g_noticeWindow = NULL;
static KeyboardWidget_t *g_keyboardWidget = NULL;

void GuiConnectUsbWidgetsInit(void)
{
    g_pageWidget = CreatePageWidget();
    GuiConnectUsbEntranceWidget(g_pageWidget->contentZone);
}

void GuiConnectUsbWidgetsDeInit(void)
{
    ClearSecretCache();
    if (g_pageWidget != NULL) {
        DestroyPageWidget(g_pageWidget);
        g_pageWidget = NULL;
    }
    GUI_DEL_OBJ(g_noticeWindow)
}

void GuiConnectUsbWidgetsRefresh(void)
{
    GuiConnectUsbNVSBarInit();
}

static void GuiConnectUsbNVSBarInit(void)
{
    SetNavBarLeftBtn(g_pageWidget->navBarWidget, NVS_LEFT_BUTTON_BUTT, NULL, NULL);
    SetNavBarRightBtn(g_pageWidget->navBarWidget, NVS_RIGHT_BUTTON_BUTT, NULL, NULL);
    SetNavBarMidBtn(g_pageWidget->navBarWidget, NVS_MID_BUTTON_BUTT, NULL, NULL);
}

void GuiConnectUsbPasswordPass(void)
{
    GuiDeleteKeyboardWidget(g_keyboardWidget);
    g_keyboardWidget = NULL;
    lv_obj_clean(g_pageWidget->contentZone);
    lv_obj_t *parent = g_pageWidget->contentZone;
    GuiConnectUsbCreateImg(parent);

    lv_obj_t *titlelabel = GuiCreateLittleTitleLabel(parent, _("Connectioning Wallet"));
    lv_obj_align(titlelabel, LV_ALIGN_TOP_MID, 0, 184);
    lv_obj_t *contentLabel = GuiCreateNoticeLabel(parent, _("Please wait until the software wallet has completed the connection, then click to close."));
    lv_obj_set_style_text_align(contentLabel, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(contentLabel, LV_ALIGN_TOP_MID, 0, 234);

    lv_obj_t *button = GuiCreateTextBtn(parent, _("Close"));
    lv_obj_align(button, LV_ALIGN_BOTTOM_MID, 0, -24);
    lv_obj_set_size(button, 408, 66);
    lv_obj_add_event_cb(button, CloseCurrentViewHandler, LV_EVENT_CLICKED, NULL);
}

void GuiConnectUsbPasswordErrorCount(void *param)
{
    PasswordVerifyResult_t *passwordVerifyResult = (PasswordVerifyResult_t *)param;
    if (passwordVerifyResult->errorCount == MAX_CURRENT_PASSWORD_ERROR_COUNT_SHOW_HINTBOX) {
        // if (GetCurrentTransactionMode() == TRANSACTION_MODE_USB) {
        //     const char *data = "Please try again after unlocking";
        //     HandleURResultViaUSBFunc(data, strlen(data), GetCurrentUSParsingRequestID(), PRS_PARSING_VERIFY_PASSWORD_ERROR);
        // }
    }
    GuiShowErrorNumber(g_keyboardWidget, passwordVerifyResult);
}

static void GuiConnectUsbEntranceWidget(lv_obj_t *parent)
{
    GuiConnectUsbCreateImg(parent);
    lv_obj_t *titlelabel = GuiCreateLittleTitleLabel(parent, _("usb_transport_connection_request"));
    lv_obj_align(titlelabel, LV_ALIGN_TOP_MID, 0, 184);
    lv_obj_t *contentLabel = GuiCreateNoticeLabel(parent, _("The software wallet is requesting to connect to your Keystone via USB.Enter your password to approve."));
    lv_obj_set_style_text_align(contentLabel, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(contentLabel, LV_ALIGN_TOP_MID, 0, 234);

    lv_obj_t *button = GuiCreateTextBtn(parent, _("Reject"));
    lv_obj_align(button, LV_ALIGN_BOTTOM_LEFT, 36, -24);
    lv_obj_set_size(button, 192, 66);
    lv_obj_add_event_cb(button, RejectButtonHandler, LV_EVENT_CLICKED, NULL);
    lv_obj_set_style_bg_color(button, WHITE_COLOR, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(button, LV_OPA_20, LV_PART_MAIN);
    
    button = GuiCreateTextBtn(parent, _("Approve"));
    lv_obj_align(button, LV_ALIGN_BOTTOM_RIGHT, -36, -24);
    lv_obj_set_size(button, 192, 66);
    lv_obj_add_event_cb(button, ApproveButtonHandler, LV_EVENT_CLICKED, NULL);
}

static void GuiConnectUsbCreateImg(lv_obj_t *parent)
{
    lv_obj_t *img = GuiCreateImg(parent, &imgSoftwareWallet);
    lv_obj_align(img, LV_ALIGN_DEFAULT, 110, 40);

    img = GuiCreateImg(parent, &imgLogoGraph);
    lv_obj_align(img, LV_ALIGN_DEFAULT, 298, 40);
}

static void ApproveButtonHandler(lv_event_t *e)
{
    printf("approve ...\n");
    // todo push reject message to usb

    static uint16_t sig = SIG_INIT_CONNECT_USB;
    g_keyboardWidget = GuiCreateKeyboardWidget(g_pageWidget->contentZone);
    SetKeyboardWidgetSelf(g_keyboardWidget, &g_keyboardWidget);
    SetKeyboardWidgetSig(g_keyboardWidget, &sig);
}

static void RejectButtonHandler(lv_event_t *e)
{
    printf("recject ...\n");
    // todo push reject message to usb
    GuiCLoseCurrentWorkingView();
}