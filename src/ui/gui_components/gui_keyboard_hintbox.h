#ifndef _GUI_KEYBOARD_HINTBOX_H
#define _GUI_KEYBOARD_HINTBOX_H

#include "gui.h"
#include "gui_keyboard.h"
#include "gui_model.h"

enum {
    KEYBOARD_HINTBOX_PIN = 0,
    KEYBOARD_HINTBOX_PASSWORD = 1,
};

typedef struct KeyboardWidget {
    lv_obj_t *keyboardHintBox;
    KeyBoard_t *kb;
    lv_obj_t *led[6];
    lv_obj_t *btnm;
    lv_obj_t *errLabel;
    lv_obj_t *eyeImg;
    lv_obj_t *switchLabel;
    uint8_t currentNum;
    uint16_t *sig;
    lv_timer_t *countDownTimer;
    uint8_t *timerCounter;
    lv_obj_t *errHintBox;
    lv_obj_t *errHintBoxBtn;
    struct KeyboardWidget **self;
} KeyboardWidget_t;

KeyboardWidget_t *GuiCreateKeyboardWidget(lv_obj_t *parent);
KeyboardWidget_t *GuiCreateKeyboardWidgetView(lv_obj_t *parent, lv_event_cb_t buttonCb, uint16_t *signal);
void SetKeyboardWidgetSig(KeyboardWidget_t *keyboardWidget, uint16_t *sig);
void SetKeyboardWidgetSelf(KeyboardWidget_t *keyboardWidget, KeyboardWidget_t **self);
void SetKeyboardWidgetMode(uint8_t mode);
uint8_t GetKeyboardWidgetMode(void);
void PassWordPinHintRefresh(KeyboardWidget_t *keyboardWidget);

void GuiDeleteKeyboardWidget(KeyboardWidget_t *keyboardWidget);
const char *GuiGetKeyboardInput(KeyboardWidget_t *keyboardWidget);
void GuiClearKeyboardInput(KeyboardWidget_t *keyboardWidget);
void GuiSetErrorLabel(KeyboardWidget_t *keyboardWidget, char *errorMessage);
void GuiShowErrorLabel(KeyboardWidget_t *keyboardWidget);
void GuiHideErrorLabel(KeyboardWidget_t *keyboardWidget);
void GuiShowErrorNumber(KeyboardWidget_t *keyboardWidget, PasswordVerifyResult_t *passwordVerifyResult);

#endif