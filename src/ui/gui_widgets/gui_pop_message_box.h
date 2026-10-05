#ifndef _GUI_POP_MESSAGE_BOX_H
#define _GUI_POP_MESSAGE_BOX_H

#include "stdint.h"
#include "stdbool.h"
#include "err_code.h"

typedef enum {
    GUI_FIRMWARE_UPDATE_DENY_PRIORITY                       = 1,
    GUI_LOW_BATTERY_PRIORITY,
    GUI_USB_CONNECTION_PRIORITY,
    GUI_POWER_OPTION_PRIORITY,
    GUI_FIRMWARE_PROCESS_PRIORITY,
} GuiPagePriority;

typedef void (*MsgBoxFunc_t)(void);

typedef struct {
    MsgBoxFunc_t init;
    MsgBoxFunc_t deinit;
    GuiPagePriority pagePriority;
} GuiMsgBox_t;

void OpenMsgBox(const GuiMsgBox_t *msgBox);
void CloseMsgBox(const GuiMsgBox_t *msgBox);
void CloseCurrentMsgBox(void);

#endif
