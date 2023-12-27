#include "gui.h"
#include "secret_cache.h"
#include "gui_page.h"
#include "gui_hintbox.h"
#include "user_memory.h"
#include "gui_create_wallet_widgets.h"
#include "sha256.h"
#include "log_print.h"

static void GuiCreatePage(lv_obj_t *parent);
static void OpenQuitHintBoxHandler(lv_event_t *e);
static void CloseQuitHintBoxHandler(lv_event_t *e);
static void ClickDiceHandler(lv_event_t *e);
static void InitDiceImg(lv_obj_t *img, lv_obj_t *anchor, size_t x, size_t y);
static void OnTextareaValueChangeHandler(lv_event_t *e);
static void QuitConfirmHandler(lv_event_t *e);
static void UndoClickHandler(lv_event_t *e);
static void UndoLongPressHandler(lv_event_t *e);
static void ConfirmHandler(lv_event_t *e);

static PageWidget_t *g_page;
static uint8_t g_seedType;
static lv_obj_t *g_quitHintBox;
static lv_obj_t *g_diceTextArea;
static lv_obj_t *g_rollsLabel;
static lv_obj_t *g_line;
static lv_obj_t *g_diceImgs[6];
static lv_obj_t *g_errLabel;
static lv_obj_t *g_hintLabel;
static lv_obj_t *g_confirmBtn;

void GuiDiceRollsWidgetsInit(uint8_t seed_type)
{
    g_seedType = seed_type;
    g_page = CreatePageWidget();
    GuiCreatePage(g_page->contentZone);
    SetNavBarLeftBtn(g_page->navBarWidget, NVS_BAR_RETURN, OpenQuitHintBoxHandler, NULL);
    SetNavBarRightBtn(g_page->navBarWidget, NVS_BAR_UNDO, UndoClickHandler, g_diceTextArea);
}
void GuiDiceRollsWidgetsDeInit()
{
    GUI_PAGE_DEL(g_page);
    GUI_DEL_OBJ(g_quitHintBox);
}
void GuiDiceRollsWidgetsRefresh()
{
}

static void GuiCreatePage(lv_obj_t *parent)
{
    // text area
    lv_obj_t *textArea = lv_textarea_create(parent);
    lv_obj_align(textArea, LV_ALIGN_TOP_LEFT, 48, 26);
    lv_obj_set_width(textArea, 384);
    lv_obj_set_height(textArea, 49); // 10+29+10
    lv_obj_set_style_pad_hor(textArea, 0, LV_PART_MAIN);
    lv_obj_set_style_bg_color(textArea, BLACK_COLOR, LV_PART_MAIN);
    lv_obj_set_style_radius(textArea, 0, LV_PART_MAIN);
    lv_obj_set_style_border_width(textArea, 0, LV_PART_MAIN);
    lv_obj_set_style_text_color(textArea, WHITE_COLOR, LV_PART_MAIN);
    lv_obj_set_style_text_font(textArea, &openSans_24, LV_PART_MAIN);
    lv_obj_set_scrollbar_mode(textArea, LV_SCROLLBAR_MODE_OFF);
    lv_obj_add_event_cb(textArea, OnTextareaValueChangeHandler, LV_EVENT_ALL, NULL);
    lv_obj_clear_flag(textArea, LV_OBJ_FLAG_CLICK_FOCUSABLE);
    lv_obj_t *anchor = textArea;
    g_diceTextArea = textArea;

    static lv_style_t style_hidden_cursor;
    lv_style_init(&style_hidden_cursor);

    lv_style_set_bg_opa(&style_hidden_cursor, LV_OPA_TRANSP);
    lv_style_set_border_opa(&style_hidden_cursor, LV_OPA_TRANSP);
    lv_style_set_outline_opa(&style_hidden_cursor, LV_OPA_TRANSP);
    lv_style_set_pad_all(&style_hidden_cursor, 0);

    lv_obj_add_style(textArea, &style_hidden_cursor, LV_PART_CURSOR);

    static lv_point_t points[2] = {{0, 0}, {384, 0}};
    lv_obj_t *line = GuiCreateLine(parent, points, 2);
    lv_obj_align_to(line, anchor, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 0);
    g_line = line;
    anchor = line;

    lv_obj_t *label, *img, *btn;

    label = GuiCreateIllustrateLabel(parent, _("dice_roll_error_label"));
    lv_obj_set_style_text_color(label, DEEP_ORANGE_COLOR, LV_PART_MAIN);
    lv_obj_align_to(label, anchor, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 4);
    lv_obj_add_flag(label, LV_OBJ_FLAG_HIDDEN);
    g_errLabel = label;


    img = GuiCreateImg(parent, &imgDice1);
    InitDiceImg(img, parent, 12 + 48, 258);
    g_diceImgs[0] = img;

    img = GuiCreateImg(parent, &imgDice2);
    InitDiceImg(img, parent, 148 + 48, 258);
    g_diceImgs[1] = img;

    img = GuiCreateImg(parent, &imgDice3);
    InitDiceImg(img, parent, 284 + 48, 258);
    g_diceImgs[2] = img;

    img = GuiCreateImg(parent, &imgDice4);
    InitDiceImg(img, parent, 12 + 48, 394);
    g_diceImgs[3] = img;

    img = GuiCreateImg(parent, &imgDice5);
    InitDiceImg(img, parent, 148 + 48, 394);
    g_diceImgs[4] = img;

    img = GuiCreateImg(parent, &imgDice6);
    InitDiceImg(img, parent, 284 + 48, 394);
    g_diceImgs[5] = img;

    static lv_point_t bottom_points[2] = {{0, 0}, {480, 0}};
    line = GuiCreateLine(parent, bottom_points, 2);
    lv_obj_align_to(line, parent, LV_ALIGN_BOTTOM_LEFT, 0, -114);

    btn = GuiCreateBtn(parent, USR_SYMBOL_CHECK);
    lv_obj_set_size(btn, 96, 66);
    lv_obj_align(btn, LV_ALIGN_BOTTOM_LEFT, 348, -24);
    lv_obj_clear_flag(btn, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(btn, ConfirmHandler, LV_EVENT_CLICKED, textArea);
    lv_obj_add_style(btn, &g_numBtnmDisabledStyle, LV_PART_MAIN);
    g_confirmBtn = btn;

    label = GuiCreateIllustrateLabel(parent, "0");
    lv_obj_set_style_text_color(label, ORANGE_COLOR, LV_PART_MAIN);
    lv_obj_align(label, LV_ALIGN_BOTTOM_LEFT, 36, -54);
    g_rollsLabel = label;

    label = GuiCreateIllustrateLabel(parent, _("dice_roll_hint_label"));
    lv_obj_set_style_text_color(label, WHITE_COLOR, LV_PART_MAIN);
    lv_obj_align(label, LV_ALIGN_BOTTOM_LEFT, 36, -24);
    lv_obj_add_flag(label, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_style_text_opa(label, LV_OPA_64, LV_PART_MAIN);
    g_hintLabel = label;
}

static void OpenQuitHintBoxHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        GUI_DEL_OBJ(g_quitHintBox);
        g_quitHintBox = GuiCreateHintBox(lv_scr_act(), 480, 386, true);
        lv_obj_t *img, *label, *btn;
        img = GuiCreateImg(g_quitHintBox, &imgWarn);
        lv_obj_align(img, LV_ALIGN_TOP_LEFT, 36, 462);

        label = GuiCreateLittleTitleLabel(g_quitHintBox, _("dice_roll_cancel_title"));
        lv_obj_align(label, LV_ALIGN_TOP_LEFT, 36, 558);

        label = GuiCreateIllustrateLabel(g_quitHintBox, _("dice_roll_cancel_desc"));
        lv_obj_align(label, LV_ALIGN_TOP_LEFT, 36, 610);

        btn = GuiCreateBtn(g_quitHintBox, _("Cancel"));
        lv_obj_set_size(btn, 192, 66);
        lv_obj_set_style_bg_color(btn, DEEP_ORANGE_COLOR, LV_PART_MAIN);
        lv_obj_align(btn, LV_ALIGN_BOTTOM_RIGHT, -36, -24);
        lv_obj_add_event_cb(btn, QuitConfirmHandler, LV_EVENT_CLICKED, NULL);

        btn = GuiCreateBtn(g_quitHintBox, _("not_now"));
        lv_obj_set_size(btn, 192, 66);
        lv_obj_set_style_bg_color(btn, WHITE_COLOR_OPA20, LV_PART_MAIN);
        lv_obj_align(btn, LV_ALIGN_BOTTOM_LEFT, 36, -24);
        lv_obj_add_event_cb(btn, CloseQuitHintBoxHandler, LV_EVENT_CLICKED, NULL);
    }
}

static void QuitConfirmHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        GUI_DEL_OBJ(g_quitHintBox);
        GuiCLoseCurrentWorkingView();
    }
}

static void CloseQuitHintBoxHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        GUI_DEL_OBJ(g_quitHintBox);
    }
}

static void ClickDiceHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        lv_obj_t *img = lv_event_get_target(e);
        for (size_t i = 0; i < 6; i++) {
            if (g_diceImgs[i] == img) {
                const char *txt = lv_textarea_get_text(g_diceTextArea);
                if (strlen(txt) == 256) {
                    return;
                }

                lv_textarea_set_cursor_pos(g_diceTextArea, LV_TEXTAREA_CURSOR_LAST);
                lv_textarea_add_char(g_diceTextArea, '1' + i);
            }
        }
    }
}

static void InitDiceImg(lv_obj_t *img, lv_obj_t *parent, size_t x, size_t y)
{
    lv_obj_align(img, LV_ALIGN_OUT_BOTTOM_LEFT, x, y);
    lv_obj_set_style_bg_opa(img, LV_OPA_0, LV_PART_MAIN);
    lv_obj_set_style_bg_color(img, WHITE_COLOR, LV_STATE_PRESSED);
    lv_obj_set_style_bg_opa(img, LV_OPA_12, LV_STATE_PRESSED);
    lv_obj_add_flag(img, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(img, ClickDiceHandler, LV_EVENT_CLICKED, NULL);
}

static void OnTextareaValueChangeHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *ta = lv_event_get_target(e);
    if (code == LV_EVENT_VALUE_CHANGED) {
        const char *txt = lv_textarea_get_text(ta);
        lv_coord_t font_height = lv_obj_get_style_text_font(ta, LV_PART_MAIN)->line_height;
        // 27chars per line in reality;
        uint32_t length = strlen(txt);
        uint32_t line_count = length / 27 + 1;
        if (line_count > 4)
            return;
        lv_obj_set_height(ta, line_count * font_height + 2 * lv_obj_get_style_pad_top(ta, LV_PART_MAIN));
        lv_obj_update_layout(ta);
        lv_obj_align_to(g_line, ta, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 0);
        lv_obj_align_to(g_errLabel, g_line, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 4);

        if (length > 0 && length < 50) {
            lv_obj_clear_flag(g_hintLabel, LV_OBJ_FLAG_HIDDEN);
        } else {
            lv_obj_add_flag(g_hintLabel, LV_OBJ_FLAG_HIDDEN);
        }

        lv_label_set_text_fmt(g_rollsLabel, "%d", length);
        
        if (length >= 50) {
            lv_obj_remove_style(g_confirmBtn, &g_numBtnmDisabledStyle, LV_PART_MAIN);
            lv_obj_add_flag(g_confirmBtn, LV_OBJ_FLAG_CLICKABLE);
            float counts[6] = {0};
            for (size_t i = 0; i < length; i++) {
                counts[txt[i] - '1']++;
            }
            float len = length;
            bool stillValid = true;
            for (size_t i = 0; i < 6; i++) {
                if (counts[i] / len > 0.3) {
                    lv_obj_clear_flag(g_errLabel, LV_OBJ_FLAG_HIDDEN);
                    stillValid = false;
                    break;
                }
            }
            if (stillValid) {
                lv_obj_add_flag(g_errLabel, LV_OBJ_FLAG_HIDDEN);
            }
        } else {
            lv_obj_add_style(g_confirmBtn, &g_numBtnmDisabledStyle, LV_PART_MAIN);
            lv_obj_clear_flag(g_confirmBtn, LV_OBJ_FLAG_CLICKABLE);
        }
    }
}

static void UndoClickHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        lv_obj_t *ta = (lv_obj_t *)lv_event_get_user_data(e);
        lv_textarea_set_cursor_pos(ta, LV_TEXTAREA_CURSOR_LAST);
        lv_textarea_del_char(ta);
    }
}
static void UndoLongPressHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_LONG_PRESSED) {
        lv_obj_t *ta = (lv_obj_t *)lv_event_get_user_data(e);
        lv_textarea_set_text(ta, "");
    }
}

static void ConfirmHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        lv_obj_t *ta = (lv_obj_t *)lv_event_get_user_data(e);

        // convert result
        const char *txt = lv_textarea_get_text(ta);
        char *temp = SRAM_MALLOC(128);
        strcpy(temp, txt);
        for (size_t i = 0; i < strlen(txt); i++) {
            char c = temp[i];
            if (c == '6') {
                temp[i] = '0';
            }
        }
        uint8_t hash[32] = {0};
        sha256((struct sha256 *)hash, temp, strlen(temp));
        uint8_t entropyMethod = 1;
        SecretCacheSetDiceRollHash(hash);
        if (g_seedType == SEED_TYPE_BIP39) {
            GuiFrameOpenViewWithParam(&g_singlePhraseView, &entropyMethod, 1);
        } else {
            GuiFrameOpenViewWithParam(&g_createShareView, &entropyMethod, 1);
        }
    }
}