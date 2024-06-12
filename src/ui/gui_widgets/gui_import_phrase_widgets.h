#ifndef _GUI_IMPORT_PHRASE_WIDGETS_H
#define _GUI_IMPORT_PHRASE_WIDGETS_H

void GuiImportPhraseInit(uint8_t num);
void GuiImportPhraseDeInit(void);
int8_t GuiImportPhraseNextTile(void);
int8_t GuiImportPhrasePrevTile(void);
void GuiImportPhraseWriteSe(bool en, int32_t errCode);
void GuiImportPhraseRefresh(void);
void GuiImportPhraseUpdateKeyboard(void);
void GuiShowTonMnemonicHint();

#endif /* _GUI_IMPORT_PHRASE_WIDGETS_H */

