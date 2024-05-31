#include "gui_views.h"
#include "gui_setup_widgets.h"
#include "gui_qr_code.h"
#include "gui_model.h"
#include "gui_hintbox.h"
#include "gui_lock_widgets.h"
#include "gui_keyboard.h"
#include "gui_create_wallet_widgets.h"
#include "gui_status_bar.h"
#include "gui_lock_device_widgets.h"
#include "gui_page.h"
#include "user_memory.h"
#include <stdio.h>
#include <string.h>

#ifndef COMPILE_SIMULATOR
#include "safe_str_lib.h"
#else
#include "simulator_mock_define.h"
#endif

#define IMPORT_WALLET_NOTICE                                            false
#define CREATE_WALLET_NOTICE                                            true
#define MAX_RUST_ERR_MESSAGE_LEN                                        1024

static void CreateWalletNotice(bool isCreate);

static lv_obj_t *g_noticeWindow = NULL;
static lv_obj_t **g_hintParam = NULL;
static PageWidget_t *g_pageViewWidget = NULL;

static ErrorWindowCallback g_errorWindowCallback = NULL;

void UnHandler(lv_event_t *e)
{
}

void OpenImportWalletHandler(lv_event_t *e)
{
    if (CHECK_BATTERY_LOW_POWER()) {
        g_noticeWindow = GuiCreateErrorCodeWindow(ERR_KEYSTORE_SAVE_LOW_POWER, &g_noticeWindow, NULL);
    } else {
        CreateWalletNotice(IMPORT_WALLET_NOTICE);
    }
}

void OpenCreateWalletHandler(lv_event_t *e)
{
    if (CHECK_BATTERY_LOW_POWER()) {
        g_noticeWindow = GuiCreateErrorCodeWindow(ERR_KEYSTORE_SAVE_LOW_POWER, &g_noticeWindow, NULL);
    } else {
        CreateWalletNotice(CREATE_WALLET_NOTICE);
    }
}

void OpenViewHandler(lv_event_t *e)
{
    GuiFrameOpenView(lv_event_get_user_data(e));
}

void CloseTimerCurrentViewHandler(lv_event_t *e)
{
    CloseQRTimer();
    GuiCLoseCurrentWorkingView();
}

void GoToHomeViewHandler(lv_event_t *e)
{
    CloseQRTimer();
    GuiCloseToTargetView(&g_homeView);
}

void CloseCurrentViewHandler(lv_event_t *e)
{
    GuiCLoseCurrentWorkingView();
}

void ReadyNextTileHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_READY) {
        GuiEmitSignal(SIG_SETUP_VIEW_TILE_NEXT, NULL, 0);
    } else if (code == LV_EVENT_VALUE_CHANGED) {
        if (lv_event_get_user_data(e) != NULL) {
            KeyBoard_t *kb = *(KeyBoard_t **)lv_event_get_user_data(e);
            if (strnlen_s(lv_textarea_get_text(kb->ta), WALLET_NAME_MAX_LEN + 1) > 0) {
                lv_obj_set_style_text_font(kb->ta, &buttonFont, 0);
            } else {
                lv_obj_set_style_text_font(kb->ta, g_defTextFont, 0);
            }
        }
    }
}

void ReturnHandler(lv_event_t *e)
{
    GuiEmitSignal(SIG_SETUP_VIEW_TILE_PREV, NULL, 0);
}

void NextTileHandler(lv_event_t *e)
{
    GuiEmitSignal(SIG_SETUP_VIEW_TILE_NEXT, NULL, 0);
}

void CloseToTargetTileView(uint8_t currentIndex, uint8_t targetIndex)
{
    for (int i = currentIndex; i > targetIndex; i--) {
        GuiEmitSignal(SIG_SETUP_VIEW_TILE_PREV, NULL, 0);
    }
}

void CloseCurrentParentHandler(lv_event_t *e)
{
    lv_obj_del(lv_obj_get_parent(lv_event_get_target(e)));
}

void CloseParentAndNextHandler(lv_event_t *e)
{
    lv_obj_del(lv_obj_get_parent(lv_event_get_target(e)));
    void **param = lv_event_get_user_data(e);
    if (param != NULL) {
        *param = NULL;
    }
    GuiEmitSignal(SIG_SETUP_VIEW_TILE_NEXT, NULL, 0);
}

void CloseCurrentUserDataHandler(lv_event_t *e)
{

    GuiViewHintBoxClear();
    GuiEmitSignal(GUI_EVENT_REFRESH, NULL, 0);
}

void CloseCurrentParentAndCloseViewHandler(lv_event_t *e)
{
    static uint16_t single = SIG_LOCK_VIEW_VERIFY_PIN;
    lv_obj_del(lv_obj_get_parent(lv_event_get_target(e)));
    GuiCLoseCurrentWorkingView();
    GuiLockScreenFpRecognize();
    GuiLockScreenTurnOn(&single);
    ResetSuccess();
    GuiModelWriteLastLockDeviceTime(0);
}

void CloseWaringPageHandler(lv_event_t *e)
{
    lv_obj_del(lv_event_get_user_data(e));
    if (g_hintParam != NULL) {
        *g_hintParam = NULL;
    }
    if (g_errorWindowCallback) {
        g_errorWindowCallback();
        g_errorWindowCallback = NULL;
    }
}

void ToggleSwitchBoxHandler(lv_event_t *e)
{
    lv_obj_t *switchBox = lv_event_get_user_data(e);
    bool en = lv_obj_has_state(switchBox, LV_STATE_CHECKED);
    if (en) {
        lv_obj_clear_state(switchBox, LV_STATE_CHECKED);
    } else {
        lv_obj_add_state(switchBox, LV_STATE_CHECKED);
    }
    lv_event_send(switchBox, LV_EVENT_VALUE_CHANGED, NULL);
}

static bool g_isWriteTon = false;
void GuiWriteSESetTon(bool isTon)
{
    g_isWriteTon = isTon;
}

void GuiWriteSeWidget(lv_obj_t *parent)
{
    lv_obj_t *label = GuiCreateTextLabel(parent, _("create_wallet_generating_title"));
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 403 - GUI_MAIN_AREA_OFFSET);
    lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);

    label = GuiCreateNoticeLabel(parent, _("write_se_desc"));
    GuiAlignToPrevObj(label, LV_ALIGN_OUT_BOTTOM_MID, 0, 18);
    lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);

    if(g_isWriteTon) {
        label = GuiCreateNoticeLabel(parent, _("ton_write_se_predict_text"));
        GuiAlignToPrevObj(label, LV_ALIGN_OUT_BOTTOM_MID, 0, 18);
        lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
        lv_obj_set_style_text_color(label, ORANGE_COLOR, LV_PART_MAIN);
    }
}

void DuplicateShareHandler(lv_event_t *e)
{
    GuiCLoseCurrentWorkingView();
    GuiCLoseCurrentWorkingView();
    GuiEmitSignal(SIG_SETUP_VIEW_TILE_PREV, NULL, 0);
    GuiEmitSignal(SIG_SETUP_VIEW_TILE_PREV, NULL, 0);
    GuiViewHintBoxClear();
}

void GuiViewHintBoxClear(void)
{
    GUI_DEL_OBJ(g_noticeWindow)
    DestroyPageWidget(g_pageViewWidget);
    g_pageViewWidget = NULL;
}

void GuiSDCardExportHandler(lv_event_t *e)
{
    void (*func)(void) = lv_event_get_user_data(e);
    if (SdCardInsert()) {
        func();
    } else {
        g_noticeWindow = GuiCreateErrorCodeWindow(ERR_EXPORT_XPUB_SDCARD_NOT_DETECTED, &g_noticeWindow, NULL);
    }
    return;
}

void GuiWriteSeResult(bool en, int32_t errCode)
{
    GuiStopCircleAroundAnimation();
    if (en) {
        WalletDesc_t wallet = {
            .iconIndex = GuiGetEmojiIconIndex(),
        };
        GuiSetupKeyboardWidgetMode();
        SetStatusBarEmojiIndex(wallet.iconIndex);
        strcpy_s(wallet.name, WALLET_NAME_MAX_LEN + 1, GetCurrentKbWalletName());
        GuiNvsBarSetWalletName(GetCurrentKbWalletName());
        GuiNvsBarSetWalletIcon(GuiGetEmojiIconImg());
        GuiModelSettingSaveWalletDesc(&wallet);
        GuiCloseToTargetView(&g_initView);
        GuiFrameOpenViewWithParam(&g_lockView, NULL, 0);
        GuiLockScreenHidden();
        GuiFrameOpenView(&g_homeView);
        GuiUpdateOldAccountIndex();
    } else {
        lv_event_cb_t cb = CloseCurrentUserDataHandler;
        const char *titleText = _("error_box_invalid_seed_phrase");
        const char *descText = _("error_box_invalid_seed_phrase_desc");
        switch (errCode) {
        case ERR_KEYSTORE_MNEMONIC_REPEAT:
            titleText = _("error_box_duplicated_seed_phrase");
            descText = _("error_box_duplicated_seed_phrase_desc");
            cb = DuplicateShareHandler;
            break;
        case ERR_KEYSTORE_MNEMONIC_INVALID:
            break;
        case ERR_KEYSTORE_SAVE_LOW_POWER:
            titleText = _("error_box_low_power");
            descText = _("error_box_low_power_desc");
            break;
        }

        GuiEmitSignal(SIG_SETUP_VIEW_TILE_PREV, NULL, 0);
        g_noticeWindow = GuiCreateConfirmHintBox(&imgFailed, titleText, descText, NULL, _("OK"), WHITE_COLOR_OPA20);
        lv_obj_t *btn = GuiGetHintBoxRightBtn(g_noticeWindow);
        lv_obj_add_event_cb(btn, cb, LV_EVENT_CLICKED, NULL);
    }
}

void *GuiCreateErrorCodeWindow(int32_t errCode, lv_obj_t **param, ErrorWindowCallback cb)
{
    g_errorWindowCallback = cb;
    g_hintParam = param;
    const char *titleText = _("error_box_invalid_seed_phrase");
    const char *descText = _("error_box_invalid_seed_phrase_desc");
    const void *imgSrc = &imgFailed;
    switch (errCode) {
    case ERR_KEYSTORE_MNEMONIC_REPEAT:
        titleText = _("error_box_duplicated_seed_phrase");
        descText = _("error_box_duplicated_seed_phrase_desc");
        break;
    case ERR_KEYSTORE_MNEMONIC_INVALID:
        break;
    case ERR_KEYSTORE_SAVE_LOW_POWER:
        titleText = _("error_box_low_power");
        descText = _("error_box_low_power_desc");
        imgSrc = &imgWarn;
        break;
    case ERR_KEYSTORE_MNEMONIC_NOT_MATCH_WALLET:
        titleText = (char *)_("error_box_mnemonic_not_match_wallet");
        descText = (char *)_("error_box_mnemonic_not_match_wallet_desc");
        break;
    case ERR_UPDATE_FIRMWARE_NOT_DETECTED:
        titleText = _("error_box_firmware_not_detected");
        descText = _("error_box_firmware_not_detected_desc");
        break;
    case ERR_UPDATE_SDCARD_NOT_DETECTED:
        titleText = _("firmware_update_sd_failed_access_title");
        descText = _("firmware_update_sd_failed_access_desc");
        break;
    case ERR_UPDATE_NO_UPGRADABLE_FIRMWARE:
        titleText = _("firmware_update_no_upgradable_firmware_title");
        descText = _("firmware_update_no_upgradable_firmware_desc");
        break;
    case ERR_KEYSTORE_IMPORT_XPUB_DUPLICATE:
        titleText = _("create_multi_wallet_xpub_duplicated_title");
        descText = _("create_multi_wallet_xpub_duplicated_desc");
        break;
    case ERR_KEYSTORE_IMPORT_XPUB_INVALID:
        titleText = _("multisig_import_xpub_error_title");
        descText = _("scan_qr_code_error_invalid_file_desc");
        break;
    case ERR_MULTISIG_WALLET_CONFIG_INVALID:
        titleText = _("multisig_import_wallet_invalid");
        descText = _("multisig_import_wallet_invalid_desc");
        break;
    case ERR_MULTISIG_WALLET_EXIST:
        titleText = _("multisig_import_wallet_exist");
        descText = _("multisig_import_wallet_exist_desc");
        break;
    case ERR_INVALID_QRCODE:
        titleText = _("scan_qr_code_error_invalid_qrcode");
        descText = _("scan_qr_code_error_invalid_qrcode_desc");
        break;
    case ERR_INVALID_FILE:
        titleText = _("scan_qr_code_error_invalid_wallet_file");
        descText = _("scan_qr_code_error_invalid_wallet_file_desc");
        break;
    case ERR_EXPORT_FILE_TO_MICRO_CARD_FAILED:
        titleText = _("multisig_export_to_sdcard_failed");
        descText = _("multisig_export_to_sdcard_failed_desc");
        break;
    case ERR_MULTISIG_IMPORT_PASSPHRASE_INVALID:
        titleText = _("manage_import_wallet_passphrase_error_title");
        descText = _("manage_import_wallet_passphrase_error_desc");
        break;
    case ERR_MULTISIG_TRANSACTION_ALREADY_SIGNED:
        titleText = _("mutlisig_transaction_already_signed");
        descText = _("mutlisig_transaction_already_signed_desc");
        break;
    case ERR_EXPORT_XPUB_SDCARD_NOT_DETECTED:
        titleText = _("firmware_update_sd_failed_access_title");
        descText = _("firmware_update_sd_failed_access_desc");
        break;
    }

    lv_obj_t *cont = GuiCreateConfirmHintBox(imgSrc, titleText, descText, NULL, _("OK"), WHITE_COLOR_OPA20);
    lv_obj_add_event_cb(GuiGetHintBoxRightBtn(cont), CloseWaringPageHandler, LV_EVENT_CLICKED, cont);
    return cont;
}

void *GuiCreateRustErrorWindow(int32_t errCode, const char* errMessage, lv_obj_t **param, ErrorWindowCallback cb)
{
    g_errorWindowCallback = cb;
    g_hintParam = param;
    const char *titleText = _("Error");
    const char *descText = _("Oops, Something happened");

    switch (errCode) {
    case BitcoinNoMyInputs:
    case BitcoinWalletTypeError:
        titleText = _("rust_error_bitcoin_no_my_inputs");
        descText = _("rust_error_bitcoin_no_my_inputs_desc");
        break;
    case BitcoinMultiSigWalletNotMyWallet:
        titleText = _("rust_error_bitcoin_not_my_multisig_wallet");
        descText = _("rust_error_bitcoin_not_my_multisig_wallet_desc");
        break;
    case BitcoinMultiSigWalletParseError:
        titleText = _("scan_qr_code_error_invalid_wallet_file");
        descText = _("scan_qr_code_error_invalid_wallet_file_desc");
        break;
    }

    lv_obj_t *cont = GuiCreateConfirmHintBox(&imgFailed, titleText, descText, NULL, _("OK"), WHITE_COLOR_OPA20);
    lv_obj_add_event_cb(GuiGetHintBoxRightBtn(cont), CloseWaringPageHandler, LV_EVENT_CLICKED, cont);
    return cont;
}

static void CreateOrImportWalletHandler(lv_event_t *e)
{
    GuiFrameOpenViewWithParam(&g_createWalletView, lv_event_get_user_data(e), sizeof(uint8_t));
    DestroyPageWidget(g_pageViewWidget);
    g_pageViewWidget = NULL;
}

static void CreateWalletNotice(bool isCreate)
{
    static uint8_t walletMethod[] = {WALLET_METHOD_IMPORT, WALLET_METHOD_CREATE};
    g_pageViewWidget = CreatePageWidget();
    SetNavBarLeftBtn(g_pageViewWidget->navBarWidget, NVS_BAR_RETURN, DestroyPageWidgetHandler, g_pageViewWidget);

    lv_obj_t *img = GuiCreateImg(g_pageViewWidget->contentZone, &imgInformation);
    lv_obj_align(img, LV_ALIGN_DEFAULT, 36, 36);
    lv_obj_t *label = GuiCreateTitleLabel(g_pageViewWidget->contentZone, _("wallet_settings_add_info_title"));
    lv_obj_align(label, LV_ALIGN_DEFAULT, 36, 130);
    label = GuiCreateIllustrateLabel(g_pageViewWidget->contentZone, _("wallet_setting_add_wallet_notice"));
    lv_obj_align(label, LV_ALIGN_DEFAULT, 36, 200);

    lv_obj_t *btn = GuiCreateTextBtn(g_pageViewWidget->contentZone, _("wallet_setting_add_wallet_confirm"));
    lv_obj_set_size(btn, 408, 66);
    lv_obj_align(btn, LV_ALIGN_BOTTOM_MID, 0, -24);

    lv_obj_add_event_cb(btn, CreateOrImportWalletHandler, LV_EVENT_CLICKED, &walletMethod[isCreate]);
}

void CreateBetaNotice(void)
{
    g_noticeWindow = GuiCreateConfirmHintBox(&imgWarn, _("beta_version_notice_title"), _("beta_version_notice_desc"), NULL, _("OK"), WHITE_COLOR_OPA20);
    lv_obj_add_event_cb(GuiGetHintBoxRightBtn(g_noticeWindow), CloseHintBoxHandler, LV_EVENT_CLICKED, &g_noticeWindow);
}
