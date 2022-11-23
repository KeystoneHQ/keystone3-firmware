/*********************************************************************
 * Copyright (c) keyst.one. 2020-2025. All rights reserved.
 * name       : gui_emoji_keyboard.h
 * Description:
 * author     : stone wang
 * data       : 2023-01-18 10:22
**********************************************************************/

#ifndef _GUI_EMOJI_KEYBOARD_H
#define _GUI_EMOJI_KEYBOARD_H

#include "lvgl.h"

typedef struct {
    lv_obj_t *cont;
} KeyBoardHead_t;

typedef struct {
    lv_obj_t *cont;
    lv_obj_t *kb;
    lv_obj_t *associateLabel[3];
    lv_obj_t *ta;
    lv_keyboard_user_mode_t mode;
    uint8_t taMinLen;
} KeyBoard_t;

enum {
    MNEMONIC_INPUT_IMPORT_VIEW = 0,
    MNEMONIC_INPUT_SETTING_VIEW,
    MNEMONIC_INPUT_FORGET_VIEW,
};

typedef struct {
    lv_obj_t *cont;
    char *mnemonicWord[44];
    uint8_t intputType;
    uint8_t wordCnt;
    uint8_t currentId;
    uint8_t threShold;
    uint8_t currentSlice;
    KeyBoard_t *letterKb;
    lv_obj_t *btnm;
    lv_obj_t *stepLabel;
    lv_obj_t *titleLabel;
    lv_obj_t *descLabel;
    lv_obj_t *nextButton;
} MnemonicKeyBoard_t;

typedef enum {
    NUM_KEYBOARD_PIN = 0,
    NUM_KEYBOARD_SLICE,
    NUM_KEYBOARD_NORMAL,

    NUM_KEYBOARD_BUTT,
} NUM_KEYBOARD_ENUM;

typedef enum {
    MNEMONIC_BUTTON_UNPRESSED = 0,
    MNEMONIC_BUTTON_CURRENT_PRESS,
    MNEMONIC_BUTTON_PRESSED,

    MNEMONIC_BUTTON_BUTT,
} MNEMONIC_BUTTON_STATE_ENUM;

#define GUI_KEYBOARD_EMOJI_NEW_INDEX                            (0xFF)
#define GUI_KEYBOARD_EMOJI_CANCEL_NEW_INDEX                     (0xFE)
#define GUI_DEFINE_MAX_PASSCODE_LEN                             (128)

void GuiDeleteKeyBoard(KeyBoard_t *kb);
void KbTextAreaHandler(lv_event_t * e);
void GuiKeyBoardSetMode(KeyBoard_t *keyBoard);
void *GuiCreateNumKeyboard(lv_obj_t *parent, lv_event_cb_t cb, NUM_KEYBOARD_ENUM numMode, void *param);
void *GuiEmojiKeyBoard(lv_obj_t *parent, lv_obj_t *img);
uint8_t GuiGetEmojiIconIndex(void);
void GuiUpdateShareKb(lv_obj_t *btnm, uint8_t memberCnt);
void *GuiCreateFullKeyBoard(lv_obj_t *parent, lv_event_cb_t kbCb, lv_keyboard_user_mode_t keyMode, void *param);
void *GuiCreateLetterKeyBoard(lv_obj_t *parent, lv_event_cb_t cb, bool bip39, void *param);
void GuiConfirmFullKeyBoard(KeyBoard_t *keyBoard);
void *GuiCreateMnemonicKeyBoard(lv_obj_t *parent, lv_event_cb_t kbCb, lv_keyboard_user_mode_t keyMode, char *mnemonic);
void GuiUpdateMnemonicKeyBoard(MnemonicKeyBoard_t *mnemonicKeyBoard, char *mnemonic, bool confirm);
void GuiConfirmMnemonicKeyBoard(MnemonicKeyBoard_t *mnemonicKeyBoard, char *mnemonic, int n, int num, int dig);
void GuiClearKeyBoard(KeyBoard_t* keyBoard);
void GuiInputMnemonicKeyBoard(MnemonicKeyBoard_t* inputMnemonicKeyBoard, char *word, int n, int dig);
bool GuiWordsWhole(const char* text);
void UpdateAssociateLabel(KeyBoard_t *keyBoard, const char *currText);
void GuiClearMnemonicKeyBoard(MnemonicKeyBoard_t *mnemonicKeyBoard);
void GuiDelMnemonicKeyBoard(MnemonicKeyBoard_t * mnemonicKeyBoard);
void GuiSetFullKeyBoardTa(KeyBoard_t *keyBoard, lv_obj_t *ta);
const lv_img_dsc_t *GuiGetEmojiIconImg(void);
void GuiSetEmojiIconIndex(uint8_t index);
void GuiSetEnterProgressLabel(lv_obj_t *label);
void GuiDelEnterProgressLabel(void);
void GuiSetFullKeyBoardConfirm(KeyBoard_t *keyBoard, bool en);
void GuiSetLetterBoardConfirm(KeyBoard_t *keyBoard, int en);
void ClearMnemonicKeyboard(MnemonicKeyBoard_t *mkb, uint8_t *inputId);
void UpdateFullKeyBoard(const char *str, KeyBoard_t *keyBoard);
void GuiSetKeyBoardMinTaLen(KeyBoard_t *keyBoard, uint8_t len);
void SetStatusBarEmojiIndex(uint8_t index);
void ClearKbCache(void);
void GuiClearEnterProgressLabel(void);
char *GuiGetTrueWord(const lv_obj_t *obj, uint16_t btn_id);
void GuiSetLetterBoardNext(KeyBoard_t *keyBoard);
void GuiKeyBoardRestoreDefault(KeyBoard_t *keyBoard);
bool GuiSingleWordsWhole(const char *text);

#endif /* _GUI_EMOJI_KEYBOARD_H */

