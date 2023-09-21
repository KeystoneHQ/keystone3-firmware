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
#include "gui_mnemonic_input.h"
#include "gui_page.h"
#ifndef COMPILE_MAC_SIMULATOR
#include "sha256.h"
#else
#include "simulator_model.h"
#endif

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
static KeyBoard_t *g_forgetPhraseKb;
static lv_obj_t *g_enterMnemonicCont;
static MnemonicKeyBoard_t *g_forgetMkb;
static uint8_t g_phraseCnt = 33;
static GuiEnterPasscodeItem_t *g_setPassCode = NULL;
static GuiEnterPasscodeItem_t *g_repeatPassCode = NULL;
static lv_obj_t *g_setPinTile;
static lv_obj_t *g_repeatPinTile;
static lv_obj_t *g_nextCont;
static lv_obj_t *g_letterKbCont;
static lv_obj_t *g_noticeHintBox = NULL;
static PageWidget_t *g_pageWidget;

static void GuiQuitHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        // printf("g_prevView ID = %d\n", g_prevView->id);
        if (g_prevView != NULL && g_prevView->id == SCREEN_LOCK) {
            GuiLockScreenUpdatePassCode();
            GuiLockScreenFpRecognize();
        }
        GuiCLoseCurrentWorkingView();
    }
}

static void ContinueStopCreateHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_CLICKED) {
        if (lv_event_get_user_data(e) != NULL) {
            g_forgetMkb->currentSlice = 0;
            SetNavBarRightBtn(g_pageWidget->navBarWidget, NVS_RIGHT_BUTTON_BUTT, NULL, NULL);
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
        lv_obj_add_flag(g_waitAnimCont, LV_OBJ_FLAG_CLICKABLE);
    } else {
        g_waitAnimCont = GuiCreateAnimHintBox(lv_scr_act(), 480, 278, 82);
        lv_obj_t *title = GuiCreateTextLabel(g_waitAnimCont, _("seed_check_wait_verify"));
        lv_obj_align(title, LV_ALIGN_BOTTOM_MID, 0, -76);
        lv_obj_add_flag(g_waitAnimCont, LV_OBJ_FLAG_CLICKABLE);
    }
}

void GuiForgetPassVerifyResult(bool en, int errCode)
{
    if (en) {
        SetNavBarRightBtn(g_pageWidget->navBarWidget, NVS_RIGHT_BUTTON_BUTT, NULL, NULL);
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
        if (g_forgetMkb->wordCnt == 33 || g_forgetMkb->wordCnt == 20) {
            Slip39Data_t slip39 = {
                .threShold = g_forgetMkb->threShold,
                .wordCnt = g_forgetMkb->wordCnt,
                .forget = true,
            };
            GuiModelSlip39CalWriteSe(slip39);
        } else {
            Bip39Data_t bip39 = {
                .wordCnt = g_forgetMkb->wordCnt,
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
        ImportSinglePhraseWords(g_forgetMkb, g_forgetPhraseKb);
    }
}

static void ImportShareNextSliceHandler(lv_event_t* e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_CLICKED) {
        ImportShareNextSlice(g_forgetMkb, g_forgetPhraseKb);
    }
}

static void ResetClearImportHandler(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        ClearMnemonicKeyboard(g_forgetMkb, &g_forgetMkb->currentId);
        GuiClearKeyBoard(g_forgetPhraseKb);
    }
}

static void *GuiWalletForgetSinglePhrase(uint8_t wordAmount)
{
    lv_keyboard_user_mode_t kbMode = GuiGetMnemonicKbType(wordAmount);
    bool bip39 = true;
    g_phraseCnt = wordAmount;

    g_forgetMkb = GuiCreateMnemonicKeyBoard(g_enterMnemonicCont, GuiMnemonicInputHandler, kbMode, NULL);
    g_forgetMkb->intputType = MNEMONIC_INPUT_FORGET_VIEW;
    g_nextCont = GuiCreateContainer(lv_obj_get_width(lv_scr_act()), 114);
    lv_obj_set_align(g_nextCont, LV_ALIGN_BOTTOM_MID);
    lv_obj_set_style_bg_opa(g_nextCont, LV_OPA_0, 0);
    g_forgetMkb->nextButton = GuiCreateBtn(g_nextCont, "");
    lv_obj_t *img = GuiCreateImg(g_forgetMkb->nextButton, &imgArrowNext);
    lv_obj_set_align(img, LV_ALIGN_CENTER);
    lv_obj_align(g_forgetMkb->nextButton, LV_ALIGN_BOTTOM_RIGHT, -20, -20);

    if (wordAmount == 20 || wordAmount == 33) {
        g_forgetMkb->stepLabel = GuiCreateNoticeLabel(g_nextCont, "");
        lv_obj_align(g_forgetMkb->stepLabel, LV_ALIGN_TOP_LEFT, 36, 39);

        g_forgetMkb->titleLabel = GuiCreateTitleLabel(g_enterMnemonicCont, _(""));
        lv_obj_align(g_forgetMkb->titleLabel, LV_ALIGN_DEFAULT, 36, 12);
        lv_label_set_recolor(g_forgetMkb->titleLabel, true);

        g_forgetMkb->descLabel = GuiCreateIllustrateLabel(g_enterMnemonicCont, _(""));
        lv_label_set_recolor(g_forgetMkb->descLabel, true);
        lv_obj_align(g_forgetMkb->descLabel, LV_ALIGN_DEFAULT, 36, 72);
        lv_obj_add_event_cb(g_forgetMkb->nextButton, ImportShareNextSliceHandler, LV_EVENT_CLICKED, NULL);

        lv_label_set_text_fmt(g_forgetMkb->stepLabel, "%d of %d", g_forgetMkb->currentSlice + 1, g_forgetMkb->threShold);
        lv_label_set_text_fmt(g_forgetMkb->titleLabel, "%s #F5870A %d#", _("import_wallet_ssb_title"), g_forgetMkb->currentSlice + 1);
        lv_label_set_text_fmt(g_forgetMkb->descLabel, "Write down your #F5870A %d#-words seed phrase of\nshare #F5870A %d# in the blanks below",
                              g_forgetMkb->wordCnt, g_forgetMkb->currentSlice + 1);
        bip39 = false;
        SetNavBarLeftBtn(g_pageWidget->navBarWidget, NVS_BAR_CLOSE, StopCreateViewHandler, NULL);
    } else {
        g_forgetMkb->titleLabel = GuiCreateTitleLabel(g_enterMnemonicCont, _("seed_check_single_phrase_title"));
        lv_obj_align(g_forgetMkb->titleLabel, LV_ALIGN_DEFAULT, 36, 12);
        g_forgetMkb->descLabel = GuiCreateIllustrateLabel(g_enterMnemonicCont, _("seed_check_share_phrase_title"));
        lv_obj_set_style_text_opa(g_forgetMkb->descLabel, LV_OPA_60, LV_PART_MAIN);
        lv_obj_align(g_forgetMkb->descLabel, LV_ALIGN_DEFAULT, 36, 72);
        lv_obj_add_event_cb(g_forgetMkb->nextButton, ImportPhraseWordsHandler, LV_EVENT_CLICKED, NULL);
        SetNavBarLeftBtn(g_pageWidget->navBarWidget, NVS_BAR_RETURN, ReturnHandler, NULL);
    }

    lv_obj_set_size(g_forgetMkb->cont, 408, 236);
    lv_obj_align_to(g_forgetMkb->cont, g_forgetMkb->descLabel, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 36);
    lv_btnmatrix_set_selected_btn(g_forgetMkb->btnm, g_forgetMkb->currentId);

    g_letterKbCont = GuiCreateContainer(lv_obj_get_width(lv_scr_act()), 242);
    lv_obj_set_align(g_letterKbCont, LV_ALIGN_BOTTOM_MID);
    lv_obj_set_style_bg_opa(g_letterKbCont, LV_OPA_0, 0);
    g_forgetPhraseKb = GuiCreateLetterKeyBoard(g_letterKbCont, NULL, bip39, g_forgetMkb);
    g_forgetMkb->letterKb = g_forgetPhraseKb;
    g_forgetMkb->currentId = 0;

    return g_enterMnemonicCont;
}

void GuiWalletRecoverySinglePhraseClear(void)
{
    if (g_forgetMkb != NULL) {
        lv_obj_del(g_forgetMkb->cont);
        g_forgetMkb = NULL;
    }
    g_forgetMkb->currentSlice = 0;
    g_forgetMkb->threShold = 0xff;
    CLEAR_OBJECT(g_forgetMkb);

    if (g_forgetPhraseKb != NULL) {
        lv_obj_del(g_forgetPhraseKb->cont);
        g_forgetPhraseKb = NULL;
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
    g_pageWidget = CreatePageWidget();
    lv_obj_t *cont = g_pageWidget->contentZone;

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
    if (g_pageWidget != NULL) {
        DestroyPageWidget(g_pageWidget);
        g_pageWidget = NULL;
    }
}

int8_t GuiForgetPassNextTile(uint8_t tileIndex)
{
    switch (g_forgetPassTileView.currentTile) {
    case FORGET_PASSWORD_ENTRANCE:
        if (CHECK_BATTERY_LOW_POWER()) {
            g_noticeHintBox = GuiCreateErrorCodeHintbox(ERR_KEYSTORE_SAVE_LOW_POWER, &g_noticeHintBox);
            return ERR_GUI_ERROR;
        } else {
            SetNavBarLeftBtn(g_pageWidget->navBarWidget, NVS_BAR_RETURN, ReturnHandler, NULL);
            SetMidBtnLabel(g_pageWidget->navBarWidget, NVS_BAR_MID_LABEL, _("Seed Phrase Check"));
        }
        break;
    case FORGET_PASSWORD_METHOD_SELECT:
        SetNavBarMidBtn(g_pageWidget->navBarWidget, NVS_MID_BUTTON_BUTT, NULL, NULL);
        SetRightBtnLabel(g_pageWidget->navBarWidget, NVS_BAR_WORD_RESET, USR_SYMBOL_RESET "Clear");
        SetRightBtnCb(g_pageWidget->navBarWidget, ResetClearImportHandler, NULL);
        if (tileIndex != 0) {
            switch (tileIndex) {
            case DEVICE_SETTING_RECOVERY_SINGLE_PHRASE_12WORDS:
                GuiWalletForgetSinglePhrase(12);
                break;
            case DEVICE_SETTING_RECOVERY_SINGLE_PHRASE_18WORDS:
                GuiWalletForgetSinglePhrase(18);
                break;
            case DEVICE_SETTING_RECOVERY_SINGLE_PHRASE_24WORDS:
                GuiWalletForgetSinglePhrase(24);
                break;
            case DEVICE_SETTING_RECOVERY_SHARE_PHRASE_20WORDS:
                GuiWalletForgetSinglePhrase(20);
                break;
            case DEVICE_SETTING_RECOVERY_SHARE_PHRASE_33WORDS:
                GuiWalletForgetSinglePhrase(33);
                break;
            }
        }
        break;
    case FORGET_PASSWORD_ENTER_MNEMONIC:
        SetNavBarLeftBtn(g_pageWidget->navBarWidget, NVS_BAR_CLOSE, StopCreateViewHandler, NULL);
        lv_obj_add_flag(g_nextCont, LV_OBJ_FLAG_HIDDEN);
        break;
    case FORGET_PASSWORD_SETPIN:
        SetNavBarLeftBtn(g_pageWidget->navBarWidget, NVS_BAR_RETURN, ReturnHandler, NULL);
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
        SetMidBtnLabel(g_pageWidget->navBarWidget, NVS_BAR_MID_LABEL, _(""));
        SetNavBarLeftBtn(g_pageWidget->navBarWidget, NVS_BAR_CLOSE, GuiQuitHandler, NULL);
        break;
    case FORGET_PASSWORD_SETPIN:
        SetNavBarMidBtn(g_pageWidget->navBarWidget, NVS_MID_BUTTON_BUTT, NULL, NULL);
        SetRightBtnLabel(g_pageWidget->navBarWidget, NVS_BAR_WORD_RESET, USR_SYMBOL_RESET "Clear");
        SetRightBtnCb(g_pageWidget->navBarWidget, ResetClearImportHandler, NULL);
        break;
    case FORGET_PASSWORD_REPEATPIN:
        SetNavBarLeftBtn(g_pageWidget->navBarWidget, NVS_BAR_CLOSE, StopCreateViewHandler, NULL);
        GuiDelEnterPasscode(g_repeatPassCode, NULL);
        g_repeatPassCode = NULL;
        GuiDelEnterPasscode(g_setPassCode, NULL);
        g_setPassCode = GuiCreateEnterPasscode(g_setPinTile, NULL, NULL, ENTER_PASSCODE_SET_PIN);
        break;
    case FORGET_PASSWORD_ENTER_MNEMONIC:
        SetNavBarLeftBtn(g_pageWidget->navBarWidget, NVS_BAR_RETURN, ReturnHandler, NULL);
        g_forgetMkb->currentId = 0;
        g_forgetMkb->currentSlice = 0;
        SetNavBarRightBtn(g_pageWidget->navBarWidget, NVS_RIGHT_BUTTON_BUTT, NULL, NULL);
        SetMidBtnLabel(g_pageWidget->navBarWidget, NVS_BAR_MID_LABEL, _("Seed Phrase Check"));
        GuiWalletRecoverySinglePhraseClear();
        break;
    }
    g_forgetPassTileView.currentTile--;
    lv_obj_set_tile_id(g_forgetPassTileView.tileView, g_forgetPassTileView.currentTile, 0, LV_ANIM_OFF);
    return SUCCESS_CODE;
}


void GuiForgetPassRefresh(void)
{
    SetMidBtnLabel(g_pageWidget->navBarWidget, NVS_BAR_MID_LABEL, _(""));
    SetNavBarLeftBtn(g_pageWidget->navBarWidget, NVS_BAR_CLOSE, GuiQuitHandler, NULL);
    SetNavBarRightBtn(g_pageWidget->navBarWidget, NVS_RIGHT_BUTTON_BUTT, NULL, NULL);
    if (g_forgetPassTileView.currentTile == FORGET_PASSWORD_ENTER_MNEMONIC && (g_phraseCnt == 33 || g_phraseCnt == 20)) {
        SetNavBarLeftBtn(g_pageWidget->navBarWidget, NVS_BAR_RETURN, StopCreateViewHandler, NULL);
    }

}
