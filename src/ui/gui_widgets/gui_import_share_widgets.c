#include "gui.h"

#include "gui_views.h"
#include "gui_status_bar.h"
#include "gui_keyboard.h"
#include "gui_button.h"
#include "gui_hintbox.h"
#include "slip39.h"
#include "user_memory.h"
#include "gui_model.h"
#include "secret_cache.h"
#include "gui_lock_widgets.h"
#include "gui_single_phrase_widgets.h"
#include "gui_mnemonic_input.h"
#include "gui_page.h"

#ifndef COMPILE_MAC_SIMULATOR
#include "sha256.h"
#else
#include "simulator_model.h"
#endif

typedef enum {
    IMPORT_SHARE_SSB_INPUT = 0,
    IMPORT_SHARE_WRITE_SE,

    IMPORT_SHARE_BUTT,
} IMPORT_SHARE_ENUM;

typedef struct ImportShareWidget {
    uint8_t     currentTile;
    lv_obj_t    *cont;
    lv_obj_t    *tileView;
    lv_obj_t    *ssbInput;
    lv_obj_t    *writeSe;
} ImportShareWidget_t;
static ImportShareWidget_t g_importShareTileView;

static lv_obj_t *g_nextCont = NULL;
static KeyBoard_t *g_ssbImportKb;
static MnemonicKeyBoard_t *g_importMkb;
static uint8_t g_phraseCnt = 33;
static lv_obj_t *g_noticeHintBox = NULL;
static PageWidget_t *g_pageWidget;

static void ContinueStopCreateHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_CLICKED) {
        if (lv_event_get_user_data(e) != NULL) {
            g_importMkb->currentSlice = 0;
            SetNavBarRightBtn(g_pageWidget->navBarWidget, NVS_RIGHT_BUTTON_BUTT, NULL, NULL);
            GuiCLoseCurrentWorkingView();
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
        lv_obj_t *label = GuiCreateLittleTitleLabel(g_noticeHintBox, _("import_wallet_ssb_cancel_title"));
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

static void ConfirmClearHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_CLICKED) {
        ClearMnemonicKeyboard(g_importMkb, &g_importMkb->currentId);
        GuiClearKeyBoard(g_ssbImportKb);
    }
}

void GuiImportShareWriteSe(bool en, int32_t errCode)
{
    if (en == true) {
        ClearMnemonicKeyboard(g_importMkb, &g_importMkb->currentId);
    } else {
        // if (errCode == ERR_KEYSTORE_MNEMONIC_REPEAT) {
        // } else {
        // }
        lv_btnmatrix_set_selected_btn(g_importMkb->btnm, g_importMkb->currentId - 1);
        g_importMkb->currentId--;
    }
    GuiWriteSeResult(en, errCode);
}

static void ImportShareNextSliceHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_CLICKED) {
        ImportShareNextSlice(g_importMkb, g_ssbImportKb);
    }
}

static void GuiShareSsbInputWidget(lv_obj_t *parent)
{
    lv_obj_set_style_bg_opa(parent, LV_OPA_0, LV_PART_SCROLLBAR | LV_STATE_SCROLLED);
    lv_obj_set_style_bg_opa(parent, LV_OPA_0, LV_PART_SCROLLBAR | LV_STATE_DEFAULT);
    lv_obj_t *label = NULL;

    g_importMkb = GuiCreateMnemonicKeyBoard(parent, GuiMnemonicInputHandler, g_phraseCnt == 33 ? KEY_STONE_MNEMONIC_33 : KEY_STONE_MNEMONIC_20, NULL);
    g_importMkb->intputType = MNEMONIC_INPUT_IMPORT_VIEW;
    label = GuiCreateTitleLabel(parent, "");
    lv_obj_align(label, LV_ALIGN_DEFAULT, 36, 12);
    lv_label_set_recolor(label, true);
    lv_label_set_text_fmt(label, _("import_wallet_ssb_title_fmt"), g_importMkb->currentSlice + 1);
    g_importMkb->titleLabel = label;

    label = GuiCreateIllustrateLabel(parent, _("import_wallet_ssb_desc_fmt"));
    lv_obj_align(label, LV_ALIGN_DEFAULT, 36, 72);
    lv_label_set_recolor(label, true);
    lv_obj_set_style_text_color(label, DARK_GRAY_COLOR, LV_PART_MAIN);
    lv_label_set_text_fmt(label, _("import_wallet_ssb_desc_fmt"), g_importMkb->wordCnt, g_importMkb->currentSlice + 1);
    g_importMkb->descLabel = label;

    lv_obj_set_size(g_importMkb->cont, 408, 236);
    lv_obj_align_to(g_importMkb->cont, label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 36);
    lv_btnmatrix_set_selected_btn(g_importMkb->btnm, g_importMkb->currentId);

    lv_obj_t *cont = GuiCreateContainer(lv_obj_get_width(lv_scr_act()), 114);
    lv_obj_add_flag(cont, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_align(cont, LV_ALIGN_BOTTOM_MID);
    g_nextCont = cont;

    label = GuiCreateIllustrateLabel(cont, "");
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 36, 39);
    lv_obj_set_style_text_opa(label, LV_OPA_80, 0);
    lv_obj_add_flag(label, LV_OBJ_FLAG_HIDDEN);
    g_importMkb->stepLabel = label;

    lv_obj_t *btn = GuiCreateBtn(cont, "");
    lv_obj_t *img = GuiCreateImg(btn, &imgArrowNext);
    lv_obj_set_align(img, LV_ALIGN_CENTER);
    lv_obj_align(btn, LV_ALIGN_TOP_LEFT, 348, 24);
    lv_obj_set_style_bg_opa(btn, LV_OPA_60, LV_PART_MAIN);
    // lv_obj_clear_flag(btn, LV_OBJ_FLAG_CLICKABLE);
    g_importMkb->nextButton = btn;
    lv_obj_add_event_cb(btn, ImportShareNextSliceHandler, LV_EVENT_ALL, NULL);

    cont = GuiCreateContainer(lv_obj_get_width(lv_scr_act()), 242);
    lv_obj_set_align(cont, LV_ALIGN_BOTTOM_MID);
    lv_obj_set_style_bg_opa(cont, LV_OPA_0, 0);

    g_ssbImportKb = GuiCreateLetterKeyBoard(cont, NULL, false, g_importMkb);
    g_importMkb->letterKb = g_ssbImportKb;
    g_importMkb->currentId = 0;
}

void GuiImportShareInit(uint8_t wordsCnt)
{
    g_phraseCnt = wordsCnt;
    g_pageWidget = CreatePageWidget();
    lv_obj_t *cont = g_pageWidget->contentZone;

    lv_obj_t *tileView = GuiCreateTileView(cont);
    lv_obj_t *tile = lv_tileview_add_tile(tileView, IMPORT_SHARE_SSB_INPUT, 0, LV_DIR_HOR);
    g_importShareTileView.ssbInput = tile;
    GuiShareSsbInputWidget(tile);

    tile = lv_tileview_add_tile(tileView, IMPORT_SHARE_WRITE_SE, 0, LV_DIR_HOR);
    g_importShareTileView.writeSe = tile;
    GuiWriteSeWidget(tile);

    g_importShareTileView.currentTile = IMPORT_SHARE_SSB_INPUT;
    g_importShareTileView.tileView = tileView;
    g_importShareTileView.cont = cont;

    lv_obj_set_tile_id(g_importShareTileView.tileView, g_importShareTileView.currentTile, 0, LV_ANIM_OFF);
}

int8_t GuiImportShareNextTile(void)
{
    Slip39Data_t slip39 = {
        .threShold = g_importMkb->threShold,
        .wordCnt = g_phraseCnt,
        .forget = false,
    };
    switch (g_importShareTileView.currentTile) {
    case IMPORT_SHARE_SSB_INPUT:
        SetNavBarLeftBtn(g_pageWidget->navBarWidget, NVS_LEFT_BUTTON_BUTT, NULL, NULL);
        SetNavBarRightBtn(g_pageWidget->navBarWidget, NVS_RIGHT_BUTTON_BUTT, NULL, NULL);
        GuiCreateCircleAroundAnimation(lv_scr_act(), -40);
        GuiModelSlip39CalWriteSe(slip39);
        lv_obj_add_flag(g_nextCont, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(g_ssbImportKb->cont, LV_OBJ_FLAG_HIDDEN);
        break;
    }
    g_importShareTileView.currentTile++;
    lv_obj_set_tile_id(g_importShareTileView.tileView, g_importShareTileView.currentTile, 0, LV_ANIM_OFF);
    return SUCCESS_CODE;
}

int8_t GuiImportSharePrevTile(void)
{
    switch (g_importShareTileView.currentTile) {
    case IMPORT_SHARE_WRITE_SE:
        break;
    }
    g_importShareTileView.currentTile--;
    lv_obj_set_tile_id(g_importShareTileView.tileView, g_importShareTileView.currentTile, 0, LV_ANIM_OFF);
    return SUCCESS_CODE;
}

void GuiImportShareDeInit(void)
{
    GUI_DEL_OBJ(g_noticeHintBox)
    GUI_DEL_OBJ(g_nextCont)
    CLEAR_OBJECT(g_importMkb);
    GuiMnemonicHintboxClear();
    // lv_obj_add_flag(g_ssbImportKb->cont, LV_OBJ_FLAG_HIDDEN);
    lv_obj_del(g_ssbImportKb->cont);
    lv_obj_del(g_importShareTileView.cont);
    CLEAR_OBJECT(g_ssbImportKb);
    if (g_pageWidget != NULL) {
        DestroyPageWidget(g_pageWidget);
        g_pageWidget = NULL;
    }
}

void GuiImportShareRefresh(void)
{
    if (g_importShareTileView.currentTile == IMPORT_SHARE_SSB_INPUT) {
        SetNavBarLeftBtn(g_pageWidget->navBarWidget, NVS_BAR_CLOSE, StopCreateViewHandler, NULL);
        SetRightBtnLabel(g_pageWidget->navBarWidget, NVS_BAR_WORD_RESET, _("import_wallet_phrase_clear_btn"));
        SetRightBtnCb(g_pageWidget->navBarWidget, ConfirmClearHandler, NULL);
    } else {
        SetNavBarLeftBtn(g_pageWidget->navBarWidget, NVS_LEFT_BUTTON_BUTT, NULL, NULL);
        SetNavBarRightBtn(g_pageWidget->navBarWidget, NVS_RIGHT_BUTTON_BUTT, NULL, NULL);
    }
    SetNavBarMidBtn(g_pageWidget->navBarWidget, NVS_MID_BUTTON_BUTT, NULL, NULL);
}

 