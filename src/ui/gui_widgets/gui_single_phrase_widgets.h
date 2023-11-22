#ifndef _GUI_SINGLE_PHRASE_WIDGETS_H
#define _GUI_SINGLE_PHRASE_WIDGETS_H

void GuiSinglePhraseInit(void);
int8_t GuiSinglePhraseNextTile(void);
int8_t GuiSinglePhrasePrevTile(void);
void GuiSinglePhraseDeInit(void);
void GuiSinglePhraseRefresh(void);

void GuiSinglePhraseUpdateMnemonic(void *signalParam, uint16_t paramLen);

#endif /* _GUI_SINGLE_PHRASE_WIDGETS_H */

