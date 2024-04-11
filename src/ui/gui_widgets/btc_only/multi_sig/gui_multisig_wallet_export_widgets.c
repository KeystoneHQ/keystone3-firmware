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

#ifndef COMPILE_SIMULATOR
#include "safe_str_lib.h"
#include "drv_sdcard.h"
#else
#include "simulator_mock_define.h"
#include "simulator_model.h"
#endif

#define MAX_WALLET_NAME_LEN 48
#define MAX_WALLET_CONTENT_LEN 1024

#define MAX_ADDRESS_LEN 256
#define MAX_LABEL_LENGTH 64
#define MAX_VERIFY_CODE_LENGTH 24

typedef enum {
    IMPORT_MULTI_SHOW_WALLET_INFO = 0,
    IMPORT_MULTI_WALLET_SUCCESS,

    IMPORT_MULTI_WALLET_BUTT,
} IMPORT_MULTI_WALLET_ENUM;

typedef struct {
    uint8_t currentTile;
    lv_obj_t *tileView;
} ImportMultiWalletWidget_t;

static ImportMultiWalletWidget_t g_importMultiWallet;
static bool g_isExportMultiWallet;
static lv_obj_t *g_noticeWindow;
static lv_obj_t *g_confirmLabel;
static lv_obj_t *g_qrCodeCont = NULL;
static PageWidget_t *g_pageWidget;
static MultiSigWalletItem_t *g_multisigWalletItem = NULL;
static char *g_filename = NULL;

void CutAndFormatAddress(char *out, uint32_t maxLen, const char *address, uint32_t targetLen);
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
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        if (SdCardInsert()) {
            GuiShowSDCardExport();
        } else {
            GuiShowSDCardNotDetected();
        }
        return;
    }
}

static void GuiCloseHintBoxHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        GUI_DEL_OBJ(g_noticeWindow);
        return;
    }
}

static void GuiWriteSDCardHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        GUI_DEL_OBJ(g_noticeWindow);

        char *filename = lv_event_get_user_data(e);
        int ret = FileWrite(filename, g_multisigWalletItem->walletConfig, strnlen(g_multisigWalletItem->walletConfig, MAX_WALLET_CONTENT_LEN));
        if (ret == 0) {
            GuiShowSDCardExportSuccess();
        } else {
            GuiShowSDCardExportFailed();
        }
        return;
    }
}

static void GuiShowSDCardNotDetected()
{
    g_noticeWindow = GuiCreateHintBox(lv_scr_act(), 480, 356, false);
    lv_obj_t *img = GuiCreateImg(g_noticeWindow, &imgFailed);
    lv_obj_align(img, LV_ALIGN_DEFAULT, 38, 492);

    lv_obj_t *label = GuiCreateLittleTitleLabel(g_noticeWindow, _("multisig_export_sdcard_not_detected"));
    lv_obj_align(label, LV_ALIGN_DEFAULT, 36, 588);

    label = GuiCreateIllustrateLabel(g_noticeWindow, _("multisig_export_sdcard_not_detected_desc"));
    lv_obj_align(label, LV_ALIGN_DEFAULT, 36, 640);

    lv_obj_t *btn = GuiCreateBtnWithFont(g_noticeWindow, _("OK"), g_defTextFont);
    lv_obj_set_size(btn, 94, 66);
    lv_obj_set_style_bg_color(btn, WHITE_COLOR, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(btn, LV_OPA_20, LV_PART_MAIN);
    lv_obj_align(btn, LV_ALIGN_BOTTOM_RIGHT, -16, -24);
    lv_obj_add_event_cb(btn, GuiCloseHintBoxHandler, LV_EVENT_CLICKED, NULL);
}

static void GuiShowSDCardExport()
{
    g_noticeWindow = GuiCreateHintBox(lv_scr_act(), 480, 356, false);
    lv_obj_t *img = GuiCreateImg(g_noticeWindow, &imgSdCardL);
    lv_obj_align(img, LV_ALIGN_DEFAULT, 38, 492);

    lv_obj_t *label = GuiCreateLittleTitleLabel(g_noticeWindow, _("multisig_export_to_sdcard"));
    lv_obj_align(label, LV_ALIGN_DEFAULT, 36, 588);

    label = GuiCreateIllustrateLabel(g_noticeWindow, _("multisig_export_to_sdcard_desc"));
    lv_obj_align(label, LV_ALIGN_DEFAULT, 36, 640);

    g_filename = SRAM_MALLOC(MAX_WALLET_NAME_LEN);
    snprintf_s(g_filename, MAX_WALLET_NAME_LEN, "%s.txt", g_multisigWalletItem->name);
    if (FileExists(g_filename)) {
        snprintf_s(g_filename, MAX_WALLET_NAME_LEN, "%s_%d.txt", g_multisigWalletItem->name, GetCurrentStampTime());
    }
    label = GuiCreateIllustrateLabel(g_noticeWindow, g_filename);
    lv_obj_align(label, LV_ALIGN_DEFAULT, 36, 670);

    lv_obj_t *btn = GuiCreateBtnWithFont(g_noticeWindow, _("got_it"), g_defTextFont);
    lv_obj_set_size(btn, 122, 66);
    lv_obj_set_style_bg_color(btn, ORANGE_COLOR, LV_PART_MAIN);
    lv_obj_align(btn, LV_ALIGN_BOTTOM_RIGHT, -16, -24);
    lv_obj_add_event_cb(btn, GuiWriteSDCardHandler, LV_EVENT_CLICKED, g_filename);
}

static void GuiShowSDCardExportSuccess()
{
    g_noticeWindow = GuiCreateHintBox(lv_scr_act(), 480, 356, false);
    lv_obj_t *img = GuiCreateImg(g_noticeWindow, &imgSuccess);
    lv_obj_align(img, LV_ALIGN_DEFAULT, 38, 492);

    lv_obj_t *label = GuiCreateLittleTitleLabel(g_noticeWindow, _("multisig_export_to_sdcard_success"));
    lv_obj_align(label, LV_ALIGN_DEFAULT, 36, 588);

    label = GuiCreateIllustrateLabel(g_noticeWindow, _("multisig_export_to_sdcard_success_desc"));
    lv_obj_align(label, LV_ALIGN_DEFAULT, 36, 640);

    lv_obj_t *btn = GuiCreateBtnWithFont(g_noticeWindow, _("Done"), g_defTextFont);
    lv_obj_set_size(btn, 122, 66);
    lv_obj_set_style_bg_color(btn, ORANGE_COLOR, LV_PART_MAIN);
    lv_obj_align(btn, LV_ALIGN_BOTTOM_RIGHT, -16, -24);
    lv_obj_add_event_cb(btn, GuiCloseHintBoxHandler, LV_EVENT_CLICKED, NULL);
}

static void GuiShowSDCardExportFailed()
{
    g_noticeWindow = GuiCreateHintBox(lv_scr_act(), 480, 356, false);
    lv_obj_t *img = GuiCreateImg(g_noticeWindow, &imgFailed);
    lv_obj_align(img, LV_ALIGN_DEFAULT, 38, 492);

    lv_obj_t *label = GuiCreateLittleTitleLabel(g_noticeWindow, _("multisig_export_to_sdcard_failed"));
    lv_obj_align(label, LV_ALIGN_DEFAULT, 36, 588);

    label = GuiCreateIllustrateLabel(g_noticeWindow, _("multisig_export_to_sdcard_failed_desc"));
    lv_obj_align(label, LV_ALIGN_DEFAULT, 36, 640);

    lv_obj_t *btn = GuiCreateBtnWithFont(g_noticeWindow, _("OK"), g_defTextFont);
    lv_obj_set_size(btn, 94, 66);
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
    SetNavBarLeftBtn(g_pageWidget->navBarWidget, NVS_BAR_RETURN, CloseCurrentViewHandler, NULL);
    SetNavBarRightBtn(g_pageWidget->navBarWidget, NVS_BAR_SDCARD, GuiSDCardHandler, NULL);
    if (g_isExportMultiWallet) {
        SetMidBtnLabel(g_pageWidget->navBarWidget, NVS_BAR_MID_LABEL, _("manage_multi_wallet_export_title"));
    } else {
        SetMidBtnLabel(g_pageWidget->navBarWidget, NVS_BAR_MID_LABEL, _("import_multi_wallet_success_title"));
    }
}

static UREncodeResult *GuiGenerateUR()
{
    uint8_t mfp[4];
    GetMasterFingerPrint(mfp);
    return export_multi_sig_wallet_by_ur(mfp, 4, g_multisigWalletItem->walletConfig);
}

static void GuiContent(lv_obj_t *parent)
{
    lv_obj_t *cont, *text, *qr_cont, *btn;

    cont = GuiCreateContainerWithParent(parent, 480, 800 - 48 - 96 - 114);
    lv_obj_add_flag(cont, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_flag(cont, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(cont, LV_SCROLLBAR_MODE_OFF);

    text = GuiCreateNoticeLabel(cont, _("multisig_import_success_hint"));
    lv_obj_align(text, LV_ALIGN_TOP_LEFT, 36, 8);
    lv_obj_set_style_text_align(text, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);

    cont = GuiCreateContainerWithParent(cont, 408, 134 + 384);
    lv_obj_set_style_bg_color(cont, DARK_BG_COLOR, LV_PART_MAIN);
    lv_obj_set_style_radius(cont, 24, LV_PART_MAIN);
    lv_obj_align(cont, LV_ALIGN_TOP_LEFT, 36, 122);

    g_qrCodeCont = GuiCreateContainerWithParent(cont, 336, 336);
    lv_obj_align(g_qrCodeCont, LV_ALIGN_TOP_LEFT, 36, 36);
    GuiAnimatingQRCodeInitWithCustomSize(g_qrCodeCont, GuiGenerateUR, true, 336, 336, NULL);

    text = GuiCreateIllustrateLabel(cont, convertFormatLabel(g_multisigWalletItem->format));
    lv_obj_align(text, LV_ALIGN_TOP_LEFT, 36, 12 + 384);

    text = GuiCreateNoticeLabel(cont, _("Sample Address:"));
    lv_obj_align(text, LV_ALIGN_TOP_LEFT, 36, 12 + 34 + 384);

    text = GuiCreateIllustrateLabel(cont, "text_placeholder");
    lv_obj_set_width(text, 360);
    lv_obj_align(text, LV_ALIGN_TOP_LEFT, 36, 12 + 34 + 384 + 30);
    lv_label_set_recolor(text, true);
    SetEgContent(text);

    btn = GuiCreateBtn(parent, _("Done"));
    lv_obj_set_size(btn, 408, 66);
    lv_obj_align(btn, LV_ALIGN_BOTTOM_MID, 0, -24);
    lv_obj_add_event_cb(btn, ImportMultisigGoToHomeViewHandler, LV_EVENT_CLICKED, NULL);
}

static char *convertFormatLabel(char *format)
{
    if (strcmp(format, FORMAT_P2WSH) == 0) {
        return "Native Segwit";
    }
    if (strcmp(format, FORMAT_P2WSH_P2SH) == 0) {
        return "Nested Segwit";
    }
    return "Legacy";
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
    CutAndFormatAddress(addrShot, sizeof(addrShot), addr, 24);
    strncpy(prefix, addrShot, prefixLen);
    strncpy(rest, addrShot + prefixLen, strnlen_s(addrShot, BUFFER_SIZE_64) - prefixLen);
    snprintf_s(eg, sizeof(eg), "#F5870A %s#%s", prefix, rest);
    lv_label_set_text(label, addrShot);
}

void GuiMultisigWalletExportWidgetsDeInit()
{
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
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        g_isExportMultiWallet = false;
        GuiAnimatingQRCodeDestroyTimer();
        GuiCloseToTargetView(&g_homeView);
    }
}

void GuiSetExportMultiSigSwitch(void)
{
    g_isExportMultiWallet = true;
}

bool GuiGetExportMultisigWalletSwitch()
{
    return g_isExportMultiWallet;
}