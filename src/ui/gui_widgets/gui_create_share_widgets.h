/*********************************************************************
 * Copyright (c) keyst.one. 2020-2025. All rights reserved.
 * name       : gui_create_wallet_widgets.h
 * Description:
 * author     : stone wang
 * data       : 2023-01-19 11:20
**********************************************************************/

#ifndef _GUI_CREATE_WALLET_WIDGETS_H
#define _GUI_CREATE_WALLET_WIDGETS_H

void GuiCreateShareInit(void);
void GuiCreateShareDeInit(void);
void GuiCreateShareRefresh(void);
int8_t GuiCreateSharePrevTile(void);
int8_t GuiCreateShareNextTile(void);
int8_t GuiCreateShareNextSlice(void);
void GuiCreateShareUpdateMnemonic(void *signalParam, uint16_t paramLen);

#endif /* _GUI_CREATE_WALLET_WIDGETS_H */

