#ifndef _GUI_CREATE_MULTI_WIDGETS_H
#define _GUI_CREATE_MULTI_WIDGETS_H

#include "librust_c.h"

void GuiCreateMultiInit();
void GuiCreateMultiDeInit(void);
void GuiCreateMultiRefresh(void);
int8_t GuiCreateMultiPrevTile(void);
int8_t GuiCreateMultiNextTile(uint8_t index);
int8_t GuiCreateMultiNextSlice(void);
void ListMicroCardXpubFile(void);
void GuiSetMultisigImportXpubByQRCode(URParseResult *urResult);

#endif /* _GUI_CREATE_MULTI_WIDGETS_H */

