#ifndef BTC_ONLY
#include "gui.h"
#include "gui_views.h"
#include "gui_page.h"
#include "gui_usb_transport_widgets.h"
#include "eapdu_services/service_export_address.h"
#include "screen_manager.h"

static lv_obj_t *g_cont;
static PageWidget_t *g_pageWidget;
static EAPDUResultPage_t *g_param;
static bool g_original_lock_screen = false;

typedef struct {
    const lv_img_dsc_t *img;
    const char *title;
} WalletInfo_t;

typedef struct {
    const char *title;
    const char *subTitle;
    const char *buttonText;
} ResolveUrInfo_t;

static void ApproveButtonHandler(lv_event_t *e);
static void RejectButtonHandler(lv_event_t *e);
static WalletInfo_t GetConnectWalletInfo();
static void GuiExportXPubViewInit();
static void GuiResolveUrResultViewInit();

static void ApproveButtonHandler(lv_event_t *e)
{
    ExportAddressApprove();
    GuiCLoseCurrentWorkingView();
}

static void RejectButtonHandler(lv_event_t *e)
{
    ExportAddressReject();
    GuiCLoseCurrentWorkingView();
}

static WalletInfo_t GetConnectWalletInfo()
{
    uint8_t wallet = GetExportWallet();
    WalletInfo_t walletInfo = {
        .img = &imgConnectWithWallet,
        .title = _("usb_transport_connect_wallet"),
    };
    switch (wallet) {
    case Rabby:
        walletInfo.img = &imgConnectWithRabby;
        walletInfo.title = _("usb_transport_connect_rabby");
        return walletInfo;
    default:
        return walletInfo;
    }
}

static void GuiExportXPubViewInit()
{
    SetLockScreen(false);
    g_pageWidget = CreatePageWidget();
    lv_obj_t *cont = g_pageWidget->contentZone;

    g_cont = cont;
    WalletInfo_t walletInfo = GetConnectWalletInfo();
    lv_obj_t *img = GuiCreateImg(cont, walletInfo.img);
    lv_obj_align(img, LV_ALIGN_TOP_MID, 0, 16);

    lv_obj_t *label;
    label = GuiCreateLittleTitleLabel(cont, _("usb_transport_connection_request"));
    lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 184);

    label = GuiCreateIllustrateLabel(cont, walletInfo.title);
    lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    lv_obj_set_style_text_opa(label, LV_OPA_90, LV_PART_MAIN);
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 236);

    lv_obj_t *button = GuiCreateTextBtn(cont, _("Reject"));
    lv_obj_align(button, LV_ALIGN_BOTTOM_LEFT, 36, -24);
    lv_obj_set_size(button, 192, 66);
    lv_obj_set_style_bg_color(button, DARK_GRAY_COLOR, LV_PART_MAIN);
    lv_obj_add_event_cb(button, RejectButtonHandler, LV_EVENT_CLICKED, NULL);

    button = GuiCreateTextBtn(cont, _("Approve"));
    lv_obj_align(button, LV_ALIGN_BOTTOM_RIGHT, -36, -24);
    lv_obj_set_size(button, 192, 66);
    lv_obj_set_style_bg_color(button, ORANGE_COLOR, LV_PART_MAIN);
    lv_obj_add_event_cb(button, ApproveButtonHandler, LV_EVENT_CLICKED, NULL);
}

static ResolveUrInfo_t CalcResolveUrPageInfo()
{
    ResolveUrInfo_t info = {
        .title = _("usb_transport_sign_completed_title"),
        .subTitle = _("usb_transport_sign_completed_subtitle"),
        .buttonText = _("Done"),
    };
    switch (g_param->error_code) {
    case RSP_SUCCESS_CODE:
        break;
    case PRS_PARSING_MISMATCHED_WALLET:
        info.title = _("usb_transport_mismatched_wallet_title");
        info.subTitle = g_param->error_message;
        info.buttonText = _("OK");
        break;
    default:
        info.title = _("usb_transport_sign_unkown_error_title");
        info.subTitle = _("usb_transport_sign_unkown_error_message");
        info.buttonText = _("OK");
        break;
    }
    return info;
}

static void GuiResolveUrResultViewInit()
{
    g_pageWidget = CreatePageWidget();
    lv_obj_t *cont = g_pageWidget->contentZone;
    g_cont = cont;

    ResolveUrInfo_t info = CalcResolveUrPageInfo();
    const char *title = info.title;
    const char *subTitle = info.subTitle;
    const char *buttonText = info.buttonText;
    lv_color_t buttonColor = ORANGE_COLOR;
    lv_obj_t *img = NULL;
    if (g_param->error_code != 0) {
        buttonColor = WHITE_COLOR;
        img = GuiCreateImg(cont, &imgFailed);
    } else {
        img = GuiCreateImg(cont, &imgSuccess);
    }

    lv_obj_align(img, LV_ALIGN_TOP_MID, 0, 36);

    lv_obj_t *label;

    label = GuiCreateLittleTitleLabel(cont, title);
    lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 140);

    label = GuiCreateIllustrateLabel(cont, subTitle);
    lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    lv_obj_set_style_text_opa(label, LV_OPA_90, LV_PART_MAIN);
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 192);

    lv_obj_t *button = GuiCreateTextBtn(cont, buttonText);
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
    if (g_param == NULL) {
        return;
    }
    switch (g_param->command) {
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
    if (g_pageWidget != NULL) {
        DestroyPageWidget(g_pageWidget);
        g_pageWidget = NULL;
    }
}

void GuiUSBTransportWidgetsRefresh()
{
    return;
}
#endif
