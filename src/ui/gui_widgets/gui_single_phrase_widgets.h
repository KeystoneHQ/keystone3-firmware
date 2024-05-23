#ifndef _GUI_SINGLE_PHRASE_WIDGETS_H
#define _GUI_SINGLE_PHRASE_WIDGETS_H

void GuiSinglePhraseInit(uint8_t entropyMethod);
int8_t GuiSinglePhraseNextTile(void);
int8_t GuiSinglePhrasePrevTile(void);
void GuiSinglePhraseDeInit(void);
void GuiSinglePhraseRefresh(void);

void GuiSinglePhraseUpdateMnemonic(void *signalParam, uint16_t paramLen);
void GuiShowTonGeneratingModal(bool enable);

#endif /* _GUI_SINGLE_PHRASE_WIDGETS_H */

