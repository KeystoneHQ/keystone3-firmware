/**************************************************************************************************
 * Copyright (c) Keystone 2020-2025. All rights reserved.
 * Description: Gui firmware update widgets.
 * Author: leon sun
 * Create: 2023-7-18
 ************************************************************************************************/

#ifndef _GUI_FIRMWARE_UPDATE_WIDGETS_H
#define _GUI_FIRMWARE_UPDATE_WIDGETS_H

#include "stdint.h"
#include "stdbool.h"


typedef enum {
    FIRMWARE_UPDATE_ENTRY_SETUP = 0,
    FIRMWARE_UPDATE_ENTRY_SETTING,

    FIRMWARE_UPDATE_ENTRY_BUTT,
} FIRMWARE_UPDATE_ENTRY_ENUM;

void GuiFirmwareUpdateInit(void *param);
void GuiFirmwareUpdateDeInit(void);
void GuiFirmwareUpdateRefresh(void);
void GuiFirmwareUpdatePrevTile(void);
void GuiFirmwareSdCardCopy(void);
void GuiCreateSdCardUpdateHintbox(char *version);
void GuiFirmwareSdCardCopyResult(bool en);
void GuiFirmwareUpdateVerifyPasswordErrorCount(void *param);
void GuiFirmwareUpdateWidgetRefresh(void);

#endif

