#include "gui_page.h"
#include "gui_obj.h"
#include "multi_sig_wallet_manager.h"
#include "keystore.h"
#include "user_memory.h"
#include "define.h"
#include "gui_hintbox.h"
#include "gui_animating_qrcode.h"
#include "gui_keyboard_hintbox.h"
#include "gui_fullscreen_mode.h"
#include "gui_multisig_wallet_export_widgets.h"
#include "sdcard_manager.h"
#include "gui_button.h"

#ifndef COMPILE_SIMULATOR
#include "drv_sdcard.h"
#else
#include "simulator_model.h"
#endif

#define MAX_WALLET_NAME_LEN 48
#define MAX_WALLET_CONTENT_LEN 4096

#define MAX_ADDRESS_LEN 256
#define MAX_LABEL_LENGTH 64
#define MAX_VERIFY_CODE_LENGTH 24

static bool g_isExportMultiWallet;
static lv_obj_t *g_noticeWindow;
static lv_obj_t *g_qrCodeCont = NULL;
static PageWidget_t *g_pageWidget;
static MultiSigWalletItem_t *g_multisigWalletItem = NULL;
static char *g_filename = NULL;

static void GuiContent(lv_obj_t *parent);
static void ModelGenerateAddress(char *addr, uint32_t maxLen);
static void SetEgContent(lv_obj_t *label);
static char *convertFormatLabel(char *format);
static void GuiSDCardHandler(lv_event_t *);
static void GuiShowSDCardNotDetected();
static void GuiCloseHintBoxHandler(lv_event_t *);
static void GuiShowSDCardExport();
static void GuiShowSDCardExportSuccess();
static void GuiShowSDCardExportFailed();
static void ImportMultisigGoToHomeViewHandler(lv_event_t *e);

static void QRCodePause(bool pause)
{
    GuiAnimatingQRCodeControl(pause);
}

static void GuiSDCardHandler(lv_event_t *e)
{
    if (SdCardInsert()) {
        GuiShowSDCardExport();
    } else {
        GuiShowSDCardNotDetected();
    }
    return;
}

static void GuiCloseHintBoxHandler(lv_event_t *e)
{
    GUI_DEL_OBJ(g_noticeWindow);
}

static void GuiWriteSDCardHandler(lv_event_t *e)
{
    GUI_DEL_OBJ(g_noticeWindow);

    char *filename = lv_event_get_user_data(e);
    int ret = FileWrite(filename, g_multisigWalletItem->walletConfig, strnlen(g_multisigWalletItem->walletConfig, MAX_WALLET_CONTENT_LEN - 1));
    if (ret == 0) {
        GuiShowSDCardExportSuccess();
    } else {
        GuiShowSDCardExportFailed();
    }
    return;
}

static void GuiShowSDCardNotDetected()
{
    g_noticeWindow = GuiCreateHintBox(356);
    lv_obj_t *img = GuiCreateImg(g_noticeWindow, &imgFailed);
    lv_obj_align(img, LV_ALIGN_DEFAULT, 38, 492);

    lv_obj_t *label = GuiCreateLittleTitleLabel(g_noticeWindow, _("firmware_update_sd_failed_access_title"));
    lv_obj_align(label, LV_ALIGN_DEFAULT, 36, 588);

    label = GuiCreateIllustrateLabel(g_noticeWindow, _("firmware_update_sd_failed_access_desc"));
    lv_obj_align(label, LV_ALIGN_DEFAULT, 36, 640);

    lv_obj_t *btn = GuiCreateTextBtn(g_noticeWindow, _("OK"));
    lv_obj_set_style_bg_color(btn, WHITE_COLOR, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(btn, LV_OPA_20, LV_PART_MAIN);
    lv_obj_align(btn, LV_ALIGN_BOTTOM_RIGHT, -16, -24);
    lv_obj_add_event_cb(btn, GuiCloseHintBoxHandler, LV_EVENT_CLICKED, NULL);
}

static void GuiShowSDCardExport()
{
    g_noticeWindow = GuiCreateHintBox(356);
    lv_obj_t *img = GuiCreateImg(g_noticeWindow, &imgSdCardL);
    lv_obj_align(img, LV_ALIGN_DEFAULT, 38, 492);

    lv_obj_t *label = GuiCreateScrollLittleTitleLabel(g_noticeWindow, _("multisig_export_to_sdcard"), 408);
    lv_obj_align(label, LV_ALIGN_DEFAULT, 36, 588);

    label = GuiCreateIllustrateLabel(g_noticeWindow, _("about_info_export_file_name"));
    lv_obj_align(label, LV_ALIGN_DEFAULT, 36, 640);

    g_filename = SRAM_MALLOC(MAX_WALLET_NAME_LEN);
    uint8_t mfp[4];
    GetMasterFingerPrint(mfp);
    Ptr_Response_MultiSigWallet result = import_multi_sig_wallet_by_file(g_multisigWalletItem->walletConfig, mfp, 4);
    if (result->error_code == 0) {
        snprintf_s(g_filename, MAX_WALLET_NAME_LEN, "%s_%s_%d.txt", g_multisigWalletItem->name, result->data->policy, GetCurrentStampTime());
        free_MultiSigWallet(result->data);
    }
    label = GuiCreateIllustrateLabel(g_noticeWindow, g_filename);
    lv_obj_align(label, LV_ALIGN_DEFAULT, 36, 670);

    lv_obj_t *btn = GuiCreateTextBtn(g_noticeWindow, _("got_it"));
    lv_obj_set_style_bg_color(btn, ORANGE_COLOR, LV_PART_MAIN);
    lv_obj_align(btn, LV_ALIGN_BOTTOM_RIGHT, -16, -24);
    lv_obj_add_event_cb(btn, GuiWriteSDCardHandler, LV_EVENT_CLICKED, g_filename);
}

static void GuiShowSDCardExportSuccess()
{
    g_noticeWindow = GuiCreateHintBox(356);
    lv_obj_t *img = GuiCreateImg(g_noticeWindow, &imgSuccess);
    lv_obj_align(img, LV_ALIGN_DEFAULT, 38, 492);

    lv_obj_t *label = GuiCreateLittleTitleLabel(g_noticeWindow, _("multisig_export_to_sdcard_success"));
    lv_obj_align(label, LV_ALIGN_DEFAULT, 36, 588);

    label = GuiCreateIllustrateLabel(g_noticeWindow, _("multisig_export_to_sdcard_success_desc"));
    lv_obj_align(label, LV_ALIGN_DEFAULT, 36, 640);

    lv_obj_t *btn = GuiCreateTextBtn(g_noticeWindow, _("Done"));
    lv_obj_set_style_bg_color(btn, ORANGE_COLOR, LV_PART_MAIN);
    lv_obj_align(btn, LV_ALIGN_BOTTOM_RIGHT, -16, -24);
    lv_obj_add_event_cb(btn, GuiCloseHintBoxHandler, LV_EVENT_CLICKED, NULL);
}

static void GuiShowSDCardExportFailed()
{
    g_noticeWindow = GuiCreateHintBox(356);
    lv_obj_t *img = GuiCreateImg(g_noticeWindow, &imgFailed);
    lv_obj_align(img, LV_ALIGN_DEFAULT, 38, 492);

    lv_obj_t *label = GuiCreateLittleTitleLabel(g_noticeWindow, _("multisig_export_to_sdcard_failed"));
    lv_obj_align(label, LV_ALIGN_DEFAULT, 36, 588);

    label = GuiCreateIllustrateLabel(g_noticeWindow, _("multisig_export_to_sdcard_failed_desc"));
    lv_obj_align(label, LV_ALIGN_DEFAULT, 36, 640);

    lv_obj_t *btn = GuiCreateTextBtn(g_noticeWindow, _("OK"));
    lv_obj_set_style_bg_color(btn, ORANGE_COLOR, LV_PART_MAIN);
    lv_obj_align(btn, LV_ALIGN_BOTTOM_RIGHT, -16, -24);
    lv_obj_add_event_cb(btn, GuiCloseHintBoxHandler, LV_EVENT_CLICKED, NULL);
}

void GuiMultisigWalletExportWidgetsInit(char *verifyCode, uint16_t len)
{
    g_pageWidget = CreatePageWidget();
    MultiSigWalletItem_t *wallet = GetMultisigWalletByVerifyCode(verifyCode);
    g_multisigWalletItem = wallet;
    if (g_multisigWalletItem == NULL) {
        // TODO: Throw error;
        GuiCLoseCurrentWorkingView();
        return;
    }
    GuiContent(g_pageWidget->contentZone);
    SetNavBarMidBtn(g_pageWidget->navBarWidget, NVS_MID_BUTTON_BUTT, NULL, NULL);
    SetNavBarRightBtn(g_pageWidget->navBarWidget, NVS_BAR_SDCARD, GuiSDCardHandler, NULL);
    if (g_isExportMultiWallet) {
        SetNavBarLeftBtn(g_pageWidget->navBarWidget, NVS_BAR_RETURN, CloseCurrentViewHandler, NULL);
        SetMidBtnLabel(g_pageWidget->navBarWidget, NVS_BAR_MID_LABEL, _("manage_multi_wallet_export_title"));
    } else {
        SetNavBarLeftBtn(g_pageWidget->navBarWidget, NVS_LEFT_BUTTON_BUTT, NULL, NULL);
        SetMidBtnLabel(g_pageWidget->navBarWidget, NVS_BAR_MID_LABEL, _("import_multi_wallet_success_title"));
    }
}

static UREncodeResult *GuiGenerateUR()
{
    uint8_t mfp[4];
    GetMasterFingerPrint(mfp);
    return export_multi_sig_wallet_by_ur(mfp, 4, g_multisigWalletItem->walletConfig);
}

static void GuiCreateVerifyAddressNotice(lv_event_t *e)
{
    g_noticeWindow = GuiCreateConfirmHintBox(&imgInformation, _("manage_wallet_confirm_address_title"), _("manage_wallet_confirm_address_desc"), NULL, _("got_it"), WHITE_COLOR_OPA20);
    lv_obj_add_event_cb(GuiGetHintBoxRightBtn(g_noticeWindow), CloseHintBoxHandler, LV_EVENT_CLICKED, &g_noticeWindow);
}

static void GuiContent(lv_obj_t *parent)
{
    lv_obj_t *cont, *text, *btn;

    cont = GuiCreateContainerWithParent(parent, 480, 800 - 48 - 96 - 114);
    lv_obj_add_flag(cont, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_flag(cont, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(cont, LV_SCROLLBAR_MODE_OFF);

    text = GuiCreateNoticeLabel(cont, _("multisig_import_success_hint"));
    lv_obj_align(text, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_width(text, 450);
    lv_obj_set_style_text_align(text, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);

    cont = GuiCreateContainerWithParent(cont, 408, 134 + 384);
    lv_obj_set_style_bg_color(cont, DARK_BG_COLOR, LV_PART_MAIN);
    lv_obj_set_style_radius(cont, 24, LV_PART_MAIN);
    lv_obj_align(cont, LV_ALIGN_TOP_LEFT, 36, 122);

    g_qrCodeCont = GuiCreateContainerWithParent(cont, 336, 336);
    lv_obj_align(g_qrCodeCont, LV_ALIGN_TOP_LEFT, 36, 36);
    GuiAnimatingQRCodeInitWithCustomSize(g_qrCodeCont, GuiGenerateUR, false, 336, 336, NULL);

    text = GuiCreateIllustrateLabel(cont, convertFormatLabel(g_multisigWalletItem->format));
    lv_obj_align(text, LV_ALIGN_TOP_LEFT, 36, 12 + 384);

    lv_obj_t *button = GuiCreateImgButton(cont, &imgInfoS, 40, GuiCreateVerifyAddressNotice, NULL);
    lv_obj_align(button, LV_ALIGN_TOP_RIGHT, -24, 416);

    text = GuiCreateNoticeLabel(cont, _("Sample Address:"));
    lv_obj_align(text, LV_ALIGN_TOP_LEFT, 36, 12 + 34 + 384);

    text = GuiCreateIllustrateLabel(cont, "text_placeholder");
    lv_obj_set_width(text, 360);
    lv_obj_align(text, LV_ALIGN_TOP_LEFT, 36, 12 + 34 + 384 + 30);
    lv_label_set_recolor(text, true);
    SetEgContent(text);

    btn = GuiCreateTextBtn(parent, _("Done"));
    lv_obj_set_size(btn, 408, 66);
    lv_obj_align(btn, LV_ALIGN_BOTTOM_MID, 0, -24);
    lv_obj_add_event_cb(btn, ImportMultisigGoToHomeViewHandler, LV_EVENT_CLICKED, NULL);
}

static char *convertFormatLabel(char *format)
{
    if (strcmp(format, FORMAT_P2WSH) == 0) {
        return "Native Segwit";
    } else if ((strcmp(format, FORMAT_P2WSH_P2SH) == 0) || (strcmp(format, FORMAT_P2SH_P2WSH)) == 0 ||
               (strcmp(format, FORMAT_P2WSH_P2SH_MID) == 0) || (strcmp(format, FORMAT_P2SH_P2WSH_MID)) == 0) {
        return "Nested Segwit";
    } else if (strcmp(format, FORMAT_P2SH) == 0) {
        return "Legacy";
    }
    return NULL;
}

static void SetEgContent(lv_obj_t *label)
{
    char eg[BUFFER_SIZE_64] = {0};
    char prefix[8] = {0};
    char rest[BUFFER_SIZE_64] = {0};
    char addr[BUFFER_SIZE_128] = {0};
    char addrShot[BUFFER_SIZE_64] = {0};
    int8_t prefixLen = (strcmp(g_multisigWalletItem->format, FORMAT_P2WSH) == 0) ? 4 : 1;
    memset_s(addrShot, sizeof(addrShot), 0, sizeof(addrShot));
    ModelGenerateAddress(addr, sizeof(addr));
    CutAndFormatString(addrShot, sizeof(addrShot), addr, 24);
    strncpy(prefix, addrShot, prefixLen);
    strncpy(rest, addrShot + prefixLen, strnlen_s(addrShot, BUFFER_SIZE_64) - prefixLen);
    snprintf_s(eg, sizeof(eg), "#F5870A %s#%s", prefix, rest);
    lv_label_set_text(label, addrShot);
}

void GuiMultisigWalletExportWidgetsDeInit()
{
    GuiAnimatingQRCodeDestroyTimer();
    GUI_DEL_OBJ(g_noticeWindow);
    if (g_pageWidget != NULL) {
        DestroyPageWidget(g_pageWidget);
        g_pageWidget = NULL;
    }
    g_isExportMultiWallet = false;
    if (g_filename) {
        SRAM_FREE(g_filename);
        g_filename = NULL;
    }
}

void GuiMultisigWalletExportWidgetsRefresh()
{
    GuiAnimatingQRCodeControl(false);
}

static void ModelGenerateAddress(char *address, uint32_t maxLen)
{
    uint8_t mfp[4];
    GetMasterFingerPrint(mfp);
    SimpleResponse_c_char *result = generate_address_for_multisig_wallet_config(g_multisigWalletItem->walletConfig, 0, 0, mfp, 4);
    if (result->error_code != 0) {
        printf("errorMessage: %s\r\n", result->error_message);
        GuiCLoseCurrentWorkingView();
        return;
    }
    strncpy_s(address, maxLen, result->data, strnlen_s(result->data, maxLen));
}

void ModelGenerateMultiSigAddress(char *address, uint32_t maxLen, char *walletConfig, uint32_t index)
{
    uint8_t mfp[4];
    GetMasterFingerPrint(mfp);
    SimpleResponse_c_char *result = generate_address_for_multisig_wallet_config(walletConfig, 0, index, mfp, 4);
    if (result->error_code != 0) {
        printf("errorMessage: %s\r\n", result->error_message);
        GuiCLoseCurrentWorkingView();
        return;
    }
    strncpy_s(address, maxLen, result->data, strnlen_s(result->data, maxLen));
    address[strnlen_s(result->data, maxLen)] = '\0';
}

static void ImportMultisigGoToHomeViewHandler(lv_event_t *e)
{
    g_isExportMultiWallet = false;
    GuiCloseToTargetView(&g_homeView);
}

void GuiSetExportMultiSigSwitch(void)
{
    g_isExportMultiWallet = true;
}

bool GuiGetExportMultisigWalletSwitch()
{
    return g_isExportMultiWallet;
}