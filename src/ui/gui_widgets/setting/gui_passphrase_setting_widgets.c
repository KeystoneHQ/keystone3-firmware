#include "gui.h"
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
#include "gui_setting_widgets.h"
#include "gui_lock_widgets.h"
#include "bip39_english.h"
#include "bip39.h"
#include "slip39.h"
#include "version.h"
#include "presetting.h"
#include "assert.h"
#include "gui_qr_hintbox.h"

#include "gui_mnemonic_input.h"
#include "motor_manager.h"
#include "account_manager.h"
#include "gui_keyboard_hintbox.h"
#ifndef COMPILE_SIMULATOR
#include "sha256.h"
#include "keystore.h"
#else
#include "simulator_model.h"
#include "simulator_mock_define.h"
#endif

typedef struct PassphraseWidget {
    lv_obj_t *inputTa;
    lv_obj_t *repeatTa;
    lv_obj_t *errLabel;
    lv_obj_t *lenOverLabel;
} PassphraseWidget_t;
static PassphraseWidget_t g_passphraseWidget;

static void PassphraseQuickAccessHandler(lv_event_t *e);
static void SetKeyboardTaHandler(lv_event_t *e);
static void UpdatePassPhraseHandler(lv_event_t *e);

static lv_obj_t *g_passphraseQuickAccessSwitch = NULL;
static KeyBoard_t *g_setPassPhraseKb = NULL;         // setting keyboard

void GuiWalletPassphrase(lv_obj_t *parent)
{
    static uint16_t walletSetting = DEVICE_SETTING_PASSPHRASE_ENTER;
    uint16_t height = 60;

    lv_obj_set_style_bg_opa(parent, LV_OPA_0, LV_PART_SCROLLBAR | LV_STATE_SCROLLED);
    lv_obj_set_style_bg_opa(parent, LV_OPA_0, LV_PART_SCROLLBAR | LV_STATE_DEFAULT);

    lv_obj_t *label = GuiCreateTextLabel(parent, _("Passphrase"));
    lv_obj_t *imgArrow = GuiCreateImg(parent, &imgArrowRight);

    GuiButton_t table[3] = {
        {
            .obj = label,
            .align = LV_ALIGN_LEFT_MID,
            .position = {24, 0},
        },
        {
            .obj = imgArrow,
            .align = LV_ALIGN_LEFT_MID,
            .position = {376, 0},
        },
    };
    lv_obj_t *button = GuiCreateButton(parent, 456, 84, table, 2, WalletSettingHandler, &walletSetting);
    lv_obj_align(button, LV_ALIGN_DEFAULT, 12, 144 - GUI_MAIN_AREA_OFFSET);

    g_passphraseQuickAccessSwitch = lv_switch_create(parent);
    lv_obj_set_style_bg_color(g_passphraseQuickAccessSwitch, ORANGE_COLOR, LV_STATE_CHECKED | LV_PART_INDICATOR);
    lv_obj_set_style_bg_color(g_passphraseQuickAccessSwitch, WHITE_COLOR, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(g_passphraseQuickAccessSwitch, LV_OPA_30, LV_PART_MAIN);
    if (GetPassphraseQuickAccess()) {
        lv_obj_add_state(g_passphraseQuickAccessSwitch, LV_STATE_CHECKED);
    } else {
        lv_obj_clear_state(g_passphraseQuickAccessSwitch, LV_STATE_CHECKED);
    }
    lv_obj_clear_flag(g_passphraseQuickAccessSwitch, LV_OBJ_FLAG_CLICKABLE);
    label = GuiCreateTextLabel(parent, _("passphrase_access_switch_title"));
    lv_obj_set_width(label, 336);
    lv_obj_refr_size(label);
    height += lv_obj_get_self_height(label);
    lv_obj_t *descLabel = GuiCreateIllustrateLabel(parent, _("passphrase_access_switch_desc"));
    lv_obj_set_width(descLabel, 336);
    lv_obj_refr_size(descLabel);
    height += lv_obj_get_self_height(descLabel);
    lv_obj_set_style_text_opa(descLabel, LV_OPA_60, LV_PART_MAIN);
    table[0].obj = label;
    table[0].align = LV_ALIGN_DEFAULT;
    table[0].position.x = 24;
    table[0].position.y = 24;
    table[1].obj = g_passphraseQuickAccessSwitch;
    table[1].align = LV_ALIGN_DEFAULT;
    table[1].position.x = 376;
    table[1].position.y = 24;
    table[2].obj = descLabel;
    table[2].align = LV_ALIGN_BOTTOM_LEFT;
    table[2].position.x = 24;
    table[2].position.y = -24;
    button = GuiCreateButton(parent, 456, height, table, NUMBER_OF_ARRAYS(table), PassphraseQuickAccessHandler, NULL);
    lv_obj_align(button, LV_ALIGN_DEFAULT, 12, 254 - GUI_MAIN_AREA_OFFSET);
}

void GuiWalletPassphraseEnter(lv_obj_t *parent)
{
    lv_obj_set_style_bg_opa(parent, LV_OPA_0, LV_PART_SCROLLBAR | LV_STATE_SCROLLED);
    lv_obj_set_style_bg_opa(parent, LV_OPA_0, LV_PART_SCROLLBAR | LV_STATE_DEFAULT);

    lv_obj_t *btn = lv_btn_create(parent);
    lv_obj_set_style_border_width(btn, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(btn, LV_OPA_0, LV_PART_MAIN);
    lv_obj_set_style_outline_width(btn, 0, LV_PART_MAIN);
    lv_obj_set_style_shadow_width(btn, 0, LV_PART_MAIN);
    lv_obj_set_size(btn, 352, 60);
    lv_obj_align(btn, LV_ALIGN_DEFAULT, 26, 162 - GUI_MAIN_AREA_OFFSET);
    lv_obj_t *ta = lv_textarea_create(btn);
    lv_obj_set_align(ta, LV_ALIGN_CENTER);
    lv_obj_set_size(ta, 352, 60);
    lv_textarea_set_password_mode(ta, true);
    lv_textarea_set_placeholder_text(ta, _("passphrase_enter_input"));
    lv_obj_set_style_bg_color(ta, BLACK_COLOR, LV_PART_MAIN);
    lv_obj_set_style_text_color(ta, WHITE_COLOR, LV_PART_MAIN);
    lv_obj_set_style_border_opa(ta, LV_OPA_0, LV_PART_MAIN);
    lv_obj_set_style_text_font(ta, g_defTextFont, 0);
    lv_obj_add_event_cb(ta, SetKeyboardTaHandler, LV_EVENT_ALL, ta);
    lv_obj_t *img = GuiCreateImg(parent, &imgEyeOff);
    lv_obj_align(img, LV_ALIGN_DEFAULT, 411, 168 - GUI_MAIN_AREA_OFFSET);
    lv_obj_add_flag(img, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(img, SwitchPasswordModeHandler, LV_EVENT_CLICKED, ta);
    lv_textarea_set_max_length(ta, PASSWORD_MAX_LEN);
    lv_textarea_set_one_line(ta, true);
    lv_obj_set_scrollbar_mode(ta, LV_SCROLLBAR_MODE_OFF);
    g_passphraseWidget.inputTa = ta;
    lv_obj_add_state(ta, LV_STATE_FOCUSED);

    btn = lv_btn_create(parent);
    lv_obj_set_style_border_width(btn, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(btn, LV_OPA_0, LV_PART_MAIN);
    lv_obj_set_style_outline_width(btn, 0, LV_PART_MAIN);
    lv_obj_set_style_shadow_width(btn, 0, LV_PART_MAIN);
    lv_obj_set_size(btn, 352, 60);
    lv_obj_align(btn, LV_ALIGN_DEFAULT, 26, 246 - GUI_MAIN_AREA_OFFSET);
    lv_obj_t *repeatTa = lv_textarea_create(btn);
    lv_obj_set_align(repeatTa, LV_ALIGN_CENTER);
    lv_obj_set_size(repeatTa, 352, 60);
    lv_textarea_set_password_mode(repeatTa, true);
    lv_textarea_set_placeholder_text(repeatTa, _("passphrase_enter_repeat"));
    lv_obj_set_style_bg_color(repeatTa, BLACK_COLOR, LV_PART_MAIN);
    lv_obj_set_style_text_color(repeatTa, WHITE_COLOR, LV_PART_MAIN);
    lv_obj_set_style_border_opa(repeatTa, LV_OPA_0, LV_PART_MAIN);
    lv_textarea_set_max_length(repeatTa, PASSWORD_MAX_LEN);
    lv_textarea_set_one_line(repeatTa, true);
    lv_obj_set_scrollbar_mode(repeatTa, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_text_font(repeatTa, g_defTextFont, 0);

    lv_obj_add_event_cb(repeatTa, SetKeyboardTaHandler, LV_EVENT_ALL, repeatTa);
    img = GuiCreateImg(parent, &imgEyeOff);
    lv_obj_align(img, LV_ALIGN_DEFAULT, 411, 252 - GUI_MAIN_AREA_OFFSET);
    lv_obj_add_flag(img, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(img, SwitchPasswordModeHandler, LV_EVENT_CLICKED, repeatTa);
    g_passphraseWidget.repeatTa = repeatTa;

    g_setPassPhraseKb = GuiCreateFullKeyBoard(parent, UpdatePassPhraseHandler, KEY_STONE_FULL_L, NULL);
    GuiSetKeyBoardMinTaLen(g_setPassPhraseKb, 0);
    GuiSetFullKeyBoardTa(g_setPassPhraseKb, ta);
    lv_obj_t *label = GuiCreateIllustrateLabel(parent, _("passphrase_error_not_match"));
    lv_obj_align(label, LV_ALIGN_DEFAULT, 36, 304 - GUI_MAIN_AREA_OFFSET);
    lv_obj_set_style_text_color(label, RED_COLOR, LV_PART_MAIN);
    lv_obj_add_flag(label, LV_OBJ_FLAG_HIDDEN);
    g_passphraseWidget.errLabel = label;

    label = GuiCreateIllustrateLabel(parent, _("passphrase_error_too_long"));
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 415 - GUI_MAIN_AREA_OFFSET);
    lv_obj_set_style_text_color(label, RED_COLOR, LV_PART_MAIN);
    lv_obj_add_flag(label, LV_OBJ_FLAG_HIDDEN);
    g_passphraseWidget.lenOverLabel = label;
}

static void SetKeyboardTaHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        lv_obj_t *ta = lv_event_get_user_data(e);
        if (ta == g_passphraseWidget.repeatTa) {
            GuiSetFullKeyBoardConfirm(g_setPassPhraseKb, true);
            lv_obj_clear_state(g_passphraseWidget.inputTa, LV_STATE_FOCUSED);
        } else {
            lv_obj_clear_state(g_passphraseWidget.repeatTa, LV_STATE_FOCUSED);
            GuiSetFullKeyBoardConfirm(g_setPassPhraseKb, false);
        }
        GuiSetFullKeyBoardTa(g_setPassPhraseKb, ta);

        lv_obj_add_flag(g_passphraseWidget.lenOverLabel, LV_OBJ_FLAG_HIDDEN);
    } else if (code == KEY_STONE_KEYBOARD_CHANGE) {
        lv_keyboard_user_mode_t *keyMode = lv_event_get_param(e);
        g_setPassPhraseKb->mode = *keyMode;
        GuiKeyBoardSetMode(g_setPassPhraseKb);
        if (*keyMode == KEY_STONE_SYMBOL) {
            GuiUpdatePassPhraseKb(g_setPassPhraseKb);
        }
    }
}

static void PassphraseQuickAccessHandler(lv_event_t *e)
{
    lv_obj_t *switchBox = g_passphraseQuickAccessSwitch;
    bool en = lv_obj_has_state(switchBox, LV_STATE_CHECKED);
    if (en) {
        lv_obj_clear_state(switchBox, LV_STATE_CHECKED);
    } else {
        lv_obj_add_state(switchBox, LV_STATE_CHECKED);
    }
    SetPassphraseQuickAccess(!en);
    lv_event_send(switchBox, LV_EVENT_VALUE_CHANGED, NULL);
}

static void UpdatePassPhraseHandler(lv_event_t *e)
{
    static bool delayFlag = false;
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_READY) {
        if (lv_keyboard_get_textarea(g_setPassPhraseKb->kb) == g_passphraseWidget.inputTa) {
            lv_keyboard_set_textarea(g_setPassPhraseKb->kb, g_passphraseWidget.repeatTa);
            lv_obj_clear_state(g_passphraseWidget.inputTa, LV_STATE_FOCUSED);
            lv_obj_add_state(g_passphraseWidget.repeatTa, LV_STATE_FOCUSED);
        } else {
            const char *input = lv_textarea_get_text(g_passphraseWidget.inputTa);
            const char *repeat = lv_textarea_get_text(g_passphraseWidget.repeatTa);
            if (!strcmp(input, repeat)) {
                SecretCacheSetPassphrase((char *)repeat);
                static uint16_t signal = SIG_SETTING_WRITE_PASSPHRASE;
                GuiShowKeyboard(&signal, true, NULL);
            } else {
                delayFlag = true;
                lv_obj_clear_flag(g_passphraseWidget.errLabel, LV_OBJ_FLAG_HIDDEN);
            }
        }
    }

    if (code == LV_EVENT_VALUE_CHANGED) {
        Vibrate(SLIGHT);
        const char *intputText = lv_textarea_get_text(g_passphraseWidget.inputTa);
        if (!lv_obj_has_flag(g_passphraseWidget.errLabel, LV_OBJ_FLAG_HIDDEN)) {
            if (delayFlag == true) {
                delayFlag = false;
            } else {
                lv_obj_add_flag(g_passphraseWidget.errLabel, LV_OBJ_FLAG_HIDDEN);
            }
        }

        if (lv_keyboard_get_textarea(lv_event_get_target(e)) == g_passphraseWidget.inputTa) {
            size_t inputTextLength = strnlen_s(intputText, PASSWORD_MAX_LEN);
            if (inputTextLength >= PASSWORD_MAX_LEN) {
                lv_obj_clear_flag(g_passphraseWidget.lenOverLabel, LV_OBJ_FLAG_HIDDEN);
            } else {
                lv_obj_add_flag(g_passphraseWidget.lenOverLabel, LV_OBJ_FLAG_HIDDEN);
            }
        }
    }
}