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
void GuiCreateSdCardUpdateHintbox(bool checkSumDone);
void GuiFirmwareSdCardCopyResult(bool en);
void GuiFirmwareUpdateVerifyPasswordErrorCount(void *param);
void GuiFirmwareUpdateWidgetRefresh(void);
void GuiFirmwareUpdateSha256Percent(uint8_t percent);
void GuiFirmwareWindowDeinit(void);
void GuiCreateSdCardVerifyBinWindow(void);

#endif
