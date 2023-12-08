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
#ifndef COMPILE_SIMULATOR
#include "safe_mem_lib.h"
#else
#define memset_s(p, s, c, l) memset(p, c, l)
#endif

typedef enum {
    CREATE_WALLET_SETPIN = 0,
    CREATE_WALLET_REPEATPIN,
    CREATE_WALLET_NAMEWALLET,
    CREATE_WALLET_BACKUPFROM,

    CREATE_WALLET_BUTT,
} CREATE_WALLET_ENUM;

typedef struct CreateWalletWidget {
    uint8_t     currentTile;
    uint8_t     walletMethod;
    lv_obj_t    *cont;
    lv_obj_t    *tileView;
    lv_obj_t    *instructions;
    lv_obj_t    *setPin;
    lv_obj_t    *repeatPin;
    lv_obj_t    *nameWallet;
    lv_obj_t    *backupForm;
} CreateWalletWidget_t;
static CreateWalletWidget_t g_createWalletTileView;

static void CloseChooseWordsAmountHandler(lv_event_t* e);
static void GuiRefreshNavBar(void);

static PageWidget_t *g_pageWidget;
static KeyBoard_t *g_nameWalletKb = NULL;
static lv_obj_t *g_nameWalletIcon = NULL;
static lv_obj_t *g_wordsAmountView = NULL;
static GuiEnterPasscodeItem_t *g_setPassCode = NULL;
static GuiEnterPasscodeItem_t *g_repeatPassCode = NULL;
static lv_obj_t *g_setPinTile = NULL;
static lv_obj_t *g_repeatPinTile = NULL;
static lv_obj_t *g_noticeHintBox = NULL;
static char g_pinBuf[GUI_DEFINE_MAX_PASSCODE_LEN + 1];

void GuiSetupKeyboardWidgetMode(void)
{
    printf("g_setPassCode->mode = %d\n", g_setPassCode->mode % 2);
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

static void UpdateWalletNameIconHandler(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_READY) {
        GuiEmitSignal(SIG_SETUP_VIEW_TILE_NEXT, NULL, 0);
    }
}

const char *GetCurrentKbWalletName(void)
{
    if (g_nameWalletKb != NULL) {
        return lv_textarea_get_text(g_nameWalletKb->ta);
    }
    return "";
}

static void QuestionMarkEventCb(lv_event_t* e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        uint8_t index = TUTORIAL_SHAMIR_BACKUP;
        GuiFrameOpenViewWithParam(&g_tutorialView, &index, sizeof(index));
    }
}

static void OpenEmojiKbHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_CLICKED) {
        GuiCreateEmojiKeyBoard(lv_scr_act(), g_nameWalletIcon);
    }
}

static void GuiCreateNameWalletWidget(lv_obj_t *parent)
{
    lv_obj_t *label = GuiCreateTitleLabel(parent, _("single_backup_namewallet_title"));
    lv_obj_align(label, LV_ALIGN_DEFAULT, 36, 156 - GUI_MAIN_AREA_OFFSET);

    label = GuiCreateIllustrateLabel(parent, _("single_backup_namewallet_desc"));
    lv_obj_set_style_text_opa(label, LV_OPA_60, LV_PART_MAIN);
    lv_obj_align(label, LV_ALIGN_DEFAULT, 36, 216 - GUI_MAIN_AREA_OFFSET);

    GuiSetEmojiIconIndex(GUI_KEYBOARD_EMOJI_NEW_INDEX);
    g_nameWalletKb = GuiCreateFullKeyBoard(parent, UpdateWalletNameIconHandler, KEY_STONE_FULL_L, NULL);
    GuiSetKeyBoardMinTaLen(g_nameWalletKb, 0);
    lv_obj_set_size(g_nameWalletKb->ta, 300, 60);
    lv_obj_set_style_text_opa(g_nameWalletKb->ta, LV_OPA_100, 0);
    lv_obj_align(g_nameWalletKb->ta, LV_ALIGN_DEFAULT, 128, 316 - GUI_MAIN_AREA_OFFSET);
    lv_textarea_set_placeholder_text(g_nameWalletKb->ta, "Wallet Name");
    lv_textarea_set_max_length(g_nameWalletKb->ta, 16);
    lv_textarea_set_one_line(g_nameWalletKb->ta, true);
    char tempBuf[16] = {0};
    sprintf(tempBuf, "%d/16", strlen(GuiNvsBarGetWalletName()));
    lv_obj_t *progresslabel = GuiCreateNoticeLabel(parent, tempBuf);
    lv_obj_align(progresslabel, LV_ALIGN_TOP_RIGHT, -36, 384 - GUI_MAIN_AREA_OFFSET);
    GuiSetEnterProgressLabel(progresslabel);

    lv_obj_t *img = GuiCreateImg(parent, &emojiBitcoin);
    lv_obj_t *arrowDownImg = GuiCreateImg(parent, &imgArrowDownS);
    g_nameWalletIcon = img;
    GuiButton_t table[] = {
        {.obj = img, .align = LV_ALIGN_LEFT_MID, .position = {15, 0},},
        {.obj = arrowDownImg, .align = LV_ALIGN_LEFT_MID, .position = {59, 0},},
    };
    lv_obj_t *button = GuiCreateButton(parent, 100, 70, table, NUMBER_OF_ARRAYS(table), OpenEmojiKbHandler, NULL);
    lv_obj_align(button, LV_ALIGN_DEFAULT, 24, 312 - GUI_MAIN_AREA_OFFSET);
    lv_obj_set_style_radius(button, 16, LV_PART_MAIN);

    lv_obj_t *line = GuiCreateDividerLine(parent);
    lv_obj_align(line, LV_ALIGN_DEFAULT, 0, 380 - GUI_MAIN_AREA_OFFSET);
}

static void OpenNoticeHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_CLICKED) {
        g_noticeHintBox = GuiCreateHintBox(lv_scr_act(), 480, 518, true);
        lv_obj_add_event_cb(lv_obj_get_child(g_noticeHintBox, 0), CloseHintBoxHandler, LV_EVENT_CLICKED, &g_noticeHintBox);
        lv_obj_t *img = GuiCreateImg(g_noticeHintBox, &imgRedEye);
        lv_obj_align(img, LV_ALIGN_DEFAULT, 36, 330);
        lv_obj_t *label = GuiCreateLittleTitleLabel(g_noticeHintBox, _("single_backup_notice_title"));
        lv_obj_align(label, LV_ALIGN_DEFAULT, 36, 420);

        img = GuiCreateImg(g_noticeHintBox, &imgClose);
        lv_obj_add_flag(img, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_add_event_cb(img, CloseHintBoxHandler, LV_EVENT_CLICKED, &g_noticeHintBox);
        lv_obj_align(img, LV_ALIGN_BOTTOM_RIGHT, -36, -455);

        label = GuiCreateIllustrateLabel(g_noticeHintBox, _("single_backup_notice_desc1"));
        lv_obj_align(label, LV_ALIGN_DEFAULT, 36, 478);

        label = GuiCreateIllustrateLabel(g_noticeHintBox, _("single_backup_notice_desc2"));
        lv_obj_align(label, LV_ALIGN_DEFAULT, 36, 550);

        lv_obj_t *btn = GuiCreateBtn(g_noticeHintBox, USR_SYMBOL_CHECK);
        lv_obj_add_event_cb(btn, CloseParentAndNextHandler, LV_EVENT_CLICKED, &g_noticeHintBox);
        lv_obj_align(btn, LV_ALIGN_DEFAULT, 348, 710);
    }
}

static void OpenSecretShareHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_CLICKED) {
        GuiFrameOpenView(&g_createShareView);
    }
}

static void OpenImportPhraseHandler(lv_event_t* e)
{
    lv_event_code_t code = lv_event_get_code(e);
    uint8_t *wordsAmount = lv_event_get_user_data(e);
    if (code == LV_EVENT_CLICKED) {
        // todo
        Vibrate(SLIGHT);
        lv_obj_add_flag(lv_obj_get_parent(lv_event_get_target(e)), LV_OBJ_FLAG_HIDDEN);
        GuiFrameOpenViewWithParam(&g_importPhraseView, wordsAmount, sizeof(*wordsAmount));
    }
}

static void ChooseWordsAmountHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_CLICKED) {
        static uint8_t wordsAmount12 = 12;
        static uint8_t wordsAmount18 = 18;
        static uint8_t wordsAmount24 = 24;
        g_wordsAmountView = GuiCreateHintBox(lv_scr_act(), 480, 378, true);
        lv_obj_add_event_cb(lv_obj_get_child(g_wordsAmountView, 0), CloseHintBoxHandler, LV_EVENT_CLICKED, &g_wordsAmountView);
        lv_obj_t *label = GuiCreateIllustrateLabel(g_wordsAmountView, _("import_wallet_phrase_words_title"));
        lv_obj_align(label, LV_ALIGN_DEFAULT, 36, 460);
        lv_obj_set_style_text_opa(label, LV_OPA_60, LV_PART_MAIN);
        lv_obj_t *img = GuiCreateImg(g_wordsAmountView, &imgClose);
        GuiButton_t tableHintbox = {img, .position = {10, 10} };
        lv_obj_t *buttonClose = GuiCreateButton(g_wordsAmountView, 40, 40, &tableHintbox, 1, CloseChooseWordsAmountHandler, g_wordsAmountView);
        lv_obj_align(buttonClose, LV_ALIGN_DEFAULT, 407, 450);

        label = GuiCreateTextLabel(g_wordsAmountView, _("import_wallet_phrase_24words"));
        img = GuiCreateImg(g_wordsAmountView, &imgArrowRight);
        GuiButton_t next24[2] = {
            {.obj = img, .position = {423, 20},},
            {.obj = label, .position = {36, 20},},
        };
        lv_obj_t *nextbutton24 = GuiCreateButton(g_wordsAmountView, 480, 110, next24, NUMBER_OF_ARRAYS(next24), OpenImportPhraseHandler, &wordsAmount24);
        lv_obj_align(nextbutton24, LV_ALIGN_DEFAULT, 0, 860 - GUI_MAIN_AREA_OFFSET);

        label = GuiCreateTextLabel(g_wordsAmountView, _("import_wallet_phrase_18words"));
        img = GuiCreateImg(g_wordsAmountView, &imgArrowRight);
        GuiButton_t next18[2] = {
            {.obj = img, .position = {423, 20},},
            {.obj = label, .position = {36, 20},},
        };
        lv_obj_t *nextbutton18 = GuiCreateButton(g_wordsAmountView, 480, 110, next18, NUMBER_OF_ARRAYS(next18), OpenImportPhraseHandler, &wordsAmount18);
        lv_obj_align(nextbutton18, LV_ALIGN_DEFAULT, 0, 760 - GUI_MAIN_AREA_OFFSET);

        label = GuiCreateTextLabel(g_wordsAmountView, _("import_wallet_phrase_12words"));
        img = GuiCreateImg(g_wordsAmountView, &imgArrowRight);
        GuiButton_t next12[2] = {
            {.obj = img, .position = {423, 20},},
            {.obj = label, .position = {36, 20},},
        };
        lv_obj_t *nextbutton12 = GuiCreateButton(g_wordsAmountView, 480, 110, next12, NUMBER_OF_ARRAYS(next12), OpenImportPhraseHandler, &wordsAmount12);
        lv_obj_align(nextbutton12, LV_ALIGN_DEFAULT, 0, 660 - GUI_MAIN_AREA_OFFSET);
    }
}

static void CloseChooseWordsAmountHandler(lv_event_t* e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_CLICKED) {
        GUI_DEL_OBJ(g_wordsAmountView)
    }
}

static void OpenImportShareHandler(lv_event_t* e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        Vibrate(SLIGHT);
        lv_obj_del(lv_obj_get_parent(lv_event_get_target(e)));
        g_noticeHintBox = NULL;
        GuiFrameOpenViewWithParam(&g_importShareView, (uint8_t *)lv_event_get_user_data(e), sizeof(uint8_t));
    }
}

static void SelectImportShareHandler(lv_event_t* e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        static uint8_t wordsAmount[] = {33, 20};
        g_noticeHintBox = GuiCreateHintBox(lv_scr_act(), 480, 282, true);
        lv_obj_add_event_cb(lv_obj_get_child(g_noticeHintBox, 0), CloseHintBoxHandler, LV_EVENT_CLICKED, &g_noticeHintBox);
        lv_obj_t *label = GuiCreateNoticeLabel(g_noticeHintBox, _("import_wallet_phrase_words_title"));
        lv_obj_align(label, LV_ALIGN_BOTTOM_LEFT, 36, -222);

        lv_obj_t *img = GuiCreateImg(g_noticeHintBox, &imgClose);
        lv_obj_add_flag(img, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_add_event_cb(img, CloseCurrentParentHandler, LV_EVENT_CLICKED, NULL);
        lv_obj_align(img, LV_ALIGN_BOTTOM_RIGHT, -36, -222);

        for (int i = 0; i < 2; i++) {
            GuiButton_t table[] = {
                {.obj = GuiCreateTextLabel(g_noticeHintBox, _("import_wallet_ssb_33words")), .align = LV_ALIGN_LEFT_MID, .position = {24, 0},},
                {.obj = GuiCreateImg(g_noticeHintBox, &imgArrowRight), .align = LV_ALIGN_RIGHT_MID, .position = {-24, 0},},
            };
            if (i == 1) {
                lv_label_set_text(table[0].obj, _("import_wallet_ssb_20words"));
            }
            lv_obj_t *button = GuiCreateButton(g_noticeHintBox, 456, 84, table, NUMBER_OF_ARRAYS(table), OpenImportShareHandler, &wordsAmount[i]);
            lv_obj_align(button, LV_ALIGN_BOTTOM_MID, 0, -12 - 96 * i);
        }
    }
}

static void GuiCreateBackupWidget(lv_obj_t *parent)
{
    lv_obj_t *label = GuiCreateTitleLabel(parent, _("single_backup_choose_backup_title"));
    lv_obj_align(label, LV_ALIGN_DEFAULT, 36, 156 - GUI_MAIN_AREA_OFFSET);

    label = GuiCreateIllustrateLabel(parent, _("single_backup_choose_backup_desc"));
    lv_obj_set_style_text_opa(label, LV_OPA_60, LV_PART_MAIN);
    lv_obj_align(label, LV_ALIGN_DEFAULT, 36, 216 - GUI_MAIN_AREA_OFFSET);

    lv_obj_t *img = GuiCreateImg(parent, &imgSingleBackup);

    label = GuiCreateLittleTitleLabel(parent, _("single_backup_single_phrase_title"));
    lv_obj_set_style_text_color(label, ORANGE_COLOR, LV_PART_MAIN);

    lv_obj_t *labelNotice = GuiCreateIllustrateLabel(parent, _("single_backup_single_phrase_desc"));
    lv_obj_set_width(labelNotice, 384);
    lv_obj_set_style_text_opa(labelNotice, LV_OPA_60, LV_PART_MAIN);

    lv_obj_t *imgArrow = GuiCreateImg(parent, &imgArrowRightO);

    GuiButton_t table[] = {
        {.obj = img, .align = LV_ALIGN_DEFAULT, .position = {24, 24},},
        {.obj = label, .align = LV_ALIGN_DEFAULT, .position = {24, 84},},
        {.obj = imgArrow, .align = LV_ALIGN_DEFAULT, .position = {372, 86},},
        {.obj = labelNotice, .align = LV_ALIGN_DEFAULT, .position = {24, 132},},
    };
    lv_obj_t *button = GuiCreateButton(parent, 432, 216, table, NUMBER_OF_ARRAYS(table), OpenNoticeHandler, NULL);
    lv_obj_align(button, LV_ALIGN_TOP_MID, 0, 300 - GUI_MAIN_AREA_OFFSET);

    label = GuiCreateLittleTitleLabel(parent, _("single_backup_shamir_title"));
    lv_obj_align(label, LV_ALIGN_DEFAULT, 36, 510 - GUI_MAIN_AREA_OFFSET);

    lv_obj_t *line = GuiCreateDividerLine(parent);
    lv_obj_align(line, LV_ALIGN_DEFAULT, 0, 516 - GUI_MAIN_AREA_OFFSET);

    labelNotice = GuiCreateNoticeLabel(parent, _("single_backup_shamir_desc"));
    lv_obj_align(labelNotice, LV_ALIGN_DEFAULT, 36, 604 - GUI_MAIN_AREA_OFFSET);
    lv_obj_set_width(labelNotice, 384);
    imgArrow = GuiCreateImg(parent, &imgArrowRight);
    table[0].obj = label;
    table[0].position.x = 24;
    table[0].position.y = 24;
    table[1].obj = labelNotice;
    table[1].position.x = 24;
    table[1].position.y = 72;
    table[2].obj = imgArrow;
    table[2].position.x = 372;
    table[2].position.y = 26;
    button = GuiCreateButton(parent, 432, 157, table, 3, OpenSecretShareHandler, NULL);
    lv_obj_align(button, LV_ALIGN_TOP_MID, 0, 516 - GUI_MAIN_AREA_OFFSET);
}

static void GuiImportBackupWidget(lv_obj_t *parent)
{
    lv_obj_t *label = GuiCreateTitleLabel(parent, _("import_wallet_choose_method"));
    lv_obj_align(label, LV_ALIGN_DEFAULT, 36, 156 - GUI_MAIN_AREA_OFFSET);

    label = GuiCreateIllustrateLabel(parent, _("import_wallet_single_backup_desc"));
    lv_obj_set_style_text_opa(label, LV_OPA_60, LV_PART_MAIN);
    lv_obj_align(label, LV_ALIGN_DEFAULT, 36, 216 - GUI_MAIN_AREA_OFFSET);

    lv_obj_t *img = GuiCreateImg(parent, &imgSingleBackup);

    label = GuiCreateLittleTitleLabel(parent, _("import_wallet_single_phrase"));
    lv_obj_set_style_text_color(label, ORANGE_COLOR, LV_PART_MAIN);

    lv_obj_t *labelNotice = GuiCreateIllustrateLabel(parent, _("import_wallet_single_phrase_desc"));
    lv_obj_set_style_text_opa(labelNotice, LV_OPA_60, LV_PART_MAIN);

    lv_obj_t *imgArrow = GuiCreateImg(parent, &imgArrowRightO);

    GuiButton_t table[4] = {
        {.obj = img, .align = LV_ALIGN_DEFAULT, .position = {24, 24},},
        {.obj = label, .align = LV_ALIGN_DEFAULT, .position = {24, 84},},
        {.obj = imgArrow, .align = LV_ALIGN_DEFAULT, .position = {372, 86},},
        {.obj = labelNotice, .align = LV_ALIGN_DEFAULT, .position = {24, 132},},
    };
    lv_obj_t *button = GuiCreateButton(parent, 432, 216, table, NUMBER_OF_ARRAYS(table), ChooseWordsAmountHandler, NULL);
    lv_obj_align(button, LV_ALIGN_DEFAULT, 24, 330 - GUI_MAIN_AREA_OFFSET);

    lv_obj_t *line = GuiCreateDividerLine(parent);
    lv_obj_align(line, LV_ALIGN_DEFAULT, 0, 545 - GUI_MAIN_AREA_OFFSET);

    label = GuiCreateLittleTitleLabel(parent, _("import_wallet_shamir_backup"));
    imgArrow = GuiCreateImg(parent, &imgArrowRight);
    labelNotice = GuiCreateNoticeLabel(parent, _("import_wallet_shamir_backup_desc"));
    table[0].obj = label;
    table[0].position.x = 24;
    table[0].position.y = 24;
    table[1].obj = labelNotice;
    table[1].position.x = 24;
    table[1].position.y = 72;
    table[2].obj = imgArrow;
    table[2].position.x = 372;
    table[2].position.y = 26;
    button = GuiCreateButton(parent, 432, 156, table, 3, SelectImportShareHandler, NULL);
    lv_obj_align(button, LV_ALIGN_DEFAULT, 24, 546 - GUI_MAIN_AREA_OFFSET);
}

void GuiCreateWalletInit(uint8_t walletMethod)
{
    CLEAR_OBJECT(g_createWalletTileView);

    g_pageWidget = CreatePageWidget();
    lv_obj_t *cont = g_pageWidget->contentZone;

    lv_obj_t *tileView = lv_tileview_create(cont);
    if (GuiDarkMode()) {
        lv_obj_set_style_bg_color(tileView, BLACK_COLOR, LV_PART_MAIN);
    } else {
        lv_obj_set_style_bg_color(tileView, WHITE_COLOR, LV_PART_MAIN);
    }
    lv_obj_set_style_bg_opa(tileView, LV_OPA_0, LV_PART_SCROLLBAR | LV_STATE_SCROLLED);
    lv_obj_set_style_bg_opa(tileView, LV_OPA_0, LV_PART_SCROLLBAR | LV_STATE_DEFAULT);
    lv_obj_t *tile  = lv_tileview_add_tile(tileView, CREATE_WALLET_SETPIN, 0, LV_DIR_HOR);
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
        return GuiFrameOpenView(&g_singlePhraseView);
    case CREATE_WALLET_NAMEWALLET:
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

void GuiCreateWalletSetPinPass(const char* buf)
{
    GuiEmitSignal(SIG_SETUP_VIEW_TILE_NEXT, NULL, 0);
    strcpy(g_pinBuf, buf);
}

void GuiCreateWalletRepeatPinPass(const char* buf)
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

void GuiCreateWalletNameUpdate(const void * src)
{
    if (g_nameWalletIcon != NULL) {
        lv_img_set_src(g_nameWalletIcon, src);
    }
}

void GuiCreateWalletDeInit(void)
{
    GUI_DEL_OBJ(g_noticeHintBox)
    GUI_DEL_OBJ(g_wordsAmountView)
    GuiDelEnterProgressLabel();

    if (g_setPassCode != NULL) {
        GuiDelEnterPasscode(g_setPassCode, NULL);
        g_setPassCode = NULL;
    }
    if (g_repeatPassCode != NULL) {
        GuiDelEnterPasscode(g_repeatPassCode, NULL);
        g_repeatPassCode = NULL;
    }

    GuiDeleteKeyBoard(g_nameWalletKb);
    lv_obj_del(g_createWalletTileView.tileView);
    lv_obj_del(g_createWalletTileView.cont);
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
        SetNavBarRightBtn(g_pageWidget->navBarWidget, NVS_BAR_QUESTION_MARK, QuestionMarkEventCb, NULL);
    }
    if (CREATE_WALLET_SETPIN == g_createWalletTileView.currentTile) {
        SetNavBarLeftBtn(g_pageWidget->navBarWidget, NVS_BAR_RETURN, CloseCurrentViewHandler, NULL);
    }
    if (CREATE_WALLET_BACKUPFROM == g_createWalletTileView.currentTile) {
        SetNavBarRightBtn(g_pageWidget->navBarWidget, NVS_BAR_QUESTION_MARK, QuestionMarkEventCb, NULL);
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

