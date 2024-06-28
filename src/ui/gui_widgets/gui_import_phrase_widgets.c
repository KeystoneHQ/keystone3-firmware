#include "gui.h"
#include "gui_views.h"
#include "gui_status_bar.h"
#include "gui_keyboard.h"
#include "gui_button.h"
#include "gui_hintbox.h"
#include "bip39.h"
#include "user_memory.h"
#include "bip39_english.h"
#include "gui_model.h"
#include "secret_cache.h"
#include "background_task.h"
#include "gui_import_phrase_widgets.h"
#include "gui_single_phrase_widgets.h"
#include "gui_mnemonic_input.h"
#include "gui_page.h"
typedef enum {
    SINGLE_PHRASE_INPUT_PHRASE = 0,
    SINGLE_PHRASE_WRITE_SE,

    SINGLE_PHRASE_BUTT,
} SINGLE_PHRASE_ENUM;

typedef struct ImportSinglePhraseWidget {
    uint8_t     currentTile;
    lv_obj_t    *cont;
    lv_obj_t    *tileView;
    lv_obj_t    *inputPhrase;
    lv_obj_t    *writeSe;
} ImportSinglePhraseWidget_t;

static ImportSinglePhraseWidget_t g_importSinglePhraseTileView;
static MnemonicKeyBoard_t *g_importMkb;
static KeyBoard_t *g_importPhraseKb;
static lv_obj_t *g_importPhraseKbCont = NULL;
static uint8_t g_inputWordsCnt = 0;
static lv_obj_t *g_buttonCont = NULL;
static PageWidget_t *g_pageWidget;
static lv_obj_t *g_noticeWindow = NULL;

void GuiImportPhraseWriteSe(bool en, int32_t errCode)
{
    if (en == true) {
        ClearMnemonicKeyboard(g_importMkb, &g_importMkb->currentId);
    } else {
        lv_btnmatrix_set_selected_btn(g_importMkb->btnm, g_importMkb->currentId - 1);
        g_importMkb->currentId--;
    }
    GuiWriteSeResult(en, errCode);
}

static void ResetClearImportHandler(lv_event_t * e)
{
    ClearMnemonicKeyboard(g_importMkb, &g_importMkb->currentId);
    GuiClearKeyBoard(g_importPhraseKb);
}

static void ImportPhraseWordsHandler(lv_event_t* e)
{
    ImportSinglePhraseWords(g_importMkb, g_importPhraseKb);
}

static void GuiImportTonMnemonicHandler(lv_event_t *e)
{
    GUI_DEL_OBJ(g_noticeWindow)
    GuiEmitSignal(SIG_SETUP_VIEW_TILE_NEXT, NULL, 0);
    TonData_t ton = {
        .forget = false
    };

    GuiUpdateTonWriteSeWidget(g_importSinglePhraseTileView.writeSe);
    GuiCreateCircleAroundAnimation(lv_scr_act(), -40);
    GuiModelTonCalWriteSe(ton);
}

void GuiShowTonMnemonicHint()
{
    GUI_DEL_OBJ(g_noticeWindow)

    g_noticeWindow = GuiCreateGeneralHintBox(&coinTon, _("import_ton_mnemonic_title"), _("import_ton_mnemonic_desc"), NULL, _("Cancel"), WHITE_COLOR_OPA20, _("Import"), ORANGE_COLOR);

    lv_obj_t *btn = GuiGetHintBoxRightBtn(g_noticeWindow);
    lv_obj_add_event_cb(btn, GuiImportTonMnemonicHandler, LV_EVENT_CLICKED, &g_noticeWindow);

    btn = GuiGetHintBoxLeftBtn(g_noticeWindow);
    lv_obj_add_event_cb(btn, CloseHintBoxHandler, LV_EVENT_CLICKED, &g_noticeWindow);
}

static void GuiInputPhraseWidget(lv_obj_t *parent)
{
    lv_obj_set_scrollbar_mode(parent, LV_SCROLLBAR_MODE_OFF);
    lv_obj_clear_flag(parent, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(parent, LV_OBJ_FLAG_CHECKABLE);

    lv_keyboard_user_mode_t kbMode = GuiGetMnemonicKbType(g_inputWordsCnt);
    lv_obj_set_style_bg_opa(parent, LV_OPA_0, LV_PART_SCROLLBAR | LV_STATE_SCROLLED);
    lv_obj_set_style_bg_opa(parent, LV_OPA_0, LV_PART_SCROLLBAR | LV_STATE_DEFAULT);
    lv_obj_t *label = GuiCreateTitleLabel(parent, _("import_wallet_phrase_title"));
    lv_obj_align(label, LV_ALIGN_DEFAULT, 36, 156 - GUI_MAIN_AREA_OFFSET);

    label = GuiCreateNoticeLabel(parent, _("import_wallet_phrase_desc"));
    GuiAlignToPrevObj(label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 12);

    g_importMkb = GuiCreateMnemonicKeyBoard(parent, GuiMnemonicInputHandler, kbMode, NULL);
    g_importMkb->currentId = 0;
    g_importMkb->intputType = MNEMONIC_INPUT_IMPORT_VIEW;
    lv_obj_set_size(g_importMkb->cont, 408, 236);
    lv_obj_align(g_importMkb->cont, LV_ALIGN_TOP_MID, 0, 312 - GUI_MAIN_AREA_OFFSET);
    lv_btnmatrix_set_selected_btn(g_importMkb->btnm, g_importMkb->currentId);

    //button container
    lv_obj_t *cont = GuiCreateContainer(lv_obj_get_width(lv_scr_act()), 114);
    g_buttonCont = cont;
    lv_obj_set_align(cont, LV_ALIGN_BOTTOM_MID);
    lv_obj_set_style_bg_opa(cont, LV_OPA_0, 0);
    lv_obj_t *btn = GuiCreateBtn(cont, USR_SYMBOL_ARROW_NEXT);
    lv_obj_add_event_cb(btn, ImportPhraseWordsHandler, LV_EVENT_CLICKED, NULL);
    lv_obj_align(btn, LV_ALIGN_BOTTOM_RIGHT, -20, -20);
    g_importMkb->nextButton = btn;

    //Keyboard
    g_importPhraseKbCont = GuiCreateContainer(lv_obj_get_width(lv_scr_act()), 242);
    lv_obj_set_align(g_importPhraseKbCont, LV_ALIGN_BOTTOM_MID);
    lv_obj_set_style_bg_opa(g_importPhraseKbCont, LV_OPA_0, 0);
    g_importPhraseKb = GuiCreateLetterKeyBoard(g_importPhraseKbCont, NULL, true, g_importMkb);
    g_importMkb->letterKb = g_importPhraseKb;
    g_importMkb->currentId = 0;
}

void GuiImportPhraseUpdateKeyboard(void)
{
    GuiKeyBoardSetMode(g_importPhraseKb);
}

void GuiImportPhraseInit(uint8_t num)
{
    g_inputWordsCnt = num;
    g_pageWidget = CreatePageWidget();
    lv_obj_t *cont = g_pageWidget->contentZone;
    lv_obj_t *tileView = GuiCreateTileView(cont);
    lv_obj_t *tile = lv_tileview_add_tile(tileView, SINGLE_PHRASE_INPUT_PHRASE, 0, LV_DIR_RIGHT);
    g_importSinglePhraseTileView.inputPhrase = tile;
    GuiInputPhraseWidget(tile);

    tile = lv_tileview_add_tile(tileView, SINGLE_PHRASE_WRITE_SE, 0, LV_DIR_HOR);
    g_importSinglePhraseTileView.writeSe = tile;
    GuiWriteSeWidget(tile);

    g_importSinglePhraseTileView.currentTile = SINGLE_PHRASE_INPUT_PHRASE;
    g_importSinglePhraseTileView.tileView = tileView;
    g_importSinglePhraseTileView.cont = cont;

    lv_obj_set_tile_id(g_importSinglePhraseTileView.tileView, g_importSinglePhraseTileView.currentTile, 0, LV_ANIM_OFF);
}

int8_t GuiImportPhraseNextTile(void)
{
    switch (g_importSinglePhraseTileView.currentTile) {
    case SINGLE_PHRASE_INPUT_PHRASE:
        if (g_buttonCont != NULL) lv_obj_add_flag(g_buttonCont, LV_OBJ_FLAG_HIDDEN);
        SetNavBarLeftBtn(g_pageWidget->navBarWidget, NVS_LEFT_BUTTON_BUTT, NULL, NULL);
        SetNavBarRightBtn(g_pageWidget->navBarWidget, NVS_RIGHT_BUTTON_BUTT, NULL, NULL);
        break;
    }

    g_importSinglePhraseTileView.currentTile++;
    lv_obj_set_tile_id(g_importSinglePhraseTileView.tileView, g_importSinglePhraseTileView.currentTile, 0, LV_ANIM_OFF);
    return SUCCESS_CODE;
}

int8_t GuiImportPhrasePrevTile(void)
{
    switch (g_importSinglePhraseTileView.currentTile) {
    case SINGLE_PHRASE_INPUT_PHRASE:
        break;
    case SINGLE_PHRASE_WRITE_SE:
        if (g_buttonCont != NULL) lv_obj_clear_flag(g_buttonCont, LV_OBJ_FLAG_HIDDEN);
        SetNavBarLeftBtn(g_pageWidget->navBarWidget, NVS_BAR_RETURN, CloseCurrentViewHandler, NULL);
        GuiImportPhraseRefresh();
        break;
    }

    g_importSinglePhraseTileView.currentTile--;
    lv_obj_set_tile_id(g_importSinglePhraseTileView.tileView, g_importSinglePhraseTileView.currentTile, 0, LV_ANIM_OFF);
    return SUCCESS_CODE;
}

void GuiImportPhraseRefresh(void)
{
    SetNavBarLeftBtn(g_pageWidget->navBarWidget, NVS_BAR_RETURN, ReturnHandler, NULL);
    if (g_importSinglePhraseTileView.currentTile == SINGLE_PHRASE_INPUT_PHRASE) {
        SetNavBarLeftBtn(g_pageWidget->navBarWidget, NVS_BAR_RETURN, CloseCurrentViewHandler, NULL);
    }
    SetNavBarMidBtn(g_pageWidget->navBarWidget, NVS_MID_BUTTON_BUTT, NULL, NULL);
    SetRightBtnLabel(g_pageWidget->navBarWidget, NVS_BAR_WORD_RESET, _("import_wallet_phrase_clear_btn"));
    SetRightBtnCb(g_pageWidget->navBarWidget, ResetClearImportHandler, NULL);
}

void GuiImportPhraseDeInit(void)
{
    GuiMnemonicHintboxClear();
    GuiClearKeyBoard(g_importPhraseKb);
    GuiClearMnemonicKeyBoard(g_importMkb);
    GUI_DEL_OBJ(g_noticeWindow)
    lv_obj_del(g_importPhraseKbCont);
    lv_obj_del(g_buttonCont);
    lv_obj_del(g_importSinglePhraseTileView.cont);
    CLEAR_OBJECT(g_importMkb);
    if (g_pageWidget != NULL) {
        DestroyPageWidget(g_pageWidget);
        g_pageWidget = NULL;
    }
}
