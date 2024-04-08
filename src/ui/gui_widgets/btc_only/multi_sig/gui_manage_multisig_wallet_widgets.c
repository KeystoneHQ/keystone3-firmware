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
#ifdef COMPILE_SIMULATOR
#include "simulator_model.h"
#include "simulator_mock_define.h"
#else
#include "user_fatfs.h"
#include "safe_str_lib.h"
#endif
#define MULTI_SIG_WALLET_MAX_NUM               (3)

typedef enum {
    MULTI_MULTI_SIG_HOME = 0,
    MULTI_MULTI_SIG_DETAIL,
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
    lv_obj_t *walletName;
    lv_obj_t *policy;
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

static void ReloadAndUpdateMultisigConfig(void);
static void CreateMultiSigWalletWidget(lv_obj_t *parent);
static void ManageMultiSigWalletHandler(lv_event_t *e);
static void CreateMultiSigWalletDetailWidget(lv_obj_t *parent);
static void CreateCoSignerDetailWidget(lv_obj_t *parent);
static void ReloadAndUpdateMultisigItem(DEFAULT_WALLET_INDEX_ENUM index);
static void DeleteMultiWalletHandler(lv_event_t *e);
static void SetDefaultMultiWalletHandler(lv_event_t *e);
static void SelectWalletIndexAndNextHandler(lv_event_t *e);
static void GuiConfirmDeleteHandler(lv_event_t *e);
static void ExportMultiWalletHandler(lv_event_t *e);
static void UpdateDefaultWalletState(void);

void GuiManageMultisigWalletInit(void)
{
    g_pageWidget = CreatePageWidget();
    lv_obj_t *tileView = GuiCreateTileView(g_pageWidget->contentZone);
    lv_obj_t *tile = lv_tileview_add_tile(tileView, MULTI_MULTI_SIG_HOME, 0, LV_DIR_HOR);
    g_manageMultisig.homeTile = tile;

    tile = lv_tileview_add_tile(tileView, MULTI_MULTI_SIG_DETAIL, 0, LV_DIR_HOR);
    CreateMultiSigWalletDetailWidget(tile);

    tile = lv_tileview_add_tile(tileView, MULTI_MULTI_SIG_CO_SIGNERS_DETAIL, 0, LV_DIR_HOR);
    g_manageMultisig.showCoSingersTile = tile;

    g_manageMultisig.currentTile = MULTI_MULTI_SIG_HOME;
    g_manageMultisig.tileView = tileView;

    lv_obj_set_tile_id(g_manageMultisig.tileView, 0, 0, LV_ANIM_OFF);
}


int8_t GuiManageMultisigWalletNextTile(uint8_t index)
{
    switch (g_manageMultisig.currentTile) {
    case MULTI_MULTI_SIG_HOME:
        ReloadAndUpdateMultisigItem((DEFAULT_WALLET_INDEX_ENUM)index);
        lv_label_set_text(g_multisigItem.walletName, g_walletItem->name);
        lv_label_set_text(g_multisigItem.policy, g_multiSigWallet->policy);
        UpdateDefaultWalletState();
        break;
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
    case MULTI_MULTI_SIG_HOME:
        GuiCLoseCurrentWorkingView();
        break;
    case MULTI_MULTI_SIG_DETAIL:
        break;
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
}

static void GuiMoreHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_CLICKED) {
        // GuiFrameOpenView(&g_multisigSelectImportMethodView);
        return;
    }
}

void GuiManageMultisigWalletRefresh(void)
{
    char tempBuff[BUFFER_SIZE_32] = {0};
    SetNavBarLeftBtn(g_pageWidget->navBarWidget, NVS_BAR_RETURN, ReturnHandler, NULL);
    SetNavBarRightBtn(g_pageWidget->navBarWidget, NVS_BAR_MORE_INFO, GuiMoreHandler, NULL);
    if (g_manageMultisig.currentTile == MULTI_MULTI_SIG_HOME) {
        ReloadAndUpdateMultisigConfig();
        SetNavBarLeftBtn(g_pageWidget->navBarWidget, NVS_BAR_RETURN, CloseCurrentViewHandler, NULL);
        SetMidBtnLabel(g_pageWidget->navBarWidget, NVS_BAR_MID_LABEL, _("wallet_profile_multi_sign_title"));
    } else if (g_manageMultisig.currentTile == MULTI_MULTI_SIG_DETAIL) {
        SetMidBtnLabel(g_pageWidget->navBarWidget, NVS_BAR_MID_LABEL, _("manage_multi_wallet_detail_title"));
    } else if (g_manageMultisig.currentTile == MULTI_MULTI_SIG_CO_SIGNERS_DETAIL) {
        snprintf_s(tempBuff, sizeof(tempBuff), "%s", g_multiSigWallet->policy);
        SetMidBtnLabel(g_pageWidget->navBarWidget, NVS_BAR_MID_LABEL, tempBuff);
    }
}



static void ReloadAndUpdateMultisigConfig(void)
{
    static HOME_WALLET_CARD_ENUM chainCard = HOME_WALLET_CARD_BTC;
    static DEFAULT_WALLET_INDEX_ENUM defaultIndex[] = {0, 1, 2};
    int multiSigNum = GetCurrentAccountMultisigWalletNum();
    DEFAULT_WALLET_INDEX_ENUM defaultWallet = GetDefaultWalletIndex();
    printf("getdefaultwallet = %d\n", defaultWallet);
    printf("multisignum = %d\n", multiSigNum);
    lv_obj_clean(g_manageMultisig.homeTile);
    if (multiSigNum == 0) {
        return CreateMultiSigWalletWidget(g_manageMultisig.homeTile);
    }
    for (int i = 0; i < multiSigNum; ++i) {
        char desc[BUFFER_SIZE_16] = {0};
        uint16_t height = 84;
        uint16_t offSet = 0;
        MultiSigWalletItem_t *item = GetCurrenMultisigWalletByIndex(i);
        if (defaultWallet == item->order) {
            strcpy_s(desc, sizeof(desc), _("wallet_profile_default_desc"));
            height = 118;
            offSet += 96;
        } else if (i != 0) {
            offSet += 96;
        }
        lv_obj_t *button = GuiCreateSettingItemButton(g_manageMultisig.homeTile, 456, item->name, desc,
                           &imgTwoKey, &imgArrowRight, SelectWalletIndexAndNextHandler, &defaultIndex[i]);
        lv_obj_set_height(button, height);
        lv_obj_align(button, LV_ALIGN_TOP_LEFT, 12, 96 * i);
    }
    if (MULTI_SIG_WALLET_MAX_NUM != multiSigNum) {
        lv_obj_t *button = GuiCreateSelectButton(g_manageMultisig.homeTile, _("wallet_profile_add_multi_wallet"), &imgAdd,
                           ManageMultiSigWalletHandler, NULL, true);
        lv_obj_align(button, LV_ALIGN_TOP_LEFT, 12, 96 * multiSigNum);
    }

    lv_obj_t *line = GuiCreateDividerLine(g_manageMultisig.homeTile);
    uint32_t offSet = 0;
    if (multiSigNum == 3) {
        offSet = 354;
        lv_obj_t *label = GuiCreateNoticeLabel(g_manageMultisig.homeTile, _("manage_multi_wallet_add_limit_desc"));
        lv_obj_align(label, LV_ALIGN_TOP_LEFT, 36, 288);
    } else if (0) {     // testnet
    } else {
        offSet = (multiSigNum + 1) * 96;
    }
    lv_obj_align(line, LV_ALIGN_DEFAULT, 0, offSet);
    lv_obj_t *button = GuiCreateSelectButton(g_manageMultisig.homeTile, _("wallet_profile_multi_wallet_show_xpub"), &imgWalletExport,
                       OpenExportViewHandler, &chainCard, true);
    lv_obj_align_to(button, line, LV_ALIGN_OUT_BOTTOM_LEFT, 12, 12);
}

static void CreateMultiSigWalletWidget(lv_obj_t *parent)
{
    lv_obj_t *img = GuiCreateImg(parent, &imgLockMulti);
    lv_obj_align(img, LV_ALIGN_TOP_MID, 0, 128);

    lv_obj_t *label = GuiCreateTextLabel(parent, _("wallet_profile_no_multi_wallet_notice"));
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 232);

    lv_obj_t *button = GuiCreateBtn(parent, _("wallet_profile_add_multi_wallet"));
    lv_obj_set_width(button, 408);
    lv_obj_set_style_text_color(lv_obj_get_child(button, 0), ORANGE_COLOR, LV_PART_MAIN);
    lv_obj_set_style_bg_color(button, WHITE_COLOR_OPA20, LV_PART_MAIN);
    lv_obj_align(button, LV_ALIGN_TOP_MID, 0, 436);
    lv_obj_add_event_cb(button, ManageMultiSigWalletHandler, LV_EVENT_CLICKED, NULL);

    button = GuiCreateBtn(parent, _("wallet_profile_multi_wallet_show_xpub"));
    lv_obj_set_width(button, 408);
    lv_obj_set_style_bg_color(button, WHITE_COLOR_OPA20, LV_PART_MAIN);
    lv_obj_align(button, LV_ALIGN_TOP_MID, 0, 526);
    lv_obj_add_event_cb(button, ManageMultiSigWalletHandler, LV_EVENT_CLICKED, NULL);
}

static void CreateMultiSigWalletDetailWidget(lv_obj_t *parent)
{
    lv_obj_t *button = GuiCreateSettingItemButton(parent, 456, "", NULL,
                       &imgWallet2, &imgEdit, UnHandler, NULL);
    lv_obj_align(button, LV_ALIGN_DEFAULT, 12, 0);
    g_multisigItem.walletName = lv_obj_get_child(button, 1);

    button = GuiCreateSettingItemButton(parent, 456, "", NULL,
                                        &imgTwoKey, &imgArrowRight, NextTileHandler, NULL);
    lv_obj_align(button, LV_ALIGN_DEFAULT, 12, 96);
    g_multisigItem.policy = lv_obj_get_child(button, 1);

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
}

static void ReloadAndUpdateMultisigItem(DEFAULT_WALLET_INDEX_ENUM index)
{
    if (g_multiSigWallet != NULL) {
        free_MultiSigWallet(g_multiSigWallet);
        g_multiSigWallet = NULL;
    }

    uint8_t mfp[4];
    GetMasterFingerPrint(mfp);
    g_walletItem = GetCurrenMultisigWalletByIndex(index);
    Ptr_Response_MultiSigWallet result = import_multi_sig_wallet_by_file(g_walletItem->walletConfig, mfp, 4, MainNet);
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
        label = GuiCreateNoticeLabel(cont, g_multiSigWallet->derivations->data[i]);
        lv_obj_align(label, LV_ALIGN_DEFAULT, 24, i * 204 + 174);
    }
}

static void OpenCreateMultiViewHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_CLICKED) {
        GUI_DEL_OBJ(g_noticeWindow)
        GuiFrameOpenView(lv_event_get_user_data(e));
    }
}

static void ManageMultiSigWalletHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_CLICKED) {
        MoreInfoTable_t moreInfoTable[] = {
            {.name = _("wallet_profile_create_multi_wallet"), .src = &imgArrowRight, .callBack = OpenCreateMultiViewHandler, &g_createMultisigWalletView},
            {.name = _("wallet_profile_import_multi_wallet"), .src = &imgArrowRight, .callBack = OpenCreateMultiViewHandler, &g_multisigSelectImportMethodView},
        };
        g_noticeWindow = GuiCreateMoreInfoHintBox(&imgClose, _("wallet_profile_add_multi_wallet"), moreInfoTable, NUMBER_OF_ARRAYS(moreInfoTable), false);
        lv_obj_t *closeBtn = GuiCreateImgButton(g_noticeWindow,  &imgClose, 64, CloseHintBoxHandler, &g_noticeWindow);
        GuiAlignToPrevObj(closeBtn, LV_ALIGN_LEFT_MID, 358, 0);
    }
}

static void DeleteMultiWalletHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_CLICKED) {
        char tempBuff[BUFFER_SIZE_32] = {0};
        snprintf_s(tempBuff, sizeof(tempBuff), "%s %s?", _("Delete"), g_walletItem->name);
        g_noticeWindow = GuiCreateGeneralHintBox(lv_scr_act(), &imgWarn, tempBuff, _("manage_multi_wallet_delete_desc"), NULL,
                         _("Cancel"), WHITE_COLOR_OPA20, _("Delete"), DEEP_ORANGE_COLOR);
        lv_obj_t *leftBtn = GuiGetHintBoxLeftBtn(g_noticeWindow);
        lv_obj_add_event_cb(leftBtn, CloseHintBoxHandler, LV_EVENT_CLICKED, &g_noticeWindow);
        lv_obj_t *rightBtn = GuiGetHintBoxRightBtn(g_noticeWindow);
        lv_obj_add_event_cb(rightBtn, GuiConfirmDeleteHandler, LV_EVENT_CLICKED, NULL);
    }
}

static void SetDefaultMultiWalletHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_CLICKED) {
        SetDefaultWalletIndex((DEFAULT_WALLET_INDEX_ENUM)g_walletItem->order);
        UpdateDefaultWalletState();
    }
}

static void ExportMultiWalletHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_CLICKED) {
        GuiSetMultisigImportWalletDataBySDCard(g_walletItem->walletConfig);
        GuiSetExportMultiSigSwitch();
        GuiFrameOpenView(&g_importMultisigWalletInfoView);
        GuiEmitSignal(SIG_MULTISIG_WALLET_SET_WALLET_EXPORT, NULL, 0);
    }
}

static void SelectWalletIndexAndNextHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_CLICKED) {
        GuiManageMultisigWalletNextTile(*(uint8_t *)lv_event_get_user_data(e));
    }
}

static void GuiConfirmDeleteHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        GUI_DEL_OBJ(g_noticeWindow)
        static uint16_t sig = SIG_MULTISIG_WALLET_IMPORT_VERIFY_PASSWORD;
        g_keyboardWidget = GuiCreateKeyboardWidget(g_pageWidget->contentZone);
        SetKeyboardWidgetSelf(g_keyboardWidget, &g_keyboardWidget);
        SetKeyboardWidgetSig(g_keyboardWidget, &sig);
    }
}

void DeleteMultisigWallet(void)
{
    DeleteMultisigWalletByVerifyCode(g_walletItem->verifyCode, SecretCacheGetPassword());
    GuiDeleteKeyboardWidget(g_keyboardWidget);
    ClearSecretCache();
    GuiManageMultiWalletPrevTile();
}

static void UpdateDefaultWalletState(void)
{
    lv_obj_t *label = lv_obj_get_child(g_multisigItem.setDefaultBtn, 1);
    if (GetDefaultWalletIndex() == g_walletItem->order) {
        lv_obj_clear_flag(g_multisigItem.deleteBtn, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_clear_flag(g_multisigItem.setDefaultBtn, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_set_style_text_opa(label, LV_OPA_80, LV_PART_MAIN);
        lv_label_set_text(label, _("wallet_profile_current_default_desc"));
        label = lv_obj_get_child(g_multisigItem.deleteBtn, 1);
        lv_obj_set_style_text_opa(label, LV_OPA_80, LV_PART_MAIN);
    } else {
        lv_obj_add_flag(g_multisigItem.deleteBtn, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_add_flag(g_multisigItem.setDefaultBtn, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_set_style_text_opa(label, LV_OPA_100, LV_PART_MAIN);
        lv_label_set_text(label, _("manage_multi_wallet_set_default"));
        label = lv_obj_get_child(g_multisigItem.deleteBtn, 1);
        lv_obj_set_style_text_opa(label, LV_OPA_100, LV_PART_MAIN);
    }
}