#include "gui.h"
#include "gui_views.h"
#include "gui_status_bar.h"
#include "gui_keyboard.h"
#include "gui_button.h"
#include "gui_hintbox.h"
#include "gui_create_wallet_widgets.h"
#include "gui_model.h"
#include "secret_cache.h"
#include "user_memory.h"
#include "gui_enter_passcode.h"
#include "motor_manager.h"
#include "gui_tutorial_widgets.h"
#include "gui_keyboard_hintbox.h"
#include "gui_page.h"

#ifdef COMPILE_SIMULATOR
#include "simulator_mock_define.h"
#endif

typedef enum {
    CREATE_WALLET_SETPIN = 0,
    CREATE_WALLET_REPEATPIN,
    CREATE_WALLET_NAMEWALLET,
    CREATE_WALLET_BACKUPFROM,

    CREATE_WALLET_BUTT,
} CREATE_WALLET_ENUM;

typedef struct CreateWalletWidget {
    uint8_t currentTile;
    uint8_t walletMethod;
    lv_obj_t *cont;
    lv_obj_t *tileView;
    lv_obj_t *instructions;
    lv_obj_t *setPin;
    lv_obj_t *repeatPin;
    lv_obj_t *nameWallet;
    lv_obj_t *backupForm;
    lv_obj_t *diceRollsHint;
} CreateWalletWidget_t;
static CreateWalletWidget_t g_createWalletTileView;

static void OpenMoreHandler(lv_event_t *e);
static void OpenChangeEntropyHandler(lv_event_t *e);
static void TonPhraseButtonHandler(lv_event_t *e);
static void GuiRefreshNavBar(void);
static void CloseChangeEntropyHandler(lv_event_t *e);
static void OpenChangeEntropyTutorialHandler(lv_event_t *e);

static PageWidget_t *g_pageWidget;
static KeyBoard_t *g_nameWalletKb = NULL;
static lv_obj_t *g_nameWalletIcon = NULL;
static GuiEnterPasscodeItem_t *g_setPassCode = NULL;
static GuiEnterPasscodeItem_t *g_repeatPassCode = NULL;
static lv_obj_t *g_setPinTile = NULL;
static lv_obj_t *g_repeatPinTile = NULL;
static lv_obj_t *g_noticeWindow = NULL;
static char g_pinBuf[PASSWORD_MAX_LEN + 1];
static lv_obj_t *g_openMoreHintBox;
static PageWidget_t *g_changeEntropyPage;
//indicates the way to generate entropy;
static uint8_t g_selectedEntropyMethod = ENTROPY_TYPE_STANDARD;

void GuiSetupKeyboardWidgetMode(void)
{
    SetKeyboardWidgetMode(KEYBOARD_HINTBOX_PIN + g_setPassCode->mode % 2);
}

static void GuiCreateSetpinWidget(lv_obj_t *parent)
{
    g_setPinTile = parent;
    g_setPassCode = GuiCreateEnterPasscode(parent, NULL, NULL, ENTER_PASSCODE_SET_PIN);
}

static void GuiCreateRepeatPinWidget(lv_obj_t *parent)
{
    g_repeatPinTile = parent;
}

const char *GetCurrentKbWalletName(void)
{
    if (g_nameWalletKb != NULL) {
        return lv_textarea_get_text(g_nameWalletKb->ta);
    }
    return "";
}

static void QuestionMarkEventCb(lv_event_t *e)
{
    uint8_t index = TUTORIAL_SHAMIR_BACKUP;
    GUI_DEL_OBJ(g_openMoreHintBox);
    GuiFrameOpenViewWithParam(&g_tutorialView, &index, sizeof(index));
}

static void OpenEmojiKbHandler(lv_event_t *e)
{
    GuiCreateEmojiKeyBoard(lv_scr_act(), g_nameWalletIcon);
}

static void GuiCreateNameWalletWidget(lv_obj_t *parent)
{
    char tempBuf[BUFFER_SIZE_16] = {0};
    lv_obj_t *label = GuiCreateScrollTitleLabel(parent, _("single_backup_namewallet_title"));
    lv_obj_align(label, LV_ALIGN_DEFAULT, 36, 156 - GUI_MAIN_AREA_OFFSET);

    label = GuiCreateIllustrateLabel(parent, _("single_backup_namewallet_desc"));
    lv_obj_set_style_text_opa(label, LV_OPA_60, LV_PART_MAIN);
    GuiAlignToPrevObj(label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 12);

    GuiSetEmojiIconIndex(GUI_KEYBOARD_EMOJI_NEW_INDEX);
    g_nameWalletKb = GuiCreateFullKeyBoard(parent, ReadyNextTileHandler, KEY_STONE_FULL_L, NULL);
    GuiSetKeyBoardMinTaLen(g_nameWalletKb, 0);
    lv_obj_set_size(g_nameWalletKb->ta, 300, 60);
    lv_obj_set_style_text_opa(g_nameWalletKb->ta, LV_OPA_100, 0);
    lv_obj_align(g_nameWalletKb->ta, LV_ALIGN_DEFAULT, 128, 316 - GUI_MAIN_AREA_OFFSET);
    lv_textarea_set_placeholder_text(g_nameWalletKb->ta, _("single_backup_namewallet_previnput"));
    lv_textarea_set_max_length(g_nameWalletKb->ta, 16);
    lv_textarea_set_one_line(g_nameWalletKb->ta, true);
    snprintf_s(tempBuf, BUFFER_SIZE_16, "%d/16", strnlen_s(GuiNvsBarGetWalletName(), 16));
    lv_obj_t *progresslabel = GuiCreateNoticeLabel(parent, tempBuf);
    lv_obj_align(progresslabel, LV_ALIGN_TOP_RIGHT, -36, 384 - GUI_MAIN_AREA_OFFSET);
    GuiSetEnterProgressLabel(progresslabel);

    lv_obj_t *img = GuiCreateImg(parent, &emojiBitcoin);
    lv_obj_t *arrowDownImg = GuiCreateImg(parent, &imgArrowDownS);
    g_nameWalletIcon = img;
    GuiButton_t table[] = {
        {
            .obj = img,
            .align = LV_ALIGN_LEFT_MID,
            .position = {15, 0},
        },
        {
            .obj = arrowDownImg,
            .align = LV_ALIGN_LEFT_MID,
            .position = {59, 0},
        },
    };
    lv_obj_t *button = GuiCreateButton(parent, 100, 70, table, NUMBER_OF_ARRAYS(table), OpenEmojiKbHandler, NULL);
    lv_obj_align(button, LV_ALIGN_DEFAULT, 24, 312 - GUI_MAIN_AREA_OFFSET);
    lv_obj_set_style_radius(button, 16, LV_PART_MAIN);

    lv_obj_t *line = GuiCreateDividerLine(parent);
    lv_obj_align(line, LV_ALIGN_DEFAULT, 0, 380 - GUI_MAIN_AREA_OFFSET);
}

static void OpenNoticeHandler(lv_event_t *e)
{
    g_noticeWindow = GuiCreateConfirmHintBox(&imgRedEye, _("single_backup_notice_title"), _("single_backup_notice_desc1"), _("single_backup_notice_desc2"), USR_SYMBOL_CHECK, ORANGE_COLOR);
    lv_obj_add_event_cb(lv_obj_get_child(g_noticeWindow, 0), CloseHintBoxHandler, LV_EVENT_CLICKED, &g_noticeWindow);

    lv_obj_t *btn = GuiGetHintBoxRightBtn(g_noticeWindow);
    lv_obj_set_width(btn, 96);
    lv_obj_set_style_text_font(lv_obj_get_child(btn, 0), &buttonFont, 0);
    lv_obj_add_event_cb(btn, CloseParentAndNextHandler, LV_EVENT_CLICKED, &g_noticeWindow);

    lv_obj_t *img = GuiCreateImg(g_noticeWindow, &imgClose);
    lv_obj_add_flag(img, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(img, CloseHintBoxHandler, LV_EVENT_CLICKED, &g_noticeWindow);
    lv_obj_align_to(img, lv_obj_get_child(g_noticeWindow, 1), LV_ALIGN_TOP_RIGHT, -36, 36);
}

static void OpenSecretShareHandler(lv_event_t *e)
{
    if ((g_selectedEntropyMethod & ENTROPY_TYPE_MASK) == 0) {
        GuiFrameOpenViewWithParam(&g_createShareView, &g_selectedEntropyMethod, sizeof(g_selectedEntropyMethod));
    } else {
        uint8_t index = SEED_TYPE_SLIP39;
        GuiFrameOpenViewWithParam(&g_diceRollsView, &index, sizeof(index));
    }
}

static void OpenImportPhraseHandler(lv_event_t *e)
{
    Vibrate(SLIGHT);
    lv_obj_add_flag(lv_obj_get_parent(lv_event_get_target(e)), LV_OBJ_FLAG_HIDDEN);
    GuiFrameOpenViewWithParam(&g_importPhraseView, lv_event_get_user_data(e), sizeof(uint8_t));
}

static void ChooseWordsAmountHandler(lv_event_t *e)
{
    static uint8_t walletAmounts[] = {12, 18, 24};
    MoreInfoTable_t moreInfoTable[] = {
        {.name = _("wallet_phrase_12words"), .src = &imgArrowRight, .callBack = OpenImportPhraseHandler, &walletAmounts[0]},
        {.name = _("wallet_phrase_18words"), .src = &imgArrowRight, .callBack = OpenImportPhraseHandler, &walletAmounts[1]},
        {.name = _("wallet_phrase_24words"), .src = &imgArrowRight, .callBack = OpenImportPhraseHandler, &walletAmounts[2]},
    };
    g_noticeWindow = GuiCreateMoreInfoHintBox(&imgClose, _("single_phrase_word_amount_select"), moreInfoTable, NUMBER_OF_ARRAYS(moreInfoTable), false, &g_noticeWindow);
}

static void OpenImportShareHandler(lv_event_t *e)
{
    Vibrate(SLIGHT);
    lv_obj_del(lv_obj_get_parent(lv_event_get_target(e)));
    g_noticeWindow = NULL;
    GuiFrameOpenViewWithParam(&g_importShareView, (uint8_t *)lv_event_get_user_data(e), sizeof(uint8_t));
}

static void SelectImportShareHandler(lv_event_t *e)
{
    static uint8_t walletAmounts[] = {20, 33};
    MoreInfoTable_t moreInfoTable[] = {
        {.name = _("wallet_phrase_20words"), .src = &imgArrowRight, .callBack = OpenImportShareHandler, &walletAmounts[0]},
        {.name = _("wallet_phrase_33words"), .src = &imgArrowRight, .callBack = OpenImportShareHandler, &walletAmounts[1]},
    };
    g_noticeWindow = GuiCreateMoreInfoHintBox(&imgClose, _("single_phrase_word_amount_select"), moreInfoTable, NUMBER_OF_ARRAYS(moreInfoTable), false, &g_noticeWindow);
}

static void GuiCreateBackupWidget(lv_obj_t *parent)
{
    lv_obj_t *label = GuiCreateScrollTitleLabel(parent, _("single_backup_choose_backup_title"));
    lv_obj_align(label, LV_ALIGN_DEFAULT, 36, 156 - GUI_MAIN_AREA_OFFSET);

    label = GuiCreateIllustrateLabel(parent, _("single_backup_choose_backup_desc"));
    lv_obj_set_style_text_opa(label, LV_OPA_60, LV_PART_MAIN);
    GuiAlignToPrevObj(label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 12);

    lv_obj_t *img = GuiCreateImg(parent, &imgSingleBackup);
    lv_obj_t *labelNotice = GuiCreateIllustrateLabel(parent, _("single_backup_single_phrase_desc"));
    lv_obj_set_width(labelNotice, 384);
    lv_obj_set_style_text_opa(labelNotice, LV_OPA_60, LV_PART_MAIN);

    lv_obj_t *imgArrow = GuiCreateImg(parent, &imgArrowRightO);
    GuiButton_t table[] = {
        {.obj = img, .align = LV_ALIGN_DEFAULT, .position = {24, 24}},
        {.obj = GuiCreateLabelWithFontAndTextColor(parent, _("single_backup_single_phrase_title"), g_defLittleTitleFont, 0xF5870A), .align = LV_ALIGN_DEFAULT, .position = {24, 84}},
        {.obj = imgArrow, .align = LV_ALIGN_DEFAULT, .position = {372, 86}},
        {.obj = labelNotice, .align = LV_ALIGN_DEFAULT, .position = {24, 132}}
    };
    lv_obj_t *button = GuiCreateButton(parent, 432, 216, table, NUMBER_OF_ARRAYS(table), OpenNoticeHandler, NULL);
    lv_obj_align_to(button, label, LV_ALIGN_OUT_BOTTOM_LEFT, -12, 20);

    lv_obj_t *line = GuiCreateDividerLine(parent);
    lv_obj_align_to(line, button, LV_ALIGN_OUT_BOTTOM_LEFT, -24, 10);

    labelNotice = GuiCreateNoticeLabel(parent, _("single_backup_shamir_desc"));
    lv_obj_set_width(labelNotice, 384);
    GuiButton_t importTable[] = {
        {.obj = GuiCreateScrollLittleTitleLabel(parent, _("single_backup_shamir_title"), 350), .align = LV_ALIGN_DEFAULT, .position = {24, 24}},
        {.obj = labelNotice, .align = LV_ALIGN_DEFAULT, .position = {24, 72}},
        {.obj = GuiCreateImg(parent, &imgArrowRight), .align = LV_ALIGN_DEFAULT, .position = {372, 26}},
    };
    lv_obj_t *importButton = GuiCreateButton(parent, 432, 157, importTable, 3, OpenSecretShareHandler, NULL);
    lv_obj_align_to(importButton, button, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 20);

    lv_obj_t *obj = GuiCreateContainerWithParent(parent, 222, 30);
    lv_obj_align(obj, LV_ALIGN_BOTTOM_MID, 0, -54);

    img = GuiCreateImg(obj, &imgDiceGrey);
    lv_obj_set_style_size(img, 24, 24);
    lv_obj_align(img, LV_ALIGN_LEFT_MID, 0, 0);

    label = GuiCreateIllustrateLabel(obj, _("dice_rolls_entropy_hint"));
    lv_obj_align(label, LV_ALIGN_LEFT_MID, 32, 0);
    lv_obj_set_style_text_opa(label, LV_OPA_64, LV_PART_MAIN);
    lv_obj_add_flag(obj, LV_OBJ_FLAG_HIDDEN);

    g_createWalletTileView.diceRollsHint = obj;
}

static void GuiImportBackupWidget(lv_obj_t *parent)
{
    lv_obj_t *label = GuiCreateScrollTitleLabel(parent, _("import_wallet_choose_method"));
    lv_obj_align(label, LV_ALIGN_DEFAULT, 36, 156 - GUI_MAIN_AREA_OFFSET);

    label = GuiCreateIllustrateLabel(parent, _("import_wallet_single_backup_desc"));
    lv_obj_set_style_text_opa(label, LV_OPA_60, LV_PART_MAIN);
    GuiAlignToPrevObj(label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 12);

    lv_obj_t *img = GuiCreateImg(parent, &imgSingleBackup);
    lv_obj_t *labelNotice = GuiCreateIllustrateLabel(parent, _("import_wallet_single_phrase_desc"));
    lv_obj_set_style_text_opa(labelNotice, LV_OPA_60, LV_PART_MAIN);
    lv_obj_t *imgArrow = GuiCreateImg(parent, &imgArrowRightO);
    GuiButton_t table[] = {
        {.obj = img, .align = LV_ALIGN_DEFAULT, .position = {24, 24}},
        {.obj = GuiCreateLabelWithFontAndTextColor(parent, _("import_wallet_single_phrase"), g_defLittleTitleFont, 0xF5870A), .align = LV_ALIGN_DEFAULT, .position = {24, 84}},
        {.obj = imgArrow, .align = LV_ALIGN_DEFAULT, .position = {372, 86}},
        {.obj = labelNotice, .align = LV_ALIGN_DEFAULT, .position = {24, 132}}
    };
    lv_obj_t *button = GuiCreateButton(parent, 432, 216, table, NUMBER_OF_ARRAYS(table), ChooseWordsAmountHandler, NULL);
    lv_obj_align_to(button, label, LV_ALIGN_OUT_BOTTOM_LEFT, -12, 20);

    lv_obj_t *line = GuiCreateDividerLine(parent);
    lv_obj_align_to(line, button, LV_ALIGN_OUT_BOTTOM_LEFT, -24, 10);

    labelNotice = GuiCreateNoticeLabel(parent, _("import_wallet_shamir_backup_desc"));
    lv_obj_set_width(labelNotice, 384);
    GuiButton_t importTable[] = {
        {.obj = GuiCreateScrollLittleTitleLabel(parent, _("import_wallet_shamir_backup"), 350), .align = LV_ALIGN_DEFAULT, .position = {24, 24}},
        {.obj = labelNotice, .align = LV_ALIGN_DEFAULT, .position = {24, 72}},
        {.obj = GuiCreateImg(parent, &imgArrowRight), .align = LV_ALIGN_DEFAULT, .position = {372, 26}},
    };

    lv_obj_t *importButton = GuiCreateButton(parent, 432, 156, importTable, 3, SelectImportShareHandler, NULL);
    lv_obj_align_to(importButton, button, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 20);
}

void GuiCreateWalletInit(uint8_t walletMethod)
{
    CLEAR_OBJECT(g_createWalletTileView);
    g_selectedEntropyMethod = ENTROPY_TYPE_STANDARD;

    g_pageWidget = CreatePageWidget();
    lv_obj_t *cont = g_pageWidget->contentZone;
    lv_obj_t *tileView = GuiCreateTileView(cont);
    lv_obj_t *tile = lv_tileview_add_tile(tileView, CREATE_WALLET_SETPIN, 0, LV_DIR_HOR);
    g_createWalletTileView.setPin = tile;
    GuiCreateSetpinWidget(tile);

    tile = lv_tileview_add_tile(tileView, CREATE_WALLET_REPEATPIN, 0, LV_DIR_HOR);
    g_createWalletTileView.repeatPin = tile;
    GuiCreateRepeatPinWidget(tile);

    tile = lv_tileview_add_tile(tileView, CREATE_WALLET_NAMEWALLET, 0, LV_DIR_HOR);
    g_createWalletTileView.nameWallet = tile;
    GuiCreateNameWalletWidget(tile);

    tile = lv_tileview_add_tile(tileView, CREATE_WALLET_BACKUPFROM, 0, LV_DIR_HOR);
    g_createWalletTileView.backupForm = tile;
    if (walletMethod == WALLET_METHOD_CREATE) {
        GuiCreateBackupWidget(tile);
    } else {
        GuiImportBackupWidget(tile);
    }

    g_createWalletTileView.currentTile = CREATE_WALLET_SETPIN;
    g_createWalletTileView.tileView = tileView;
    g_createWalletTileView.cont = cont;
    g_createWalletTileView.walletMethod = walletMethod;

    lv_obj_set_tile_id(g_createWalletTileView.tileView, g_createWalletTileView.currentTile, 0, LV_ANIM_OFF);
    lv_obj_clear_flag(tileView, LV_OBJ_FLAG_SCROLLABLE);
}

int8_t GuiCreateWalletNextTile(void)
{
    switch (g_createWalletTileView.currentTile) {
    case CREATE_WALLET_BACKUPFROM:
        if ((g_selectedEntropyMethod & ENTROPY_TYPE_MASK) == 0) {
            return GuiFrameOpenViewWithParam(&g_singlePhraseView, &g_selectedEntropyMethod, sizeof(g_selectedEntropyMethod));
        } else {
            uint8_t index = SEED_TYPE_BIP39;
            return GuiFrameOpenViewWithParam(&g_diceRollsView, &index, sizeof(index));
        }
        break;
    case CREATE_WALLET_NAMEWALLET:
        SecretCacheSetWalletIndex(GuiGetEmojiIconIndex());
        SecretCacheSetWalletName(GetCurrentKbWalletName());
        break;
    case CREATE_WALLET_SETPIN:
        if (g_repeatPassCode == NULL) {
            g_repeatPassCode = GuiCreateEnterPasscode(g_repeatPinTile, NULL, NULL,
                               g_setPassCode->mode + 2);
        }
        break;
    case CREATE_WALLET_REPEATPIN:
        GuiClearEnterProgressLabel();
        break;
    default:
        return SUCCESS_CODE;
    }
    g_createWalletTileView.currentTile++;
    GuiRefreshNavBar();
    lv_obj_set_tile_id(g_createWalletTileView.tileView, g_createWalletTileView.currentTile, 0, LV_ANIM_OFF);
    return SUCCESS_CODE;
}

int8_t GuiCreateWalletPrevTile(void)
{
    switch (g_createWalletTileView.currentTile) {
    case CREATE_WALLET_SETPIN:
        return SUCCESS_CODE;
    case CREATE_WALLET_NAMEWALLET:
        g_createWalletTileView.currentTile--;
    case CREATE_WALLET_REPEATPIN:
        if (g_repeatPassCode != NULL) {
            GuiDelEnterPasscode(g_repeatPassCode, NULL);
            g_repeatPassCode = NULL;
        }
        if (g_setPassCode != NULL) {
            GuiDelEnterPasscode(g_setPassCode, NULL);
            g_setPassCode = NULL;
        }
        g_setPassCode = GuiCreateEnterPasscode(g_setPinTile, NULL, NULL, ENTER_PASSCODE_SET_PIN);
        break;
    case CREATE_WALLET_BACKUPFROM:
        break;
    }

    g_createWalletTileView.currentTile--;
    GuiRefreshNavBar();
    lv_obj_set_tile_id(g_createWalletTileView.tileView, g_createWalletTileView.currentTile, 0, LV_ANIM_OFF);
    return SUCCESS_CODE;
}

void GuiCreateWalletSetPinPass(const char *buf)
{
    GuiEmitSignal(SIG_SETUP_VIEW_TILE_NEXT, NULL, 0);
    strcpy_s(g_pinBuf, PASSWORD_MAX_LEN, buf);
}

void GuiCreateWalletRepeatPinPass(const char *buf)
{
    if (!strcmp(buf, g_pinBuf)) {
        SecretCacheSetNewPassword((char *)buf);
        memset_s(g_pinBuf, sizeof(g_pinBuf), 0, sizeof(g_pinBuf));
        GuiEmitSignal(SIG_SETUP_VIEW_TILE_NEXT, NULL, 0);
    } else {
        UnlimitedVibrate(LONG);
        GuiEnterPassCodeStatus(g_repeatPassCode, false);
    }
}

void GuiCreateWalletNameUpdate(const void *src)
{
    if (g_nameWalletIcon != NULL) {
        lv_img_set_src(g_nameWalletIcon, src);
    }
}

void GuiCreateWalletDeInit(void)
{
    GUI_DEL_OBJ(g_noticeWindow)
    GUI_PAGE_DEL(g_changeEntropyPage)
    GuiDelEnterProgressLabel();
    if (g_setPassCode != NULL) {
        GuiDelEnterPasscode(g_setPassCode, NULL);
        g_setPassCode = NULL;
    }
    if (g_repeatPassCode != NULL) {
        GuiDelEnterPasscode(g_repeatPassCode, NULL);
        g_repeatPassCode = NULL;
    }

    g_createWalletTileView.currentTile = 0;
    CLEAR_OBJECT(g_createWalletTileView);
    ClearSecretCache();
    if (g_pageWidget != NULL) {
        DestroyPageWidget(g_pageWidget);
        g_pageWidget = NULL;
    }
}

static void GuiRefreshNavBar(void)
{
    SetNavBarLeftBtn(g_pageWidget->navBarWidget, NVS_BAR_RETURN, ReturnHandler, NULL);
    SetNavBarRightBtn(g_pageWidget->navBarWidget, NVS_RIGHT_BUTTON_BUTT, NULL, NULL);
    if (CREATE_WALLET_BACKUPFROM == g_createWalletTileView.currentTile) {
        //create wallet, show change entropy
        if (g_createWalletTileView.walletMethod == 0) {
            SetNavBarRightBtn(g_pageWidget->navBarWidget, NVS_BAR_MORE_INFO, OpenMoreHandler, NULL);
        }
        //import wallet, dont't show change entropy
        else {
            SetNavBarRightBtn(g_pageWidget->navBarWidget, NVS_BAR_QUESTION_MARK, QuestionMarkEventCb, NULL);
        }
    }
    if (CREATE_WALLET_SETPIN == g_createWalletTileView.currentTile) {
        SetNavBarLeftBtn(g_pageWidget->navBarWidget, NVS_BAR_RETURN, CloseCurrentViewHandler, NULL);
    }
    SetNavBarMidBtn(g_pageWidget->navBarWidget, NVS_MID_BUTTON_BUTT, NULL, NULL);
}

void GuiCreateWalletRefresh(void)
{
    GuiRefreshNavBar();
    if (CREATE_WALLET_SETPIN == g_createWalletTileView.currentTile) {
        if (g_setPassCode != NULL) {
            GuiUpdateEnterPasscodeParam(g_setPassCode, NULL);
        }
    }
    if (CREATE_WALLET_REPEATPIN == g_createWalletTileView.currentTile) {
        if (g_repeatPassCode != NULL) {
            GuiUpdateEnterPasscodeParam(g_repeatPassCode, NULL);
        }
    }
}

static void OpenMoreHandler(lv_event_t *e)
{
    int hintboxHeight = 312;
    g_openMoreHintBox = GuiCreateHintBox(hintboxHeight);
    lv_obj_add_event_cb(lv_obj_get_child(g_openMoreHintBox, 0), CloseHintBoxHandler, LV_EVENT_CLICKED, &g_openMoreHintBox);
    lv_obj_t *label = GuiCreateTextLabel(g_openMoreHintBox, _("Tutorial"));
    lv_obj_t *img = GuiCreateImg(g_openMoreHintBox, &imgTutorial);

    GuiButton_t table[] = {
        {
            .obj = img,
            .align = LV_ALIGN_LEFT_MID,
            .position = {24, 0},
        },
        {
            .obj = label,
            .align = LV_ALIGN_LEFT_MID,
            .position = {76, 0},
        },
    };
    lv_obj_t *btn = GuiCreateButton(g_openMoreHintBox, 456, 84, table, NUMBER_OF_ARRAYS(table),
                                    QuestionMarkEventCb, NULL);
    lv_obj_align(btn, LV_ALIGN_BOTTOM_MID, 0, -24);

    label = GuiCreateTextLabel(g_openMoreHintBox, _("generate_ton_mnenonic"));
    img = GuiCreateImg(g_openMoreHintBox, &imgTonPhrase);
    table[0].obj = img;
    table[1].obj = label;
    btn = GuiCreateButton(g_openMoreHintBox, 456, 84, table, NUMBER_OF_ARRAYS(table),
                          TonPhraseButtonHandler, NULL);
    lv_obj_align(btn, LV_ALIGN_BOTTOM_MID, 0, -120);

    label = GuiCreateTextLabel(g_openMoreHintBox, _("change_entropy"));
    img = GuiCreateImg(g_openMoreHintBox, &imgDice);
    table[0].obj = img;
    table[1].obj = label;
    btn = GuiCreateButton(g_openMoreHintBox, 456, 84, table, NUMBER_OF_ARRAYS(table),
                          OpenChangeEntropyHandler, NULL);
    lv_obj_align(btn, LV_ALIGN_BOTTOM_MID, 0, -216);
}

// Change Entropy
typedef struct {
    lv_obj_t *checkBox;
    lv_obj_t *checkedImg;
    lv_obj_t *uncheckedImg;
    lv_obj_t *descCont;
} MethodWidgetItem_t;

static MethodWidgetItem_t g_entropyMethods[2];
static uint8_t g_selectedEntropyMethodCache;
static lv_obj_t *g_entropyMethodContainer;

static void ChangeEntropyMethodHandler(lv_event_t *e);
static void ChangeEntropyMethodConfirmHandler(lv_event_t *e);

static void CreateChangeEntropyView(void)
{
    uint16_t height = 0;
    g_selectedEntropyMethodCache = g_selectedEntropyMethod;
    g_changeEntropyPage = CreatePageWidget();
    SetNavBarLeftBtn(g_changeEntropyPage->navBarWidget, NVS_BAR_RETURN, CloseChangeEntropyHandler, NULL);
    SetMidBtnLabel(g_changeEntropyPage->navBarWidget, NVS_BAR_MID_LABEL, _("change_entropy"));
    SetNavBarRightBtn(g_changeEntropyPage->navBarWidget, NVS_BAR_QUESTION_MARK, OpenChangeEntropyTutorialHandler, NULL);
    lv_obj_t *contentZone = g_changeEntropyPage->contentZone;
    lv_obj_t *cont = GuiCreateContainerWithParent(contentZone, 480, 542);
    g_entropyMethodContainer = cont;
    lv_obj_align(cont, LV_ALIGN_TOP_LEFT, 0, 0);

    lv_obj_t *label;
    label = GuiCreateIllustrateLabel(cont, _("change_entropy_desc"));
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 36, 0);
    lv_obj_t *method_cont = GuiCreateWhiteOpa12Container(cont, 408, 205);
    lv_obj_align(method_cont, LV_ALIGN_TOP_LEFT, 36, 84);
    label = GuiCreateTextLabel(method_cont, _("change_entropy_system"));
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 24, 16);
    label = GuiCreateScrollIllustrateLabel(method_cont, _("change_entropy_system_subtitle"), 270);
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 24, 56);
    lv_obj_set_style_text_opa(label, LV_OPA_64, LV_PART_MAIN);

    static lv_point_t points[2] = {{0, 0}, {360, 0}};
    lv_obj_t *line = GuiCreateLine(method_cont, points, 2);
    lv_obj_align(line, LV_ALIGN_TOP_LEFT, 24, 102);

    label = GuiCreateTextLabel(method_cont, _("change_entropy_dice_rolls"));
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 24, 16 + 102);
    label = GuiCreateScrollIllustrateLabel(method_cont, _("change_entropy_dice_rolls_subtitle"), 270);
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 24, 56 + 102);
    lv_obj_set_style_text_opa(label, LV_OPA_64, LV_PART_MAIN);
    lv_obj_refr_size(label);
    height = 205 - 30 + lv_obj_get_self_height(label);
    lv_obj_set_height(method_cont, height);

    for (size_t i = 0; i < 2; i++) {
        g_entropyMethods[i].checkBox = GuiCreateSelectPathCheckBox(method_cont);
        lv_obj_align(g_entropyMethods[i].checkBox, LV_ALIGN_TOP_LEFT, 0, 10 + 102 * i);
        lv_obj_add_event_cb(g_entropyMethods[i].checkBox, ChangeEntropyMethodHandler, LV_EVENT_CLICKED, NULL);

        g_entropyMethods[i].checkedImg = GuiCreateImg(g_entropyMethods[i].checkBox, &imgMessageSelect);
        lv_obj_align(g_entropyMethods[i].checkedImg, LV_ALIGN_CENTER, 162, 0);
        lv_obj_add_flag(g_entropyMethods[i].checkedImg, LV_OBJ_FLAG_HIDDEN);
        g_entropyMethods[i].uncheckedImg = GuiCreateImg(g_entropyMethods[i].checkBox, &imgUncheckCircle);
        lv_obj_align(g_entropyMethods[i].uncheckedImg, LV_ALIGN_CENTER, 162, 0);
        lv_obj_clear_flag(g_entropyMethods[i].uncheckedImg, LV_OBJ_FLAG_HIDDEN);
    }

    // System Desc Container
    lv_obj_t *descCont = GuiCreateContainerWithParent(cont, 408, 114);
    g_entropyMethods[0].descCont = descCont;
    lv_obj_align_to(descCont, method_cont, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 24);
    lv_obj_set_style_radius(descCont, 24, LV_PART_MAIN);
    lv_obj_set_style_bg_color(descCont, WHITE_COLOR, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(descCont, LV_OPA_12, LV_PART_MAIN);

    label = GuiCreateIllustrateLabel(descCont, _("change_entropy_system_desc"));
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 24, 12);
    lv_obj_set_width(label, 360);
    lv_obj_set_style_text_opa(label, LV_OPA_64, LV_PART_MAIN);

    lv_obj_add_flag(descCont, LV_OBJ_FLAG_HIDDEN);

    // Dice Roll Desc Container
    descCont = GuiCreateWhiteOpa12Container(cont, 408, 200);
    g_entropyMethods[1].descCont = descCont;
    lv_obj_align_to(descCont, method_cont, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 24);

    label = GuiCreateIllustrateLabel(descCont, _("change_entropy_dice_desc"));
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 24, 12);
    lv_obj_set_width(label, 360);
    lv_obj_set_style_text_opa(label, LV_OPA_64, LV_PART_MAIN);

    lv_obj_t *dot = GuiCreateIllustrateLabel(descCont, "·");
    lv_obj_set_style_text_color(dot, ORANGE_COLOR, LV_PART_MAIN);
    GuiAlignToPrevObj(dot, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 16);

    label = GuiCreateIllustrateLabel(descCont, _("change_entropy_dice_detail_desc_1"));
    lv_label_set_recolor(label, true);
    GuiAlignToPrevObj(label, LV_ALIGN_OUT_RIGHT_TOP, 10, 0);
    lv_obj_set_width(label, 344);
    height = lv_obj_get_self_height(label) + 12;

    label = GuiCreateIllustrateLabel(descCont, "·");
    lv_obj_set_style_text_color(label, ORANGE_COLOR, LV_PART_MAIN);
    lv_obj_align_to(label, dot, LV_ALIGN_OUT_BOTTOM_LEFT, 0, height);

    label = GuiCreateIllustrateLabel(descCont, _("change_entropy_dice_detail_desc_2"));
    GuiAlignToPrevObj(label, LV_ALIGN_OUT_RIGHT_TOP, 10, 0);
    lv_obj_set_width(label, 344);
    lv_obj_refr_size(label);
    lv_obj_set_style_pad_bottom(label, 12, LV_STATE_DEFAULT);

    lv_obj_add_flag(descCont, LV_OBJ_FLAG_HIDDEN | LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(g_entropyMethods[g_selectedEntropyMethodCache].checkedImg, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(g_entropyMethods[g_selectedEntropyMethodCache].uncheckedImg, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(g_entropyMethods[g_selectedEntropyMethodCache].descCont, LV_OBJ_FLAG_HIDDEN);

    // Check Btn
    lv_obj_t *bottomCont = GuiCreateContainerWithParent(contentZone, 480, 114);
    lv_obj_align(bottomCont, LV_ALIGN_BOTTOM_LEFT, 0, 0);
    lv_obj_t *btn = GuiCreateBtn(bottomCont, USR_SYMBOL_CHECK);
    lv_obj_add_event_cb(btn, ChangeEntropyMethodConfirmHandler, LV_EVENT_CLICKED, NULL);
    lv_obj_align(btn, LV_ALIGN_BOTTOM_RIGHT, -36, -24);
}

static void OpenChangeEntropyHandler(lv_event_t *e)
{
    GUI_DEL_OBJ(g_openMoreHintBox);
    CreateChangeEntropyView();
}

static void TonPhraseButtonHandler(lv_event_t *e)
{
    GUI_DEL_OBJ(g_openMoreHintBox);
    GuiFrameOpenView(&g_tonMnemonicHintView);
}

static void CloseChangeEntropyHandler(lv_event_t *e)
{
    GUI_PAGE_DEL(g_changeEntropyPage);
}

static void ChangeEntropyMethodConfirmHandler(lv_event_t *e)
{
    GUI_PAGE_DEL(g_changeEntropyPage);
    g_selectedEntropyMethod = g_selectedEntropyMethodCache;
    if ((g_selectedEntropyMethod & ENTROPY_TYPE_MASK) == 1) {
        if (lv_obj_has_flag(g_createWalletTileView.diceRollsHint, LV_OBJ_FLAG_HIDDEN)) {
            lv_obj_clear_flag(g_createWalletTileView.diceRollsHint, LV_OBJ_FLAG_HIDDEN);
        }
    } else {
        if (!lv_obj_has_flag(g_createWalletTileView.diceRollsHint, LV_OBJ_FLAG_HIDDEN)) {
            lv_obj_add_flag(g_createWalletTileView.diceRollsHint, LV_OBJ_FLAG_HIDDEN);
        }
    }
}

static void ChangeEntropyMethodHandler(lv_event_t *e)
{
    lv_obj_scroll_to_y(g_entropyMethodContainer, 0, LV_ANIM_OFF);
    lv_obj_t *checkBox = lv_event_get_target(e);
    for (uint32_t i = 0; i < 2; i++) {
        if (checkBox == g_entropyMethods[i].checkBox) {
            lv_obj_add_state(g_entropyMethods[i].checkBox, LV_STATE_CHECKED);
            lv_obj_clear_flag(g_entropyMethods[i].checkedImg, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(g_entropyMethods[i].uncheckedImg, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(g_entropyMethods[i].descCont, LV_OBJ_FLAG_HIDDEN);
            if (g_selectedEntropyMethodCache != i) {
                g_selectedEntropyMethodCache = i;
            }
        } else {
            lv_obj_clear_state(g_entropyMethods[i].checkBox, LV_STATE_CHECKED);
            lv_obj_add_flag(g_entropyMethods[i].checkedImg, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clear_flag(g_entropyMethods[i].uncheckedImg, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(g_entropyMethods[i].descCont, LV_OBJ_FLAG_HIDDEN);
        }
    }
}

static void OpenChangeEntropyTutorialHandler(lv_event_t *e)
{
    uint8_t index = TUTORIAL_CHANGE_ENTROPY;
    GuiFrameOpenViewWithParam(&g_tutorialView, &index, sizeof(index));
}