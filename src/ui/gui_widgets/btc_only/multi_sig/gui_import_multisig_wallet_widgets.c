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
#include "gui_import_multisig_wallet_widgets.h"
#ifndef COMPILE_SIMULATOR
#include "safe_str_lib.h"
#else
#include "simulator_mock_define.h"
#endif

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
static lv_obj_t *g_noticeWindow;
static lv_obj_t *g_confirmLabel;
static lv_obj_t *g_qrCodeCont = NULL;
static PageWidget_t *g_pageWidget;
static char *g_walletConfig = NULL;
static MultiSigWallet *g_wallet = NULL;
static bool g_isQRCode = false;
static bool g_isExportMultiWallet = false;
static KeyboardWidget_t *g_keyboardWidget = NULL;

void CutAndFormatAddress(char *out, uint32_t maxLen, const char *address, uint32_t targetLen);
static void GuiConfirmHandler(lv_event_t *e);
static void GuiOnFailedHandler(lv_event_t *event);
static void GuiShowInvalidQRCode();
static void GuiShowWalletExisted();
static void GuiImportWalletSuccessContent(lv_obj_t *parent);
static void SetEgContent(lv_obj_t *label);
static char* convertFormatLabel(char *format);
static void GuiSDCardHandler(lv_event_t *);
static void GuiShowSDCardNotDetected();
static void GuiCloseHintBoxHandler(lv_event_t *);
static void GuiShowSDCardExport();
static void GuiShowSDCardExportSuccess();
static void GuiShowSDCardExportFailed();
void GuiSetMultisigImportWalletDataBySDCard(char *walletConfig);
static void GuiMultiShowWalletInfoWidget(lv_obj_t *parent);
static void GuiMultiCreateSuccessWidget(lv_obj_t *parent);
static void ImportMultisigGoToHomeViewHandler(lv_event_t *e);

static void prepareWalletByQRCode(void *);
static void prepareWalletBySDCard(char *);

static void GuiOnFailedHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        GUI_DEL_OBJ(g_noticeWindow);
        GuiCLoseCurrentWorkingView();
    }
}

static void QRCodePause(bool pause)
{
    GuiAnimatingQRCodeControl(pause);
}

static void GuiSDCardHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        //TODO: check SD Card exist;
        if (true) {
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
        if (false) {
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

    char* filename = "xxxx.txt";
    label = GuiCreateIllustrateLabel(g_noticeWindow, filename);
    lv_obj_align(label, LV_ALIGN_DEFAULT, 36, 670);

    lv_obj_t *btn = GuiCreateBtnWithFont(g_noticeWindow, _("got_it"), g_defTextFont);
    lv_obj_set_size(btn, 122, 66);
    lv_obj_set_style_bg_color(btn, ORANGE_COLOR, LV_PART_MAIN);
    lv_obj_align(btn, LV_ALIGN_BOTTOM_RIGHT, -16, -24);
    lv_obj_add_event_cb(btn, GuiWriteSDCardHandler, LV_EVENT_CLICKED, NULL);
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

static void GuiImportWalletSuccessNVSBarInit()
{
    SetNavBarLeftBtn(g_pageWidget->navBarWidget, NVS_BAR_RETURN, CloseCurrentViewHandler, NULL);
    SetMidBtnLabel(g_pageWidget->navBarWidget, NVS_BAR_MID_LABEL, _("Import Success"));
    SetNavBarRightBtn(g_pageWidget->navBarWidget, NVS_BAR_SDCARD, GuiSDCardHandler, NULL);
}

void GuiImportMultisigWalletWidgetsInit(char *walletConfig, uint16_t len)
{
    GuiSetMultisigImportWalletDataBySDCard(walletConfig);
    if (g_wallet == NULL) {
        // TODO: Throw error;
        GuiCLoseCurrentWorkingView();
        return;
    }
    g_walletConfig = SRAM_MALLOC(len + 1);
    strcpy_s(g_walletConfig, len + 1, walletConfig);

    g_pageWidget = CreatePageWidget();
    lv_obj_t *tileView = GuiCreateTileView(g_pageWidget->contentZone);
    lv_obj_t *tile = lv_tileview_add_tile(tileView, IMPORT_MULTI_SHOW_WALLET_INFO, 0, LV_DIR_HOR);
    GuiMultiShowWalletInfoWidget(tile);

    tile = lv_tileview_add_tile(tileView, IMPORT_MULTI_WALLET_SUCCESS, 0, LV_DIR_HOR);
    GuiImportWalletSuccessContent(tile);

    g_importMultiWallet.currentTile = IMPORT_MULTI_SHOW_WALLET_INFO;
    g_importMultiWallet.tileView = tileView;

    lv_obj_set_tile_id(g_importMultiWallet.tileView, g_importMultiWallet.currentTile, 0, LV_ANIM_OFF);
}

static UREncodeResult *GuiGenerateUR()
{
    uint8_t mfp[4];
    GetMasterFingerPrint(mfp);
    return export_multi_sig_wallet_by_ur(mfp, 4, g_walletConfig,  MainNet);
}

static void GuiMultiShowWalletInfoWidget(lv_obj_t *parent)
{
    lv_obj_t *bgCont = GuiCreateContainerWithParent(parent, 480, 542);
    GuiAddObjFlag(bgCont, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE);

    lv_obj_t *cont = GuiCreateContainerWithParent(bgCont, 408, 100);
    lv_obj_set_style_radius(cont, 24, LV_PART_MAIN);
    lv_obj_set_style_bg_color(cont, WHITE_COLOR_OPA12, LV_PART_MAIN);
    lv_obj_align(cont, LV_ALIGN_DEFAULT, 36, 0);

    lv_obj_t *label = GuiCreateNoticeLabel(cont, _("single_backup_namewallet_previnput"));
    lv_obj_align(label, LV_ALIGN_DEFAULT, 24, 16);
    label = GuiCreateIllustrateLabel(cont, g_wallet->name);
    lv_obj_align(label, LV_ALIGN_DEFAULT, 24, 54);

    cont = GuiCreateContainerWithParent(bgCont, 408, 62);
    lv_obj_set_style_radius(cont, 24, LV_PART_MAIN);
    lv_obj_set_style_bg_color(cont, WHITE_COLOR_OPA12, LV_PART_MAIN);
    lv_obj_align(cont, LV_ALIGN_DEFAULT, 36, 116);

    label = GuiCreateNoticeLabel(cont, _("Policy"));
    lv_obj_align(label, LV_ALIGN_LEFT_MID, 24, 0);
    char buff[8] = {0};
    label = GuiCreateIllustrateLabel(cont, g_wallet->policy);
    GuiAlignToPrevObj(label, LV_ALIGN_OUT_RIGHT_MID, 16, 0);

    cont = GuiCreateContainerWithParent(bgCont, 408, 62);
    lv_obj_set_style_radius(cont, 24, LV_PART_MAIN);
    lv_obj_set_style_bg_color(cont, WHITE_COLOR_OPA12, LV_PART_MAIN);
    lv_obj_align(cont, LV_ALIGN_DEFAULT, 36, 194);

    label = GuiCreateNoticeLabel(cont, _("sdcard_format_confirm"));
    lv_obj_align(label, LV_ALIGN_LEFT_MID, 24, 0);
    label = GuiCreateIllustrateLabel(cont, g_wallet->format);
    GuiAlignToPrevObj(label, LV_ALIGN_OUT_RIGHT_MID, 16, 0);

    cont = GuiCreateContainerWithParent(bgCont, 408, g_wallet->total * 220 - 16);
    lv_obj_set_style_radius(cont, 24, LV_PART_MAIN);
    lv_obj_set_style_bg_color(cont, WHITE_COLOR_OPA12, LV_PART_MAIN);
    lv_obj_align(cont, LV_ALIGN_DEFAULT, 36, 272);
    for (int i = 0; i < g_wallet->total; i++) {
        char buff[8] = {0};
        snprintf(buff, sizeof(buff), "%d/%d", i + 1, g_wallet->total);
        lv_obj_t *label = GuiCreateIllustrateLabel(cont, buff);
        lv_obj_align(label, LV_ALIGN_DEFAULT, 24, i * 204 + 16);
        lv_obj_set_style_text_color(label, ORANGE_COLOR, LV_PART_MAIN);

        label = GuiCreateNoticeLabel(cont, g_wallet->xpub_items->data[i].xfp);
        GuiAlignToPrevObj(label, LV_ALIGN_OUT_RIGHT_MID, 16, 0);
        label = GuiCreateIllustrateLabel(cont, g_wallet->xpub_items->data[i].xpub);
        lv_obj_align(label, LV_ALIGN_DEFAULT, 24, i * 204 + 50);
        lv_obj_set_width(label, 360);
        label = GuiCreateNoticeLabel(cont, g_wallet->derivations->data[i]);
        lv_obj_align(label, LV_ALIGN_DEFAULT, 24, i * 204 + 174);
    }

    lv_obj_t *btn = GuiCreateBtn(parent, _("Confirm"));
    lv_obj_add_event_cb(btn, GuiConfirmHandler, LV_EVENT_CLICKED, NULL);
    lv_obj_align(btn, LV_ALIGN_BOTTOM_MID, 0, -24);
    lv_obj_set_size(btn, 408, 66);
    g_confirmLabel = lv_obj_get_child(btn, 0);
}

static void GuiImportWalletSuccessContent(lv_obj_t *parent)
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

    text = GuiCreateIllustrateLabel(cont, convertFormatLabel(g_wallet->format));
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

static char* convertFormatLabel(char *format)
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
    int8_t prefixLen = (strcmp(g_wallet->format, FORMAT_P2WSH) == 0) ? 4 : 1;
    memset_s(addrShot, sizeof(addrShot), 0, sizeof(addrShot));
    ModelGenerateMultiSigAddress(addr, sizeof(addr), g_walletConfig, 0);
    CutAndFormatAddress(addrShot, sizeof(addrShot), addr, 24);
    strncpy(prefix, addrShot, prefixLen);
    strncpy(rest, addrShot + prefixLen, strnlen_s(addrShot, BUFFER_SIZE_64) - prefixLen);
    snprintf_s(eg, sizeof(eg), "#F5870A %s#%s", prefix, rest);
    lv_label_set_text(label, addrShot);
}

void GuiImportMultisigWalletWidgetsDeInit()
{
    if (g_walletConfig != NULL) {
        SRAM_FREE(g_walletConfig);
        g_walletConfig = NULL;
    }

    if (g_pageWidget != NULL) {
        DestroyPageWidget(g_pageWidget);
        g_pageWidget = NULL;
    }

    if (g_wallet != NULL) {
        free_MultiSigWallet(g_wallet);
        g_wallet = NULL;
    }
}

void GuiImportMultisigWalletWidgetsRefresh()
{
    SetNavBarMidBtn(g_pageWidget->navBarWidget, NVS_MID_BUTTON_BUTT, NULL, NULL);
    if (g_importMultiWallet.currentTile == IMPORT_MULTI_SHOW_WALLET_INFO) {
        SetNavBarLeftBtn(g_pageWidget->navBarWidget, NVS_BAR_RETURN, CloseCurrentViewHandler, NULL);
        SetMidBtnLabel(g_pageWidget->navBarWidget, NVS_BAR_MID_LABEL, _("import_multi_wallet_info_title"));
    } else if (g_importMultiWallet.currentTile == IMPORT_MULTI_WALLET_SUCCESS) {
        SetNavBarLeftBtn(g_pageWidget->navBarWidget, NVS_BAR_RETURN, ReturnHandler, NULL);
        if (g_isExportMultiWallet) {
            SetMidBtnLabel(g_pageWidget->navBarWidget, NVS_BAR_MID_LABEL, _("manage_multi_wallet_export_title"));
        } else {
            SetMidBtnLabel(g_pageWidget->navBarWidget, NVS_BAR_MID_LABEL, _("import_multi_wallet_success_title"));
        }
    }
}

int8_t GuiImportMultiNextTile(void)
{
    switch (g_importMultiWallet.currentTile) {
    case IMPORT_MULTI_SHOW_WALLET_INFO:
        GuiDeleteKeyboardWidget(g_keyboardWidget);
        GuiAnimatingQRCodeInitWithCustomSize(g_qrCodeCont, GuiGenerateUR, false, 336, 336, NULL);
        break;
    case IMPORT_MULTI_WALLET_SUCCESS:
        break;
    }
    g_importMultiWallet.currentTile++;
    GuiImportMultisigWalletWidgetsRefresh();
    lv_obj_set_tile_id(g_importMultiWallet.tileView, g_importMultiWallet.currentTile, 0, LV_ANIM_OFF);
    return SUCCESS_CODE;
}

int8_t GuiImportMultiPrevTile(void)
{
    switch (g_importMultiWallet.currentTile) {
    case IMPORT_MULTI_SHOW_WALLET_INFO:
        break;
    case IMPORT_MULTI_WALLET_SUCCESS:
        break;
    }
    g_importMultiWallet.currentTile--;
    GuiImportMultisigWalletWidgetsRefresh();
    lv_obj_set_tile_id(g_importMultiWallet.tileView, g_importMultiWallet.currentTile, 0, LV_ANIM_OFF);
}

void ModelGenerateMultiSigAddress(char *address, uint32_t maxLen, char *walletConfig, uint32_t index)
{
    printf("walletconfig = \n%s\n", walletConfig);
    uint8_t mfp[4];
    GetMasterFingerPrint(mfp);
    SimpleResponse_c_char *result = generate_address_for_multisig_wallet_config(walletConfig, 0, index, mfp, 4, MainNet);
    if (result->error_code != 0) {
        printf("errorMessage: %s\r\n", result->error_message);
        GuiCLoseCurrentWorkingView();
        return;
    }
    strncpy_s(address, maxLen, result->data, strnlen_s(result->data, maxLen));
    address[strnlen_s(result->data, maxLen)] = '\0';
}

static void processResult(Ptr_Response_MultiSigWallet result)
{
    if (result->error_code != 0) {
        printf("%s\r\n", result->error_message);
        return;
    } else {
        g_wallet = result->data;
    }
}

static void prepareWalletByQRCode(void *wallet_info_data)
{
    uint8_t mfp[4];
    GetMasterFingerPrint(mfp);
    Ptr_Response_MultiSigWallet result = import_multi_sig_wallet_by_ur(wallet_info_data, mfp, 4, MainNet);
    processResult(result);
}

static void prepareWalletBySDCard(char *walletConfig)
{
    uint8_t mfp[4];
    GetMasterFingerPrint(mfp);
    Ptr_Response_MultiSigWallet result = import_multi_sig_wallet_by_file(walletConfig, mfp, 4, MainNet);
    processResult(result);
}

void GuiSetMultisigImportWalletDataByQRCode(URParseResult *urResult, URParseMultiResult *multiResult, bool multi)
{
    g_isQRCode = true;
    prepareWalletByQRCode(multi ? multiResult->data : urResult->data);
    CHECK_FREE_UR_RESULT(urResult, false);
    CHECK_FREE_UR_RESULT(multiResult, true);
}

void GuiSetMultisigImportWalletDataBySDCard(char *walletConfig)
{
    g_isQRCode = false;
    prepareWalletBySDCard(walletConfig);
}

static void GuiShowInvalidQRCode()
{
    g_noticeWindow = GuiCreateHintBox(lv_scr_act(), 480, 356, false);
    lv_obj_t *img = GuiCreateImg(g_noticeWindow, &imgFailed);
    lv_obj_align(img, LV_ALIGN_DEFAULT, 38, 492);

    lv_obj_t *label = GuiCreateLittleTitleLabel(g_noticeWindow, _("scan_qr_code_error_invalid_qrcode"));
    lv_obj_align(label, LV_ALIGN_DEFAULT, 36, 588);

    label = GuiCreateIllustrateLabel(g_noticeWindow, _("scan_qr_code_error_invalid_qrcode_desc"));
    lv_obj_align(label, LV_ALIGN_DEFAULT, 36, 640);

    lv_obj_t *btn = GuiCreateBtnWithFont(g_noticeWindow, _("OK"), g_defTextFont);
    lv_obj_set_size(btn, 94, 66);
    lv_obj_set_style_bg_color(btn, WHITE_COLOR, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(btn, LV_OPA_20, LV_PART_MAIN);
    lv_obj_align(btn, LV_ALIGN_BOTTOM_RIGHT, -16, -24);
    lv_obj_add_event_cb(btn, GuiOnFailedHandler, LV_EVENT_CLICKED, NULL);
}

static void GuiShowInvalidWalletFile()
{
    g_noticeWindow = GuiCreateHintBox(lv_scr_act(), 480, 356, false);
    lv_obj_t *img = GuiCreateImg(g_noticeWindow, &imgFailed);
    lv_obj_align(img, LV_ALIGN_DEFAULT, 38, 492);

    lv_obj_t *label = GuiCreateLittleTitleLabel(g_noticeWindow, _("scan_qr_code_error_invalid_wallet_file"));
    lv_obj_align(label, LV_ALIGN_DEFAULT, 36, 588);

    label = GuiCreateIllustrateLabel(g_noticeWindow, _("scan_qr_code_error_invalid_wallet_file_desc"));
    lv_obj_align(label, LV_ALIGN_DEFAULT, 36, 640);

    lv_obj_t *btn = GuiCreateBtnWithFont(g_noticeWindow, _("OK"), g_defTextFont);
    lv_obj_set_size(btn, 94, 66);
    lv_obj_set_style_bg_color(btn, WHITE_COLOR, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(btn, LV_OPA_20, LV_PART_MAIN);
    lv_obj_align(btn, LV_ALIGN_BOTTOM_RIGHT, -16, -24);
    lv_obj_add_event_cb(btn, GuiOnFailedHandler, LV_EVENT_CLICKED, NULL);
}

static void GuiShowWalletExisted()
{
#define PLUS_PADDING +384
    g_noticeWindow = GuiCreateHintBox(lv_scr_act(), 480, 416, false);
    lv_obj_t *img = GuiCreateImg(g_noticeWindow, &imgFailed);
    lv_obj_align(img, LV_ALIGN_TOP_LEFT, 36, 48 PLUS_PADDING);

    lv_obj_t *label = GuiCreateLittleTitleLabel(g_noticeWindow, _("multisig_import_wallet_exist"));
    lv_obj_align(label, LV_ALIGN_DEFAULT, 36, 144 PLUS_PADDING);

    label = GuiCreateIllustrateLabel(g_noticeWindow, _("multisig_import_wallet_exist_desc"));
    lv_obj_align(label, LV_ALIGN_DEFAULT, 36, 196 PLUS_PADDING);

    lv_obj_t *btn = GuiCreateBtnWithFont(g_noticeWindow, _("OK"), g_defTextFont);
    lv_obj_set_size(btn, 94, 66);
    lv_obj_set_style_bg_color(btn, WHITE_COLOR, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(btn, LV_OPA_20, LV_PART_MAIN);
    lv_obj_align(btn, LV_ALIGN_BOTTOM_RIGHT, -16, -24);
    lv_obj_add_event_cb(btn, GuiOnFailedHandler, LV_EVENT_CLICKED, NULL);
}

static void SignByPasswordCb(bool cancel)
{
    g_keyboardWidget = GuiCreateKeyboardWidget(g_pageWidget->contentZone);
    SetKeyboardWidgetSelf(g_keyboardWidget, &g_keyboardWidget);
    static uint16_t sig = SIG_MULTISIG_WALLET_IMPORT_VERIFY_PASSWORD;
    SetKeyboardWidgetSig(g_keyboardWidget, &sig);
}

static void GuiConfirmHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        if (g_isExportMultiWallet) {
            return GuiImportMultiNextTile();
        }
        MultiSigWalletItem_t *wallet = GetMultisigWalletByVerifyCode(g_wallet->verify_code);
        if (wallet != NULL) {
            GuiShowWalletExisted();
            return;
        }
        SignByPasswordCb(false);
    }
}

void GuiImportMultisigWalletInfoVerifyPasswordSuccess(void)
{
    MultiSigWalletItem_t *wallet = AddMultisigWalletToCurrentAccount(g_wallet, SecretCacheGetPassword());
    if (wallet == NULL) {
        printf("multi sigwallet not found\n");
        return;
    }
    GuiDeleteKeyboardWidget(g_keyboardWidget);
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
    lv_label_set_text(g_confirmLabel, _("Export"));
}