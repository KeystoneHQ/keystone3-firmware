#include "gui.h"
#include "gui_views.h"
#include "gui_status_bar.h"
#include "gui_keyboard.h"
#include "gui_button.h"
#include "gui_hintbox.h"
#include "gui_model.h"
#include "user_memory.h"
#include "presetting.h"
#include "version.h"
#include "err_code.h"
#include "firmware_update.h"
#include "gui_page.h"
#include "gui_import_multisig_wallet_info_widgets.h"
#include "gui_multisig_wallet_export_widgets.h"
#include "librust_c.h"
#include "keystore.h"
#include "fingerprint_process.h"
#include "account_public_info.h"
#include "multi_sig_wallet_manager.h"
#include "gui_chain.h"
#include "stdint.h"
#include "user_memory.h"
#include "gui_multisig_wallet_export_widgets.h"
#ifndef COMPILE_SIMULATOR
#include "drv_sdcard.h"
#include "user_fatfs.h"
#else
#include "simulator_mock_define.h"
#endif
#include <gui_keyboard_hintbox.h>

#define MAX_VERIFY_CODE_LEN 24

static PageWidget_t *g_pageWidget;
static lv_obj_t *g_noticeWindow = NULL;
static MultiSigWallet *g_wallet = NULL;
static KeyboardWidget_t *g_keyboardWidget = NULL;
static lv_obj_t *g_confirmLabel;

static void GuiConfirmHandler(lv_event_t *e);
static void GuiVerifyPassword();
static void prepareWalletByQRCode(void *);
static uint32_t prepareWalletBySDCard(char *);
static uint32_t processResult(Ptr_Response_MultiSigWallet result);
static void GuiContent(lv_obj_t *);
static void GuiCloseWarnningDialog();

static bool g_isQRCode = false;

void GuiSetMultisigImportWalletDataByQRCode(URParseResult *urResult, URParseMultiResult *multiResult, bool multi)
{
    g_isQRCode = true;
    prepareWalletByQRCode(multi ? multiResult->data : urResult->data);
    CHECK_FREE_UR_RESULT(urResult, false);
    CHECK_FREE_UR_RESULT(multiResult, true);
}

uint32_t GuiSetMultisigImportWalletDataBySDCard(char *walletConfig)
{
    g_isQRCode = false;
    return prepareWalletBySDCard(walletConfig);
}

static void CloseWaringAndCurrentPageHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        GUI_DEL_OBJ(g_noticeWindow)
        GuiCLoseCurrentWorkingView();
    }
}

static void CreateCheckTheWalletInfoNotice(lv_obj_t *parent)
{
    g_noticeWindow = GuiCreateConfirmHintBox(lv_scr_act(), &imgObserve, _("manage_import_wallet_notice_title"),
                     _("manage_import_wallet_notice_desc1"), _("manage_import_wallet_notice_desc1"), _("OK"), ORANGE_COLOR);
    lv_obj_add_event_cb(GuiGetHintBoxRightBtn(g_noticeWindow), CloseHintBoxHandler, LV_EVENT_CLICKED, &g_noticeWindow);
}

void GuiImportMultisigWalletInfoWidgetsInit(void)
{
    if ((GetCurrentAccountMultisigWalletNum() >= MAX_MULTI_SIG_WALLET_NUMBER) && (!GuiGetExportMultisigWalletSwitch())) {
        g_noticeWindow = GuiCreateConfirmHintBox(lv_scr_act(), &imgFailed, _("manage_multi_wallet_add_limit_title"),
                         _("manage_multi_wallet_add_scan_limit_title"), NULL, _("OK"), WHITE_COLOR_OPA20);
        lv_obj_add_event_cb(GuiGetHintBoxRightBtn(g_noticeWindow), CloseWaringAndCurrentPageHandler, LV_EVENT_CLICKED, NULL);
        return;
    }
    g_pageWidget = CreatePageWidget();
    if (g_wallet == NULL) {
        if (g_isQRCode) {
            if (PassphraseExist(GetCurrentAccountIndex()) == true) {
                g_noticeWindow = GuiCreateConfirmHintBox(lv_scr_act(), &imgFailed, _("manage_import_wallet_passphrase_error_title"),
                                 _("manage_import_wallet_passphrase_error_desc"), NULL, _("OK"), WHITE_COLOR_OPA20);
            } else {
                g_noticeWindow = GuiCreateConfirmHintBox(lv_scr_act(), &imgFailed, _("scan_qr_code_error_invalid_qrcode"),
                                 _("scan_qr_code_error_invalid_qrcode_desc"), NULL, _("OK"), WHITE_COLOR_OPA20);
            }
        } else {
            g_noticeWindow = GuiCreateConfirmHintBox(lv_scr_act(), &imgFailed, _("scan_qr_code_error_invalid_wallet_file"),
                             _("scan_qr_code_error_invalid_wallet_file_desc"), NULL, _("OK"), WHITE_COLOR_OPA20);
        }
        lv_obj_add_event_cb(GuiGetHintBoxRightBtn(g_noticeWindow), CloseWaringAndCurrentPageHandler, LV_EVENT_CLICKED, NULL);
        return;
    }

    if (!GuiGetExportMultisigWalletSwitch()) {
        CreateCheckTheWalletInfoNotice(g_pageWidget->contentZone);
    }

    GuiContent(g_pageWidget->contentZone);
    SetMidBtnLabel(g_pageWidget->navBarWidget, NVS_BAR_MID_LABEL, _("import_multi_wallet_info_title"));
    SetNavBarLeftBtn(g_pageWidget->navBarWidget, NVS_BAR_RETURN, CloseCurrentViewHandler, NULL);
}

void GuiImportMultisigWalletInfoWidgetsDeInit()
{
    if (g_pageWidget != NULL) {
        DestroyPageWidget(g_pageWidget);
        g_pageWidget = NULL;
    }

    if (g_wallet != NULL) {
        free_MultiSigWallet(g_wallet);
        g_wallet = NULL;
    }

    if (g_keyboardWidget != NULL) {
        GuiDeleteKeyboardWidget(g_keyboardWidget);
        g_keyboardWidget = NULL;
    }

    GUI_DEL_OBJ(g_noticeWindow);

    ClearSecretCache();
}

void GuiImportMultisigWalletInfoWidgetsRestart()
{
}

void GuiImportMultisigWalletInfoVerifyPasswordSuccess(void)
{
    uint8_t seed[64] = {0};
    int len = (GetMnemonicType() == MNEMONIC_TYPE_BIP39) ? sizeof(seed) : GetCurrentAccountEntropyLen();
    GetAccountSeed(GetCurrentAccountIndex(), seed, SecretCacheGetPassword());
    uint8_t mfp[4] = {0};
    GetMasterFingerPrint(mfp);
    Response_MultiSigWallet *response = parse_and_verify_multisig_config(seed, len, g_wallet->config_text, mfp, 4);
    if (response->error_code != 0) {
        printf("errorMessage: %s\r\n", response->error_message);
        g_noticeWindow = GuiCreateErrorCodeWindow(ERR_MULTISIG_WALLET_CONFIG_INVALID, &g_noticeWindow, GuiCloseWarnningDialog);
        free_MultiSigWallet(response->data);
        return;
    }
    MultiSigWalletItem_t *wallet = AddMultisigWalletToCurrentAccount(g_wallet, SecretCacheGetPassword());
    if (wallet == NULL) {
        printf("multi sigwallet not found\n");
        return;
    }
    GuiDeleteKeyboardWidget(g_keyboardWidget);
    char *verifyCode = SRAM_MALLOC(MAX_VERIFY_CODE_LEN);
    strcpy_s(verifyCode, MAX_VERIFY_CODE_LEN, wallet->verifyCode);
    GuiCLoseCurrentWorkingView();
    GuiFrameOpenViewWithParam(&g_multisigWalletExportView, verifyCode, strnlen_s(verifyCode, MAX_VERIFY_CODE_LEN));
}

void GuiImportMultisigPasswordErrorCount(void *param)
{
    PasswordVerifyResult_t *passwordVerifyResult = (PasswordVerifyResult_t *)param;
    GuiShowErrorNumber(g_keyboardWidget, passwordVerifyResult);
}

static void GuiCloseWarnningDialog()
{
    GuiDeleteKeyboardWidget(g_keyboardWidget);
}

static void prepareWalletByQRCode(void *wallet_info_data)
{
    uint8_t mfp[4];
    GetMasterFingerPrint(mfp);
    Ptr_Response_MultiSigWallet result = import_multi_sig_wallet_by_ur(wallet_info_data, mfp, 4);
    processResult(result);
}

static uint32_t prepareWalletBySDCard(char *walletConfig)
{
    uint8_t mfp[4];
    GetMasterFingerPrint(mfp);
    Ptr_Response_MultiSigWallet result = import_multi_sig_wallet_by_file(walletConfig, mfp, 4);
    return processResult(result);
}

static uint32_t processResult(Ptr_Response_MultiSigWallet result)
{
    if (result->error_code != 0) {
        printf("%s\r\n", result->error_message);
        return result->error_code;
    }
    g_wallet = result->data;
    return SUCCESS_CODE;
}

static void GuiContent(lv_obj_t *parent)
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
        if (g_wallet->derivations->size == 1) {
            label = GuiCreateNoticeLabel(cont, g_wallet->derivations->data[0]);
        } else {
            label = GuiCreateNoticeLabel(cont, g_wallet->derivations->data[i]);
        }
        lv_obj_align(label, LV_ALIGN_DEFAULT, 24, i * 204 + 174);
    }
    bool isExport = GuiGetExportMultisigWalletSwitch();
    lv_obj_t *btn = GuiCreateBtn(parent, isExport ? _("Export") : _("Confirm"));
    lv_obj_add_event_cb(btn, GuiConfirmHandler, LV_EVENT_CLICKED, NULL);
    lv_obj_align(btn, LV_ALIGN_BOTTOM_MID, 0, -24);
    lv_obj_set_size(btn, 408, 66);
    g_confirmLabel = lv_obj_get_child(btn, 0);
}

static void GuiConfirmHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        if (GuiGetExportMultisigWalletSwitch()) {
            char *verifyCode = SRAM_MALLOC(MAX_VERIFY_CODE_LEN);
            strcpy_s(verifyCode, MAX_VERIFY_CODE_LEN, g_wallet->verify_code);
            GuiCLoseCurrentWorkingView();
            GuiFrameOpenViewWithParam(&g_multisigWalletExportView, verifyCode, strnlen_s(verifyCode, MAX_VERIFY_CODE_LEN));
            return;
        }
        MultiSigWalletItem_t *wallet = GetMultisigWalletByVerifyCode(g_wallet->verify_code);
        if (wallet != NULL) {
            g_noticeWindow = GuiCreateErrorCodeWindow(ERR_MULTISIG_WALLET_EXIST, &g_noticeWindow, GuiCLoseCurrentWorkingView);
            return;
        }
        GuiVerifyPassword();
    }
}

static void GuiVerifyPassword()
{
    g_keyboardWidget = GuiCreateKeyboardWidget(g_pageWidget->contentZone);
    SetKeyboardWidgetSelf(g_keyboardWidget, &g_keyboardWidget);
    static uint16_t sig = SIG_MULTISIG_WALLET_IMPORT_VERIFY_PASSWORD;
    SetKeyboardWidgetSig(g_keyboardWidget, &sig);
}
