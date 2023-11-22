#ifndef _GUI_CREATE_WALLET_WIDGETS_H
#define _GUI_CREATE_WALLET_WIDGETS_H

void GuiImportShareInit(uint8_t wordsCnt);
void GuiImportShareDeInit(void);
void GuiImportShareRefresh(void);
int8_t GuiImportSharePrevTile(void);
int8_t GuiImportShareNextTile(void);
int8_t GuiImportShareNextSlice(void);
void GuiImportShareWriteSe(bool en, int32_t ret);

#endif /* _GUI_CREATE_WALLET_WIDGETS_H */

