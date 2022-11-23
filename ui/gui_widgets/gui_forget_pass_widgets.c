#include "gui.h"
#include "gui_obj.h"
#include "gui_views.h"
#include "gui_status_bar.h"
#include "gui_keyboard.h"
#include "gui_button.h"
#include "gui_hintbox.h"
#include "gui_enter_passcode.h"
#include "gui_model.h"
#include "user_memory.h"
#include "secret_cache.h"
#include "keystore.h"
#include "gui_lock_widgets.h"
#include "bip39_english.h"
#include "bip39.h"
#include "slip39.h"
#include "gui_forget_pass_widgets.h"
#include "gui_setting_widgets.h"
#include "sha256.h"
#include "gui_mnemonic_input.h"

#define DEVICE_SETTING_PASS_MAX_LEN                                 16
#define DEVICE_SETTING_RIGHT_LABEL_MAX_LEN                          DEVICE_SETTING_PASS_MAX_LEN
#define DEVICE_SETTING_MID_LABEL_MAX_LEN                            32
#define DEVICE_SETTING_LEVEL_MAX                                    8

typedef struct {
    uint8_t     currentTile;                                    // current x tile
    lv_obj_t    *cont;                                          // setting container
    lv_obj_t    *tileView;                                      // setting tile view
} ForgetPassWidget_t;
static ForgetPassWidget_t g_forgetPassTileView;
static lv_obj_t *g_waitAnimCont;
static GUI_VIEW *g_prevView;

typedef enum {
    FORGET_PASSWORD_ENTRANCE = 0,
    FORGET_PASSWORD_METHOD_SELECT,
    FORGET_PASSWORD_ENTER_MNEMONIC,
    FORGET_PASSWORD_SETPIN,
    FORGET_PASSWORD_REPEATPIN,

} FORGET_PASSWORD_ENUM;

// static void ImportPhraseWords(void);

static char g_pinBuf[GUI_DEFINE_MAX_PASSCODE_LEN + 1];
static MnemonicKeyBoard_t *g_recoveryMkb;
static KeyBoard_t *g_recoveryPhraseKb;
static lv_obj_t *g_enterMnemonicCont;
static uint8_t g_phraseCnt = 33;
static GuiEnterPasscodeItem_t *g_setPassCode = NULL;
static GuiEnterPasscodeItem_t *g_repeatPassCode = NULL;
static lv_obj_t *g_setPinTile;
static lv_obj_t *g_repeatPinTile;
static lv_obj_t *g_nextCont;
static lv_obj_t *g_letterKbCont;
static lv_obj_t *g_noticeHintBox = NULL;

static void GuiQuitHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        printf("g_prevView ID = %d\n", g_prevView->id);
        if (g_prevView->id == SCREEN_LOCK) {
            GuiLockScreenUpdatePassCode();
        }
        GuiCLoseCurrentWorkingView();
    }
}

static void ContinueStopCreateHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_CLICKED) {
        if (lv_event_get_user_data(e) != NULL) {
            g_recoveryMkb->currentSlice = 0;
            GuiNvsBarSetRightCb(NVS_RIGHT_BUTTON_BUTT, NULL, NULL);
            CloseToTargetTileView(g_forgetPassTileView.currentTile, FORGET_PASSWORD_METHOD_SELECT);
        }
        GUI_DEL_OBJ(g_noticeHintBox)
    }
}

static void StopCreateViewHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_CLICKED) {
        g_noticeHintBox = GuiCreateHintBox(lv_scr_act(), 480, 416, false);
        lv_obj_t *img = GuiCreateImg(g_noticeHintBox, &imgWarn);
        lv_obj_align(img, LV_ALIGN_DEFAULT, 36, 432);
        lv_obj_t *label = GuiCreateLittleTitleLabel(g_noticeHintBox, _("Cancel forget password?"));
        lv_obj_align(label, LV_ALIGN_DEFAULT, 36, 528);
        label = GuiCreateIllustrateLabel(g_noticeHintBox, _("import_wallet_ssb_cancel_desc"));
        lv_obj_align(label, LV_ALIGN_DEFAULT, 36, 580);
        lv_obj_t *btn = GuiCreateBtn(g_noticeHintBox, _("shamir_phrase_continue_btn"));
        lv_obj_set_style_bg_color(btn, WHITE_COLOR_OPA20, LV_PART_MAIN);
        lv_obj_align(btn, LV_ALIGN_DEFAULT, 36, 710);
        lv_obj_set_size(btn, 162, 66);
        lv_obj_add_event_cb(btn, ContinueStopCreateHandler, LV_EVENT_CLICKED, NULL);

        btn = GuiCreateBtn(g_noticeHintBox, _("shamir_phrase_cancel_create_btn"));
        lv_obj_set_style_bg_color(btn, RED_COLOR, LV_PART_MAIN);
        lv_obj_align(btn, LV_ALIGN_DEFAULT, 229, 710);
        lv_obj_set_size(btn, 215, 66);
        lv_obj_add_event_cb(btn, ContinueStopCreateHandler, LV_EVENT_CLICKED, g_noticeHintBox);
    }
}

void GuiForgetAnimContDel(int errCode)
{
    if (g_waitAnimCont != NULL) {
        lv_obj_del(g_waitAnimCont);
        g_waitAnimCont = NULL;
    }

    if (errCode == 0) {
        g_waitAnimCont = GuiCreateAnimHintBox(lv_scr_act(), 480, 326, 82);
        printf("g_waitAnimCont = %p\n", g_waitAnimCont);
        lv_obj_t *title = GuiCreateTextLabel(g_waitAnimCont, _("Resetting, Keep Screen ON"));
        lv_obj_align(title, LV_ALIGN_BOTTOM_MID, 0, -124);
        lv_obj_t *desc = GuiCreateNoticeLabel(g_waitAnimCont, _("Writing Secure Element..."));
        lv_obj_align(desc, LV_ALIGN_BOTTOM_MID, 0, -76);
    } else {
        g_waitAnimCont = GuiCreateAnimHintBox(lv_scr_act(), 480, 278, 82);
        lv_obj_t *title = GuiCreateTextLabel(g_waitAnimCont, _("seed_check_wait_verify"));
        lv_obj_align(title, LV_ALIGN_BOTTOM_MID, 0, -76);
    }
}

void GuiForgetPassVerifyResult(bool en, int errCode)
{
    if (en) {
        GuiNvsBarSetRightCb(NVS_RIGHT_BUTTON_BUTT, NULL, NULL);
        GuiForgetPassNextTile(0);
    } else {
        g_noticeHintBox = GuiCreateErrorCodeHintbox(errCode, &g_noticeHintBox);
    }
    if (g_waitAnimCont != NULL) {
        lv_obj_del(g_waitAnimCont);
        g_waitAnimCont = NULL;
    }
}

void GuiForgetPassResetPass(bool en, int errCode)
{
    lv_obj_t *cont = GuiCreateResultHintbox(lv_scr_act(), 356, &imgSuccess, "Reset Successful",
                                            "Your passcode has been reset successfully.", NULL, DARK_GRAY_COLOR, "Done", ORANGE_COLOR);
    lv_obj_t *rightBtn = GuiGetHintBoxRightBtn(cont);
    lv_obj_add_event_cb(rightBtn, CloseCurrentParentAndCloseViewHandler, LV_EVENT_CLICKED, NULL);
    if (g_waitAnimCont != NULL) {
        lv_obj_del(g_waitAnimCont);
        g_waitAnimCont = NULL;
    }
}

void GuiForgetPassSetPinPass(const char* buf)
{
    GuiEmitSignal(SIG_SETUP_VIEW_TILE_NEXT, NULL, 0);
    strcpy(g_pinBuf, buf);
}

void GuiForgetPassRepeatPinPass(const char* buf)
{
    if (!strcmp(buf, g_pinBuf)) {
        SecretCacheSetNewPassword((char *)buf);
        memset(g_pinBuf, 0, sizeof(g_pinBuf));
        GuiForgetAnimContDel(0);
        if (g_recoveryMkb->wordCnt == 33 || g_recoveryMkb->wordCnt == 20) {
            Slip39Data_t slip39 = {
                .threShold = g_recoveryMkb->threShold,
                .wordCnt = g_recoveryMkb->wordCnt,
                .forget = true,
            };
            GuiModelSlip39CalWriteSe(slip39);
        } else {
            Bip39Data_t bip39 = {
                .wordCnt = g_recoveryMkb->wordCnt,
                .forget = true,
            };
            GuiModelBip39CalWriteSe(bip39);
        }
    } else {
        GuiEnterPassCodeStatus(g_repeatPassCode, false);
    }
}

static void GuiCreateSetpinWidget(lv_obj_t *parent)
{
    g_setPinTile = parent;
    g_setPassCode = GuiCreateEnterPasscode(g_setPinTile, NULL, NULL, ENTER_PASSCODE_SET_PIN);
}

static void GuiCreateRepeatPinWidget(lv_obj_t *parent)
{
    g_repeatPinTile = parent;
}

static void ImportPhraseWordsHandler(lv_event_t* e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_CLICKED) {
        ImportSinglePhraseWords(g_recoveryMkb, g_recoveryPhraseKb);
    }
}

static void ImportShareNextSliceHandler(lv_event_t* e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_CLICKED) {
        ImportShareNextSlice(g_recoveryMkb, g_recoveryPhraseKb);
    }
}

static void ResetClearImportHandler(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        ClearMnemonicKeyboard(g_recoveryMkb, &g_recoveryMkb->currentId);
    }
}

static void *GuiWalletRecoverySinglePhrase(uint8_t wordAmount)
{
    lv_keyboard_user_mode_t kbMode = GuiGetMnemonicKbType(wordAmount);
    bool bip39 = true;
    g_phraseCnt = wordAmount;

    g_recoveryMkb = GuiCreateMnemonicKeyBoard(g_enterMnemonicCont, GuiMnemonicInputHandler, kbMode, NULL);
    g_recoveryMkb->intputType = MNEMONIC_INPUT_FORGET_VIEW;
    g_nextCont = GuiCreateContainer(lv_obj_get_width(lv_scr_act()), 114);
    lv_obj_set_align(g_nextCont, LV_ALIGN_BOTTOM_MID);
    lv_obj_set_style_bg_opa(g_nextCont, LV_OPA_0, 0);
    g_recoveryMkb->nextButton = GuiCreateBtn(g_nextCont, "");
    lv_obj_t *img = GuiCreateImg(g_recoveryMkb->nextButton, &imgArrowNext);
    lv_obj_set_align(img, LV_ALIGN_CENTER);
    lv_obj_align(g_recoveryMkb->nextButton, LV_ALIGN_BOTTOM_RIGHT, -20, -20);

    if (wordAmount == 20 || wordAmount == 33) {
        g_recoveryMkb->stepLabel = GuiCreateNoticeLabel(g_nextCont, "");
        lv_obj_align(g_recoveryMkb->stepLabel, LV_ALIGN_TOP_LEFT, 36, 39);

        g_recoveryMkb->titleLabel = GuiCreateTitleLabel(g_enterMnemonicCont, _(""));
        lv_obj_align(g_recoveryMkb->titleLabel, LV_ALIGN_DEFAULT, 36, 12);
        lv_label_set_recolor(g_recoveryMkb->titleLabel, true);

        g_recoveryMkb->descLabel = GuiCreateIllustrateLabel(g_enterMnemonicCont, _(""));
        lv_label_set_recolor(g_recoveryMkb->descLabel, true);
        lv_obj_align(g_recoveryMkb->descLabel, LV_ALIGN_DEFAULT, 36, 72);
        lv_obj_add_event_cb(g_recoveryMkb->nextButton, ImportShareNextSliceHandler, LV_EVENT_CLICKED, NULL);

        lv_label_set_text_fmt(g_recoveryMkb->stepLabel, "%d of %d", g_recoveryMkb->currentSlice + 1, g_recoveryMkb->threShold);
        lv_label_set_text_fmt(g_recoveryMkb->titleLabel, "%s #F5870A %d#", _("import_wallet_ssb_title"), g_recoveryMkb->currentSlice + 1);
        lv_label_set_text_fmt(g_recoveryMkb->descLabel, "Write down your #F5870A %d#-words seed phrase of\nshare #F5870A %d#in the blanks below",
                              g_recoveryMkb->wordCnt, g_recoveryMkb->currentSlice + 1);
        bip39 = false;
        GuiNvsBarSetLeftCb(NVS_BAR_CLOSE, StopCreateViewHandler, NULL);
    } else {
        g_recoveryMkb->titleLabel = GuiCreateTitleLabel(g_enterMnemonicCont, _("seed_check_single_phrase_title"));
        lv_obj_align(g_recoveryMkb->titleLabel, LV_ALIGN_DEFAULT, 36, 12);
        g_recoveryMkb->descLabel = GuiCreateIllustrateLabel(g_enterMnemonicCont, _("seed_check_share_phrase_title"));
        lv_obj_set_style_text_opa(g_recoveryMkb->descLabel, LV_OPA_60, LV_PART_MAIN);
        lv_obj_align(g_recoveryMkb->descLabel, LV_ALIGN_DEFAULT, 36, 72);
        lv_obj_add_event_cb(g_recoveryMkb->nextButton, ImportPhraseWordsHandler, LV_EVENT_CLICKED, NULL);
        GuiNvsBarSetLeftCb(NVS_BAR_RETURN, ReturnHandler, NULL);
    }

    lv_obj_set_size(g_recoveryMkb->cont, 408, 236);
    lv_obj_align_to(g_recoveryMkb->cont, g_recoveryMkb->descLabel, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 36);
    lv_btnmatrix_set_selected_btn(g_recoveryMkb->btnm, g_recoveryMkb->currentId);

    g_letterKbCont = GuiCreateContainer(lv_obj_get_width(lv_scr_act()), 242);
    lv_obj_set_align(g_letterKbCont, LV_ALIGN_BOTTOM_MID);
    lv_obj_set_style_bg_opa(g_letterKbCont, LV_OPA_0, 0);
    g_recoveryPhraseKb = GuiCreateLetterKeyBoard(g_letterKbCont, NULL, bip39, g_recoveryMkb);
    g_recoveryMkb->letterKb = g_recoveryPhraseKb;
    g_recoveryMkb->currentId = 0;

    return g_enterMnemonicCont;
}

void GuiWalletRecoverySinglePhraseClear(void)
{
    if (g_recoveryMkb != NULL) {
        lv_obj_del(g_recoveryMkb->cont);
        g_recoveryMkb = NULL;
    }
    g_recoveryMkb->currentSlice = 0;
    g_recoveryMkb->threShold = 0xff;
    CLEAR_OBJECT(g_recoveryMkb);

    if (g_recoveryPhraseKb != NULL) {
        lv_obj_del(g_recoveryPhraseKb->cont);
        g_recoveryPhraseKb = NULL;
    }

    lv_obj_clean(g_enterMnemonicCont);
    GUI_DEL_OBJ(g_nextCont)
    GUI_DEL_OBJ(g_letterKbCont)
}

void GuiForgetPassEntranceWidget(void *parent)
{
    lv_obj_t *img = GuiCreateImg(parent, &imgForget);
    lv_obj_align(img, LV_ALIGN_TOP_MID, 0, 204 - GUI_MAIN_AREA_OFFSET);

    lv_obj_t *label = GuiCreateLittleTitleLabel(parent, _("Forget Password"));
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 284 - GUI_MAIN_AREA_OFFSET);

    lv_obj_t *desc = GuiCreateNoticeLabel(parent, _("You have to verify the seed phrase of this wallet to reset the passcode."));
    lv_obj_align(desc, LV_ALIGN_TOP_MID, 0, 336 - GUI_MAIN_AREA_OFFSET);
    lv_obj_set_style_text_align(desc, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);

    lv_obj_t *btn = GuiCreateBtn(parent, _("Reset Passcode"));
    lv_obj_set_size(btn, 233, 66);
    lv_obj_align(btn, LV_ALIGN_BOTTOM_MID, 0, -64);
    lv_obj_set_style_bg_color(btn, BLACK_COLOR, LV_PART_MAIN);
    lv_obj_set_style_bg_color(btn, WHITE_COLOR, LV_STATE_PRESSED | LV_PART_MAIN);
    lv_obj_set_style_bg_opa(btn, LV_OPA_10 + LV_OPA_2, LV_STATE_PRESSED | LV_PART_MAIN);
    lv_obj_set_style_radius(btn, 24, LV_PART_MAIN);
    lv_obj_add_event_cb(btn, NextTileHandler, LV_EVENT_CLICKED, NULL);
}

void GuiForgetPassMethodCheck(void *parent)
{
    lv_obj_t *label = GuiCreateTextLabel(parent, _("seed_check_single_phrase"));
    lv_obj_t *imgArrow = GuiCreateImg(parent, &imgArrowRight);

    GuiButton_t table[2] = {
        {.obj = label, .align = LV_ALIGN_LEFT_MID, .position = {24, 0},},
        {.obj = imgArrow, .align = LV_ALIGN_LEFT_MID, .position = {411, 0},},
    };
    lv_obj_t *button = GuiCreateButton(parent, 456, 84, table, NUMBER_OF_ARRAYS(table), OpenSinglePhraseHandler, NULL);
    lv_obj_align(button, LV_ALIGN_DEFAULT, 12, 144 - GUI_MAIN_AREA_OFFSET);

    label = GuiCreateTextLabel(parent, _("seed_check_share_phrase"));
    imgArrow = GuiCreateImg(parent, &imgArrowRight);
    table[0].obj = label;
    table[1].obj = imgArrow;
    button = GuiCreateButton(parent, 456, 84, table, NUMBER_OF_ARRAYS(table), OpenSharePhraseHandler, NULL);
    lv_obj_align(button, LV_ALIGN_DEFAULT, 12, 240 - GUI_MAIN_AREA_OFFSET);
}

void GuiForgetPassEnterMnemonic(void *parent)
{
    lv_obj_set_style_bg_opa(parent, LV_OPA_0, LV_PART_SCROLLBAR | LV_STATE_SCROLLED);
    lv_obj_set_style_bg_opa(parent, LV_OPA_0, LV_PART_SCROLLBAR | LV_STATE_DEFAULT);

    g_enterMnemonicCont = parent;
}

void GuiForgetPassInit(void *param)
{
    g_prevView = param;
    lv_obj_t *cont = GuiCreateContainer(lv_obj_get_width(lv_scr_act()), lv_obj_get_height(lv_scr_act()) -
                                        GUI_MAIN_AREA_OFFSET);
    lv_obj_align(cont, LV_ALIGN_DEFAULT, 0, GUI_STATUS_BAR_HEIGHT + GUI_NAV_BAR_HEIGHT);

    lv_obj_t *tileView = GuiCreateTileView(cont);
    lv_obj_t *tile = lv_tileview_add_tile(tileView, FORGET_PASSWORD_ENTRANCE, 0, LV_DIR_HOR);
    GuiForgetPassEntranceWidget(tile);

    tile = lv_tileview_add_tile(tileView, FORGET_PASSWORD_METHOD_SELECT, 0, LV_DIR_HOR);
    GuiForgetPassMethodCheck(tile);

    tile = lv_tileview_add_tile(tileView, FORGET_PASSWORD_ENTER_MNEMONIC, 0, LV_DIR_HOR);
    GuiForgetPassEnterMnemonic(tile);

    tile = lv_tileview_add_tile(tileView, FORGET_PASSWORD_SETPIN, 0, LV_DIR_HOR);
    GuiCreateSetpinWidget(tile);

    tile = lv_tileview_add_tile(tileView, FORGET_PASSWORD_REPEATPIN, 0, LV_DIR_HOR);
    GuiCreateRepeatPinWidget(tile);

    g_forgetPassTileView.currentTile = FORGET_PASSWORD_ENTRANCE;
    g_forgetPassTileView.tileView = tileView;
    g_forgetPassTileView.cont = cont;

    lv_obj_set_tile_id(g_forgetPassTileView.tileView, g_forgetPassTileView.currentTile, 0, LV_ANIM_OFF);
}

void GuiForgetPassDeInit(void)
{
    GUI_DEL_OBJ(g_noticeHintBox)
    GuiMnemonicHintboxClear();
    GuiWalletRecoverySinglePhraseClear();
    g_enterMnemonicCont = NULL;
    g_repeatPinTile = NULL;
    g_setPinTile = NULL;
    if (g_setPassCode != NULL) {
        GuiDelEnterPasscode(g_setPassCode, NULL);
        g_setPassCode = NULL;
    }

    if (g_repeatPassCode != NULL) {
        GuiDelEnterPasscode(g_repeatPassCode, NULL);
        g_repeatPassCode = NULL;
    }

    GUI_DEL_OBJ(g_forgetPassTileView.cont)
    GuiSettingCloseSelectAmountHintBox();
}

int8_t GuiForgetPassNextTile(uint8_t tileIndex)
{
    switch (g_forgetPassTileView.currentTile) {
    case FORGET_PASSWORD_ENTRANCE:
        GuiNvsBarSetLeftCb(NVS_BAR_RETURN, ReturnHandler, NULL);
        GuiNvsBarSetMidBtnLabel(NVS_BAR_MID_LABEL, _("Seed Phrase Check"));
        if (CHECK_BATTERY_LOW_POWER()) {
            g_noticeHintBox = GuiCreateErrorCodeHintbox(ERR_KEYSTORE_SAVE_LOW_POWER, &g_noticeHintBox);
            return ERR_GUI_ERROR;
        }
        break;
    case FORGET_PASSWORD_METHOD_SELECT:
        GuiNvsBarSetMidCb(NVS_MID_BUTTON_BUTT, NULL, NULL);
        GuiNvsBarSetRightBtnLabel(NVS_BAR_WORD_RESET, USR_SYMBOL_RESET "Clear");
        GuiNvsBarSetRightCb(NVS_BAR_WORD_RESET, ResetClearImportHandler, NULL);
        if (tileIndex != 0) {
            switch (tileIndex) {
            case DEVICE_SETTING_RECOVERY_SINGLE_PHRASE_12WORDS:
                GuiWalletRecoverySinglePhrase(12);
                break;
            case DEVICE_SETTING_RECOVERY_SINGLE_PHRASE_18WORDS:
                GuiWalletRecoverySinglePhrase(18);
                break;
            case DEVICE_SETTING_RECOVERY_SINGLE_PHRASE_24WORDS:
                GuiWalletRecoverySinglePhrase(24);
                break;
            case DEVICE_SETTING_RECOVERY_SHARE_PHRASE_20WORDS:
                GuiWalletRecoverySinglePhrase(20);
                break;
            case DEVICE_SETTING_RECOVERY_SHARE_PHRASE_33WORDS:
                GuiWalletRecoverySinglePhrase(33);
                break;
            }
        }
        break;
    case FORGET_PASSWORD_ENTER_MNEMONIC:
        GuiNvsBarSetLeftCb(NVS_BAR_CLOSE, StopCreateViewHandler, NULL);
        lv_obj_add_flag(g_nextCont, LV_OBJ_FLAG_HIDDEN);
        break;
    case FORGET_PASSWORD_SETPIN:
        GuiNvsBarSetLeftCb(NVS_BAR_RETURN, ReturnHandler, NULL);
        if (g_repeatPassCode == NULL) {
            g_repeatPassCode = GuiCreateEnterPasscode(g_repeatPinTile, NULL, NULL,
                               g_setPassCode->mode + 2);
        }
        break;
    case FORGET_PASSWORD_REPEATPIN:
        break;
    }

    g_forgetPassTileView.currentTile++;
    lv_obj_set_tile_id(g_forgetPassTileView.tileView, g_forgetPassTileView.currentTile, 0, LV_ANIM_OFF);
    return SUCCESS_CODE;
}

int8_t GuiForgetPassPrevTile(uint8_t tileIndex)
{
    switch (g_forgetPassTileView.currentTile) {
    case FORGET_PASSWORD_ENTRANCE:
        return SUCCESS_CODE;
    case FORGET_PASSWORD_METHOD_SELECT:
        GuiNvsBarSetMidBtnLabel(NVS_BAR_MID_LABEL, _(""));
        GuiNvsBarSetLeftCb(NVS_BAR_CLOSE, GuiQuitHandler, NULL);
        break;
    case FORGET_PASSWORD_SETPIN:
        GuiNvsBarSetMidCb(NVS_MID_BUTTON_BUTT, NULL, NULL);
        GuiNvsBarSetRightBtnLabel(NVS_BAR_WORD_RESET, USR_SYMBOL_RESET "Clear");
        GuiNvsBarSetRightCb(NVS_BAR_WORD_RESET, ResetClearImportHandler, NULL);
        break;
    case FORGET_PASSWORD_REPEATPIN:
        GuiNvsBarSetLeftCb(NVS_BAR_CLOSE, StopCreateViewHandler, NULL);
        GuiDelEnterPasscode(g_repeatPassCode, NULL);
        g_repeatPassCode = NULL;
        GuiDelEnterPasscode(g_setPassCode, NULL);
        g_setPassCode = GuiCreateEnterPasscode(g_setPinTile, NULL, NULL, ENTER_PASSCODE_SET_PIN);
        break;
    case FORGET_PASSWORD_ENTER_MNEMONIC:
        GuiNvsBarSetLeftCb(NVS_BAR_RETURN, ReturnHandler, NULL);
        g_recoveryMkb->currentId = 0;
        g_recoveryMkb->currentSlice = 0;
        GuiNvsBarSetRightCb(NVS_RIGHT_BUTTON_BUTT, NULL, NULL);
        GuiNvsBarSetMidBtnLabel(NVS_BAR_MID_LABEL, _("Seed Phrase Check"));
        GuiWalletRecoverySinglePhraseClear();
        break;
    }
    g_forgetPassTileView.currentTile--;
    lv_obj_set_tile_id(g_forgetPassTileView.tileView, g_forgetPassTileView.currentTile, 0, LV_ANIM_OFF);
    return SUCCESS_CODE;
}


void GuiForgetPassRefresh(void)
{
    GuiNvsBarSetMidBtnLabel(NVS_BAR_MID_LABEL, _(""));
    GuiNvsBarSetLeftCb(NVS_BAR_CLOSE, GuiQuitHandler, NULL);
    GuiNvsBarSetRightCb(NVS_RIGHT_BUTTON_BUTT, NULL, NULL);
    if (g_forgetPassTileView.currentTile == FORGET_PASSWORD_ENTER_MNEMONIC && (g_phraseCnt == 33 || g_phraseCnt == 20)) {
        GuiNvsBarSetLeftCb(NVS_BAR_RETURN, StopCreateViewHandler, NULL);
    }

}
