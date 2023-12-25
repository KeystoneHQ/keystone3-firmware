#ifndef _GUI_CREATE_WALLET_WIDGETS_H
#define _GUI_CREATE_WALLET_WIDGETS_H

void GuiCreateShareInit(uint8_t entropyMethod);
void GuiCreateShareDeInit(void);
void GuiCreateShareRefresh(void);
int8_t GuiCreateSharePrevTile(void);
int8_t GuiCreateShareNextTile(void);
int8_t GuiCreateShareNextSlice(void);
void GuiCreateShareUpdateMnemonic(void *signalParam, uint16_t paramLen);



#endif /* _GUI_CREATE_WALLET_WIDGETS_H */

