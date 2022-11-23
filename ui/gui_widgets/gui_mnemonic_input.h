/*********************************************************************
 * Copyright (c) keyst.one. 2020-2025. All rights reserved.
 * name       : gui_emoji_keyboard.h
 * Description:
 * author     : stone wang
 * data       : 2023-01-18 10:22
**********************************************************************/

#ifndef _GUI_MNEMONIC_INPUT_H
#define _GUI_MNEMONIC_INPUT_H

#include "lvgl.h"

void GuiMnemonicInputHandler(lv_event_t *e);
void GuiSetMnemonicCache(KeyBoard_t *keyBoard, char *word);
void ImportShareNextSlice(MnemonicKeyBoard_t *mkb, KeyBoard_t *letterKb);
void ImportSinglePhraseWords(MnemonicKeyBoard_t *mkb, KeyBoard_t *letterKb);
lv_keyboard_user_mode_t GuiGetMnemonicKbType(int wordCnt);
void GuiMnemonicHintboxClear(void);

#endif /* _GUI_MNEMONIC_INPUT_H */

