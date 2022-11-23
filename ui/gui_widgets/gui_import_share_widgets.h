/*********************************************************************
 * Copyright (c) keyst.one. 2020-2025. All rights reserved.
 * name       : gui_create_wallet_widgets.h
 * Description:
 * author     : stone wang
 * data       : 2023-01-19 11:20
**********************************************************************/

#ifndef _GUI_CREATE_WALLET_WIDGETS_H
#define _GUI_CREATE_WALLET_WIDGETS_H

typedef enum {
    CREATE_WALLET_PIN = 0,
    CREATE_WALLET_PASSWORD,

    CREATE_WALLET_MODE_BUTT,
} CREATE_WALLET_MODE_ENUM;

void GuiImportShareInit(uint8_t wordsCnt);
void GuiImportShareDeInit(void);
void GuiImportShareRefresh(void);
int8_t GuiImportSharePrevTile(void);
int8_t GuiImportShareNextTile(void);
int8_t GuiImportShareNextSlice(void);
void GuiImportShareWriteSe(bool en, int32_t ret);

#endif /* _GUI_CREATE_WALLET_WIDGETS_H */

