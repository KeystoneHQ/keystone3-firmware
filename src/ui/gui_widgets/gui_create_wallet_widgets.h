#ifndef _GUI_CREATE_WALLET_WIDGETS_H
#define _GUI_CREATE_WALLET_WIDGETS_H

#include "gui_enter_passcode.h"
#include "gui_setup_widgets.h"

void GuiCreateWalletInit(uint8_t walletMethod);
void GuiCreateWalletDeInit(void);
void GuiCreateWalletNameUpdate(const void * src);
int8_t GuiCreateWalletPrevTile(void);
int8_t GuiCreateWalletNextTile(void);
const char *GuiGetWalletName(void);
void GuiSetWalletName(const char *name, uint8_t len);
void GuiCreateWalletSetPinPass(const char* buf);
void GuiCreateWalletRepeatPinPass(const char* buf);
const char *GetCurrentKbWalletName(void);
void GuiCreateWalletRefresh(void);
void GuiSetupKeyboardWidgetMode(void);

#endif /* _GUI_CREATE_WALLET_WIDGETS_H */

