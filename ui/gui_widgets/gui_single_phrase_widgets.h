/*********************************************************************
 * Copyright (c) keyst.one. 2020-2025. All rights reserved.
 * name       : gui_single_phrase_widgets.h
 * Description:
 * author     : stone wang
 * data       : 2023-02-01 14:19
**********************************************************************/

#ifndef _GUI_SINGLE_PHRASE_WIDGETS_H
#define _GUI_SINGLE_PHRASE_WIDGETS_H

void GuiSinglePhraseInit(void);
int8_t GuiSinglePhraseNextTile(void);
int8_t GuiSinglePhrasePrevTile(void);
void GuiSinglePhraseDeInit(void);
void GuiSinglePhraseRefresh(void);

void GuiSinglePhraseUpdateMnemonic(void *signalParam, uint16_t paramLen);

#endif /* _GUI_SINGLE_PHRASE_WIDGETS_H */

