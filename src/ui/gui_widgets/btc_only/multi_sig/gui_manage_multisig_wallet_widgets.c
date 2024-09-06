#include <stdio.h>
#include "gui.h"
#include "gui_views.h"
#include "gui_status_bar.h"
#include "gui_keyboard.h"
#include "gui_button.h"
#include "gui_hintbox.h"
#include "gui_model.h"
#include "gui_create_wallet_widgets.h"
#include "user_memory.h"
#include "secret_cache.h"
#include "user_utils.h"
#include "gui_page.h"
#include "gui_keyboard_hintbox.h"
#include "account_public_info.h"
#include "gui_fullscreen_mode.h"
#include "keystore.h"
#include "multi_sig_wallet_manager.h"
#include "account_public_info.h"
#include "gui_export_pubkey_widgets.h"
#include "gui_btc_home_widgets.h"
#include "gui_manage_multisig_wallet_widgets.h"
#include "gui_import_multisig_wallet_info_widgets.h"
#include "gui_multisig_wallet_export_widgets.h"
#include "gui_api.h"
#include "account_public_info.h"
#ifndef COMPILE_SIMULATOR
#include "user_fatfs.h"
#endif

typedef enum {
    MULTI_MULTI_SIG_DETAIL = 0,
    MULTI_MULTI_SIG_CO_SIGNERS_DETAIL,

    MULTI_MULTI_SIG_BUTTON,
} MULTI_MULTI_SIG_ENUM;

typedef struct {
    uint8_t currentTile;
    lv_obj_t *tileView;
    lv_obj_t *homeTile;
    lv_obj_t *showCoSingersTile;
} ManageMultisigWidget_t;

typedef struct {
    lv_obj_t *deleteNotice;
    lv_obj_t *deleteBtn;
    lv_obj_t *setDefaultBtn;
} MultisigWidgetItem_t;

static ManageMultisigWidget_t g_manageMultisig;
static MultisigWidgetItem_t g_multisigItem;
static lv_obj_t *g_noticeWindow = NULL;
static PageWidget_t *g_pageWidget;
static MultiSigWalletItem_t *g_walletItem;
static MultiSigWallet *g_multiSigWallet = NULL;
static KeyboardWidget_t *g_keyboardWidget = NULL;
static CURRENT_WALLET_INDEX_ENUM g_currentIndex;

static void CreateMultiSigWalletDetailWidget(lv_obj_t *parent);
static void CreateCoSignerDetailWidget(lv_obj_t *parent);
static void ReloadAndUpdateMultisigItem(CURRENT_WALLET_INDEX_ENUM index);
static void DeleteMultiWalletHandler(lv_event_t *e);
static void SetDefaultMultiWalletHandler(lv_event_t *e);
static void GuiConfirmDeleteHandler(lv_event_t *e);
static void ExportMultiWalletHandler(lv_event_t *e);
static void UpdateCurrentWalletState(void);
void GuiResetCurrentUtxoAddressIndex(uint8_t index);

void GuiManageMultisigWalletInit(CURRENT_WALLET_INDEX_ENUM index)
{
    g_currentIndex = index;
    ReloadAndUpdateMultisigItem(index);
    g_pageWidget = CreatePageWidget();
    lv_obj_t *tileView = GuiCreateTileView(g_pageWidget->contentZone);
    lv_obj_t *tile = lv_tileview_add_tile(tileView, MULTI_MULTI_SIG_DETAIL, 0, LV_DIR_HOR);
    CreateMultiSigWalletDetailWidget(tile);

    tile = lv_tileview_add_tile(tileView, MULTI_MULTI_SIG_CO_SIGNERS_DETAIL, 0, LV_DIR_HOR);
    g_manageMultisig.showCoSingersTile = tile;

    g_manageMultisig.currentTile = MULTI_MULTI_SIG_DETAIL;
    g_manageMultisig.tileView = tileView;

    lv_obj_set_tile_id(g_manageMultisig.tileView, 0, 0, LV_ANIM_OFF);
}

int8_t GuiManageMultisigWalletNextTile(uint8_t index)
{
    switch (g_manageMultisig.currentTile) {
    case MULTI_MULTI_SIG_DETAIL:
        CreateCoSignerDetailWidget(g_manageMultisig.showCoSingersTile);
        break;
    case MULTI_MULTI_SIG_CO_SIGNERS_DETAIL:
        break;
    }
    g_manageMultisig.currentTile++;
    GuiManageMultisigWalletRefresh();
    lv_obj_set_tile_id(g_manageMultisig.tileView, g_manageMultisig.currentTile, 0, LV_ANIM_OFF);
    return SUCCESS_CODE;
}

int8_t GuiManageMultiWalletPrevTile(void)
{
    switch (g_manageMultisig.currentTile) {
    case MULTI_MULTI_SIG_DETAIL:
        return GuiCLoseCurrentWorkingView();
    }
    g_manageMultisig.currentTile--;
    GuiManageMultisigWalletRefresh();
    lv_obj_set_tile_id(g_manageMultisig.tileView, g_manageMultisig.currentTile, 0, LV_ANIM_OFF);
    return SUCCESS_CODE;
}

void GuiManageMultisigWalletDeInit(void)
{
    GUI_DEL_OBJ(g_noticeWindow)
    if (g_pageWidget != NULL) {
        DestroyPageWidget(g_pageWidget);
        g_pageWidget = NULL;
    }

    if (g_multiSigWallet != NULL) {
        free_MultiSigWallet(g_multiSigWallet);
        g_multiSigWallet = NULL;
    }

    if (g_keyboardWidget != NULL) {
        GuiDeleteKeyboardWidget(g_keyboardWidget);
        g_keyboardWidget = NULL;
    }
}

static void GuiMoreHandler(lv_event_t *e)
{
    // GuiFrameOpenView(&g_multisigSelectImportMethodView);
}

void GuiManageMultisigWalletRefresh(void)
{
    char tempBuff[BUFFER_SIZE_32] = {0};
    SetNavBarLeftBtn(g_pageWidget->navBarWidget, NVS_BAR_RETURN, ReturnHandler, NULL);
    SetNavBarRightBtn(g_pageWidget->navBarWidget, NVS_RIGHT_BUTTON_BUTT, NULL, NULL);
    if (g_manageMultisig.currentTile == MULTI_MULTI_SIG_DETAIL) {
        SetMidBtnLabel(g_pageWidget->navBarWidget, NVS_BAR_MID_LABEL, _("manage_multi_wallet_detail_title"));
    } else if (g_manageMultisig.currentTile == MULTI_MULTI_SIG_CO_SIGNERS_DETAIL) {
        snprintf_s(tempBuff, sizeof(tempBuff), "%s %s", g_multiSigWallet->policy, _("create_multi_wallet_co_sign_text"));
        SetMidBtnLabel(g_pageWidget->navBarWidget, NVS_BAR_MID_LABEL, tempBuff);
    }
}

static void CreateMultiSigWalletDetailWidget(lv_obj_t *parent)
{
    char tempBuffer[BUFFER_SIZE_32];
    lv_obj_t *button = GuiCreateSettingItemButton(parent, 456, g_multiSigWallet->name, NULL,
                       &imgWallet2, NULL, UnHandler, NULL);
    lv_obj_align(button, LV_ALIGN_DEFAULT, 12, 0);

    snprintf_s(tempBuffer, sizeof(tempBuffer), "%s %s", g_multiSigWallet->policy, _("create_multi_wallet_co_sign"));
    button = GuiCreateSettingItemButton(parent, 456, tempBuffer, NULL,
                                        &imgTwoKey, &imgArrowRight, NextTileHandler, NULL);
    lv_obj_align(button, LV_ALIGN_DEFAULT, 12, 96);

    button = GuiCreateSettingItemButton(parent, 456, _("manage_multi_wallet_set_default"), NULL,
                                        &imgDefaultWallet, NULL, SetDefaultMultiWalletHandler, NULL);
    lv_obj_align(button, LV_ALIGN_DEFAULT, 12, 96 * 2);
    g_multisigItem.setDefaultBtn = button;

    button = GuiCreateSettingItemButton(parent, 456, _("manage_multi_wallet_export_config"), NULL,
                                        &imgWalletExport, NULL, ExportMultiWalletHandler, NULL);
    lv_obj_align(button, LV_ALIGN_DEFAULT, 12, 96 * 3);

    lv_obj_t *line = GuiCreateDividerLine(parent);
    lv_obj_align(line, LV_ALIGN_DEFAULT, 0, 384);

    button = GuiCreateSettingItemButton(parent, 456, _("wallet_settings_delete_button"), NULL,
                                        &imgDel, NULL, DeleteMultiWalletHandler, NULL);
    g_multisigItem.deleteBtn = button;
    lv_obj_align(button, LV_ALIGN_DEFAULT, 12, 96 * 4);

    lv_obj_t *label = GuiCreateNoticeLabel(parent, _("manage_multi_wallet_delete_current_wallet_desc"));
    lv_obj_align(label, LV_ALIGN_DEFAULT, 36, 481);
    g_multisigItem.deleteNotice = label;
    UpdateCurrentWalletState();
}

static void ReloadAndUpdateMultisigItem(CURRENT_WALLET_INDEX_ENUM index)
{
    if (g_multiSigWallet != NULL) {
        free_MultiSigWallet(g_multiSigWallet);
        g_multiSigWallet = NULL;
    }

    uint8_t mfp[4];
    GetMasterFingerPrint(mfp);
    g_walletItem = GetCurrenMultisigWalletByIndex(index);
    Ptr_Response_MultiSigWallet result = import_multi_sig_wallet_by_file(g_walletItem->walletConfig, mfp, 4);
    if (result->error_code != 0) {
        return;
    }
    g_multiSigWallet = result->data;
}

static void CreateCoSignerDetailWidget(lv_obj_t *parent)
{
    lv_obj_t *cont = GuiCreateContainerWithParent(parent, 408, g_multiSigWallet->total * 220 - 16);
    lv_obj_set_style_radius(cont, 24, LV_PART_MAIN);
    lv_obj_set_style_bg_color(cont, WHITE_COLOR_OPA12, LV_PART_MAIN);
    lv_obj_align(cont, LV_ALIGN_DEFAULT, 36, 0);

    for (int i = 0; i < g_multiSigWallet->total; i++) {
        char buff[8] = {0};
        snprintf(buff, sizeof(buff), "%d/%d", i + 1, g_multiSigWallet->total);
        lv_obj_t *label = GuiCreateIllustrateLabel(cont, buff);
        lv_obj_align(label, LV_ALIGN_DEFAULT, 24, i * 204 + 16);
        lv_obj_set_style_text_color(label, ORANGE_COLOR, LV_PART_MAIN);

        label = GuiCreateNoticeLabel(cont, g_multiSigWallet->xpub_items->data[i].xfp);
        GuiAlignToPrevObj(label, LV_ALIGN_OUT_RIGHT_MID, 16, 0);
        label = GuiCreateIllustrateLabel(cont, g_multiSigWallet->xpub_items->data[i].xpub);
        lv_obj_align(label, LV_ALIGN_DEFAULT, 24, i * 204 + 50);
        lv_obj_set_width(label, 360);
        if (g_multiSigWallet->derivations->size == 1) {
            label = GuiCreateNoticeLabel(cont, g_multiSigWallet->derivations->data[0]);
        } else {
            label = GuiCreateNoticeLabel(cont, g_multiSigWallet->derivations->data[i]);
        }
        lv_obj_align(label, LV_ALIGN_DEFAULT, 24, i * 204 + 174);
    }
}

void GuiManageMultisigPasswordErrorCount(void *param)
{
    PasswordVerifyResult_t *passwordVerifyResult = (PasswordVerifyResult_t *)param;
    GuiShowErrorNumber(g_keyboardWidget, passwordVerifyResult);
}

static void DeleteMultiWalletHandler(lv_event_t *e)
{
    char tempBuff[BUFFER_SIZE_32] = {0};
    snprintf_s(tempBuff, sizeof(tempBuff), "%s %s?", _("Delete"), g_walletItem->name);
    g_noticeWindow = GuiCreateGeneralHintBox(&imgWarn, tempBuff, _("manage_multi_wallet_delete_desc"), NULL,
                     _("Cancel"), WHITE_COLOR_OPA20, _("Delete"), DEEP_ORANGE_COLOR);
    lv_obj_t *leftBtn = GuiGetHintBoxLeftBtn(g_noticeWindow);
    lv_obj_add_event_cb(leftBtn, CloseHintBoxHandler, LV_EVENT_CLICKED, &g_noticeWindow);
    lv_obj_t *rightBtn = GuiGetHintBoxRightBtn(g_noticeWindow);
    lv_obj_add_event_cb(rightBtn, GuiConfirmDeleteHandler, LV_EVENT_CLICKED, NULL);
}

static void SetDefaultMultiWalletHandler(lv_event_t *e)
{
    SetCurrentWalletIndex(g_currentIndex);
    UpdateCurrentWalletState();
    GuiResetCurrentUtxoAddressIndex(GetCurrentAccountIndex());
}

static void ExportMultiWalletHandler(lv_event_t *e)
{
    if (SUCCESS_CODE == GuiSetMultisigImportWalletDataBySDCard(g_walletItem->walletConfig)) {
        GuiSetExportMultiSigSwitch();
        GuiFrameOpenView(&g_importMultisigWalletInfoView);
        GuiEmitSignal(SIG_MULTISIG_WALLET_SET_WALLET_EXPORT, NULL, 0);
    }
}

static void GuiConfirmDeleteHandler(lv_event_t *e)
{
    GUI_DEL_OBJ(g_noticeWindow)
    static uint16_t sig = SIG_MULTISIG_WALLET_IMPORT_VERIFY_PASSWORD;
    g_keyboardWidget = GuiCreateKeyboardWidget(g_pageWidget->contentZone);
    SetKeyboardWidgetSelf(g_keyboardWidget, &g_keyboardWidget);
    SetKeyboardWidgetSig(g_keyboardWidget, &sig);
}

static void CorrectDefalutWalletIndex(int deleteIndex)
{
    if (deleteIndex != -1) {
        if (GetCurrentWalletIndex() == deleteIndex) {
            SetCurrentWalletIndex(SINGLE_WALLET);
        } else if (GetCurrentWalletIndex() != SINGLE_WALLET) {
            if (GetCurrentWalletIndex() > deleteIndex) {
                SetCurrentWalletIndex(GetCurrentWalletIndex() - 1);
            }
        }
    }
}

void DeleteMultisigWallet(void)
{
    DeleteAccountMultiReceiveIndex("BTC", 0, g_walletItem->verifyCode);
    int index = DeleteMultisigWalletByVerifyCode(g_walletItem->verifyCode, SecretCacheGetPassword());
    CorrectDefalutWalletIndex(index);
    GuiDeleteKeyboardWidget(g_keyboardWidget);
    ClearSecretCache();
    GuiManageMultiWalletPrevTile();
}

static void UpdateCurrentWalletState(void)
{
    lv_obj_t *label = lv_obj_get_child(g_multisigItem.setDefaultBtn, 1);
    GuiEmitSignal(SIG_STATUS_BAR_TEST_NET, NULL, 0);
    if (GetCurrentWalletIndex() == g_currentIndex) {
        lv_obj_clear_flag(g_multisigItem.deleteNotice, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(g_multisigItem.deleteBtn, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_clear_flag(g_multisigItem.setDefaultBtn, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_set_style_text_opa(label, LV_OPA_80, LV_PART_MAIN);
        lv_label_set_text(label, _("wallet_profile_current_default_desc"));
        label = lv_obj_get_child(g_multisigItem.deleteBtn, 1);
        lv_obj_set_style_text_opa(label, LV_OPA_80, LV_PART_MAIN);
    } else {
        lv_obj_add_flag(g_multisigItem.deleteNotice, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(g_multisigItem.deleteBtn, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_add_flag(g_multisigItem.setDefaultBtn, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_set_style_text_opa(label, LV_OPA_100, LV_PART_MAIN);
        lv_label_set_text(label, _("manage_multi_wallet_set_default"));
        label = lv_obj_get_child(g_multisigItem.deleteBtn, 1);
        lv_obj_set_style_text_opa(label, LV_OPA_100, LV_PART_MAIN);
    }
}