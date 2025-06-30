#ifndef _GUI_MANAGE_MULTI_SIG_WIDGETS_H
#define _GUI_MANAGE_MULTI_SIG_WIDGETS_H

#include "gui_home_widgets.h"

void GuiManageMultisigWalletInit(int index);
void GuiManageMultisigWalletDeInit(void);
void GuiManageMultisigWalletRefresh(void);
int8_t GuiManageMultiWalletPrevTile(void);
int8_t GuiManageMultisigWalletNextTile(uint8_t index);
void DeleteMultisigWallet(void);
void GuiManageMultisigPasswordErrorCount(void *param);

#endif /* _GUI_MANAGE_MULTI_SIG_WIDGETS_H */

