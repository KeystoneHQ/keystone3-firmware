/*********************************************************************
 * Copyright (c) keyst.one. 2020-2025. All rights reserved.
 * name       : gui_create_wallet_widgets.h
 * Description:
 * author     : stone wang
 * data       : 2023-01-19 11:20
**********************************************************************/

#ifndef _GUI_CREATE_WALLET_WIDGETS_H
#define _GUI_CREATE_WALLET_WIDGETS_H

#include "gui_enter_passcode.h"
#include "gui_setup_widgets.h"

typedef enum {
    CREATE_WALLET_PIN = 0,
    CREATE_WALLET_PASSWORD,

    CREATE_WALLET_MODE_BUTT,
} CREATE_WALLET_MODE_ENUM;

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

#endif /* _GUI_CREATE_WALLET_WIDGETS_H */

