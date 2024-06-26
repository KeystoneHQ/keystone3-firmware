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

#define WALLET_TYPE_TON             0b00000010
#define ENTROPY_TYPE_STANDARD       0b00000000
#define ENTORPY_TYPE_DICE_ROLLS     0b00000001
#define WALLET_TYPE_MASK            0b00000010
#define ENTROPY_TYPE_MASK           0b00000001

typedef enum {
    SEED_TYPE_BIP39,
    SEED_TYPE_SLIP39,
} SEED_TYPE;

#endif /* _GUI_CREATE_WALLET_WIDGETS_H */

