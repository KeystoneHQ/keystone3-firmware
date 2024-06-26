#include "gui_enter_passcode.h"
#include "gui_obj.h"
#include "gui_led.h"
#include "gui_views.h"
#include "gui_button.h"
#include "user_memory.h"
#include "secret_cache.h"
#include "gui_model.h"
#include "keystore.h"
#include "gui_setting_widgets.h"
#include "gui_lock_widgets.h"
#include "motor_manager.h"
#include "account_manager.h"
#include "gui_keyboard_hintbox.h"

#ifdef COMPILE_SIMULATOR
#include "simulator_mock_define.h"
#endif

typedef enum {
    PASSWORD_STRENGTH_LEN,
    PASSWORD_STRENGTH_LITTLE_LETTER,
    PASSWORD_STRENGTH_CAPITAL_LETTER,
    PASSWORD_STRENGTH_DIGIT,
    PASSWORD_STRENGTH_SYMBOL,

    PASSWORD_STRENGTH_BUTT,
} PassWordStrength_t;

typedef struct EnterPassCodeParam {
    void *userParam;
    void *setpinParam;
} EnterPassCodeParam_t;

static char g_pinBuf[PASSWORD_MAX_LEN + 1];
static void *g_userParam;
static EnterPassCodeParam_t g_passParam;
static bool g_isHandle = true;
#define SET_HANDLE_FLAG() (g_isHandle = true)
#define CLEAR_HANDLE_FLAG() (g_isHandle = false)

typedef struct EnterPassLabel {
    const char *title;
    const char *desc;
    const char *passSwitch;
} EnterPassLabel_t;
static EnterPassLabel_t g_enterPassLabel[ENTER_PASSCODE_BUTT];

void GuiEnterPassLabelRefresh(void)
{
    g_enterPassLabel[ENTER_PASSCODE_SET_PIN].title = _("single_backup_setpin_title");
    g_enterPassLabel[ENTER_PASSCODE_SET_PIN].desc = _("single_backup_setpin_desc");
    g_enterPassLabel[ENTER_PASSCODE_SET_PIN].passSwitch = _("single_backup_setpin_use_pass");

    g_enterPassLabel[ENTER_PASSCODE_SET_PASSWORD].title = _("single_backup_setpass_title");
    g_enterPassLabel[ENTER_PASSCODE_SET_PASSWORD].desc = _("single_backup_setpass_desc");
    g_enterPassLabel[ENTER_PASSCODE_SET_PASSWORD].passSwitch = _("single_backup_setpin_use_pin");

    g_enterPassLabel[ENTER_PASSCODE_REPEAT_PIN].title = _("repeat_passcode_title");
    g_enterPassLabel[ENTER_PASSCODE_REPEAT_PIN].desc = _("repeat_passcode_desc");
    g_enterPassLabel[ENTER_PASSCODE_REPEAT_PIN].passSwitch = "";

    g_enterPassLabel[ENTER_PASSCODE_REPEAT_PASSWORD].title = _("single_backup_repeatpass_title");
    g_enterPassLabel[ENTER_PASSCODE_REPEAT_PASSWORD].desc = _("single_backup_repeatpass_desc");
    g_enterPassLabel[ENTER_PASSCODE_REPEAT_PASSWORD].passSwitch = "";
}

static void SetPinEventHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *obj = lv_event_get_target(e);
    if (code == LV_EVENT_RELEASED) {
        Vibrate(SLIGHT);
        GuiEnterPasscodeItem_t *item = g_passParam.setpinParam;
#ifndef COMPILE_SIMULATOR
        if (!g_isHandle && item->mode == ENTER_PASSCODE_VERIFY_PIN) {
            return;
        }
#endif

        uint32_t id = lv_btnmatrix_get_selected_btn(obj);
        if (id == 9) {
            return;
        }
        const char *txt = lv_btnmatrix_get_btn_text(obj, id);
        if (!strcmp(txt, USR_SYMBOL_DELETE)) {
            if (item->currentNum > 0) {
                --item->currentNum;
                GuiSetLedStatus(item->numLed[item->currentNum], PASSCODE_LED_OFF);
                g_pinBuf[item->currentNum] = '\0';
            } else {
                for (int i = 0; i < CREATE_PIN_NUM; i++) {
                    GuiSetLedStatus(item->numLed[i], PASSCODE_LED_OFF);
                }
            }
            if (!lv_obj_has_flag(item->errLabel, LV_OBJ_FLAG_HIDDEN)) {
                lv_obj_add_flag(item->errLabel, LV_OBJ_FLAG_HIDDEN);
            }
        } else {
            if (item->currentNum < CREATE_PIN_NUM) {
                snprintf_s(g_pinBuf + item->currentNum, PASSWORD_MAX_LEN - item->currentNum, "%s", txt);
                GuiSetLedStatus(item->numLed[item->currentNum], PASSCODE_LED_ON);
                item->currentNum++;
                if (item->mode == ENTER_PASSCODE_SET_PIN &&
                        !lv_obj_has_flag(item->repeatLabel, LV_OBJ_FLAG_HIDDEN)) {
                    lv_obj_add_flag(item->repeatLabel, LV_OBJ_FLAG_HIDDEN);
                }
                if (item->mode == ENTER_PASSCODE_VERIFY_PIN &&
                        !lv_obj_has_flag(item->fpErrLabel, LV_OBJ_FLAG_HIDDEN)) {
                    lv_obj_add_flag(item->fpErrLabel, LV_OBJ_FLAG_HIDDEN);
                }
                if (!lv_obj_has_flag(item->errLabel, LV_OBJ_FLAG_HIDDEN)) {
                    lv_obj_add_flag(item->errLabel, LV_OBJ_FLAG_HIDDEN);
                    for (int i = 1; i < CREATE_PIN_NUM; i++) {
                        GuiSetLedStatus(item->numLed[i], PASSCODE_LED_OFF);
                    }
                }
            }

            if (item->currentNum == CREATE_PIN_NUM) {
                for (int i = 0; i < CREATE_PIN_NUM; i++) {
                    GuiSetLedStatus(item->numLed[i], PASSCODE_LED_OFF);
                }

                g_userParam = g_passParam.userParam;
                uint8_t index = 0xff;
                if (g_userParam != NULL && *(uint16_t *)g_userParam == DEVICE_SETTING_RESET_PASSCODE_VERIFY) {
                    index = GetCurrentAccountIndex();
                }

                switch (item->mode) {
                case ENTER_PASSCODE_VERIFY_PIN:
                    SecretCacheSetPassword(g_pinBuf);
                    GuiModelVerifyAccountPassWord(g_userParam);
                    break;
                case ENTER_PASSCODE_SET_PIN:
                    if (CheckPasswordExisted(g_pinBuf, index)) {
                        UnlimitedVibrate(LONG);
                        lv_obj_clear_flag(item->repeatLabel, LV_OBJ_FLAG_HIDDEN);
                    } else {
                        GuiEmitSignal(SIG_SETTING_SET_PIN, g_pinBuf, strnlen_s(g_pinBuf, PASSWORD_MAX_LEN));
                    }
                    break;
                case ENTER_PASSCODE_REPEAT_PIN:
                    GuiEmitSignal(SIG_SETTING_REPEAT_PIN, g_pinBuf, strnlen_s(g_pinBuf, PASSWORD_MAX_LEN));
                    break;
                default:
                    break;
                }
                CLEAR_HANDLE_FLAG();
                item->currentNum = 0;
                memset_s(g_pinBuf, sizeof(g_pinBuf), 0, sizeof(g_pinBuf));
                item->setPassCb = NULL;
            }
        }
        // if (item->setPassCb != NULL) {
        //     item->setPassCb(e);
        // }
    }
}

static void SetPassWordHandler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *kb = lv_event_get_target(e);
    lv_obj_t *ta = lv_keyboard_get_textarea(kb);
    static bool delayFlag = false;
    static bool delayErrFlag = false;
    static lv_obj_t *lenLabel = NULL;
    GuiEnterPasscodeItem_t *item = g_passParam.setpinParam;
    if (code == LV_EVENT_READY) {
        const char *currText = lv_textarea_get_text(ta);
        if (item->mode == ENTER_PASSCODE_SET_PASSWORD && strnlen_s(currText, CREATE_PIN_NUM) < CREATE_PIN_NUM) {
            UnlimitedVibrate(LONG);
            if (lenLabel == NULL) {
                lenLabel = GuiCreateIllustrateLabel(lv_obj_get_parent(lv_obj_get_parent(kb)), _("password_error_too_short"));
                lv_obj_align(lenLabel, LV_ALIGN_TOP_MID, 0, 375 - GUI_MAIN_AREA_OFFSET);
                lv_obj_set_style_text_color(lenLabel, RED_COLOR, LV_PART_MAIN);
                delayFlag = true;
            } else {
                delayFlag = true;
                lv_obj_clear_flag(lenLabel, LV_OBJ_FLAG_HIDDEN);
            }
        } else {
            g_userParam = g_passParam.userParam;
            if (item->mode == ENTER_PASSCODE_SET_PASSWORD) {
                uint8_t index = 0xff;

                if (g_userParam != NULL && *(uint16_t *)g_userParam == DEVICE_SETTING_RESET_PASSCODE_VERIFY) {
                    index = GetCurrentAccountIndex();
                }
                if (CheckPasswordExisted(currText, index)) {
                    UnlimitedVibrate(LONG);
                    lv_obj_clear_flag(item->repeatLabel, LV_OBJ_FLAG_HIDDEN);
                    delayFlag = true;
                } else {
                    GuiEmitSignal(SIG_SETTING_SET_PIN, (char *)currText, strnlen_s(currText, CREATE_PIN_NUM));
                }
            } else if (item->mode == ENTER_PASSCODE_REPEAT_PASSWORD) {
                GuiEmitSignal(SIG_SETTING_REPEAT_PIN, (char *)currText, strnlen_s(currText, CREATE_PIN_NUM));
            } else if ((item->mode == ENTER_PASSCODE_VERIFY_PASSWORD)) {
                g_userParam = g_passParam.userParam;
                if (strnlen_s(currText, PASSWORD_MAX_LEN) > 0) {
                    SecretCacheSetPassword((char *)currText);
                    GuiModelVerifyAccountPassWord(g_userParam);
                }
            }
            lv_textarea_set_text(ta, "");
        }
    }

    if (code == LV_EVENT_VALUE_CHANGED) {
        Vibrate(SLIGHT);
        const char *currText = lv_textarea_get_text(ta);
        if (!lv_obj_has_flag(item->repeatLabel, LV_OBJ_FLAG_HIDDEN)) {
            if (delayFlag == true) {
                delayFlag = false;
            } else {
                lv_obj_add_flag(item->repeatLabel, LV_OBJ_FLAG_HIDDEN);
            }
        }

        if (item->mode == ENTER_PASSCODE_SET_PASSWORD) {
            const char *password = lv_textarea_get_text(item->kb->ta);
            uint8_t passwordLen = strnlen_s(password, PASSWORD_MAX_LEN);
            int8_t score = GetPassWordStrength(password, passwordLen);
            if (item->scoreBar != NULL && item->scoreLevel != NULL) {
                if (passwordLen < 6) {
                    if (!lv_obj_has_flag(item->scoreBar, LV_OBJ_FLAG_HIDDEN)) {
                        lv_obj_add_flag(item->scoreBar, LV_OBJ_FLAG_HIDDEN);
                    }
                    if (!lv_obj_has_flag(item->scoreLevel, LV_OBJ_FLAG_HIDDEN)) {
                        lv_obj_add_flag(item->scoreLevel, LV_OBJ_FLAG_HIDDEN);
                    }
                } else {
                    if (lv_obj_has_flag(item->scoreBar, LV_OBJ_FLAG_HIDDEN)) {
                        lv_obj_clear_flag(item->scoreBar, LV_OBJ_FLAG_HIDDEN);
                    }
                    if (lv_obj_has_flag(item->scoreLevel, LV_OBJ_FLAG_HIDDEN)) {
                        lv_obj_clear_flag(item->scoreLevel, LV_OBJ_FLAG_HIDDEN);
                    }
                    lv_bar_set_value(item->scoreBar, score, LV_ANIM_ON);
                    if (score <= 40) {
                        lv_style_set_bg_color(item->scoreBarStyle, ORANGE_COLOR);
                        lv_obj_set_style_text_color(item->scoreLevel, ORANGE_COLOR, LV_PART_MAIN);
                        lv_label_set_text(item->scoreLevel, _("password_score_weak"));
                    } else if (score <= 70) {
                        lv_style_set_bg_color(item->scoreBarStyle, BLUE_COLOR);
                        lv_obj_set_style_text_color(item->scoreLevel, BLUE_COLOR, LV_PART_MAIN);
                        lv_label_set_text(item->scoreLevel, _("password_score_normal"));
                    } else {
                        lv_style_set_bg_color(item->scoreBarStyle, GREEN_COLOR);
                        lv_obj_set_style_text_color(item->scoreLevel, GREEN_COLOR, LV_PART_MAIN);
                        lv_label_set_text(item->scoreLevel, _("password_score_good"));
                    }
                }
            }
        }

        if (!lv_obj_has_flag(item->errLabel, LV_OBJ_FLAG_HIDDEN)) {
            if (item->mode == ENTER_PASSCODE_VERIFY_PASSWORD) {
                lv_obj_add_flag(item->errLabel, LV_OBJ_FLAG_HIDDEN);
            }

            if (item->mode == ENTER_PASSCODE_REPEAT_PASSWORD) {
                if (delayErrFlag == false) {
                    delayErrFlag = true;
                } else {
                    lv_obj_add_flag(item->errLabel, LV_OBJ_FLAG_HIDDEN);
                    delayErrFlag = false;
                }
            }
        }

        if (lenLabel != NULL && !lv_obj_has_flag(lenLabel, LV_OBJ_FLAG_HIDDEN)) {
            if (delayFlag == true) {
                delayFlag = false;
            } else {
                lv_obj_add_flag(lenLabel, LV_OBJ_FLAG_HIDDEN);
            }
        }

        if (item->mode == ENTER_PASSCODE_SET_PASSWORD && item->lenOverLabel != NULL) {
            if (strnlen_s(currText, PASSWORD_MAX_LEN) >= PASSWORD_MAX_LEN) {
                lv_obj_clear_flag(item->lenOverLabel, LV_OBJ_FLAG_HIDDEN);
            } else {
                lv_obj_add_flag(item->lenOverLabel, LV_OBJ_FLAG_HIDDEN);
            }
        }
    }
}

void PassWordPinSwitch(GuiEnterPasscodeItem_t *item)
{
    if (lv_obj_has_flag(item->passWdCont, LV_OBJ_FLAG_HIDDEN)) {
        lv_obj_clear_flag(item->passWdCont, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(item->pinCont, LV_OBJ_FLAG_HIDDEN);
        item->mode++; // This operation is related to the ENTER_PASSCODE_ENUM
        lv_obj_set_parent(item->errLabel, item->passWdCont);
        lv_obj_set_parent(item->repeatLabel, item->passWdCont);
        if (item->fpErrLabel) {
            lv_obj_set_parent(item->fpErrLabel, item->passWdCont);
        }
        if (item->mode == ENTER_PASSCODE_SET_PASSWORD) {
            GuiSetKeyBoardMinTaLen(item->kb, 6);
        }
        if (item->mode == ENTER_PASSCODE_VERIFY_PASSWORD) {
            SetKeyboardWidgetMode(KEYBOARD_HINTBOX_PASSWORD);
        }
    } else {
        lv_obj_add_flag(item->passWdCont, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(item->pinCont, LV_OBJ_FLAG_HIDDEN);
        item->mode--; // This operation is related to the ENTER_PASSCODE_ENUM
        lv_obj_set_parent(item->errLabel, item->pinCont);
        lv_obj_set_parent(item->repeatLabel, item->pinCont);
        if (item->fpErrLabel) {
            lv_obj_set_parent(item->fpErrLabel, item->pinCont);
        }
        if (item->mode == ENTER_PASSCODE_VERIFY_PIN) {
            SetKeyboardWidgetMode(KEYBOARD_HINTBOX_PIN);
        }
    }
    lv_obj_add_flag(item->errLabel, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(item->repeatLabel, LV_OBJ_FLAG_HIDDEN);
    if (lv_obj_is_valid(item->fpErrLabel)) {
        lv_obj_add_flag(item->fpErrLabel, LV_OBJ_FLAG_HIDDEN);
    }
    if (item->mode == ENTER_PASSCODE_VERIFY_PIN || item->mode == ENTER_PASSCODE_LOCK_VERIFY_PIN) {
        GuiEmitSignal(SIG_PASSCODE_SWITCH_TO_PIN, NULL, 0);
    } else if (item -> mode == ENTER_PASSCODE_VERIFY_PASSWORD || item->mode == ENTER_PASSCODE_LOCK_VERIFY_PASSWORD) {
        GuiEmitSignal(SIG_PASSCODE_SWITCH_TO_PASSWORD, NULL, 0);
    }
    GuiEnterPassCodeStatus(item, true);
}

static void PassWordPinSwitchHandler(lv_event_t *e)
{
    GuiEnterPasscodeItem_t *item = g_passParam.setpinParam;
    PassWordPinSwitch(item);
}

void GuiCreateEnterVerify(GuiEnterPasscodeItem_t *item, EnterPassCodeParam_t *passCodeParam)
{
    lv_obj_t *pinCont = item->pinCont;
    lv_obj_t *passWdCont = item->passWdCont;
    lv_obj_t *label;
    lv_obj_t *img;
    item->fpErrLabel = NULL;
    if (item->mode == ENTER_PASSCODE_VERIFY_PIN) {
        lv_obj_t *btnm = GuiCreateNumKeyboard(pinCont, SetPinEventHandler, NUM_KEYBOARD_PIN, passCodeParam);
        lv_obj_add_style(btnm, &g_enterPassBtnmStyle, LV_PART_ITEMS);
        lv_obj_add_style(btnm, &g_enterPressBtnmStyle, LV_STATE_PRESSED | LV_PART_ITEMS);
        item->btnm = btnm;
        lv_obj_t *button = GuiCreateImgLabelAdaptButton(pinCont, _("FORGET"), &imgLock, OpenForgetPasswordHandler, &g_lockView);
        lv_obj_align(button, LV_ALIGN_BOTTOM_RIGHT, -32, -27);
        lv_obj_align(btnm, LV_ALIGN_BOTTOM_MID, 0, -84);
        lv_obj_set_style_bg_color(btnm, BLACK_COLOR, LV_PART_MAIN);

        label = GuiCreateIllustrateLabel(pinCont, _("password_error_not_match"));
        lv_obj_set_style_text_color(label, lv_color_hex(0x8f8f8f), LV_PART_MAIN);
        lv_label_set_recolor(label, true);
        lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 12);
        lv_obj_add_flag(label, LV_OBJ_FLAG_HIDDEN);
        item->errLabel = label;

        label = GuiCreateNoticeLabel(pinCont, _("password_error_cannot_verify_fingerprint"));
        lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 0);
        lv_obj_add_flag(label, LV_OBJ_FLAG_HIDDEN);
        item->fpErrLabel = label;

        label = GuiCreateIllustrateLabel(pinCont, _(""));
        lv_obj_set_style_text_color(label, RED_COLOR, LV_PART_MAIN);
        lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 210);
        lv_obj_add_flag(label, LV_OBJ_FLAG_HIDDEN);
        item->repeatLabel = label;

        button = GuiCreateImgLabelAdaptButton(pinCont, _("password_label"), &imgSwitch, PassWordPinSwitchHandler, passCodeParam);
        lv_obj_align(button, LV_ALIGN_BOTTOM_LEFT, 24, -24);
    }

    KeyBoard_t *kb = GuiCreateFullKeyBoard(passWdCont, SetPassWordHandler, KEY_STONE_FULL_L, passCodeParam);
    lv_obj_align(kb->cont, LV_ALIGN_BOTTOM_MID, 0, -84);
    lv_obj_set_size(kb->ta, 382, 60);
    lv_obj_set_style_text_opa(kb->ta, LV_OPA_100, 0);
    lv_obj_align(kb->ta, LV_ALIGN_DEFAULT, 36, 292 - GUI_MAIN_AREA_OFFSET);
    lv_textarea_set_placeholder_text(kb->ta, _("password_input_desc"));
    lv_textarea_set_max_length(kb->ta, PASSWORD_MAX_LEN);
    lv_textarea_set_one_line(kb->ta, true);
    lv_textarea_set_password_mode(kb->ta, true);
    item->kb = kb;
    img = GuiCreateImg(passWdCont, &imgEyeOff);
    lv_obj_align_to(img, kb->ta, LV_ALIGN_LEFT_MID, 375, 0);
    lv_obj_add_flag(img, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(img, SwitchPasswordModeHandler, LV_EVENT_CLICKED, kb->ta);
    item->eyeImg = img;

    static lv_point_t points[2] = {{0, 0}, {410, 0}};
    lv_obj_t *line = (lv_obj_t *)GuiCreateLine(passWdCont, points, 2);
    lv_obj_align_to(line, kb->ta, LV_ALIGN_BOTTOM_MID, 0, 0);

    lv_obj_t *button = GuiCreateImgLabelAdaptButton(passWdCont, _("FORGET"), &imgLock, OpenForgetPasswordHandler, &g_lockView);
    lv_obj_align(button, LV_ALIGN_BOTTOM_RIGHT, -32, -27);

    button = GuiCreateImgLabelAdaptButton(passWdCont, _("pin_code"), &imgSwitch, PassWordPinSwitchHandler, passCodeParam);
    lv_obj_align(button, LV_ALIGN_BOTTOM_LEFT, 24, -24);
}

void GuiCreateEnterPinCode(GuiEnterPasscodeItem_t *item, EnterPassCodeParam_t *passCodeParam)
{
    lv_obj_t *pinCont = item->pinCont;
    lv_obj_t *passWdCont = item->passWdCont;
    lv_obj_t *btnm = GuiCreateNumKeyboard(pinCont, SetPinEventHandler, NUM_KEYBOARD_PIN, passCodeParam);
    UNUSED(btnm);
    lv_obj_t *label;
    lv_obj_t *img;
    lv_obj_t *button;
    ENTER_PASSCODE_ENUM mode = item->mode;
    item->fpErrLabel = NULL;

    if (mode == ENTER_PASSCODE_SET_PIN) {
        button = GuiCreateImgLabelAdaptButton(pinCont, g_enterPassLabel[mode].passSwitch, &imgSwitch, PassWordPinSwitchHandler, passCodeParam);
        lv_obj_align(button, LV_ALIGN_TOP_MID, 0, 449 - GUI_MAIN_AREA_OFFSET);

        label = GuiCreateIllustrateLabel(pinCont, _("password_error_duplicated_pincode"));
        lv_obj_set_style_text_color(label, RED_COLOR, LV_PART_MAIN);
        lv_obj_align(label, LV_ALIGN_DEFAULT, 36, 378 - GUI_MAIN_AREA_OFFSET);
        lv_obj_add_flag(label, LV_OBJ_FLAG_HIDDEN);
        item->repeatLabel = label;

        label = GuiCreateScrollTitleLabel(passWdCont, g_enterPassLabel[ENTER_PASSCODE_SET_PASSWORD].title);
        lv_obj_align(label, LV_ALIGN_DEFAULT, 36, 156 - GUI_MAIN_AREA_OFFSET);

        label = GuiCreateIllustrateLabel(passWdCont, g_enterPassLabel[ENTER_PASSCODE_SET_PASSWORD].desc);
        lv_obj_set_style_text_opa(label, LV_OPA_80, LV_PART_MAIN);
        GuiAlignToPrevObj(label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 12);

        KeyBoard_t *kb = GuiCreateFullKeyBoard(passWdCont, SetPassWordHandler, KEY_STONE_FULL_L, passCodeParam);
        lv_obj_set_size(kb->ta, 382, 60);
        lv_obj_set_style_text_opa(kb->ta, LV_OPA_100, 0);
        lv_obj_align(kb->ta, LV_ALIGN_DEFAULT, 36, 308 - GUI_MAIN_AREA_OFFSET);
        lv_textarea_set_placeholder_text(kb->ta, _("password_error_too_weak"));
        lv_textarea_set_password_mode(kb->ta, true);
        lv_textarea_set_max_length(kb->ta, PASSWORD_MAX_LEN);
        lv_textarea_set_one_line(kb->ta, true);
        item->kb = kb;

        button = GuiCreateImgLabelAdaptButton(passWdCont, g_enterPassLabel[ENTER_PASSCODE_SET_PASSWORD].passSwitch, &imgSwitch, PassWordPinSwitchHandler, passCodeParam);
        lv_obj_align(button, LV_ALIGN_TOP_MID, 0, 449 - GUI_MAIN_AREA_OFFSET);

        static lv_style_t style_indic;

        lv_style_init(&style_indic);
        lv_style_set_bg_opa(&style_indic, LV_OPA_COVER);
        lv_style_set_bg_color(&style_indic, lv_palette_main(LV_PALETTE_BLUE));
        lv_style_set_anim_time(&style_indic, 500);
        lv_style_set_radius(&style_indic, 0);

        lv_obj_t *bar = lv_bar_create(passWdCont);
        lv_obj_remove_style_all(bar); /*To have a clean start*/
        lv_obj_add_style(bar, &style_indic, LV_PART_INDICATOR);

        lv_obj_set_size(bar, 60, 4);
        lv_obj_center(bar);
        lv_bar_set_range(bar, 0, 90);

        lv_obj_set_style_radius(bar, 0, LV_PART_MAIN);
        lv_obj_align(bar, LV_ALIGN_DEFAULT, 36, 390 - GUI_MAIN_AREA_OFFSET);
        lv_obj_add_flag(bar, LV_OBJ_FLAG_HIDDEN);
        item->scoreBarStyle = &style_indic;
        item->scoreBar = bar;

        label = GuiCreateIllustrateLabel(passWdCont, "");
        lv_obj_set_style_text_color(label, GREEN_COLOR, LV_PART_MAIN);
        lv_obj_align(label, LV_ALIGN_DEFAULT, 108, 378 - GUI_MAIN_AREA_OFFSET);
        lv_obj_add_flag(label, LV_OBJ_FLAG_HIDDEN);
        item->scoreLevel = label;

        label = GuiCreateIllustrateLabel(passWdCont, _("password_error_too_long"));
        lv_obj_set_style_text_color(label, RED_COLOR, LV_PART_MAIN);
        lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 415 - GUI_MAIN_AREA_OFFSET);
        lv_obj_add_flag(label, LV_OBJ_FLAG_HIDDEN);
        item->lenOverLabel = label;

        img = GuiCreateImg(passWdCont, &imgEyeOff);
        lv_obj_add_flag(img, LV_OBJ_FLAG_HIDDEN);
        item->eyeImg = img;
    }
    if (mode == ENTER_PASSCODE_REPEAT_PIN) {
        KeyBoard_t *kb = GuiCreateFullKeyBoard(passWdCont, SetPassWordHandler, KEY_STONE_FULL_L, passCodeParam);
        lv_obj_set_size(kb->ta, 382, 60);
        lv_obj_set_style_text_opa(kb->ta, LV_OPA_100, 0);
        lv_obj_align(kb->ta, LV_ALIGN_DEFAULT, 36, 308 - GUI_MAIN_AREA_OFFSET);
        lv_textarea_set_placeholder_text(kb->ta, _("password_error_weak"));
        lv_textarea_set_password_mode(kb->ta, true);
        lv_textarea_set_max_length(kb->ta, PASSWORD_MAX_LEN);
        lv_textarea_set_one_line(kb->ta, true);
        item->kb = kb;
    }

    label = GuiCreateScrollTitleLabel(pinCont, g_enterPassLabel[mode].title);
    lv_obj_align(label, LV_ALIGN_DEFAULT, 36, 156 - GUI_MAIN_AREA_OFFSET);

    lv_obj_t *subLabel = GuiCreateNoticeLabel(pinCont, g_enterPassLabel[mode].desc);
    lv_obj_align_to(subLabel, label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 24);

    label = GuiCreateIllustrateLabel(pinCont, _("password_error_not_match"));
    lv_obj_set_style_text_color(label, RED_COLOR, LV_PART_MAIN);
    lv_obj_align(label, LV_ALIGN_DEFAULT, 36, 378 - GUI_MAIN_AREA_OFFSET);
    lv_obj_add_flag(label, LV_OBJ_FLAG_HIDDEN);
    item->errLabel = label;
}

void GuiCreateEnterPassWord(GuiEnterPasscodeItem_t *item, EnterPassCodeParam_t *passCodeParam)
{
    lv_obj_t *passWdCont = item->passWdCont;
    ENTER_PASSCODE_ENUM mode = item->mode;
    lv_obj_t *label;
    item->fpErrLabel = NULL;

    label = GuiCreateScrollTitleLabel(passWdCont, g_enterPassLabel[mode].title);
    lv_obj_align(label, LV_ALIGN_DEFAULT, 36, 156 - GUI_MAIN_AREA_OFFSET);

    label = GuiCreateIllustrateLabel(passWdCont, g_enterPassLabel[mode].desc);
    lv_obj_set_style_text_opa(label, LV_OPA_80, LV_PART_MAIN);
    GuiAlignToPrevObj(label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 12);

    KeyBoard_t *kb = GuiCreateFullKeyBoard(passWdCont, SetPassWordHandler, KEY_STONE_FULL_L, passCodeParam);
    lv_obj_set_size(kb->ta, 382, 60);
    lv_obj_set_style_text_opa(kb->ta, LV_OPA_100, 0);
    lv_obj_align(kb->ta, LV_ALIGN_DEFAULT, 36, 308 - GUI_MAIN_AREA_OFFSET);
    lv_textarea_set_placeholder_text(kb->ta, _("password_error_too_weak"));
    lv_textarea_set_password_mode(kb->ta, true);
    lv_textarea_set_max_length(kb->ta, PASSWORD_MAX_LEN);
    lv_textarea_set_one_line(kb->ta, true);
    item->kb = kb;

    label = GuiCreateIllustrateLabel(passWdCont, _("password_error_not_match"));
    lv_obj_set_style_text_color(label, RED_COLOR, LV_PART_MAIN);
    lv_obj_align(label, LV_ALIGN_DEFAULT, 36, 378 - GUI_MAIN_AREA_OFFSET);
    lv_obj_add_flag(label, LV_OBJ_FLAG_HIDDEN);
    item->errLabel = label;

    label = GuiCreateIllustrateLabel(passWdCont, _(""));
    lv_obj_set_style_text_color(label, RED_COLOR, LV_PART_MAIN);
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 210);
    lv_obj_add_flag(label, LV_OBJ_FLAG_HIDDEN);
    item->repeatLabel = label;

    label = GuiCreateIllustrateLabel(passWdCont, _("password_error_too_long"));
    lv_obj_set_style_text_color(label, GREEN_COLOR, LV_PART_MAIN);
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 415 - GUI_MAIN_AREA_OFFSET);
    lv_obj_add_flag(label, LV_OBJ_FLAG_HIDDEN);
    item->lenOverLabel = label;

    lv_obj_t *img = GuiCreateImg(passWdCont, &imgEyeOff);
    lv_obj_add_flag(img, LV_OBJ_FLAG_HIDDEN);
    item->eyeImg = img;
}

void *GuiCreateEnterPasscode(lv_obj_t *parent, lv_event_cb_t Cb, void *param, ENTER_PASSCODE_ENUM mode)
{
    GuiEnterPasscodeItem_t *passCodeItem = SRAM_MALLOC(sizeof(GuiEnterPasscodeItem_t));
    lv_obj_t *pinCont = GuiCreateContainerWithParent(parent, lv_obj_get_width(lv_scr_act()),
                        lv_obj_get_height(lv_scr_act()) - GUI_MAIN_AREA_OFFSET);
    lv_obj_set_align(pinCont, LV_ALIGN_BOTTOM_MID);
    lv_obj_add_flag(pinCont, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_t *passWdCont = GuiCreateContainerWithParent(parent, lv_obj_get_width(lv_scr_act()),
                           lv_obj_get_height(lv_scr_act()) - GUI_MAIN_AREA_OFFSET);
    lv_obj_set_align(passWdCont, LV_ALIGN_BOTTOM_MID);
    lv_obj_add_flag(passWdCont, LV_OBJ_FLAG_CLICKABLE);

    passCodeItem->pinCont = pinCont;
    passCodeItem->passWdCont = passWdCont;
    passCodeItem->mode = mode;
    passCodeItem->setPassCb = Cb;
    passCodeItem->currentNum = 0;
    g_passParam.setpinParam = passCodeItem;
    g_passParam.userParam = param;
    passCodeItem->eyeImg = NULL;

    switch (mode) {
    case ENTER_PASSCODE_VERIFY_PIN:
    case ENTER_PASSCODE_LOCK_VERIFY_PIN:
        lv_obj_add_flag(passWdCont, LV_OBJ_FLAG_HIDDEN);
        GuiCreateEnterVerify(passCodeItem, &g_passParam);
        break;
    case ENTER_PASSCODE_VERIFY_PASSWORD:
    case ENTER_PASSCODE_LOCK_VERIFY_PASSWORD:
        lv_obj_add_flag(pinCont, LV_OBJ_FLAG_HIDDEN);
        GuiCreateEnterVerify(passCodeItem, &g_passParam);
        return passCodeItem;
    case ENTER_PASSCODE_SET_PIN:
    case ENTER_PASSCODE_REPEAT_PIN:
        lv_obj_add_flag(passWdCont, LV_OBJ_FLAG_HIDDEN);
        GuiCreateEnterPinCode(passCodeItem, &g_passParam);
        break;
    case ENTER_PASSCODE_SET_PASSWORD:
    case ENTER_PASSCODE_REPEAT_PASSWORD:
        lv_obj_add_flag(pinCont, LV_OBJ_FLAG_HIDDEN);
        GuiCreateEnterPassWord(passCodeItem, &g_passParam);
        break;
    default:
        // todo ENTER_PASSCODE_BUTT
        break;
    }

    if (mode == ENTER_PASSCODE_LOCK_VERIFY_PIN) {
        // lv_label_set_text(passCodeItem->errLabel, _());
    }

    for (int i = 0; i < CREATE_PIN_NUM; i++) {
        lv_obj_t *led = GuiCreateLed(pinCont);
        if (mode == ENTER_PASSCODE_VERIFY_PIN) {
            lv_obj_align(led, LV_ALIGN_DEFAULT, 159 + 30 * i, 160);
        } else {
            lv_obj_align(led, LV_ALIGN_DEFAULT, 36 + 30 * i, 194);
        }
        passCodeItem->numLed[i] = led;
    }

    return passCodeItem;
}

void GuiUpdateEnterPasscodeParam(GuiEnterPasscodeItem_t *item, void *param)
{
    SET_HANDLE_FLAG();

    g_passParam.setpinParam = item;
    g_passParam.userParam = param;

    if (item->kb != NULL) {
        lv_textarea_set_password_mode(item->kb->ta, true);
        lv_textarea_set_text(item->kb->ta, "");
        if (item->eyeImg != NULL) {
            lv_img_set_src(item->eyeImg, &imgEyeOff);
        }
    }
    if (item->btnm != NULL) {
        item->currentNum = 0;
        memset_s(g_pinBuf, sizeof(g_pinBuf), 0, sizeof(g_pinBuf));
        for (int i = 0; i < CREATE_PIN_NUM; i++) {
            GuiSetLedStatus(item->numLed[i], PASSCODE_LED_OFF);
        }
    }

    if (item->errLabel != NULL) {
        if (!lv_obj_has_flag(item->errLabel, LV_OBJ_FLAG_HIDDEN)) {
            lv_obj_add_flag(item->errLabel, LV_OBJ_FLAG_HIDDEN);
        }
    }
}

// set title label
void GuiSetPasscodeTileLabel(GuiEnterPasscodeItem_t *item, const char *text)
{
    if (item != NULL) {
        lv_label_set_text(item->titleLabel, text);
    }
}

// set desc label
void GuiSetPasscodeDescLabel(GuiEnterPasscodeItem_t *item, const char *text)
{
    if (item != NULL) {
        lv_label_set_text(item->descLabel, text);
    }
}

void GuiDelEnterPasscode(void *obj, void *param)
{
    GuiEnterPasscodeItem_t *item = obj;
    if (item != NULL) {
        // lv_obj_del(item->pinCont);
        // item->pinCont = NULL;
        // lv_obj_del(item->passWdCont);
        // item->pinCont = NULL;
        SRAM_FREE(item);
    }
}

void GuiEnterPassCodeStatus(GuiEnterPasscodeItem_t *item, bool en)
{
    SET_HANDLE_FLAG();

    if (!en) {
        if (item->mode % 2 == 0) {
            for (int i = 0; i < CREATE_PIN_NUM; i++) {
                GuiSetLedStatus(item->numLed[i], PASSCODE_LED_ERR);
            }
        }
        lv_obj_clear_flag(item->errLabel, LV_OBJ_FLAG_HIDDEN);
        if (item->mode == ENTER_PASSCODE_SET_PASSWORD) {
            UnlimitedVibrate(LONG);
            lv_obj_clear_flag(item->repeatLabel, LV_OBJ_FLAG_HIDDEN);
        }
    } else {
        if (item->mode % 2 == 0) {
            for (int i = 0; i < CREATE_PIN_NUM; i++) {
                GuiSetLedStatus(item->numLed[i], PASSCODE_LED_OFF);
            }
            item->currentNum = 0;
        }
        lv_obj_add_flag(item->repeatLabel, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(item->errLabel, LV_OBJ_FLAG_HIDDEN);
        lv_textarea_set_text(item->kb->ta, "");
    }
    if (lv_obj_is_valid(item->fpErrLabel)) {
        lv_obj_add_flag(item->fpErrLabel, LV_OBJ_FLAG_HIDDEN);
    }
    memset_s(g_pinBuf, sizeof(g_pinBuf), 0, sizeof(g_pinBuf));
}

void GuiFingerPrintStatus(GuiEnterPasscodeItem_t *item, bool en, uint8_t errCnt)
{
    if (en) {
        lv_obj_add_flag(item->fpErrLabel, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_clear_flag(item->fpErrLabel, LV_OBJ_FLAG_HIDDEN);
        lv_obj_align(item->fpErrLabel, LV_ALIGN_TOP_MID, 0, 0);
        lv_obj_set_style_text_align(item->fpErrLabel, LV_TEXT_ALIGN_CENTER, 0);
        if (errCnt < 5) {
            lv_label_set_text(item->fpErrLabel, _("password_error_cannot_verify_fingerprint"));
        } else {
            lv_label_set_text(item->fpErrLabel, _("password_error_fingerprint_attempts_exceed"));
            lv_label_set_long_mode(item->fpErrLabel, LV_LABEL_LONG_WRAP);
            lv_obj_set_width(item->fpErrLabel, 408);
        }
    }
}

void SwitchPasswordModeHandler(lv_event_t *e)
{
    lv_obj_t *ta = lv_event_get_user_data(e);
    lv_obj_t *img = lv_event_get_target(e);
    bool en = lv_textarea_get_password_mode(ta);
    lv_textarea_set_password_mode(ta, !en);
    if (en) {
        lv_img_set_src(img, &imgEyeOn);
    } else {
        lv_img_set_src(img, &imgEyeOff);
    }
}

const int8_t MAX_SCORE = 90; // MAX SCORE = 25 + 10 + 10 + 20 + 25 = 90

uint8_t GetPassWordStrength(const char *password, uint8_t len)
{
    uint8_t totalScore = 0;
    uint8_t score[PASSWORD_STRENGTH_BUTT] = {0};
    score[PASSWORD_STRENGTH_LEN] = len;
    char c;
    for (int i = 0; i < len; i++) {
        c = password[i];
        if (c >= 'a' && c <= 'z') {
            score[PASSWORD_STRENGTH_LITTLE_LETTER]++;
        } else if (c >= 'A' && c <= 'Z') {
            score[PASSWORD_STRENGTH_CAPITAL_LETTER]++;
        } else if (c >= '0' && c <= '9') {
            score[PASSWORD_STRENGTH_DIGIT]++;
        } else {
            score[PASSWORD_STRENGTH_SYMBOL]++;
        }
    }

    if (len < 6) {
        return 0;
    } else if (len <= 8) {
        totalScore += 10;
    } else {
        totalScore += 25;
    }

    if (score[PASSWORD_STRENGTH_LITTLE_LETTER] > 0 || score[PASSWORD_STRENGTH_CAPITAL_LETTER] > 0) {
        totalScore += 10;
    }

    if (score[PASSWORD_STRENGTH_LITTLE_LETTER] > 0 && score[PASSWORD_STRENGTH_CAPITAL_LETTER] > 0) {
        totalScore += 10;
    }

    if (score[PASSWORD_STRENGTH_DIGIT] >= 3) {
        totalScore += 20;
    } else if (score[PASSWORD_STRENGTH_DIGIT] >= 1) {
        totalScore += 10;
    }

    if (score[PASSWORD_STRENGTH_SYMBOL] >= 3) {
        totalScore += 25;
    } else if (score[PASSWORD_STRENGTH_SYMBOL] >= 1) {
        totalScore += 10;
    }

    return totalScore;
}