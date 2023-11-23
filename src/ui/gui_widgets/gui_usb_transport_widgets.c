#include "gui.h"
#include "gui_views.h"
#include "gui_page.h"
#include "gui_usb_transport_widgets.h"
#include "eapdu_services/service_export_address.h"

static lv_obj_t *g_cont;
static PageWidget_t *g_pageWidget;
static EAPDUResultPage_t *g_param;
static bool g_original_lock_screen = false;

static void ApproveButtonHandler(lv_event_t *e);
static void RejectButtonHandler(lv_event_t *e);
static lv_img_dsc_t *GetConnectWalletImg();
static void GuiExportXPubViewInit();
static void GuiResolveUrResultViewInit();

static void ApproveButtonHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_CLICKED)
    {
        ExportAddressApprove();
    }
}

static void RejectButtonHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_CLICKED)
    {
        ExportAddressReject();
    }
}

static lv_img_dsc_t *GetConnectWalletImg()
{
    uint8_t wallet = GetExportWallet();
    switch (wallet)
    {
    case Rabby:
        return &imgConnectWithRabby;
    default:
        return &imgConnectWithWallet;
    }
}

static void GuiExportXPubViewInit()
{
    SetLockScreen(false);
    g_pageWidget = CreatePageWidget();
    lv_obj_t *cont = g_pageWidget->contentZone;

    g_cont = cont;
    lv_obj_t *img = GuiCreateImg(cont, GetConnectWalletImg());
    lv_obj_align(img, LV_ALIGN_TOP_MID, 0, 16);

    lv_obj_t *label;
    label = GuiCreateLittleTitleLabel(cont, _("usb_transport_connection_request"));
    lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 184);

    label = GuiCreateLabel(cont, _("usb_transport_connect_rabby"));
    lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    lv_obj_set_style_text_opa(label, LV_OPA_90, LV_PART_MAIN);
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 236);

    lv_obj_t *button = GuiCreateBtnWithFont(cont, _("usb_transport_connection_reject"), g_defTextFont);
    lv_obj_align(button, LV_ALIGN_BOTTOM_LEFT, 36, -24);
    lv_obj_set_size(button, 192, 66);
    lv_obj_set_style_bg_color(button, DARK_GRAY_COLOR, LV_PART_MAIN);
    lv_obj_add_event_cb(button, RejectButtonHandler, LV_EVENT_CLICKED, NULL);

    button = GuiCreateBtnWithFont(cont, _("usb_transport_connection_approve"), g_defTextFont);
    lv_obj_align(button, LV_ALIGN_BOTTOM_RIGHT, -36, -24);
    lv_obj_set_size(button, 192, 66);
    lv_obj_set_style_bg_color(button, ORANGE_COLOR, LV_PART_MAIN);
    lv_obj_add_event_cb(button, ApproveButtonHandler, LV_EVENT_CLICKED, NULL);
}

static void GuiResolveUrResultViewInit()
{
    g_pageWidget = CreatePageWidget();
    lv_obj_t *cont = g_pageWidget->contentZone;
    g_cont = cont;

    char *title = _("usb_transport_sign_completed_title");
    char *subTitle = _("usb_transport_sign_completed_subtitle");
    char *buttonText = _("usb_transport_sign_completed_btn");
    lv_color_t buttonColor = ORANGE_COLOR;
    lv_obj_t *img = NULL;
    lv_img_dsc_t statusImg = imgSuccess;
    if (g_param->error_code != 0)
    {
        title = _("usb_transport_sign_failed_title");
        subTitle = g_param->error_message;
        buttonColor = WHITE_COLOR;
        img = GuiCreateImg(cont, &imgFailed);
        buttonText = _("usb_transport_sign_failed_btn");
    }
    else
    {
        img = GuiCreateImg(cont, &imgSuccess);
    }

    lv_obj_align(img, LV_ALIGN_TOP_MID, 0, 36);

    lv_obj_t *label;

    label = GuiCreateLittleTitleLabel(cont, title);
    lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 140);

    label = GuiCreateLabel(cont, subTitle);
    lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    lv_obj_set_style_text_opa(label, LV_OPA_90, LV_PART_MAIN);
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 192);

    lv_obj_t *button = GuiCreateBtnWithFont(cont, buttonText, g_defTextFont);
    lv_obj_align(button, LV_ALIGN_BOTTOM_MID, 0, -24);
    lv_obj_set_size(button, 408, 66);
    lv_obj_set_style_bg_color(button, buttonColor, LV_PART_MAIN);
    if (g_param->error_code != 0) {
        lv_obj_set_style_bg_opa(button, LV_OPA_12, LV_PART_MAIN);
    }
    lv_obj_add_event_cb(button, GoToHomeViewHandler, LV_EVENT_CLICKED, NULL);
}

void GuiUSBTransportWidgetsInit(EAPDUResultPage_t *param)
{
    g_original_lock_screen = IsPreviousLockScreenEnable();
    g_param = param;
    if (g_param == NULL)
    {
        return;
    }
    switch (g_param->command)
    {
    case CMD_RESOLVE_UR:
        GuiResolveUrResultViewInit();
        break;
    case CMD_EXPORT_ADDRESS:
        GuiExportXPubViewInit();
        break;
    default:
        break;
    }
}

void GuiUSBTransportWidgetsDeInit()
{
    SetLockScreen(g_original_lock_screen);
    g_param = NULL;
    GUI_DEL_OBJ(g_cont)
    if (g_pageWidget != NULL)
    {
        DestroyPageWidget(g_pageWidget);
        g_pageWidget = NULL;
    }
}

void GuiUSBTransportWidgetsRefresh()
{
    return;
}