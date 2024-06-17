#include "gui.h"
#include "gui_views.h"
#include "gui_status_bar.h"
#include "gui_keyboard.h"
#include "gui_button.h"
#include "gui_hintbox.h"
#include "gui_model.h"
#include "gui_single_phrase_widgets.h"
#include "gui_create_wallet_widgets.h"
#include "user_memory.h"
#include "bip39.h"
#include "secret_cache.h"
#include "background_task.h"
#include "gui_lock_widgets.h"
#include "motor_manager.h"
#include "user_delay.h"
#include "gui_page.h"
#include "gui_pending_hintbox.h"
#include "gui_tutorial_widgets.h"

#define SINGLE_PHRASE_MAX_WORDS         24
typedef enum {
    SINGLE_PHRASE_RANDOM_PHRASE = 0,
    SINGLE_PHRASE_CONFIRM_PHRASE,
    SINGLE_PHRASE_WRITE_SE,
    SINGLE_PHRASE_CONNECT,

    SINGLE_PHRASE_BUTT,
} SINGLE_PHRASE_ENUM;

typedef struct SinglePhraseWidget {
    uint8_t     currentTile;
    lv_obj_t    *cont;
    lv_obj_t    *tileView;
    lv_obj_t    *notice;
    lv_obj_t    *randomPhrase;
    lv_obj_t    *confirmPhrase;
    lv_obj_t    *writeSe;
    lv_obj_t    *backupForm;
} SinglePhraseWidget_t;
static SinglePhraseWidget_t g_singlePhraseTileView;

static MnemonicKeyBoard_t *g_randomPhraseKb = NULL;
static MnemonicKeyBoard_t *g_confirmPhraseKb = NULL;
static lv_obj_t *g_changeCont = NULL;
static uint8_t g_phraseCnt = SINGLE_PHRASE_MAX_WORDS;
static uint8_t g_pressedBtn[SINGLE_PHRASE_MAX_WORDS];
static uint8_t g_pressedBtnFlag[SINGLE_PHRASE_MAX_WORDS];
static uint8_t g_currId = 0;
static char g_randomBuff[BUFFER_SIZE_512];
static lv_obj_t *g_noticeHintBox = NULL;
static PageWidget_t *g_pageWidget;
static uint8_t g_entropyMethod = 0;
static lv_obj_t *g_noticeWindow;
static bool g_isTon = false;
static bool g_isDiceRolls = false;

static void ResetConfirmInput(void);
static void SelectPhraseCntHandler(lv_event_t *e);

static void TonUpdateMnemonicHandler(lv_event_t *e)
{
    GUI_DEL_OBJ(g_noticeWindow);
    GuiModelTonUpdateMnemonic();
}

static void TonRegenrateConfirm()
{
    GUI_DEL_OBJ(g_noticeWindow);
    g_noticeWindow = GuiCreateGeneralHintBox(&imgBlueInformation, _("ton_mnemonic_regenerate"), NULL, NULL, _("Cancel"), WHITE_COLOR_OPA20, _("Regenerate"), ORANGE_COLOR);

    lv_obj_t *btn = GuiGetHintBoxRightBtn(g_noticeWindow);
    lv_obj_add_event_cb(btn, TonUpdateMnemonicHandler, LV_EVENT_CLICKED, &g_noticeWindow);

    btn = GuiGetHintBoxLeftBtn(g_noticeWindow);
    lv_obj_add_event_cb(btn, CloseHintBoxHandler, LV_EVENT_CLICKED, &g_noticeWindow);
}

static void UpdatePhraseHandler(lv_event_t *e)
{
    if (g_isTon) {
        TonRegenrateConfirm();
    } else GuiModelBip39UpdateMnemonic(g_phraseCnt);
}

static void WriteSE()
{
    if (g_isTon) {
        GuiUpdateTonWriteSeWidget(g_singlePhraseTileView.writeSe);
        GuiModelTonWriteSe();
    } else GuiModelWriteSe();
}

static void OpenTonTutorial(lv_event_t *e)
{
    uint8_t index = TUTORIAL_TON_MNEMONIC;
    GuiFrameOpenViewWithParam(&g_tutorialView, &index, sizeof(index));
}

static void GuiRandomPhraseWidget(lv_obj_t *parent)
{
    uint16_t height = 296;
    lv_obj_set_scrollbar_mode(parent, LV_SCROLLBAR_MODE_OFF);
    lv_obj_clear_flag(parent, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(parent, LV_OBJ_FLAG_CHECKABLE);
    lv_obj_set_size(parent, lv_obj_get_width(lv_scr_act()), lv_obj_get_height(lv_scr_act()) - GUI_MAIN_AREA_OFFSET - 114);
    lv_obj_t *label = GuiCreateScrollTitleLabel(parent, _("single_phrase_title"));
    lv_obj_align(label, LV_ALIGN_DEFAULT, 36, 156 - GUI_MAIN_AREA_OFFSET);

    label = GuiCreateIllustrateLabel(parent, _("single_phrase_desc"));
    lv_obj_set_style_text_opa(label, LV_OPA_60, LV_PART_MAIN);
    GuiAlignToPrevObj(label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 12);
    lv_obj_refr_size(label);
    height -= lv_obj_get_self_height(label);
    g_randomPhraseKb = GuiCreateMnemonicKeyBoard(parent, NULL, g_phraseCnt == 12 ? KEY_STONE_MNEMONIC_12 : KEY_STONE_MNEMONIC_24, NULL);
    lv_obj_set_size(g_randomPhraseKb->cont, 408, height);
    lv_obj_set_size(g_randomPhraseKb->cont, 408, 305);

    lv_obj_align(g_randomPhraseKb->cont, LV_ALIGN_TOP_MID,
                 0, 310 - GUI_MAIN_AREA_OFFSET);
    lv_obj_t *cont = GuiCreateContainer(lv_obj_get_width(lv_scr_act()), 114);
    lv_obj_set_align(cont, LV_ALIGN_BOTTOM_MID);
    g_changeCont = cont;
    if (!g_isDiceRolls) {
        lv_obj_t *button = GuiCreateImgLabelAdaptButton(cont, _("Regenerate"), &imgChange, UpdatePhraseHandler, NULL);
        lv_obj_set_height(button, 66);
        lv_obj_align(button, LV_ALIGN_DEFAULT, 24, 24);
    }
    lv_obj_t *btn = GuiCreateBtn(cont, USR_SYMBOL_KB_NEXT);
    lv_obj_align(btn, LV_ALIGN_DEFAULT, 348, 24);
    lv_obj_add_event_cb(btn, NextTileHandler, LV_EVENT_CLICKED, cont);
    if (!g_isTon) {
        SetRightBtnLabel(g_pageWidget->navBarWidget, NVS_BAR_WORD_SELECT, g_phraseCnt == 12 ? "12" : "24");
        SetRightBtnCb(g_pageWidget->navBarWidget, SelectPhraseCntHandler, NULL);
    } else {
        SetNavBarRightBtn(g_pageWidget->navBarWidget, NVS_BAR_QUESTION_MARK, OpenTonTutorial, NULL);
    }
    if (!g_isDiceRolls) {
        if (g_isTon) {
            GuiModelTonUpdateMnemonic();
        } else {
            GuiModelBip39UpdateMnemonic(g_phraseCnt);
        }
    } else {
        GuiModelBip39UpdateMnemonicWithDiceRolls(g_phraseCnt);
    }
}

static void MnemonicConfirmHandler(lv_event_t *e)
{
    int i = 0, j;
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *obj = lv_event_get_target(e);
    if (code == LV_EVENT_DRAW_PART_BEGIN) {
        lv_obj_draw_part_dsc_t *dsc = lv_event_get_draw_part_dsc(e);
        if (dsc->class_p == &lv_btnmatrix_class && dsc->type == LV_BTNMATRIX_DRAW_PART_BTN) {
            if (g_pressedBtnFlag[dsc->id] == MNEMONIC_BUTTON_UNPRESSED) {
                dsc->rect_dsc->bg_color = DARK_BG_COLOR;
            } else if (g_pressedBtnFlag[dsc->id] == MNEMONIC_BUTTON_CURRENT_PRESS) {
                dsc->rect_dsc->bg_color = GREEN_COLOR;
            } else if (g_pressedBtnFlag[dsc->id] == MNEMONIC_BUTTON_PRESSED) {
                dsc->rect_dsc->bg_color = WHITE_COLOR;
                dsc->rect_dsc->bg_opa = LV_OPA_30;
            }
        }
    }

    if (code == LV_EVENT_CLICKED) {
        uint32_t currentId = lv_btnmatrix_get_selected_btn(obj);
        if (currentId >= 0xff) {
            return;
        }
        Vibrate(SLIGHT);
        for (i = 0; i < g_currId; i++) {
            if (g_pressedBtn[i] == currentId + 1) {
                break;
            }
        }

        if (i == g_currId - 1) {
            lv_btnmatrix_clear_btn_ctrl(g_confirmPhraseKb->btnm, g_currId, LV_BTNMATRIX_CTRL_CHECKED);
            g_pressedBtnFlag[g_pressedBtn[g_currId - 1] - 1] = MNEMONIC_BUTTON_UNPRESSED;
            GuiConfirmMnemonicKeyBoard(g_confirmPhraseKb, g_randomBuff, g_pressedBtn[g_currId - 1] - 1, g_currId - 1, 0);
            g_pressedBtn[g_currId - 1] = 0;
            if (g_currId >= 2) {
                g_pressedBtnFlag[g_pressedBtn[g_currId - 2] - 1] = MNEMONIC_BUTTON_CURRENT_PRESS;
            }
            g_currId--;
        } else if (i == g_currId) {
            g_pressedBtn[g_currId] = currentId + 1;
            g_pressedBtnFlag[g_pressedBtn[g_currId] - 1] = MNEMONIC_BUTTON_CURRENT_PRESS;
            if (g_currId >= 1) {
                g_pressedBtnFlag[g_pressedBtn[g_currId - 1] - 1] = MNEMONIC_BUTTON_PRESSED;
            }
            GuiConfirmMnemonicKeyBoard(g_confirmPhraseKb, g_randomBuff, currentId, g_currId + 1, 1);
            g_currId++;
        }

        if (g_currId == g_phraseCnt) {
            char *confirmMnemonic = SRAM_MALLOC(BIP39_MAX_WORD_LEN * g_phraseCnt + 1);
            confirmMnemonic[0] = 0;
            for (i = 0; i < g_phraseCnt; i++) {
                j = (g_pressedBtn[i] - 1) + (g_pressedBtn[i] - 1) / 3;
                strcat(confirmMnemonic, strchr(g_confirmPhraseKb->mnemonicWord[j], '\n') + 1);
                if (i < g_phraseCnt - 1) {
                    strcat(confirmMnemonic, " ");
                }
            }
            if (strcmp(confirmMnemonic, SecretCacheGetMnemonic()) == 0) {
                WriteSE();
                GuiEmitSignal(SIG_SETUP_VIEW_TILE_NEXT, NULL, 0);
            } else {
                g_noticeHintBox = GuiCreateErrorCodeWindow(ERR_KEYSTORE_MNEMONIC_NOT_MATCH_WALLET, &g_noticeHintBox, NULL);
            }
            memset_s(confirmMnemonic, BIP39_MAX_WORD_LEN * g_phraseCnt + 1, 0, BIP39_MAX_WORD_LEN * g_phraseCnt + 1);
            SRAM_FREE(confirmMnemonic);
        }
    }
}

static void ResetConfirmInput(void)
{
    for (int i = 0; i < SINGLE_PHRASE_MAX_WORDS; i++) {
        g_pressedBtn[i] = 0;
        g_pressedBtnFlag[i] = 0;
    }
    g_currId = 0;
    GuiUpdateMnemonicKeyBoard(g_confirmPhraseKb, g_randomBuff, true);
    lv_btnmatrix_clear_btn_ctrl_all(g_confirmPhraseKb->btnm, LV_BTNMATRIX_CTRL_CHECKED);
}

static void GuiConfirmPhraseWidget(lv_obj_t *parent)
{
    lv_obj_set_style_bg_opa(parent, LV_OPA_0, LV_PART_SCROLLBAR | LV_STATE_SCROLLED);
    lv_obj_set_style_bg_opa(parent, LV_OPA_0, LV_PART_SCROLLBAR | LV_STATE_DEFAULT);
    lv_obj_t *label = GuiCreateTitleLabel(parent, _("single_phrase_confirm_title"));
    lv_obj_align(label, LV_ALIGN_DEFAULT, 36, 156 - GUI_MAIN_AREA_OFFSET);

    label = GuiCreateIllustrateLabel(parent, _("single_phrase_confirm_desc"));
    lv_obj_set_style_text_opa(label, LV_OPA_60, LV_PART_MAIN);
    GuiAlignToPrevObj(label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 12);
    g_confirmPhraseKb = GuiCreateMnemonicKeyBoard(parent, MnemonicConfirmHandler,
                        g_phraseCnt == 12 ? KEY_STONE_MNEMONIC_12 : KEY_STONE_MNEMONIC_24, NULL);
    lv_obj_align(g_confirmPhraseKb->cont, LV_ALIGN_TOP_MID, 0, 310 - GUI_MAIN_AREA_OFFSET);
}

void GuiSinglePhraseInit(uint8_t entropyMethod)
{
    g_entropyMethod = entropyMethod;
    g_isTon = g_entropyMethod & WALLET_TYPE_MASK;
    g_isDiceRolls = g_entropyMethod & ENTROPY_TYPE_MASK;
    CLEAR_OBJECT(g_singlePhraseTileView);
    g_pageWidget = CreatePageWidget();
    lv_obj_t *cont = g_pageWidget->contentZone;

    lv_obj_t *tileView = GuiCreateTileView(cont);
    lv_obj_t *tile = lv_tileview_add_tile(tileView, SINGLE_PHRASE_RANDOM_PHRASE, 0, LV_DIR_HOR);
    g_singlePhraseTileView.randomPhrase = tile;
    GuiRandomPhraseWidget(tile);

    tile = lv_tileview_add_tile(tileView, SINGLE_PHRASE_CONFIRM_PHRASE, 0, LV_DIR_HOR);
    g_singlePhraseTileView.confirmPhrase = tile;
    GuiConfirmPhraseWidget(tile);

    tile = lv_tileview_add_tile(tileView, SINGLE_PHRASE_WRITE_SE, 0, LV_DIR_HOR);
    g_singlePhraseTileView.writeSe = tile;
    GuiWriteSeWidget(tile);

    g_singlePhraseTileView.currentTile = SINGLE_PHRASE_RANDOM_PHRASE;
    g_singlePhraseTileView.tileView = tileView;
    g_singlePhraseTileView.cont = cont;

    lv_obj_set_tile_id(g_singlePhraseTileView.tileView, g_singlePhraseTileView.currentTile, 0, LV_ANIM_OFF);
    SetNavBarLeftBtn(g_pageWidget->navBarWidget, NVS_BAR_RETURN, ReturnHandler, NULL);
    if (!g_isTon) {
        SetNavBarRightBtn(g_pageWidget->navBarWidget, NVS_BAR_WORD_SELECT, SelectPhraseCntHandler, NULL);
    } else {
        SetNavBarRightBtn(g_pageWidget->navBarWidget, NVS_BAR_QUESTION_MARK, OpenTonTutorial, NULL);
    }
}

void GuiSinglePhraseUpdateMnemonic(void *signalParam, uint16_t paramLen)
{
    g_randomPhraseKb->wordCnt = g_phraseCnt;
    GuiUpdateMnemonicKeyBoard(g_randomPhraseKb, SecretCacheGetMnemonic(), false);
    lv_obj_set_size(g_randomPhraseKb->cont, 408, 360);
}

void GuiShowTonGeneratingModal(bool enable)
{
    if (enable) {
        GuiPendingHintBoxOpen(_("ton_mnemonic_generating_title"), _("ton_mnemonic_generating_desc"));
    } else {
        GuiPendingHintBoxRemove();
    }
}

static void SelectCheckBoxHandler(lv_event_t* e)
{
    uint32_t* active_id = lv_event_get_user_data(e);
    lv_obj_t *actCb = lv_event_get_target(e);
    lv_obj_t *oldCb = lv_obj_get_child(g_noticeHintBox, *active_id);

    if (actCb == g_noticeHintBox) {
        return;
    }
    Vibrate(SLIGHT);
    lv_obj_clear_state(oldCb, LV_STATE_CHECKED);
    lv_obj_add_state(actCb, LV_STATE_CHECKED);
    *active_id = lv_obj_get_index(actCb);

    // TODO: use id to identity position
    const char *currText = lv_checkbox_get_text(actCb);
    if (!strcmp(currText, _("wallet_phrase_12words"))) {
        SetRightBtnLabel(g_pageWidget->navBarWidget, NVS_BAR_WORD_SELECT, "12");
        SetRightBtnCb(g_pageWidget->navBarWidget, SelectPhraseCntHandler, NULL);
        if (g_phraseCnt != 12) {
            g_phraseCnt = 12;
            if (!g_isDiceRolls) {
                GuiModelBip39UpdateMnemonic(g_phraseCnt);
            } else {
                GuiModelBip39UpdateMnemonicWithDiceRolls(g_phraseCnt);
            }
        }
    } else if (!strcmp(currText, _("wallet_phrase_24words"))) {
        SetRightBtnLabel(g_pageWidget->navBarWidget, NVS_BAR_WORD_SELECT, "24");
        SetRightBtnCb(g_pageWidget->navBarWidget, SelectPhraseCntHandler, NULL);
        if (g_phraseCnt != 24) {
            g_phraseCnt = 24;
            if (!g_isDiceRolls) {
                GuiModelBip39UpdateMnemonic(g_phraseCnt);
            } else {
                GuiModelBip39UpdateMnemonicWithDiceRolls(g_phraseCnt);
            }
        }
    }
    lv_obj_scroll_to_y(g_randomPhraseKb->cont, 0, LV_ANIM_ON);
    GUI_DEL_OBJ(g_noticeHintBox)
}

static void SelectPhraseCntHandler(lv_event_t *e)
{
    static uint32_t currentIndex = 0;
    lv_obj_t *checkBox = NULL;

    g_noticeHintBox = GuiCreateHintBox(282);
    lv_obj_add_event_cb(lv_obj_get_child(g_noticeHintBox, 0), CloseHintBoxHandler, LV_EVENT_CLICKED, &g_noticeHintBox);
    lv_obj_t *label = GuiCreateIllustrateLabel(g_noticeHintBox, _("single_phrase_word_amount_select"));
    lv_obj_align(label, LV_ALIGN_DEFAULT, 36, 560);
    lv_obj_set_style_text_opa(label, LV_OPA_60, LV_PART_MAIN);
    lv_obj_t *button = GuiCreateImgButton(g_noticeHintBox, &imgClose, 50, CloseHintBoxHandler, &g_noticeHintBox);
    lv_obj_align(button, LV_ALIGN_DEFAULT, 407, 550);

    if (g_phraseCnt == 24) {
        checkBox = GuiCreateSingleCheckBox(g_noticeHintBox, _("wallet_phrase_12words"));
        lv_obj_align(checkBox, LV_ALIGN_DEFAULT, 30, 630);
        checkBox = GuiCreateSingleCheckBox(g_noticeHintBox, _("wallet_phrase_24words"));
        lv_obj_align(checkBox, LV_ALIGN_DEFAULT, 30, 618 + 100);
        lv_obj_add_state(checkBox, LV_STATE_CHECKED);
    } else {
        checkBox = GuiCreateSingleCheckBox(g_noticeHintBox, _("wallet_phrase_12words"));
        lv_obj_align(checkBox, LV_ALIGN_DEFAULT, 30, 630);
        lv_obj_add_state(checkBox, LV_STATE_CHECKED);
        checkBox = GuiCreateSingleCheckBox(g_noticeHintBox, _("wallet_phrase_24words"));
        lv_obj_align(checkBox, LV_ALIGN_DEFAULT, 30, 618 + 100);
    }

    lv_obj_add_event_cb(g_noticeHintBox, SelectCheckBoxHandler, LV_EVENT_CLICKED, &currentIndex);
}

static void ResetBtnHandler(lv_event_t *e)
{
    ResetConfirmInput();
}

int8_t GuiSinglePhraseNextTile(void)
{
    switch (g_singlePhraseTileView.currentTile) {
    case SINGLE_PHRASE_CONNECT:
        return SUCCESS_CODE;
    case SINGLE_PHRASE_RANDOM_PHRASE:
        SetNavBarLeftBtn(g_pageWidget->navBarWidget, NVS_BAR_RETURN, ReturnHandler, NULL);
        SetRightBtnLabel(g_pageWidget->navBarWidget, NVS_BAR_WORD_RESET, _("single_phrase_reset"));
        SetRightBtnCb(g_pageWidget->navBarWidget, ResetBtnHandler, NULL);
        g_confirmPhraseKb->wordCnt = g_phraseCnt;
        lv_obj_add_flag(g_changeCont, LV_OBJ_FLAG_HIDDEN);
        while (!SecretCacheGetMnemonic()) {
            UserDelay(10);
        }
        ArrayRandom(SecretCacheGetMnemonic(), g_randomBuff, g_phraseCnt);
        GuiUpdateMnemonicKeyBoard(g_confirmPhraseKb, g_randomBuff, true);
        break;
    case SINGLE_PHRASE_CONFIRM_PHRASE:
        SetNavBarLeftBtn(g_pageWidget->navBarWidget, NVS_LEFT_BUTTON_BUTT, NULL, NULL);
        SetNavBarRightBtn(g_pageWidget->navBarWidget, NVS_RIGHT_BUTTON_BUTT, NULL, NULL);
        break;
    }

    g_singlePhraseTileView.currentTile++;
    lv_obj_set_tile_id(g_singlePhraseTileView.tileView, g_singlePhraseTileView.currentTile, 0, LV_ANIM_OFF);
    return SUCCESS_CODE;
}

int8_t GuiSinglePhrasePrevTile(void)
{
    switch (g_singlePhraseTileView.currentTile) {
    case SINGLE_PHRASE_RANDOM_PHRASE:
        GuiCLoseCurrentWorkingView();
        return SUCCESS_CODE;
    case SINGLE_PHRASE_CONFIRM_PHRASE:
        ResetConfirmInput();
        lv_obj_clear_flag(g_changeCont, LV_OBJ_FLAG_HIDDEN);
        SetNavBarLeftBtn(g_pageWidget->navBarWidget, NVS_BAR_RETURN, CloseCurrentViewHandler, NULL);
        if (!g_isTon) {
            SetRightBtnLabel(g_pageWidget->navBarWidget, NVS_BAR_WORD_SELECT, g_phraseCnt == 12 ? "12" : "24");
            SetRightBtnCb(g_pageWidget->navBarWidget, SelectPhraseCntHandler, NULL);
        } else {
            SetNavBarRightBtn(g_pageWidget->navBarWidget, NVS_BAR_QUESTION_MARK, OpenTonTutorial, NULL);
        }
        break;
    case SINGLE_PHRASE_WRITE_SE:
        SetRightBtnLabel(g_pageWidget->navBarWidget, NVS_BAR_WORD_RESET, _("single_phrase_reset"));
        SetRightBtnCb(g_pageWidget->navBarWidget, ResetBtnHandler, NULL);
        ResetConfirmInput();
        break;
    }

    g_singlePhraseTileView.currentTile--;
    lv_obj_set_tile_id(g_singlePhraseTileView.tileView, g_singlePhraseTileView.currentTile, 0, LV_ANIM_OFF);
    return SUCCESS_CODE;
}

void GuiSinglePhraseDeInit(void)
{
    GUI_DEL_OBJ(g_noticeHintBox)
    GUI_DEL_OBJ(g_noticeWindow);

    for (int i = 0; i < SINGLE_PHRASE_MAX_WORDS; i++) {
        g_pressedBtn[i] = 0;
        g_pressedBtnFlag[i] = 0;
    }
    g_currId = 0;
    g_phraseCnt = SINGLE_PHRASE_MAX_WORDS;
    lv_obj_del(g_changeCont);
    lv_obj_del(g_randomPhraseKb->cont);
    lv_obj_del(g_confirmPhraseKb->cont);
    GUI_DEL_OBJ(g_singlePhraseTileView.cont)
    memset_s(g_randomBuff, 512, 0, 512);
    if (g_pageWidget != NULL) {
        DestroyPageWidget(g_pageWidget);
        g_pageWidget = NULL;
    }
}

void GuiSinglePhraseRefresh(void)
{
    SetNavBarLeftBtn(g_pageWidget->navBarWidget, NVS_BAR_RETURN, ReturnHandler, NULL);
    if (g_singlePhraseTileView.currentTile == SINGLE_PHRASE_RANDOM_PHRASE) {
        if (!g_isTon) {
            SetRightBtnLabel(g_pageWidget->navBarWidget, NVS_BAR_WORD_SELECT, g_phraseCnt == 12 ? "12" : "24");
            SetRightBtnCb(g_pageWidget->navBarWidget, SelectPhraseCntHandler, NULL);
        } else {
            SetNavBarRightBtn(g_pageWidget->navBarWidget, NVS_BAR_QUESTION_MARK, OpenTonTutorial, NULL);
        }
        SetNavBarLeftBtn(g_pageWidget->navBarWidget, NVS_BAR_RETURN, CloseCurrentViewHandler, NULL);
    } else if (g_singlePhraseTileView.currentTile == SINGLE_PHRASE_CONFIRM_PHRASE) {
        SetRightBtnLabel(g_pageWidget->navBarWidget, NVS_BAR_WORD_RESET, _("single_phrase_reset"));
        SetRightBtnCb(g_pageWidget->navBarWidget, ResetBtnHandler, NULL);
    }
    SetNavBarMidBtn(g_pageWidget->navBarWidget, NVS_MID_BUTTON_BUTT, NULL, NULL);
}