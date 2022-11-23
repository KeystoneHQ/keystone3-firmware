/*********************************************************************
 * Copyright (c) keyst.one. 2020-2025. All rights reserved.
 * name       : gui_api.h
 * Description:
 * author     : stone wang
 * data       : 2023-01-09 11:58
**********************************************************************/

#ifndef _GUI_API_H
#define _GUI_API_H

#include "err_code.h"

#ifdef COMPILE_SIMULATOR
#include "stdint.h"
#include "stdbool.h"

#define UI_MSG_BASE                     0x00040000

typedef struct {
    uint32_t id;
    uint32_t value;
    uint8_t *buffer;
    uint32_t length;
} Message_t;

enum {
    UI_MSG_EMIT_SIGNAL_TO_WORKING_VIEW = UI_MSG_BASE,
};
#else

int32_t GuiApiEmitSignal(uint16_t signal, void *param, uint16_t usLen);
int32_t GuiApiEmitSignalWithValue(uint16_t signal, uint32_t value);

#endif

typedef struct {
    uint16_t signal;
    uint8_t param[0];
} GuiEmitMsg_t;

#endif /* _GUI_API_H */

