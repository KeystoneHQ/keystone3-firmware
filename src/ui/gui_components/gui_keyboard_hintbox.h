#ifndef _GUI_KEYBOARD_HINTBOX_H
#define _GUI_KEYBOARD_HINTBOX_H

#include "gui.h"
#include "gui_keyboard.h"
#include "gui_model.h"

typedef struct KeyboardWidget {
    lv_obj_t *keyboardHintBox;
    KeyBoard_t *kb;
    lv_obj_t *errLabel;
    uint16_t *sig;
    lv_timer_t *countDownTimer;
    uint8_t *timerCounter;
    lv_obj_t *errHintBox;
    lv_obj_t *errHintBoxBtn;
    struct KeyboardWidget **self;
} KeyboardWidget_t;

KeyboardWidget_t *GuiCreateKeyboardWidget(lv_obj_t *parent);
void SetKeyboardWidgetSig(KeyboardWidget_t *keyboardWidget, uint16_t *sig);
void SetKeyboardWidgetSelf(KeyboardWidget_t *keyboardWidget, KeyboardWidget_t **self);

void GuiDeleteKeyboardWidget(KeyboardWidget_t *keyboardWidget);
const char *GuiGetKeyboardInput(KeyboardWidget_t *keyboardWidget);
void GuiClearKeyboardInput(KeyboardWidget_t *keyboardWidget);
void GuiSetErrorLabel(KeyboardWidget_t *keyboardWidget, char *errorMessage);
void GuiShowErrorLabel(KeyboardWidget_t *keyboardWidget);
void GuiHideErrorLabel(KeyboardWidget_t *keyboardWidget);
void GuiShowErrorNumber(KeyboardWidget_t *keyboardWidget, PasswordVerifyResult_t *passwordVerifyResult);

#endif