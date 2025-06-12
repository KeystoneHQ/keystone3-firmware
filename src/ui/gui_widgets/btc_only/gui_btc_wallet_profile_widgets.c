#include <stdbool.h>
#include <stdio.h>
#include "gui.h"
#include "gui_views.h"
#include "gui_model.h"
#include "gui_status_bar.h"
#include "gui_page.h"
#include "gui_button.h"
#include "gui_home_widgets.h"
#include "gui_hintbox.h"
#include "gui_api.h"
#include "multi_sig_wallet_manager.h"
#include "gui_export_pubkey_widgets.h"
#include "gui_btc_wallet_profile_widgets.h"
#include "gui_manage_multisig_wallet_widgets.h"
#include "keystore.h"

typedef enum {
    WALLET_PROFILE_SELECT = 0,
    WALLET_PROFILE_SINGLE_WALLET,

} WALLET_PROFILE_ENUM;

typedef struct {
    uint8_t currentTile;
    lv_obj_t *tileView;
    lv_obj_t *profileView;
} WalletProfileWidgets_t;

static void OpenExportShowXpubHandler(lv_event_t *e);
static void OpenManageMultisigViewHandler(lv_event_t *e);
static void SetDefaultSingleWalletHandler(lv_event_t *e);
static void CreateSingleSigWalletWidget(lv_obj_t *parent);
static void CreateBtcWalletProfileEntranceRefresh(lv_obj_t *parent);
static void ManageMultiSigWalletHandler(lv_event_t *e);
static void OpenCreateMultiViewHandler(lv_event_t *e);
static void OpenBtcExportViewHandler(lv_event_t *e);
static void OpenBtcExportMultisigViewHandler(lv_event_t *e);
void GuiResetCurrentUtxoAddressIndex(uint8_t index);

static WalletProfileWidgets_t g_walletProfile;
static PageWidget_t *g_pageWidget;
static lv_obj_t *g_networkBtn = NULL;
static lv_obj_t *g_setDefaultBtn = NULL;
static lv_obj_t *g_noticeWindow = NULL;

void GuiBtcWalletProfileInit(void)
{
    g_pageWidget = CreatePageWidget();
    g_walletProfile.tileView = GuiCreateTileView(g_pageWidget->contentZone);
    lv_obj_t *tile = lv_tileview_add_tile(g_walletProfile.tileView, WALLET_PROFILE_SELECT, 0, LV_DIR_HOR);
    GuiAddObjFlag(tile, LV_OBJ_FLAG_SCROLLABLE);
    g_walletProfile.profileView = tile;

    tile = lv_tileview_add_tile(g_walletProfile.tileView, WALLET_PROFILE_SINGLE_WALLET, 0, LV_DIR_HOR);
    CreateSingleSigWalletWidget(tile);

    g_walletProfile.currentTile = WALLET_PROFILE_SELECT;
}

void GuiBtcWalletProfileDeInit(void)
{
    if (g_pageWidget != NULL) {
        DestroyPageWidget(g_pageWidget);
        g_pageWidget = NULL;
    }

    GUI_DEL_OBJ(g_noticeWindow)
}

void GuiBtcWalletProfileRefresh(void)
{
    if (g_walletProfile.currentTile == WALLET_PROFILE_SELECT) {
        SetMidBtnLabel(g_pageWidget->navBarWidget, NVS_BAR_MID_LABEL, _("wallet_profile_mid_btn"));
        SetNavBarLeftBtn(g_pageWidget->navBarWidget, NVS_BAR_RETURN, CloseCurrentViewHandler, NULL);
        CreateBtcWalletProfileEntranceRefresh(g_walletProfile.profileView);
    } else {
        SetMidBtnLabel(g_pageWidget->navBarWidget, NVS_BAR_MID_LABEL, _("wallet_profile_single_sign_title_text"));
        SetNavBarLeftBtn(g_pageWidget->navBarWidget, NVS_BAR_RETURN, ReturnHandler, NULL);
        lv_obj_t *label = lv_obj_get_child(g_setDefaultBtn, 0);
        if (GetCurrentWalletIndex() == SINGLE_WALLET) {
            lv_obj_clear_flag(g_setDefaultBtn, LV_OBJ_FLAG_CLICKABLE);
            lv_obj_set_style_text_opa(label, LV_OPA_80, LV_PART_MAIN);
            lv_label_set_text(label, _("wallet_profile_current_default_desc"));
            lv_obj_clear_flag(g_networkBtn, LV_OBJ_FLAG_HIDDEN);
        } else {
            lv_obj_add_flag(g_networkBtn, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(g_setDefaultBtn, LV_OBJ_FLAG_CLICKABLE);
            lv_obj_set_style_text_opa(label, LV_OPA_100, LV_PART_MAIN);
            lv_label_set_text(label, _("manage_multi_wallet_set_default"));
        }
    }
}

int8_t GuiBtcWalletProfilePrevTile(void)
{
    --g_walletProfile.currentTile;
    lv_obj_set_tile_id(g_walletProfile.tileView, g_walletProfile.currentTile, 0, LV_ANIM_OFF);
    GuiBtcWalletProfileRefresh();
    return 0;
}

int8_t GuiBtcWalletProfileNextTile(void)
{
    ++g_walletProfile.currentTile;
    lv_obj_set_tile_id(g_walletProfile.tileView, g_walletProfile.currentTile, 0, LV_ANIM_OFF);
    GuiBtcWalletProfileRefresh();
    return 0;
}

static void SwitchTestnetHandler(lv_event_t *e)
{
    lv_obj_t *networkSwitch = lv_event_get_user_data(e);
    bool en = lv_obj_has_state(networkSwitch, LV_STATE_CHECKED);
    SetIsTestNet(!en);
    GuiApiEmitSignal(SIG_STATUS_BAR_TEST_NET, NULL, 0);
    if (en) {
        lv_obj_clear_state(networkSwitch, LV_STATE_CHECKED);
    } else {
        lv_obj_add_state(networkSwitch, LV_STATE_CHECKED);
    }
}

static void CreateSingleWalletButton(lv_obj_t *parent, uint16_t *offset)
{
    CURRENT_WALLET_INDEX_ENUM currentWallet = GetCurrentWalletIndex();
    char *desc = (currentWallet != SINGLE_WALLET) ? NULL : (char *)_("wallet_profile_default_desc");
    uint16_t height = (currentWallet != SINGLE_WALLET) ? 84 : 118;

    lv_obj_t *button = GuiCreateSettingItemButton(parent, 456,
                       _("wallet_profile_single_sign_title"),
                       desc, &imgKey, &imgArrowRight,
                       NextTileHandler, NULL);
    lv_obj_set_height(button, height);
    lv_obj_align(button, LV_ALIGN_TOP_LEFT, 12, 0);
    *offset = height;
}

static uint16_t CreateMultiWalletButton(lv_obj_t *parent, MultiSigWalletItem_t *item,
                                        int index, CURRENT_WALLET_INDEX_ENUM currentWallet, uint16_t currentOffset)
{
    static CURRENT_WALLET_INDEX_ENUM currentIndex[] = {
        MULTI_SIG_WALLET_FIRST, MULTI_SIG_WALLET_SECOND,
        MULTI_SIG_WALLET_THIRD, MULTI_SIG_WALLET_FOURTH,
        MULTI_SIG_WALLET_FIFTH
    };

    char desc[BUFFER_SIZE_32] = {0};
    uint16_t height = 84;

    if (currentWallet == index) {
        strcpy_s(desc, sizeof(desc), _("wallet_profile_default_desc"));
        height = 118;
    }

    lv_obj_t *button = GuiCreateSettingItemButton(parent, 456,
                       item->name, desc, &imgTwoKey, &imgArrowRight,
                       OpenManageMultisigViewHandler, &currentIndex[index]);

    lv_obj_set_height(button, height);
    lv_obj_align(button, LV_ALIGN_TOP_LEFT, 12, currentOffset + 12);

    return currentOffset + ((currentWallet == index) ? 118 : 96);
}

static void CreateAddWalletButton(lv_obj_t *parent, uint16_t offset, int multiSigNum)
{
    lv_obj_t *button = GuiCreateSelectButton(parent,
                       _("wallet_profile_add_multi_wallet"),
                       &imgAddOrange,
                       ManageMultiSigWalletHandler,
                       NULL,
                       true);

    lv_obj_set_style_text_color(lv_obj_get_child(button, 0), ORANGE_COLOR, LV_PART_MAIN);
    lv_obj_align(button, LV_ALIGN_TOP_LEFT, 12, 96 * multiSigNum + offset + 12);
}

static void CreateNoticeLabel(lv_obj_t *parent, bool isPassphraseExist, int multiSigNum)
{
    lv_obj_t *label = NULL;
    if (isPassphraseExist && multiSigNum != 0) {
        label = GuiCreateNoticeLabel(parent, _("manage_multi_wallet_passphrase_add_limit_desc"));
    } else if (multiSigNum == MAX_MULTI_SIG_WALLET_NUMBER_EXCEPT_PASSPHRASE) {
        label = GuiCreateNoticeLabel(parent, _("manage_multi_wallet_add_limit_desc"));
    }

    if (label) {
        GuiAlignToPrevObj(label, LV_ALIGN_OUT_BOTTOM_LEFT, 24, 12);
    }
}

static void CreateBottomSection(lv_obj_t *parent, bool isPassphrase, int multiSigNum)
{
    lv_obj_t *line = GuiCreateDividerLine(parent);

    uint32_t maxMultiSigNum = isPassphrase ?
                              MAX_MULTI_SIG_PASSPHRASE_WALLET_NUMBER :
                              MAX_MULTI_SIG_WALLET_NUMBER_EXCEPT_PASSPHRASE;

    GuiAlignToPrevObj(line, LV_ALIGN_OUT_BOTTOM_LEFT,
                      (multiSigNum == maxMultiSigNum) ? -36 : -12, 12);

    lv_obj_t *button = GuiCreateSelectButton(parent,
                       _("wallet_profile_multi_wallet_show_xpub"),
                       &imgExport,
                       OpenExportShowXpubHandler,
                       NULL,
                       true);

    lv_obj_align_to(button, line, LV_ALIGN_OUT_BOTTOM_LEFT, 12, 12);
}

static int HandleInvalidMultiSigWallet(MultiSigWalletItem_t *item)
{
    DeleteAccountMultiReceiveIndex("BTC", item->verifyCode);
    int index = DeleteMultisigWalletByVerifyCode(item->verifyCode);
    SetCurrentWalletIndex(SINGLE_WALLET);
    return GetCurrentAccountMultisigWalletNum(item->passphrase);
}

static void CreateMultiWalletButtons(lv_obj_t *parent, uint16_t startOffset,
                                     bool isPassphrase, int *multiSigNum, bool *isPassphraseExist)
{
    uint8_t mfp[4];
    GetMasterFingerPrint(mfp);
    uint16_t offset = startOffset;
    CURRENT_WALLET_INDEX_ENUM currentWallet = GetCurrentWalletIndex();

    for (int i = 0; i < MAX_MULTI_SIG_WALLET_NUMBER; i++) {
        MultiSigWalletItem_t *item = GetCurrenMultisigWalletByIndex(i);
        if (item == NULL) {
            break;
        }

        if (item->passphrase != isPassphrase) {
            continue;
        }

        if (item->passphrase) {
            *isPassphraseExist = true;
            if (import_multi_sig_wallet_by_file(item->walletConfig, mfp, 4)->error_code != 0) {
                *multiSigNum = HandleInvalidMultiSigWallet(item);
                break;
            }
        }

        offset = CreateMultiWalletButton(parent, item, i, currentWallet, offset);
    }
}

static void CreateBtcWalletProfileEntranceRefresh(lv_obj_t *parent)
{
    lv_obj_clean(parent);

    uint16_t offset = 0;
    bool isPassphraseExist = false;
    bool isPassphrase = PassphraseExist(GetCurrentAccountIndex());
    int multiSigNum = GetCurrentAccountMultisigWalletNum(isPassphrase);

    CreateSingleWalletButton(parent, &offset);

    CreateMultiWalletButtons(parent, offset, isPassphrase, &multiSigNum, &isPassphraseExist);
    printf("isPassphraseExist: %d, multiSigNum: %d\r\n", isPassphraseExist, multiSigNum);

    if ((!isPassphraseExist && multiSigNum < MAX_MULTI_SIG_WALLET_NUMBER_EXCEPT_PASSPHRASE) || 
        (isPassphraseExist && multiSigNum == 0)) {
        CreateAddWalletButton(parent, offset, multiSigNum);
    }

    CreateNoticeLabel(parent, isPassphraseExist, multiSigNum);

    CreateBottomSection(parent, isPassphrase, multiSigNum);
}

static void CreateSingleSigWalletWidget(lv_obj_t *parent)
{
    lv_obj_t *button = GuiCreateSelectButton(parent, _("manage_multi_wallet_set_default"), &imgDefaultWallet,
                       SetDefaultSingleWalletHandler, NULL, true);
    lv_obj_align(button, LV_ALIGN_TOP_MID, 0, 0);
    g_setDefaultBtn = button;

    lv_obj_t *checkBox = GuiCreateSwitch(parent);
    if (GetIsTestNet()) {
        lv_obj_add_state(checkBox, LV_STATE_CHECKED);
    } else {
        lv_obj_clear_state(checkBox, LV_STATE_CHECKED);
    }
    lv_obj_clear_flag(checkBox, LV_OBJ_FLAG_CLICKABLE);
    GuiButton_t table[] = {
        {.obj = GuiCreateImg(parent, &imgNetwork), .align = LV_ALIGN_LEFT_MID, .position = {24, 0}},
        {.obj = GuiCreateTextLabel(parent, _("wallet_profile_network_test")), .align = LV_ALIGN_LEFT_MID, .position = {76, 0}},
        {.obj = checkBox, .align = LV_ALIGN_LEFT_MID, .position = {376, 0}}
    };
    g_networkBtn = GuiCreateButton(parent, 456, 84, table, NUMBER_OF_ARRAYS(table), SwitchTestnetHandler, checkBox);
    lv_obj_align(g_networkBtn, LV_ALIGN_DEFAULT, 12, 92);
    if (GetCurrentWalletIndex() != SINGLE_WALLET) {
        lv_obj_add_flag(g_networkBtn, LV_OBJ_FLAG_HIDDEN);
    }
}

static void SetDefaultSingleWalletHandler(lv_event_t *e)
{
    SetCurrentWalletIndex((CURRENT_WALLET_INDEX_ENUM)SINGLE_WALLET);
    lv_obj_t *label = lv_obj_get_child(g_setDefaultBtn, 0);
    lv_obj_clear_flag(g_setDefaultBtn, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(g_networkBtn, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_style_text_opa(label, LV_OPA_80, LV_PART_MAIN);
    lv_label_set_text(label, _("wallet_profile_current_default_desc"));
    GuiResetCurrentUtxoAddressIndex(GetCurrentAccountIndex());
    GuiApiEmitSignal(SIG_STATUS_BAR_TEST_NET, NULL, 0);
}

static void ManageMultiSigWalletHandler(lv_event_t *e)
{
    if (CHECK_BATTERY_LOW_POWER()) {
        g_noticeWindow = GuiCreateErrorCodeWindow(ERR_KEYSTORE_SAVE_LOW_POWER, &g_noticeWindow, NULL);
    } else {
        GuiFrameOpenView(&g_multisigSelectImportMethodView);
    }
#if 0 // hide the create multisig entrance
    MoreInfoTable_t moreInfoTable[] = {
        {.name = _("wallet_profile_create_multi_wallet"), .src = &imgArrowRight, .callBack = OpenCreateMultiViewHandler, &g_createMultisigWalletView},
        {.name = _("wallet_profile_import_multi_wallet"), .src = &imgArrowRight, .callBack = OpenCreateMultiViewHandler, &g_multisigSelectImportMethodView},
    };
    g_noticeWindow = GuiCreateMoreInfoHintBox(&imgClose, _("wallet_profile_add_multi_wallet"), moreInfoTable, NUMBER_OF_ARRAYS(moreInfoTable), false, &g_noticeWindow);
#endif
}

static void OpenCreateMultiViewHandler(lv_event_t *e)
{
    GUI_DEL_OBJ(g_noticeWindow)
    GuiFrameOpenView(lv_event_get_user_data(e));
}

static void OpenManageMultisigViewHandler(lv_event_t *e)
{
    GuiFrameOpenViewWithParam(&g_manageMultisigWalletView, lv_event_get_user_data(e), sizeof(CURRENT_WALLET_INDEX_ENUM));
}

static void OpenExportShowXpubHandler(lv_event_t *e)
{
    static bool testStatus[] = {false, true};
    uint16_t offset = 0;
    g_noticeWindow = GuiCreateHintBox(408 - offset);
    lv_obj_t *title = GuiCreateIllustrateLabel(g_noticeWindow, _("wallet_profile_multi_wallet_show_xpub"));
    lv_obj_align(title, LV_ALIGN_DEFAULT, 36, 422 + offset);
    lv_obj_t *closeBtn = GuiCreateImgButton(g_noticeWindow, &imgClose, 64, CloseHintBoxHandler, &g_noticeWindow);
    lv_obj_align(closeBtn, LV_ALIGN_DEFAULT, 394, 405 + offset);

    lv_obj_t *button = GuiCreateSelectButton(g_noticeWindow, _("wallet_profile_single_wallet_title"), &imgArrowRight, OpenBtcExportViewHandler, &testStatus[0], false);
    lv_obj_align(button, LV_ALIGN_TOP_MID, 0, 482 + offset);

    GuiButton_t table[] = {
        {.obj = GuiCreateTextLabel(g_noticeWindow, _("wallet_profile_single_wallet_title")), .align = LV_ALIGN_DEFAULT, .position = {24, 24},},
        {.obj = GuiCreateIllustrateLabel(g_noticeWindow, _("wallet_profile_network_test")), .align = LV_ALIGN_DEFAULT, .position = {24, 60},},
        {.obj = GuiCreateImg(g_noticeWindow, &imgArrowRight), .align = LV_ALIGN_RIGHT_MID, .position = {-12, 0},},
    };
    button = GuiCreateButton(g_noticeWindow, 456, 114, table, NUMBER_OF_ARRAYS(table),
                             OpenBtcExportViewHandler, &testStatus[1]);
    lv_obj_align(button, LV_ALIGN_TOP_MID, 0, 578 + offset);
    lv_obj_set_style_text_color(lv_obj_get_child(button, 1), YELLOW_COLOR, 0);
    button = GuiCreateSelectButton(g_noticeWindow, _("wallet_profile_multi_sign_title"), &imgArrowRight, OpenBtcExportMultisigViewHandler, NULL, false);
    lv_obj_align(button, LV_ALIGN_BOTTOM_MID, 0, -12);
}

static void OpenBtcExportViewHandler(lv_event_t *e)
{
    GUI_DEL_OBJ(g_noticeWindow)
    OpenExportViewHandler(e);
}

static void OpenBtcExportMultisigViewHandler(lv_event_t *e)
{
    GUI_DEL_OBJ(g_noticeWindow)
    OpenExportMultisigViewHandler(e);
}