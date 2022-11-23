/*********************************************************************
 * Copyright (c) keyst.one. 2020-2025. All rights reserved.
 * Description:
 * author     : ginger liu
 * data       : 2023-02-09
**********************************************************************/

#ifndef _GUI_IMPORT_PHRASE_WIDGETS_H
#define _GUI_IMPORT_PHRASE_WIDGETS_H

void GuiImportPhraseInit(uint8_t num);
void GuiImportPhraseDeInit(void);
int8_t GuiImportPhraseNextTile(void);
int8_t GuiImportPhrasePrevTile(void);
void GuiImportPhraseWriteSe(bool en, int32_t errCode);
void GuiImportPhraseRefresh(void);

#endif /* _GUI_IMPORT_PHRASE_WIDGETS_H */

