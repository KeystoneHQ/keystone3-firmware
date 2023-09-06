/**************************************************************************************************
 * Copyright (c) keyst.one. 2020-2025. All rights reserved.
 * Description: Inactive view.
 * Author: leon sun
 * Create: 2023-8-4
 ************************************************************************************************/

#include "gui.h"
#include "gui_obj.h"
#include "gui_views.h"
#include "gui_inactive_widgets.h"

static int32_t GuiInactiveViewInit(void)
{
    GuiInactiveInit();
    return SUCCESS_CODE;
}

static int32_t GuiInactiveViewDeInit(void)
{
    GuiInactiveDeInit();
    return SUCCESS_CODE;
}

int32_t GuiInactiveViewEventProcess(void *self, uint16_t usEvent, void *param, uint16_t usLen)
{
    switch (usEvent) {
    case GUI_EVENT_OBJ_INIT:
        return GuiInactiveViewInit();
    case GUI_EVENT_OBJ_DEINIT:
        return GuiInactiveViewDeInit();
    default:
        return ERR_GUI_UNHANDLED;
    }
    return SUCCESS_CODE;
}

GUI_VIEW g_inactiveView = {
    .id = SCREEN_INACTIVE,
    .previous = NULL,
    .isActive = false,
    .optimization = false,
    .pEvtHandler = GuiInactiveViewEventProcess,
};
