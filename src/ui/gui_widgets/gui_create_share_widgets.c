#include "gui.h"

#include "gui_views.h"
#include "gui_status_bar.h"
#include "gui_keyboard.h"
#include "gui_button.h"
#include "gui_hintbox.h"
#include "gui_model.h"
#include "gui_create_wallet_widgets.h"
#include "slip39.h"
#include "user_memory.h"
#include "secret_cache.h"
#include "background_task.h"
#include "user_utils.h"
#include "motor_manager.h"
#include "gui_page.h"
#ifndef COMPILE_SIMULATOR
#include "safe_mem_lib.h"
#else
#define memset_s(p, s, c, l) memset(p, c, l)
#endif

typedef enum {
    CREATE_SHARE_SELECT_SLICE = 0,
    CREATE_SHARE_CUSTODIAN,
    CREATE_SHARE_BACKUPFROM,
    CREATE_SHARE_CONFIRM,
    CREATE_SHARE_WRITE_SE,

    CREATE_SHARE_BUTT,
} CREATE_SHARE_ENUM;

typedef struct CreateShareWidget {
    uint8_t     currentTile;
    uint8_t     currentSlice;
    lv_obj_t    *cont;
    lv_obj_t    *tileView;
    lv_obj_t    *selectSlice;
    lv_obj_t    *custodian;
    lv_obj_t    *backupFrom;
    lv_obj_t    *confirm;
    lv_obj_t    *writeSe;
} CreateShareWidget_t;
static CreateShareWidget_t g_createShareTileView;

typedef struct {
    lv_obj_t    *cont;
    uint8_t     memberCnt;
    uint8_t     memberThreshold;
    lv_obj_t    *stepLabel;
    lv_obj_t    *memberCntKb;
    lv_obj_t    *memberThresholdKb;
    lv_obj_t    *stepCont;
} SelectSliceWidget_t;
static SelectSliceWidget_t g_selectSliceTile;

typedef struct {
    lv_obj_t    *cont;
    lv_obj_t    *titleLabel;
    lv_obj_t    *noticeLabel;
} CustodianWidget_t;
static CustodianWidget_t g_custodianTile;

typedef struct {
    MnemonicKeyBoard_t *keyBoard;
    lv_obj_t    *nextCont;
    lv_obj_t    *noticeLabel;
} ShareBackupWidget_t;
static ShareBackupWidget_t g_shareBackupTile;
static ShareBackupWidget_t g_shareConfirmTile;

static uint8_t g_phraseCnt = 33;
static uint8_t g_pressedBtn[SLIP39_MNEMONIC_WORDS_MAX + 1];
static uint8_t g_pressedBtnFlag[SLIP39_MNEMONIC_WORDS_MAX + 1];
static uint8_t g_currId = 0;
static char g_randomBuff[512];
static lv_obj_t *g_noticeHintBox = NULL;
static PageWidget_t *g_pageWidget;

static void ShareUpdateTileHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *obj = lv_event_get_target(e);

    if (code == LV_EVENT_CLICKED) {
        if (lv_event_get_user_data(e) == NULL) {
            GuiEmitSignal(SIG_SETUP_VIEW_TILE_NEXT, NULL, 0);
        } else if (lv_event_get_user_data(e) != NULL) {
            lv_obj_del(lv_obj_get_parent(obj));
            GuiEmitSignal(SIG_CREATE_SHARE_VIEW_NEXT_SLICE, &g_createShareTileView.currentSlice,
                          sizeof(g_createShareTileView.currentSlice));
        }
    }
}

static void ContinueStopCreateHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *obj = lv_event_get_target(e);

    if (code == LV_EVENT_CLICKED) {
        lv_obj_del(lv_obj_get_parent(obj));
        g_noticeHintBox = NULL;
        if (lv_event_get_user_data(e) != NULL) {
            g_createShareTileView.currentSlice = 0;
            GuiCLoseCurrentWorkingView();
        }
    }
}

static void ResetBtnTest(void)
{
    for (int i = 0; i < g_phraseCnt; i++) {
        g_pressedBtn[i] = 0;
        g_pressedBtnFlag[i] = 0;
    }
    g_currId = 0;
    lv_btnmatrix_clear_btn_ctrl_all(g_shareConfirmTile.keyBoard->btnm, LV_BTNMATRIX_CTRL_CHECKED);
    // GuiUpdateMnemonicKeyBoard(g_shareConfirmTile.keyBoard, SecretCacheGetSlip39Mnemonic(g_createShareTileView.currentSlice), true);
    GuiUpdateMnemonicKeyBoard(g_shareConfirmTile.keyBoard, g_randomBuff, true);
}

static void ResetBtnHandler(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        ResetBtnTest();
    }
}

void GuiCreateShareUpdateMnemonic(void *signalParam, uint16_t paramLen)
{
    g_shareBackupTile.keyBoard->wordCnt = g_phraseCnt;
    GuiUpdateMnemonicKeyBoard(g_shareBackupTile.keyBoard, SecretCacheGetSlip39Mnemonic(g_createShareTileView.currentSlice), false);
    g_shareConfirmTile.keyBoard->wordCnt = g_phraseCnt;
    ArrayRandom(SecretCacheGetSlip39Mnemonic(g_createShareTileView.currentSlice), g_randomBuff, g_phraseCnt);
    GuiUpdateMnemonicKeyBoard(g_shareConfirmTile.keyBoard, g_randomBuff, true);
    GuiStopCircleAroundAnimation();
}

static void StopCreateViewHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_CLICKED) {
        g_noticeHintBox = GuiCreateHintBox(lv_scr_act(), 480, 416, false);
        lv_obj_t *img = GuiCreateImg(g_noticeHintBox, &imgWarn);
        lv_obj_align(img, LV_ALIGN_DEFAULT, 36, 432);
        lv_obj_t *label = GuiCreateLittleTitleLabel(g_noticeHintBox, _("shamir_phrase_cancel_create_title"));
        lv_obj_align(label, LV_ALIGN_DEFAULT, 36, 528);
        label = GuiCreateIllustrateLabel(g_noticeHintBox, _("shamir_phrase_cancel_create_desc"));
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

static void NumSelectSliceHandler(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *obj = lv_event_get_target(e);
    uint32_t currentId = lv_btnmatrix_get_selected_btn(obj);
    if (code == LV_EVENT_DRAW_PART_BEGIN) {
        if (currentId >= 0xFFFF) {
            return;
        }
        lv_obj_draw_part_dsc_t * dsc = lv_event_get_draw_part_dsc(e);
        if (dsc->class_p == &lv_btnmatrix_class && dsc->type == LV_BTNMATRIX_DRAW_PART_BTN) {
            if (currentId == dsc->id) {
                dsc->rect_dsc->border_color = ORANGE_COLOR;
            } else {
                dsc->rect_dsc->border_color = DARK_BG_COLOR;
            }
        }
    } else if (code == LV_EVENT_CLICKED) {
        Vibrate(SLIGHT);
        if (currentId >= 0xFFFF) {
            lv_btnmatrix_set_selected_btn(g_selectSliceTile.memberThresholdKb, g_selectSliceTile.memberThreshold - 2);
            lv_btnmatrix_set_selected_btn(g_selectSliceTile.memberCntKb, g_selectSliceTile.memberCnt - 2);
            return;
        }
        if (obj == g_selectSliceTile.memberCntKb) {
            g_selectSliceTile.memberCnt = currentId + 2;
            lv_btnmatrix_set_selected_btn(g_selectSliceTile.memberThresholdKb, g_selectSliceTile.memberThreshold - 2);
            GuiUpdateSsbKeyBoard(g_selectSliceTile.memberThresholdKb, g_selectSliceTile.memberCnt);
            if (g_selectSliceTile.memberThreshold > g_selectSliceTile.memberCnt) {
                g_selectSliceTile.memberThreshold = g_selectSliceTile.memberCnt;
                lv_btnmatrix_set_selected_btn(g_selectSliceTile.memberThresholdKb, g_selectSliceTile.memberCnt - 2);
            }
        } else {
            g_selectSliceTile.memberThreshold = currentId + 2;
            lv_btnmatrix_set_selected_btn(g_selectSliceTile.memberCntKb, g_selectSliceTile.memberCnt - 2);
        }
        char tempBuf[8];
        sprintf(tempBuf, "%d/%d", g_selectSliceTile.memberThreshold, g_selectSliceTile.memberCnt);
        lv_label_set_text(g_selectSliceTile.stepLabel, tempBuf);
    }
}

static void GuiShareSelectSliceWidget(lv_obj_t *parent)
{
    lv_obj_set_style_bg_opa(parent, LV_OPA_0, LV_PART_SCROLLBAR | LV_STATE_SCROLLED);
    lv_obj_set_style_bg_opa(parent, LV_OPA_0, LV_PART_SCROLLBAR | LV_STATE_DEFAULT);
    lv_obj_t *label = GuiCreateIllustrateLabel(parent, _("shamir_phrase_number"));
    lv_obj_align(label, LV_ALIGN_DEFAULT, 36, 156 - GUI_MAIN_AREA_OFFSET);

    label = GuiCreateIllustrateLabel(parent, _("Threshold"));
    lv_obj_align(label, LV_ALIGN_DEFAULT, 36, 438 - GUI_MAIN_AREA_OFFSET);

    g_selectSliceTile.memberCnt = SLIP39_DEFAULT_MEMBER_COUNT;
    g_selectSliceTile.memberThreshold = SLIP39_DEFAULT_MEMBER_THRESHOLD;

    lv_obj_t *btnm = GuiCreateNumKeyboard(parent, NumSelectSliceHandler, NUM_KEYBOARD_SLICE, NULL);
    lv_obj_align(btnm, LV_ALIGN_DEFAULT, 36, 198 - GUI_MAIN_AREA_OFFSET);
    lv_btnmatrix_set_selected_btn(btnm, g_selectSliceTile.memberCnt - 2);
    g_selectSliceTile.memberCntKb = btnm;

    btnm = GuiCreateNumKeyboard(parent, NumSelectSliceHandler, NUM_KEYBOARD_SLICE, g_selectSliceTile.memberThresholdKb);
    lv_obj_align(btnm, LV_ALIGN_DEFAULT, 36, 480 - GUI_MAIN_AREA_OFFSET);
    lv_btnmatrix_set_selected_btn(btnm, g_selectSliceTile.memberThreshold - 2);
    g_selectSliceTile.memberThresholdKb = btnm;
    GuiUpdateSsbKeyBoard(g_selectSliceTile.memberThresholdKb, g_selectSliceTile.memberCnt);

    lv_obj_t *cont = GuiCreateContainer(lv_obj_get_width(lv_scr_act()), 114);
    lv_obj_set_align(cont, LV_ALIGN_BOTTOM_MID);
    g_selectSliceTile.stepCont = cont;

    label = GuiCreateTextLabel(cont, "3/5");
    lv_obj_set_style_text_color(label, ORANGE_COLOR, LV_PART_MAIN);
    lv_obj_align(label, LV_ALIGN_DEFAULT, 36, 24);
    g_selectSliceTile.stepLabel = label;
    label = GuiCreateIllustrateLabel(cont, _("shamir_backup"));
    lv_obj_align(label, LV_ALIGN_DEFAULT, 36, 60);

    lv_obj_t *btn = GuiCreateBtn(cont, USR_SYMBOL_CHECK);
    lv_obj_align(btn, LV_ALIGN_DEFAULT, 348, 24);
    lv_obj_add_event_cb(btn, ShareUpdateTileHandler, LV_EVENT_ALL, NULL);
}

static void GuiShareCustodianWidget(lv_obj_t *parent)
{
    uint16_t hintHeight = 48;
    lv_obj_set_style_bg_opa(parent, LV_OPA_0, LV_PART_SCROLLBAR | LV_STATE_SCROLLED);
    lv_obj_set_style_bg_opa(parent, LV_OPA_0, LV_PART_SCROLLBAR | LV_STATE_DEFAULT);
    lv_obj_t *label = GuiCreateTitleLabel(parent, _("shamir_phrase_custodian_title"));
    lv_label_set_recolor(label, true);
    lv_obj_align(label, LV_ALIGN_DEFAULT, 36, 156 - GUI_MAIN_AREA_OFFSET);
    g_custodianTile.titleLabel = label;

    label = GuiCreateIllustrateLabel(parent, _("shamir_phrase_custodian_desc"));
    lv_obj_align(label, LV_ALIGN_DEFAULT, 36, 216 - GUI_MAIN_AREA_OFFSET);
    lv_label_set_recolor(label, true);
    g_custodianTile.noticeLabel = label;

    lv_obj_t *cont = GuiCreateHintBoxWithoutTop(parent, 480, 482);
    lv_obj_t *img = GuiCreateImg(cont, &imgRedEye);
    lv_obj_align(img, LV_ALIGN_DEFAULT, 39, 48);
    hintHeight = GetHintBoxReHeight(hintHeight, img) + 24;
    label = GuiCreateLittleTitleLabel(cont, _("shamir_phrase_notice_title"));
    lv_obj_align(label, LV_ALIGN_DEFAULT, 36, 108);
    hintHeight = GetHintBoxReHeight(hintHeight, label) + 12;

    lv_obj_t *desc1 = GuiCreateIllustrateLabel(cont, _("shamir_phrase_notice_desc1"));
    lv_obj_align_to(desc1, label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 12);
    hintHeight = GetHintBoxReHeight(hintHeight, desc1) + 12;

    lv_obj_t *desc2 = GuiCreateIllustrateLabel(cont, _("shamir_phrase_notice_desc2"));
    lv_obj_align_to(desc2, desc1, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 12);
    hintHeight = GetHintBoxReHeight(hintHeight, desc1) + 130;
    // lv_obj_set_size(cont, 480, hintHeight);

    lv_obj_t *btn = GuiCreateBtn(cont, USR_SYMBOL_CHECK);
    lv_obj_align(btn, LV_ALIGN_BOTTOM_RIGHT, -36, -24);
    lv_obj_add_event_cb(btn, ShareUpdateTileHandler, LV_EVENT_CLICKED, NULL);
}

static void MnemonicConfirmHandler(lv_event_t * e)
{
    int i, j;
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *obj = lv_event_get_target(e);
    if (code == LV_EVENT_DRAW_PART_BEGIN) {
        lv_obj_draw_part_dsc_t * dsc = lv_event_get_draw_part_dsc(e);
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
        for (i = 0 ; i < g_currId; i++) {
            if (g_pressedBtn[i] == currentId + 1) {
                break;
            }
        }

        if (i == g_currId - 1) {
            lv_btnmatrix_clear_btn_ctrl(g_shareConfirmTile.keyBoard->btnm, g_currId, LV_BTNMATRIX_CTRL_CHECKED);
            g_pressedBtnFlag[g_pressedBtn[g_currId - 1] - 1] = MNEMONIC_BUTTON_UNPRESSED;
            GuiConfirmMnemonicKeyBoard(g_shareConfirmTile.keyBoard, g_randomBuff,
                                       g_pressedBtn[g_currId - 1] - 1, g_currId - 1, 0);
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
            GuiConfirmMnemonicKeyBoard(g_shareConfirmTile.keyBoard, g_randomBuff,
                                       currentId, g_currId + 1, 1);
            g_currId++;
        }

        if (g_currId == g_phraseCnt) {
            char *confirmMnemonic = SRAM_MALLOC(10 * g_phraseCnt + 1);
            confirmMnemonic[0] = 0;
            for (i = 0; i < g_phraseCnt; i++) {
                j = (g_pressedBtn[i] - 1) + (g_pressedBtn[i] - 1) / 3;
                strcat(confirmMnemonic, strchr(g_shareConfirmTile.keyBoard->mnemonicWord[j], '\n') + 1);
                if (i < g_phraseCnt - 1) {
                    strcat(confirmMnemonic, " ");
                }
            }
            Vibrate(SLIGHT);
            if (strcmp(confirmMnemonic, SecretCacheGetSlip39Mnemonic(g_createShareTileView.currentSlice)) == 0) {
                GuiEmitSignal(SIG_CREATE_SHARE_VIEW_NEXT_SLICE, NULL, 0);
                lv_obj_scroll_to_y(g_shareBackupTile.keyBoard->cont, 0, LV_ANIM_OFF);
                lv_obj_scroll_to_y(g_shareConfirmTile.keyBoard->cont, 0, LV_ANIM_OFF);
            } else {
                g_noticeHintBox = GuiCreateHintBox(lv_scr_act(), 480, 370, false);
                lv_obj_t *img = GuiCreateImg(g_noticeHintBox, &imgFailed);
                lv_obj_align(img, LV_ALIGN_DEFAULT, 36, 478);
                lv_obj_t *label = GuiCreateLittleTitleLabel(g_noticeHintBox, _("shamir_phrase_not_match_title"));
                lv_obj_align(label, LV_ALIGN_DEFAULT, 36, 574);
                label = GuiCreateIllustrateLabel(g_noticeHintBox, _("shamir_phrase_not_match_desc"));
                lv_obj_align(label, LV_ALIGN_DEFAULT, 36, 626);
                lv_obj_t *btn = GuiCreateBtn(g_noticeHintBox, _("OK"));
                lv_obj_set_style_bg_color(btn, WHITE_COLOR_OPA20, LV_PART_MAIN);
                lv_obj_align(btn, LV_ALIGN_DEFAULT, 345, 710);
                lv_obj_add_event_cb(btn, CloseHintBoxHandler, LV_EVENT_CLICKED, &g_noticeHintBox);
            }
            memset_s(confirmMnemonic, 10 * g_phraseCnt + 1, 0, 10 * g_phraseCnt + 1);
            SRAM_FREE(confirmMnemonic);
        }
    }
}

static void GuiShareBackupWidget(lv_obj_t *parent)
{
    lv_obj_set_scrollbar_mode(parent, LV_SCROLLBAR_MODE_OFF);
    // lv_obj_clear_flag(parent, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_t *label = GuiCreateTitleLabel(parent, _("shamir_phrase_backup_title"));
    lv_obj_align(label, LV_ALIGN_DEFAULT, 36, 156 - GUI_MAIN_AREA_OFFSET);

    label = GuiCreateIllustrateLabel(parent, _("shamir_phrase_backup_desc"));
    lv_label_set_recolor(label, true);
    lv_obj_align(label, LV_ALIGN_DEFAULT, 36, 216 - GUI_MAIN_AREA_OFFSET);
    g_shareBackupTile.noticeLabel = label;

    g_shareBackupTile.keyBoard = GuiCreateMnemonicKeyBoard(parent, NULL, KEY_STONE_MNEMONIC_24, NULL);
    lv_obj_align_to(g_shareBackupTile.keyBoard->cont, label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 38);
    lv_obj_set_size(g_shareBackupTile.keyBoard->cont, 408, 360);

    lv_obj_t *cont = GuiCreateContainer(lv_obj_get_width(lv_scr_act()), 114);
    lv_obj_add_flag(cont, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_align(cont, LV_ALIGN_BOTTOM_MID);
    g_shareBackupTile.nextCont = cont;

    lv_obj_t *btn = GuiCreateBtn(cont, "");
    lv_obj_t *img = GuiCreateImg(btn, &imgArrowNext);
    lv_obj_set_align(img, LV_ALIGN_CENTER);
    lv_obj_align(btn, LV_ALIGN_DEFAULT, 348, 24);
    lv_obj_add_event_cb(btn, ShareUpdateTileHandler, LV_EVENT_ALL, NULL);
}

static void GuiShareConfirmWidget(lv_obj_t *parent)
{
    lv_obj_t *label = GuiCreateTitleLabel(parent, _("shamir_phrase_confirm_title"));
    lv_obj_align(label, LV_ALIGN_DEFAULT, 36, 156 - GUI_MAIN_AREA_OFFSET);

    label = GuiCreateNoticeLabel(parent, _("shamir_phrase_confirm_desc"));
    lv_label_set_recolor(label, true);
    lv_obj_align(label, LV_ALIGN_DEFAULT, 36, 216 - GUI_MAIN_AREA_OFFSET);
    g_shareConfirmTile.noticeLabel = label;

    g_shareConfirmTile.keyBoard = GuiCreateMnemonicKeyBoard(parent, MnemonicConfirmHandler, KEY_STONE_MNEMONIC_24, NULL);
    lv_obj_align_to(g_shareConfirmTile.keyBoard->cont, label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 36);
}

void GuiCreateShareInit(void)
{
    g_pageWidget = CreatePageWidget();
    lv_obj_t *cont = g_pageWidget->contentZone;

    lv_obj_t *tileView = GuiCreateTileView(cont);
    lv_obj_t *tile = lv_tileview_add_tile(tileView, CREATE_SHARE_SELECT_SLICE, 0, LV_DIR_HOR);
    g_createShareTileView.selectSlice = tile;
    GuiShareSelectSliceWidget(tile);

    tile = lv_tileview_add_tile(tileView, CREATE_SHARE_CUSTODIAN, 0, LV_DIR_HOR);
    g_createShareTileView.custodian = tile;
    GuiShareCustodianWidget(tile);

    tile = lv_tileview_add_tile(tileView, CREATE_SHARE_BACKUPFROM, 0, LV_DIR_HOR);
    g_createShareTileView.backupFrom = tile;
    GuiShareBackupWidget(tile);

    tile = lv_tileview_add_tile(tileView, CREATE_SHARE_CONFIRM, 0, LV_DIR_HOR);
    g_createShareTileView.confirm = tile;
    GuiShareConfirmWidget(tile);

    tile = lv_tileview_add_tile(tileView, CREATE_SHARE_WRITE_SE, 0, LV_DIR_HOR);
    g_createShareTileView.writeSe = tile;
    GuiWriteSeWidget(tile);

    g_createShareTileView.currentTile = CREATE_SHARE_SELECT_SLICE;
    g_createShareTileView.tileView = tileView;
    g_createShareTileView.cont = cont;

    lv_obj_set_tile_id(g_createShareTileView.tileView, g_createShareTileView.currentTile, 0, LV_ANIM_OFF);
}

int8_t GuiCreateShareNextSlice(void)
{
    g_createShareTileView.currentSlice++;
    if (g_createShareTileView.currentSlice == g_selectSliceTile.memberCnt) {
        // GuiModelWriteSe();
        GuiEmitSignal(SIG_SETUP_VIEW_TILE_NEXT, NULL, 0);
        return SUCCESS_CODE;
    }
    lv_label_set_text_fmt(g_custodianTile.titleLabel, _("shamir_phrase_share_number_fmt"), g_createShareTileView.currentSlice + 1, g_selectSliceTile.memberCnt);
    lv_label_set_text_fmt(g_custodianTile.noticeLabel, _("shamir_phrase_share_notice_fmt"), g_createShareTileView.currentSlice + 1);
    lv_label_set_text_fmt(g_shareBackupTile.noticeLabel, _("shamir_phrase_share_backup_notice_fmt"), g_createShareTileView.currentSlice + 1);
    lv_label_set_text_fmt(g_shareConfirmTile.noticeLabel, _("shamir_phrase_share_confirm_notice_fmt"), g_createShareTileView.currentSlice + 1);

    g_createShareTileView.currentTile = CREATE_SHARE_CUSTODIAN;
    ResetBtnTest();
    GuiUpdateMnemonicKeyBoard(g_shareBackupTile.keyBoard, SecretCacheGetSlip39Mnemonic(g_createShareTileView.currentSlice), false);
    SetNavBarRightBtn(g_pageWidget->navBarWidget, NVS_RIGHT_BUTTON_BUTT, NULL, NULL);
    lv_obj_set_tile_id(g_createShareTileView.tileView, g_createShareTileView.currentTile, 0, LV_ANIM_OFF);
    ArrayRandom(SecretCacheGetSlip39Mnemonic(g_createShareTileView.currentSlice), g_randomBuff, g_phraseCnt);
    GuiUpdateMnemonicKeyBoard(g_shareConfirmTile.keyBoard, g_randomBuff, true);
    return SUCCESS_CODE;
}

int8_t GuiCreateShareNextTile(void)
{
    Slip39Data_t slip39 = {
        .threShold = g_selectSliceTile.memberThreshold,
        .memberCnt = g_selectSliceTile.memberCnt,
        .wordCnt = g_phraseCnt,
    };
    switch (g_createShareTileView.currentTile) {
    case CREATE_SHARE_SELECT_SLICE:
        GuiModelSlip39UpdateMnemonic(slip39);
        lv_label_set_text_fmt(g_custodianTile.titleLabel, _("shamir_phrase_share_number_fmt"), g_createShareTileView.currentSlice + 1, g_selectSliceTile.memberCnt);
        SetNavBarLeftBtn(g_pageWidget->navBarWidget, NVS_BAR_CLOSE, StopCreateViewHandler, NULL);
        lv_obj_add_flag(g_selectSliceTile.stepCont, LV_OBJ_FLAG_HIDDEN);
        break;
    case CREATE_SHARE_CUSTODIAN:
        lv_obj_clear_flag(g_shareBackupTile.nextCont, LV_OBJ_FLAG_HIDDEN);
        break;
    case CREATE_SHARE_BACKUPFROM:
        SetRightBtnLabel(g_pageWidget->navBarWidget, NVS_BAR_WORD_RESET, USR_SYMBOL_RESET "Reset");
        SetRightBtnCb(g_pageWidget->navBarWidget, ResetBtnHandler, NULL);
        lv_obj_add_flag(g_shareBackupTile.nextCont, LV_OBJ_FLAG_HIDDEN);
        break;
    case CREATE_SHARE_CONFIRM:
        SetNavBarLeftBtn(g_pageWidget->navBarWidget, NVS_LEFT_BUTTON_BUTT, NULL, NULL);
        SetNavBarRightBtn(g_pageWidget->navBarWidget, NVS_RIGHT_BUTTON_BUTT, NULL, NULL);
        GuiModelSlip39WriteSe();
        break;
    }
    g_createShareTileView.currentTile++;
    lv_obj_set_tile_id(g_createShareTileView.tileView, g_createShareTileView.currentTile, 0, LV_ANIM_OFF);
    return SUCCESS_CODE;
}

int8_t GuiCreateSharePrevTile(void)
{
    switch (g_createShareTileView.currentTile) {
    case CREATE_SHARE_SELECT_SLICE:
        break;
    case CREATE_SHARE_CUSTODIAN:
        lv_obj_clear_flag(g_selectSliceTile.stepCont, LV_OBJ_FLAG_HIDDEN);
        break;
    case CREATE_SHARE_BACKUPFROM:
        lv_obj_add_flag(g_shareBackupTile.nextCont, LV_OBJ_FLAG_HIDDEN);
        break;
    }

    g_createShareTileView.currentTile--;
    lv_obj_set_tile_id(g_createShareTileView.tileView, g_createShareTileView.currentTile, 0, LV_ANIM_OFF);
    return SUCCESS_CODE;
}

void GuiCreateShareDeInit(void)
{
    GUI_DEL_OBJ(g_noticeHintBox)
    for (int i = 0; i < g_phraseCnt; i++) {
        g_pressedBtn[i] = 0;
        g_pressedBtnFlag[i] = 0;
    }
    g_currId = 0;
    g_selectSliceTile.memberCnt = SLIP39_DEFAULT_MEMBER_COUNT;
    g_selectSliceTile.memberThreshold = SLIP39_DEFAULT_MEMBER_THRESHOLD;
    // for (int i = 0; i < SLIP39_MNEMONIC_WORDS_MAX; i++) {
    //     memset(g_shareBackupTile.words, 0, 10);
    //     memset(g_shareConfirmTile.words, 0, 10);
    // }
    memset_s(g_randomBuff, 512, 0, 512);
    GUI_DEL_OBJ(g_shareBackupTile.nextCont)
    GUI_DEL_OBJ(g_selectSliceTile.stepCont)
    GUI_DEL_OBJ(g_createShareTileView.cont)
    CLEAR_OBJECT(g_createShareTileView);
    if (g_pageWidget != NULL) {
        DestroyPageWidget(g_pageWidget);
        g_pageWidget = NULL;
    }
}

void GuiCreateShareRefresh(void)
{
    SetNavBarLeftBtn(g_pageWidget->navBarWidget, NVS_BAR_RETURN, ReturnHandler, NULL);
    SetNavBarRightBtn(g_pageWidget->navBarWidget, NVS_RIGHT_BUTTON_BUTT, NULL, NULL);
    if (g_createShareTileView.currentTile == CREATE_SHARE_SELECT_SLICE) {
        SetNavBarLeftBtn(g_pageWidget->navBarWidget, NVS_BAR_RETURN, CloseCurrentViewHandler, NULL);
    } else if (g_createShareTileView.currentTile == CREATE_SHARE_BACKUPFROM) {
        SetNavBarLeftBtn(g_pageWidget->navBarWidget, NVS_BAR_CLOSE, StopCreateViewHandler, NULL);
        SetNavBarRightBtn(g_pageWidget->navBarWidget, NVS_RIGHT_BUTTON_BUTT, NULL, NULL);
    } else if (g_createShareTileView.currentTile == CREATE_SHARE_CONFIRM) {
        SetNavBarLeftBtn(g_pageWidget->navBarWidget, NVS_BAR_CLOSE, StopCreateViewHandler, NULL);
        SetNavBarRightBtn(g_pageWidget->navBarWidget, NVS_BAR_WORD_RESET, ResetBtnHandler, NULL);
    } else if (g_createShareTileView.currentTile == CREATE_SHARE_WRITE_SE) {
        SetNavBarRightBtn(g_pageWidget->navBarWidget, NVS_RIGHT_BUTTON_BUTT, NULL, NULL);
    }
    SetNavBarMidBtn(g_pageWidget->navBarWidget, NVS_MID_BUTTON_BUTT, NULL, NULL);
}

